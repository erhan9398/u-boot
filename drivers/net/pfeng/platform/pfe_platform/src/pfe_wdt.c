// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2020 NXP
 *
 */

/**
 * @addtogroup  dxgr_PFE_WDT
 * @{
 *
 * @file		pfe_wdt.c
 * @brief		The WDT module source file.
 * @details		This file contains WDT-related functionality.
 *
 */

#include "oal.h"
#include "hal.h"

#include "pfe_cbus.h"
#include "pfe_wdt.h"
#include "pfe_wdt_csr.h"

struct __pfe_wdt_tag {
	void *cbus_base_va; /*	CBUS base virtual address */
	void *wdt_base_offset; /*	WDT base offset within CBUS space (WDT is member of WSP global CSR)*/
	void *wdt_base_va; /*	WDT base address (virtual) (It is actually WSP global CSR base address)*/
	oal_mutex_t lock;
};

pfe_wdt_t *
pfe_wdt_create(void *cbus_base_va, void *wdt_base)
{
	pfe_wdt_t *wdt;

#if defined(GLOBAL_CFG_NULL_ARG_CHECK)
	if (unlikely(!cbus_base_va)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}
#endif /* GLOBAL_CFG_NULL_ARG_CHECK */

	wdt = oal_mm_malloc(sizeof(pfe_wdt_t));

	if (!wdt) {
		return NULL;
	} else {
		memset(wdt, 0, sizeof(pfe_wdt_t));
		wdt->cbus_base_va = cbus_base_va;
		wdt->wdt_base_offset = wdt_base;
		wdt->wdt_base_va = (void *)((addr_t)wdt->cbus_base_va +
					    (addr_t)wdt->wdt_base_offset);

		/*	Resource protection */
		if (oal_mutex_init(&wdt->lock) != EOK) {
			NXP_LOG_DEBUG("Mutex initialization failed\n");
			oal_mm_free(wdt);
			return NULL;
		}
	}
#ifdef PFE_CFG_PARANOID_IRQ
	if (oal_mutex_lock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex lock failed\n");
	}
#endif /* PFE_CFG_PARANOID_IRQ */

	pfe_wdt_cfg_init(wdt->wdt_base_va);

#ifdef PFE_CFG_PARANOID_IRQ
	if (oal_mutex_unlock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex unlock failed\n");
	}
#endif /* PFE_CFG_PARANOID_IRQ */

	return wdt;
}

/**
 * @brief		Destroy WDT instance
 * @param[in]	wdt The WDT instance
 */
void
pfe_wdt_destroy(pfe_wdt_t *wdt)
{
	if (wdt) {
		if (oal_mutex_lock(&wdt->lock) != EOK) {
			NXP_LOG_DEBUG("Mutex lock failed\n");
		}

		pfe_wdt_cfg_fini(wdt->wdt_base_va);

		if (oal_mutex_unlock(&wdt->lock) != EOK) {
			NXP_LOG_DEBUG("Mutex unlock failed\n");
		}

		if (oal_mutex_destroy(&wdt->lock) != EOK) {
			NXP_LOG_DEBUG("Mutex destroy failed\n");
		}

		/* Free memory used for structure */
		oal_mm_free(wdt);
	}
}

/**
 * @brief		WDT ISR
 * @param[in]	wdt The WDT instance
 * @return		EOK if interrupt has been handled
 */
errno_t
pfe_wdt_isr(pfe_wdt_t *wdt)
{
	bool_t ret = FALSE;

#if defined(GLOBAL_CFG_NULL_ARG_CHECK)
	if (unlikely(!wdt)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return FALSE;
	}
#endif /* GLOBAL_CFG_NULL_ARG_CHECK */

	if (oal_mutex_lock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex lock failed\n");
	}

	/*	Run the low-level ISR to identify and process the interrupt */
	if (pfe_wdt_cfg_isr(wdt->wdt_base_va, wdt->cbus_base_va) == EOK) {
		/*	IRQ handled */
		ret = TRUE;
	}

	if (oal_mutex_unlock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Mask BMU interrupts
 * @param[in]	The WDT instance
 */
void
pfe_wdt_irq_mask(pfe_wdt_t *wdt)
{
	if (oal_mutex_lock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex lock failed\n");
	}

	pfe_wdt_cfg_irq_mask(wdt->wdt_base_va);

	if (oal_mutex_unlock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex unlock failed\n");
	}
}

/**
 * @brief		Unmask WDT interrupts
 * @param[in]	The WDT instance
 */
void
pfe_wdt_irq_unmask(pfe_wdt_t *wdt)
{
	if (oal_mutex_lock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex lock failed\n");
	}

	pfe_wdt_cfg_irq_unmask(wdt->wdt_base_va);

	if (oal_mutex_unlock(&wdt->lock) != EOK) {
		NXP_LOG_DEBUG("Mutex unlock failed\n");
	}
}

/**
 * @brief		Return WDT runtime statistics in text form
 * @details		Function writes formatted text into given buffer.
 * @param[in]	wdt		The WDT instance
 * @param[in]	buf		Pointer to the buffer to write to
 * @param[in]	buf_len	Buffer length
 * @param[in]	verb_level	Verbosity level
 * @return		Number of bytes written to the buffer
 */
uint32_t
pfe_wdt_get_text_statistics(pfe_wdt_t *wdt, char_t *buf, uint32_t buf_len,
			    uint8_t verb_level)
{
	u32 len = 0U;

#if defined(GLOBAL_CFG_NULL_ARG_CHECK)
	if (unlikely(!wdt)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return 0U;
	}
#endif /* GLOBAL_CFG_NULL_ARG_CHECK */

	len += pfe_wdt_cfg_get_text_stat(wdt->wdt_base_va, buf, buf_len,
					 verb_level);

	return len;
}

/** @}*/
