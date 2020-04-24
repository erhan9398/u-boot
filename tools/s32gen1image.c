/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2020 NXP */

#include <image.h>
#include <generated/autoconf.h>
#include <config.h>
#include "imagetool.h"
#include "s32gen1image.h"

#define UNSPECIFIED	-1

#ifdef CONFIG_FLASH_BOOT
#  define S32G2XX_COMMAND_SEQ_FILL_OFF 20
#endif

#define S32GEN1_AUTO_OFFSET ((size_t)(-1))

#ifdef CONFIG_FLASH_BOOT
#  define S32GEN1_QSPI_PARAMS_OFFSET 0x200U
#endif

#if defined(CONFIG_S32R45X) || defined(CONFIG_S32V344)
#  ifdef CONFIG_FLASH_BOOT
#    define S32GEN1_IVT_OFFSET	0x0U
#  else
#    define S32GEN1_IVT_OFFSET	0x1000U
#  endif
#elif defined(CONFIG_S32G274A)
#  define S32GEN1_IVT_OFFSET	0x0U
#endif

static struct program_image image_layout = {
	.ivt = {
		.offset = S32GEN1_IVT_OFFSET,
		.size = sizeof(struct ivt),
	},
#ifdef CONFIG_FLASH_BOOT
	.qspi_params = {
		.offset = S32GEN1_QSPI_PARAMS_OFFSET,
		.size = S32GEN1_QSPI_PARAMS_SIZE,
	},
#endif
	.dcd = {
		.offset = S32GEN1_AUTO_OFFSET,
		.alignment = 0x200U,
		.size = sizeof(struct dcd),
	},
	.app_code = {
		.offset = S32GEN1_AUTO_OFFSET,
		.alignment = 0x200U,
		.size = sizeof(struct application_boot_code),
	},
};

static struct dcd_command *last_command;

static table_entry_t dcd_commands[] = {
	{WRITE_DATA,			"WRITE_DATA",			"",},
	{WRITE_SET_BITMASK,		"WRITE_SET_BITMASK",		"",},
	{WRITE_CLEAR_BITMASK,		"WRITE_CLEAR_BITMASK",		"",},
	{CHECK_BITS_ARE_SET,		"CHECK_BITS_ARE_SET",		"",},
	{CHECK_BITS_ARE_CLEAR,		"CHECK_BITS_ARE_CLEAR",		"",},
	{CHECK_ANY_BIT_IS_SET,		"CHECK_ANY_BIT_IS_SET",		"",},
	{CHECK_ANY_BIT_IS_CLEAR,	"CHECK_ANY_BIT_IS_CLEAR",	"",},
	{NOP,				"NOP",				"",},
	{INVALID,			"",				"",},
};

static struct ivt *get_ivt(struct program_image *image)
{
	return (struct ivt *)image->ivt.data;
}

static struct dcd *get_dcd(struct program_image *image)
{
	return (struct dcd *)image->dcd.data;
}

static struct application_boot_code *get_app_code(struct program_image *image)
{
	return (struct application_boot_code *)image->app_code.data;
}

static void s32gen1_add_dcd_command_header(__u8 tag, __u8 params)
{
	size_t length = be16_to_cpu(last_command->length);

	if (tag == DCD_WRITE_COMMAND_TAG &&
	    last_command->tag == tag &&
	    last_command->params == params)
		return;

	last_command = (struct dcd_command *)((__u8 *)last_command + length);

	last_command->tag = tag;
	last_command->length = cpu_to_be16(4);
	last_command->params = params;
}

static void s32gen1_add_dcd_command(int command_type, int access_width,
				    int register_address, int register_data,
				    int count,
				    struct program_image *program_image)
{
	size_t length = 0;
	unsigned int offset;
	struct dcd *dcd;
	__u8 tag = 0;
	__u8 params = 0;
	size_t dcd_length;

