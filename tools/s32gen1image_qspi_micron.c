// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2019-2021 NXP */

#include <generated/autoconf.h>
#include "s32_common.h"
#include "s32gen1image.h"

#ifdef CONFIG_FLASH_BOOT
/* Load U-Boot using DTR octal mode @ 133 MHz*/
#if defined(CONFIG_S32G274ARDB) || defined(CONFIG_TARGET_S32R45EVB) || \
	defined(CONFIG_TARGET_S32G274AEVB) || \
	defined(CONFIG_TARGET_S32G274ABLUEBOX3)
void adjust_qspi_params(struct qspi_params *qspi_params)
{
	qspi_params->dllcr = 0x82860704;
	qspi_params->dll_slv_upd_en = 0x01;
	qspi_params->sflash_clk_freq = 133;
}
#endif
static struct qspi_params s32g2xx_qspi_conf = {
	.header   = 0x5a5a5a5a,
	.mcr      = 0x010f00cc,
	.flshcr   = 0x00010303,
	.bufgencr = 0x00000000,
	.dllcr    = 0x82860704,
	.paritycr = 0x00000000,
	.sfacr    = 0x00000000,
	.smpr     = 0x44000000,
	.dlcr     = 0x40ff40ff,
	.sflash_1_size = 0x20000000,
	.sflash_2_size = 0x20000000,
	.dlpr = 0xaa553443,
	.sfar = 0x00000000,
	.ipcr = 0x00000000,
	.tbdr = 0x00000000,
	.dll_bypass_en   = 0x01,
	.dll_slv_upd_en  = 0x01,
	.dll_auto_upd_en = 0x00,
	.ipcr_trigger_en = 0x00,
	.sflash_clk_freq = 200,
	.reserved = {0x00, 0x00, 0x00},
	/* Macronix read - 8DTRD */
	.command_seq = {0x470247fd,
			0x0f102b20,
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
			/* Enable DTR OPI */
			.config = {
				.valid_addr = 1,
				.cdata_size = 1,
				.addr_size = 24,
				.pad = 0,
				.reserved = 0,
				.opcode = 0x81,
			},
			.addr = 0x0,
			.data = 0xe7,
		},
	},
};

struct qspi_params *get_s32g2xx_qspi_conf(void)
{
	return &s32g2xx_qspi_conf;
}
#endif
