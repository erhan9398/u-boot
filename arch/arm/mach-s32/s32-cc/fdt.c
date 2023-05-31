// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017,2019-2023 NXP
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <misc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/ioport.h>
#include <s32-cc/a53_gpr.h>
#include <s32-cc/fdt_wrapper.h>
#include <s32-cc/nvmem.h>
#include <s32-cc/serdes_hwconfig.h>
#include <dt-bindings/nvmem/s32cc-siul2-nvmem.h>

#define S32_DDR_LIMIT_VAR	"ddr_limitX"
#define FDT_CLUSTER1_PATH	"/cpus/cpu-map/cluster1"

static const char *s32cc_gpio_compatible = "nxp,s32cc-siul2-gpio";
static const char *scmi_gpio_node_path = "/firmware/scmi/protocol@81";

static int get_core_id(u32 core_mpidr, u32 max_cores_per_cluster)
{
	u32 cluster_id = (core_mpidr >> 8) & 0xFF;

	return (core_mpidr & 0xf) + cluster_id * max_cores_per_cluster;
}

static int get_cores_info(u32 *max_cores_per_cluster,
			  u32 *cpu_mask)
{
	int ret;
	const char *dev_name = "siul2_0_nvram";
	struct udevice *siul2_nvmem = NULL;
	struct nvmem_cell cell;

	ret = uclass_get_device_by_name(UCLASS_MISC, dev_name,
					&siul2_nvmem);
	if (ret) {
		printf("%s: No SIUL21 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = nvmem_cell_get_by_offset(siul2_nvmem,
				       S32CC_MAX_A53_CORES_PER_CLUSTER,
				       &cell);
	if (ret) {
		printf("%s: Failed to get A53 cores per cluster cell (err = %d)\n",
		       __func__, ret);
		return -ENODEV;
	}

	ret = nvmem_cell_read(&cell, max_cores_per_cluster,
			      sizeof(*max_cores_per_cluster));
	if (ret) {
		printf("%s: Failed to read A53 cores per cluster cell (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	if (!(*max_cores_per_cluster)) {
		printf("%s: Number of max cores per cluster cannot be 0\n",
		       __func__);
		return -EINVAL;
	}

	ret = nvmem_cell_get_by_offset(siul2_nvmem,
				       S32CC_A53_CORES_MASK,
				       &cell);
	if (ret) {
		printf("%s: Failed to get A53 cores mask cell (err = %d)\n",
		       __func__, ret);
		return -ENODEV;
	}

	ret = nvmem_cell_read(&cell, cpu_mask, sizeof(*cpu_mask));
	if (ret) {
		printf("%s: Failed to read A53 cores mask cell (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	return 0;
}

static bool is_lockstep_enabled(void)
{
	int ret;
	u32 lockstep_enabled = 0;
	struct udevice *s32cc_a53_gpr = NULL;
	const char *a53_compat = "nxp,s32cc-a53-gpr";
	ofnode node;

	node = ofnode_by_compatible(ofnode_null(), a53_compat);
	if (!ofnode_valid(node)) {
		printf("%s: Couldn't find \"%s\" node\n", __func__, a53_compat);
		return false;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, node, &s32cc_a53_gpr);
	if (ret) {
		printf("%s: No A53 GPR (err = %d)\n", __func__, ret);
		return false;
	}

	ret = misc_read(s32cc_a53_gpr, S32CC_A53_GPR_LOCKSTEP_EN,
			&lockstep_enabled, sizeof(lockstep_enabled));
	if (ret != sizeof(lockstep_enabled)) {
		printf("%s: Failed to read if Lockstep Enabled (err = %d)\n",
		       __func__, ret);
		return false;
	}

	return !!lockstep_enabled;
}

static int ft_fixup_cpu(void *blob)
{
	int ret, addr_cells = 0;
	u32 max_cores_per_cluster = 0;
	u32 cpu_mask = 0;
	u64 core_mpidr, core_id;
	fdt32_t *reg;
	int off, off_prev, cluster1_off;
	bool lockstep_enabled;

	ret = get_cores_info(&max_cores_per_cluster, &cpu_mask);
	if (ret)
		return ret;

	off = fdt_path_offset(blob, "/cpus");
	if (off < 0) {
		puts("couldn't find /cpus node\n");
		return -ENODEV;
	}
	off_prev = off;

	fdt_support_default_count_cells(blob, off, &addr_cells, NULL);
	off = get_next_cpu(blob, off);

	lockstep_enabled = is_lockstep_enabled();
	if (lockstep_enabled) {
		/* Disable secondary cluster */
		cpu_mask &= ~GENMASK(max_cores_per_cluster * 2 - 1,
							 max_cores_per_cluster);
	}

	while (off != -FDT_ERR_NOTFOUND) {
		reg = (fdt32_t *)fdt_getprop(blob, off, "reg", 0);
		if (!reg)
			continue;

		core_mpidr = fdt_read_number(reg, addr_cells);
		core_id = get_core_id(core_mpidr, max_cores_per_cluster);

		if (!test_bit(core_id, &cpu_mask)) {
			/* Disable lockstep or defeatured
			 * cores on derivatives
			 */
			fdt_del_node(blob, off);
			off = off_prev;
		}

		off_prev = off;
		off = get_next_cpu(blob, off);
	}

	if (!lockstep_enabled)
		return 0;

	cluster1_off = fdt_path_offset(blob, FDT_CLUSTER1_PATH);
	if (cluster1_off < 0) {
		printf("couldn't find %s node\n", FDT_CLUSTER1_PATH);
		return -ENODEV;
	}

	return fdt_del_node(blob, cluster1_off);
}

static int apply_memory_fixups(void *blob, struct bd_info *bd)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int ret, bank, banks = 0;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (!bd->bi_dram[bank].start && !bd->bi_dram[bank].size)
			continue;

		start[banks] = bd->bi_dram[bank].start;
		size[banks] = bd->bi_dram[bank].size;
		banks++;
	}

	ret = fdt_fixup_memory_banks(blob, start, size, banks);
	if (ret)
		pr_err("s32-fdt: Failed to set memory banks\n");

	return ret;
}

static void apply_ddr_limits(struct bd_info *bd)
{
	u64 start, end, limit;
	static const size_t var_len = sizeof(S32_DDR_LIMIT_VAR);
	static const size_t digit_pos = var_len - 2;
	char ddr_limit[var_len];
	char *var_val;
	int bank;

	memcpy(ddr_limit, S32_DDR_LIMIT_VAR, var_len);

	ddr_limit[digit_pos] = '0';
	while ((var_val = env_get(ddr_limit))) {
		limit = simple_strtoull(var_val, NULL, 16);

		for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
			start = bd->bi_dram[bank].start;
			end = start + bd->bi_dram[bank].size;

			if (limit >= start && limit < end)
				bd->bi_dram[bank].size = limit - start;
		}

		if (ddr_limit[digit_pos] >= '9')
			break;

		ddr_limit[digit_pos]++;
	};
}

static int ft_fixup_memory(void *blob, struct bd_info *bd)
{
	apply_ddr_limits(bd);

	return apply_memory_fixups(blob, bd);
}

static int add_atf_reserved_memory(void *new_blob)
{
	int ret;
	struct fdt_memory carveout;
	struct resource reg;
	ofnode node;

	if (fdt_check_header(new_blob)) {
		pr_err("Invalid FDT Header for Linux DT Blob\n");
		return -EINVAL;
	}

	/* Get atf reserved-memory node offset */
	node = ofnode_path("/reserved-memory/atf");
	if (!ofnode_valid(node)) {
		pr_err("Couldn't find 'atf' reserved-memory node\n");
		return -EINVAL;
	}

	ret = ofnode_read_resource(node, 0, &reg);
	if (ret) {
		pr_err("Unable to get value of 'reg' prop of 'atf' node\n");
		return ret;
	}

	carveout.start = reg.start;
	carveout.end = reg.end;

	/* Increase Linux DT size before adding new node */
	ret = fdt_increase_size(new_blob, 512);
	if (ret < 0) {
		pr_err("Could not increase size of Linux DT: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/* Add 'atf' node to Linux DT */
	ret = fdtdec_add_reserved_memory(new_blob, "atf", &carveout,
					 NULL, 0, NULL,
					 FDTDEC_RESERVED_MEMORY_NO_MAP);
	if (ret < 0) {
		pr_err("Unable to add 'atf' node to Linux DT\n");
		return ret;
	}

	return 0;
}

static int ft_fixup_atf(void *new_blob)
{
	int ret = add_atf_reserved_memory(new_blob);

	if (ret)
		pr_err("Copying 'atf' node from U-Boot DT to Linux DT failed!\n");

	return ret;
}

static int disable_node_by_compatible(void *blob, const char *compatible,
				      uint32_t *phandle)
{
	const char *node_name;
	int nodeoff, ret;

	nodeoff = fdt_node_offset_by_compatible(blob, -1, compatible);
	if (nodeoff < 0) {
		pr_err("Failed to get a node based on compatible string '%s' (%s)\n",
		       compatible, fdt_strerror(nodeoff));
		return nodeoff;
	}

	node_name = fdt_get_name(blob, nodeoff, NULL);
	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_DISABLED);
	if (ret) {
		pr_err("Failed to disable '%s' node\n", node_name);
		return ret;
	}

	*phandle = fdt_get_phandle(blob, nodeoff);
	if (!*phandle) {
		pr_warn("The node '%s' is not referenced by any other nodes.",
			node_name);
		return 0;
	}

	ret = fdt_delprop(blob, nodeoff, "phandle");
	if (ret) {
		pr_err("Failed to remove phandle property of '%s' node: %s\n",
		       node_name, fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int enable_scmi_protocol(void *blob, const char *path, uint32_t phandle)
{
	int nodeoff, ret;

	nodeoff = fdt_path_offset(blob, path);
	if (nodeoff < 0) {
		pr_err("Failed to get offset of '%s' node\n", path);
		return nodeoff;
	}

	ret = fdt_set_phandle(blob, nodeoff, phandle);
	if (ret) {
		pr_err("Failed to set phandle property of '%s' node\n", path);
		return ret;
	}

	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_OKAY);
	if (ret) {
		pr_err("Failed to enable '%s' node\n", path);
		return ret;
	}

	return 0;
}

static int enable_scmi_gpio_node(void *blob, uint32_t phandle)
{

	return enable_scmi_protocol(blob, scmi_gpio_node_path, phandle);
}

static int disable_siul2_gpio_node(void *blob, uint32_t *phandle)
{
	return disable_node_by_compatible(blob, s32cc_gpio_compatible,
					  phandle);
}

static int enable_scmi_gpio(void *blob)
{
	ofnode node;
	u32 phandle;
	int ret;

	node = ofnode_path(scmi_gpio_node_path);
	if (!ofnode_valid(node)) {
		printf("%s: Couldn't find \"%s\" node\n", __func__,
		       scmi_gpio_node_path);
		return -EINVAL;
	}

	/* SCMI GPIO isn't enabled for U-Boot */
	if (!ofnode_is_available(node))
		return 0;

	node = ofnode_by_compatible(ofnode_null(), s32cc_gpio_compatible);
	if (!ofnode_valid(node)) {
		printf("%s: Couldn't find \"%s\" node\n", __func__,
		       s32cc_gpio_compatible);
		return -EINVAL;
	}

	/* Skip if default GPIO node is used */
	if (ofnode_is_available(node))
		return 0;

	ret = disable_siul2_gpio_node(blob, &phandle);
	if (ret)
		return ret;

	ret = enable_scmi_gpio_node(blob, phandle);
	if (ret)
		return ret;

	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret;

	/* Add some space for the following changes */
	ret = fdt_increase_size(blob, 512);
	if (ret < 0) {
		pr_err("Could not increase size of device tree: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/*
	 * Skip these fixups when reusing U-Boot dtb for Linux
	 * as they don't make sense.
	 *
	 * This block should be removed once the bindings and the dtbs
	 * used by Linux and U-Boot are fully compatible.
	 */
	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		printf("Skipping %s ...\n", __func__);
		return 0;
	}

	ret = ft_fixup_cpu(blob);
	if (ret)
		goto exit;

	ret = ft_fixup_memory(blob, bd);
	if (ret)
		goto exit;

	ret = ft_fixup_atf(blob);
	if (ret)
		goto exit;

	ret = apply_fdt_hwconfig_fixups(blob);
	if (ret)
		goto exit;

	if (CONFIG_IS_ENABLED(S32CC_SCMI_GPIO_FIXUP)) {
		ret = enable_scmi_gpio(blob);
		if (ret)
			return ret;
	}
exit:
	return ret;
}

int board_fix_fdt(void *rw_fdt_blob)
{
	return ft_fixup_cpu(rw_fdt_blob);
}
