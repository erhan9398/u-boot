/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 NXP
 * S32Gen1 PCIe driver
 */

#ifndef SERDES_S32GEN1_H
#define SERDES_S32GEN1_H
#include <pci.h>
#include <dm.h>
#include <asm/io.h>

#include "serdes_regs.h"
#include "ss_pcie_regs.h"
#include "mc_rgm_regs.h"
#include "serdes_s32gen1_io.h"

#define PCIE_LINK_UP_COUNT 100
#define PCIE_MPLL_LOCK_COUNT 10
#define PCIE_RESET_COUNT 50
#define DELAY_QUANTUM 1000

#define SERDES_LINKUP_MASK	(SMLH_LINK_UP | RDLH_LINK_UP | \
			SMLH_LTSSM_STATE)
#define SERDES_LINKUP_EXPECT	(SMLH_LINK_UP | RDLH_LINK_UP | \
			SMLH_LTSSM_STATE_VALUE(LTSSM_STATE_L0))

/* Configuration Request Retry Status (CRS) Enable. Active high. */
/* Defer incoming configuration requests. */
#define CRS_EN					0x2
/* LTSSM Enable. Active high. Set it low to hold the LTSSM in Detect state. */
#define LTSSM_EN				0x1


/* Register SS_RW_REG_0 */

#define CLKEN_MASK	0x1
#define CLKEN_OFF	23

#define RST_CTRL		0x3010
#define WARM_RST		0x2
#define COLD_RST		0x1

enum serdes_link_width {
	X1 = 0x1,
	X2 = 0x2
};

struct s32_serdes {
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource dbi_res;
	void __iomem *dbi;

	int id;
	enum serdes_dev_type devtype;
	enum serdes_xpcs_mode xpcs_mode;
	enum serdes_clock clktype;
	enum serdes_clock_fmhz fmhz;
	enum serdes_link_width linkwidth;
};

int wait_read32(void *address, uint32_t expect_data,
		uint32_t mask, int read_attempts);

#endif /* PCIE_S32GEN1_H */