	switch (command_type) {
	case WRITE_DATA:
		tag = DCD_WRITE_COMMAND_TAG;
		params = access_width;
		break;
	case WRITE_SET_BITMASK:
		tag = DCD_WRITE_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 DCD_COMMAND_PARAMS_DATA_SET	|
			 access_width;
		break;
	case WRITE_CLEAR_BITMASK:
		tag = DCD_WRITE_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 access_width;
		break;
	case CHECK_BITS_ARE_SET:
		tag = DCD_CHECK_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_SET	|
			 access_width;
		break;
	case CHECK_BITS_ARE_CLEAR:
		tag = DCD_CHECK_COMMAND_TAG;
		params = access_width;
		break;
	case CHECK_ANY_BIT_IS_SET:
		tag = DCD_CHECK_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 DCD_COMMAND_PARAMS_DATA_SET	|
			 access_width;
		break;
	case CHECK_ANY_BIT_IS_CLEAR:
		tag = DCD_CHECK_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 access_width;
		break;
	case NOP:
		tag = DCD_NOP_COMMAND_TAG;
		break;
	}

	s32gen1_add_dcd_command_header(tag, params);

	switch (tag) {
	case DCD_WRITE_COMMAND_TAG:
		offset = (be16_to_cpu(last_command->length) - 4) / 8;
		last_command->s.write[offset].addr =
						cpu_to_le32(register_address);
		last_command->s.write[offset].data =
						cpu_to_le32(register_data);
		length = be16_to_cpu(last_command->length) + 8;
		last_command->length = cpu_to_be16(length);
		break;

	case DCD_CHECK_COMMAND_TAG:
		last_command->s.check.address = cpu_to_le32(register_address);
		last_command->s.check.mask = cpu_to_le32(register_data);
		length = be16_to_cpu(last_command->length) + 8;

		if (count != UNSPECIFIED) {
			last_command->s.check.count = count;
			length = length + 4;
		}

		last_command->length = cpu_to_be16(length);
		break;

	case DCD_NOP_COMMAND_TAG:
		break;
	}

	dcd = get_dcd(program_image);
	dcd_length = (__u8 *)last_command +
		     be16_to_cpu(last_command->length) -
		     (__u8 *)dcd;

	if (dcd_length > DCD_MAXIMUM_SIZE) {
		fprintf(stderr, "DCD length of %zu exceeds the maximum of %d\n",
			dcd_length, DCD_MAXIMUM_SIZE);
		exit(EXIT_FAILURE);
	}

	dcd->length = cpu_to_be16(dcd_length);
}

static void s32gen1_parse_configuration_file(const char *filename,
					     struct program_image *program_image)
{
	FILE *f = NULL;
	char *line = NULL;
	char *token = NULL;
	size_t line_length;

	int command_type;
	int access_width;
	int register_address;
	int register_data;
	int count;

	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Can't open file %s: %s\n", filename,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ((getline(&line, &line_length, f)) > 0) {

		command_type = INVALID;
		access_width = 0;
		register_address = 0;
		register_data = 0;
		count = UNSPECIFIED;

		token = strtok(line, " \r\n");
		if (token != NULL)
			command_type = get_table_entry_id(dcd_commands, "",
							  token);
		token = strtok(NULL, " \r\n");
		if (token != NULL)
			access_width = strtoul(token, NULL, 0);

		token = strtok(NULL, " \r\n");
		if (token != NULL)
			register_address = strtoul(token, NULL, 16);

		token = strtok(NULL, " \r\n");
		if (token != NULL)
			register_data = strtoul(token, NULL, 16);

		token = strtok(NULL, " \r\n");
		if (token != NULL)
			count = strtoul(token, NULL, 0);


		if (command_type != INVALID)
			s32gen1_add_dcd_command(command_type,
						access_width,
						register_address,
						register_data,
						count,
						program_image);
	}

	fclose(f);
}

static void s32gen1_print_header(const void *header)
{
	return;
}

#ifndef CONFIG_FLASH_BOOT
static void enforce_reserved_range(void *image_start, int image_length,
				   void *reserved_start, void *reserved_end)
{
	void *image_end = (void*)((__u8*)image_start + image_length);

	if (image_start < reserved_end && image_end > reserved_start) {
		fprintf(stderr, "Loading data of size 0x%x at 0x%p forbidden.",
			image_length, image_end);
		fprintf(stderr, " Range 0x%p --- 0x%p is reserved!\n",
			reserved_start, reserved_end);
		exit(EXIT_FAILURE);
	}
}

