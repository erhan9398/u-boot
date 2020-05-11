/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2020 NXP
 *
 */

/**
 * @addtogroup  dxgr_PFE_WDT
 * @{
 *
 * @file		pfe_wdt_csr.h
 * @brief		The watchdog timer module registers definition file (s32g).
 * @details
 *
 */

#ifndef PFE_WDT_CSR_H_
#define PFE_WDT_CSR_H_

#include "pfe_wdt.h"

errno_t pfe_wdt_cfg_isr(void *base_va, void *cbus_base_va);
void pfe_wdt_cfg_irq_mask(void *base_va);
void pfe_wdt_cfg_irq_unmask(void *base_va);
void pfe_wdt_cfg_init(void *base_va);
void pfe_wdt_cfg_fini(void *base_va);
uint32_t pfe_wdt_cfg_get_text_stat(void *base_va, char_t *buf, uint32_t size,
				   uint8_t verb_level);

#endif /* PFE_WDT_CSR_H_ */

/** @}*/
