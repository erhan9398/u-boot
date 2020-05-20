// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup  dxgr_PFE_UTIL
 * @{
 *
 * @file		pfe_util_csr.c
 * @brief		The UTIL module low-level API (LS1012a).
 * @details
 *
 */

#include "oal.h"
#include "hal.h"
#include "pfe_cbus.h"
#include "pfe_util_csr.h"

/**
 * @brief		Get UTIL statistics in text form
 * @details		This is a HW-specific function providing detailed text statistics
 * 				about the UTIL block.
 * @param[in]	base_va 	Base address of UTIL register space (virtual)
 * @param[in]	buf 		Pointer to the buffer to write to
 * @param[in]	size 		Buffer length
 * @param[in]	verb_level 	Verbosity level
 * @return		Number of bytes written to the buffer
 */
uint32_t pfe_util_cfg_get_text_stat(void *base_va, char_t *buf, uint32_t size, uint8_t verb_level)
{
	uint32_t len = 0U, reg;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(NULL == base_va))
	{
		NXP_LOG_ERROR("NULL argument received\n");
		return 0U;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Get version */
	if(verb_level >= 9U)
	{
		reg = hal_read32(base_va + UTIL_VERSION);
		len += oal_util_snprintf(buf + len, size - len, "Revision             : 0x%x\n", (reg >> 24) & 0xff);
		len += oal_util_snprintf(buf + len, size - len, "Version              : 0x%x\n", (reg >> 16) & 0xff);
		len += oal_util_snprintf(buf + len, size - len, "ID                   : 0x%x\n", reg & 0xffff);
	}

	len += oal_util_snprintf(buf + len, size - len, "Max buffer count     : 0x%08x\n", hal_read32(base_va + UTIL_MAX_BUF_CNT));
	len += oal_util_snprintf(buf + len, size - len, "TQS max count:       : 0x%08x\n", hal_read32(base_va + UTIL_TSQ_MAX_CNT));

	return len;
}

/** @}*/