#else
#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
/* Load U-Boot using DTR octal mode @ 133 MHz*/
static struct qspi_params s32g2xx_qspi_conf = {
	.header   = 0x5a5a5a5a,
	.mcr      = 0x030f00cc,
	.flshcr   = 0x00010303,
	.bufgencr = 0x00000000,
	.dllcr    = 0x8280000c,
	.paritycr = 0x00000000,
	.sfacr    = 0x00020000,
	.smpr     = 0x44000000,
	.dlcr     = 0x40ff40ff,
	.sflash_1_size = 0x20000000,
	.sflash_2_size = 0x20000000,
	.dlpr = 0xaa553443,
	.sfar = 0x00000000,
	.ipcr = 0x00000000,
	.tbdr = 0x00000000,
	.dll_bypass_en   = 0x00,
	.dll_slv_upd_en  = 0x01,
	.dll_auto_upd_en = 0x01,
	.ipcr_trigger_en = 0x00,
	.sflash_clk_freq = 133,
	.reserved = {0x00, 0x00, 0x00},
	/* Macronix read - 8DTRD */
	.command_seq = {0x471147ee,
			0x0f142b20,
			0x00003b10},
	.writes = {
		{
			/* Write enable */
			.config = {
				.valid_addr = 0,
				.cdata_size = 0,
				.addr_size = 0,
				.pad = 0,
				.reserved = 0,
				.opcode = 6,
			},
			.addr = 0,
			.data = 0,
		},
		{
			/* WRCR2 - DTR OPI */
			.config = {
				.valid_addr = 1,
				.cdata_size = 1,
				.addr_size = 32,
				.pad = 0,
				.reserved = 0,
				.opcode = 0x72,
			},
			.addr = 0x0,
			.data = 0x2,
		},
	},
};

static struct qspi_params *get_qspi_params(struct program_image *image)
{
	return (struct qspi_params *)image->qspi_params.data;
}

static void s32gen1_set_qspi_params(struct qspi_params *qspi_params)
{
	memcpy(qspi_params, &s32g2xx_qspi_conf, sizeof(*qspi_params));
}
#endif /* CONFIG_TARGET_TYPE_S32GEN1_EMULATOR */
#endif

static void set_data_pointers(struct program_image *layout, void *header)
{
	uint8_t *data = (uint8_t *)header;

	layout->ivt.data = data + layout->ivt.offset;
#ifdef CONFIG_FLASH_BOOT
	layout->qspi_params.data = data + layout->qspi_params.offset;
#endif
	layout->dcd.data = data + layout->dcd.offset;
	layout->app_code.data = data + layout->app_code.offset;
}

static void s32gen1_set_header(void *header, struct stat *sbuf, int unused,
			       struct image_tool_params *tool_params)
{
	size_t code_length;
	struct dcd *dcd;
	struct ivt *ivt;
	struct application_boot_code *app_code;

	set_data_pointers(&image_layout, header);

	dcd = get_dcd(&image_layout);
	ivt = get_ivt(&image_layout);
	app_code = get_app_code(&image_layout);

	last_command = (struct dcd_command *)&dcd->commands[0];
	s32gen1_parse_configuration_file(tool_params->imagename,
					 &image_layout);

	ivt->tag = IVT_TAG;
	ivt->length = cpu_to_be16(sizeof(struct ivt));
	ivt->version = IVT_VERSION;

	ivt->dcd_pointer = image_layout.dcd.offset;
	ivt->boot_configuration_word = BCW_BOOT_TARGET_A53_0;
	ivt->application_boot_code_pointer = image_layout.app_code.offset;

	dcd->tag = DCD_TAG;
	dcd->version = DCD_VERSION;

	app_code->tag = APPLICATION_BOOT_CODE_TAG;
	app_code->version = APPLICATION_BOOT_CODE_VERSION;
	app_code->ram_start_pointer = CONFIG_SYS_TEXT_BASE;
	app_code->ram_entry_pointer = CONFIG_SYS_TEXT_BASE;

	code_length = sbuf->st_size
			- image_layout.app_code.offset
			- offsetof(struct application_boot_code, code);

	if (code_length % 0x40) {
		code_length &= ~0x3f;
		code_length += 0x40;
	}
	app_code->code_length = code_length;

#ifndef CONFIG_FLASH_BOOT
	enforce_reserved_range((void*)(__u64)
			       app_code->ram_start_pointer,
			       app_code->code_length,
			       (void*)SRAM_RESERVED_0_START,
			       (void*)SRAM_RESERVED_0_END);

	enforce_reserved_range((void*)(__u64)
			       app_code->ram_start_pointer,
			       app_code->code_length,
			       (void*)SRAM_RESERVED_1_START,
			       (void*)SRAM_RESERVED_1_END);
#else
#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	s32gen1_set_qspi_params(get_qspi_params(&image_layout));
#endif
#endif

	return;
}

static int s32gen1_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_S32GEN1IMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int image_parts_comp(const void *p1, const void *p2)
{
	const struct image_comp **part1 = (typeof(part1))p1;
	const struct image_comp **part2 = (typeof(part2))p2;

	if ((*part2)->offset > (*part1)->offset)
		return -1;

	if ((*part2)->offset < (*part1)->offset)
		return 1;

	return 0;
}

static void check_overlap(struct image_comp *comp1,
			  struct image_comp *comp2)
{
	size_t end1 = comp1->offset + comp1->size;
	size_t end2 = comp2->offset + comp2->size;

	if (end1 > comp2->offset && end2 > comp1->offset) {
		fprintf(stderr, "Detected overlap between 0x%zx@0x%zx and "
				"0x%zx@0x%zx\n",
				comp1->size, comp1->offset,
				comp2->size, comp2->offset);
		exit(EXIT_FAILURE);
	}
}

static void s32g2xx_compute_dyn_offsets(struct image_comp **parts,
					size_t n_parts)
{
	size_t i;
	size_t align_mask;
	size_t rem;

	for (i = 0U; i < n_parts; i++) {
		if (parts[i]->offset == S32GEN1_AUTO_OFFSET) {
			if (i == 0) {
				parts[i]->offset = 0U;
				continue;
			}

			parts[i]->offset = parts[i - 1]->offset +
			    parts[i - 1]->size;
		}

		/* Apply alignment constraints */
		if (parts[i]->alignment != 0U) {
			align_mask = (parts[i]->alignment << 1U) - 1U;
			rem = parts[i]->offset & align_mask;
			if (rem != 0U) {
				parts[i]->offset -= rem;
				parts[i]->offset += parts[i]->alignment;
			}
		}

		if (i != 0)
			check_overlap(parts[i - 1], parts[i]);
	}
}

static int s32g2xx_build_layout(struct program_image *program_image,
				size_t *header_size, void **image)
{
	uint8_t *image_layout;
	struct image_comp *parts[] = {&program_image->ivt,
		&program_image->dcd,
#ifdef CONFIG_FLASH_BOOT
		&program_image->qspi_params,
#endif
		&program_image->app_code,
	};
	size_t last_comp = ARRAY_SIZE(parts) - 1;

	qsort(&parts[0], ARRAY_SIZE(parts), sizeof(parts[0]), image_parts_comp);

	/* Compute auto-offsets */
	s32g2xx_compute_dyn_offsets(parts, ARRAY_SIZE(parts));

	*header_size = parts[last_comp]->offset + parts[last_comp]->size;

	image_layout = calloc(*header_size, sizeof(*image_layout));
	if (!image_layout) {
		perror("Call to calloc() failed");
		return -ENOMEM;
	}

	*image = image_layout;
	return 0;
}

static int s32gen1_vrec_header(struct image_tool_params *tool_params,
			       struct image_type_params *type_params)
{
	size_t header_size;
	void *image = NULL;

	s32g2xx_build_layout(&image_layout, &header_size, &image);
	type_params->header_size = header_size;
	type_params->hdr = image;

	return 0;
}

U_BOOT_IMAGE_TYPE(
	s32gen1_image,
	"NXP S32GEN1 Boot Image",
	0,
	NULL,
	NULL,
	NULL,
	s32gen1_print_header,
	s32gen1_set_header,
	NULL,
	s32gen1_check_image_type,
	NULL,
	s32gen1_vrec_header
);
