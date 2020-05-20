/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup    dxgr_PFE_PE PE
 * @brief		The Processing Engine
 * @details     This is the software representation of the PE block.
 *
 * @addtogroup  dxgr_PFE_HIF
 * @{
 *
 * @file		pfe_pe.h
 * @brief		The PE module header file.
 * @details		This file contains PE-related API.
 *
 */

#ifndef PFE_PE_H_
#define PFE_PE_H_

#include "pfe_ct.h" /* fw/host shared types */

/*	Processing engine type */
typedef enum
{
	PE_TYPE_INVALID,
	PE_TYPE_CLASS,
	PE_TYPE_TMU,
	PE_TYPE_UTIL,
	PE_TYPE_MAX
} pfe_pe_type_t;

/*	Processing Engine representation */
typedef struct __pfe_pe_tag pfe_pe_t;

pfe_pe_t * pfe_pe_create(void *cbus_base_va, pfe_pe_type_t type, uint8_t id);
void pfe_pe_set_dmem(pfe_pe_t *pe, addr_t elf_base, addr_t len);
void pfe_pe_set_imem(pfe_pe_t *pe, addr_t elf_base, addr_t len);
void pfe_pe_set_lmem(pfe_pe_t *pe, addr_t elf_base, addr_t len);
void pfe_pe_set_ddr(pfe_pe_t *pe, void *base_pa, void *base_va, addr_t len);
void pfe_pe_set_iaccess(pfe_pe_t *pe, uint32_t wdata_reg, uint32_t rdata_reg, uint32_t addr_reg);
errno_t pfe_pe_load_firmware(pfe_pe_t *pe, const void *elf);
errno_t pfe_pe_get_mmap(pfe_pe_t *pe, pfe_ct_pe_mmap_t *mmap);
void pfe_pe_memcpy_from_host_to_dmem_32(pfe_pe_t *pe, addr_t dst, const void *src, uint32_t len);
void pfe_pe_memcpy_from_dmem_to_host_32(pfe_pe_t *pe, void *dst, addr_t src, uint32_t len);
void pfe_pe_memcpy_from_imem_to_host_32(pfe_pe_t *pe, void *dst, addr_t src, uint32_t len);
void pfe_pe_dmem_memset(pfe_pe_t *pe, uint8_t val, addr_t addr, uint32_t len);
void pfe_pe_imem_memset(pfe_pe_t *pe, uint8_t val, addr_t addr, uint32_t len);
errno_t pfe_pe_get_pe_stats(pfe_pe_t *pe, uint32_t addr, pfe_ct_pe_stats_t *stats);
errno_t pfe_pe_get_classify_stats(pfe_pe_t *pe, uint32_t addr, pfe_ct_classify_stats_t *stats);
errno_t pfe_pe_get_class_algo_stats(pfe_pe_t *pe, uint32_t addr, pfe_ct_class_algo_stats_t *stats);
uint32_t pfe_pe_stat_to_str(pfe_ct_class_algo_stats_t *stat, char *buf, uint32_t buf_len, uint8_t verb_level);
uint32_t pfe_pe_get_text_statistics(pfe_pe_t *pe, char_t *buf, uint32_t buf_len, uint8_t verb_level);
void pfe_pe_destroy(pfe_pe_t *pe);
errno_t pfe_pe_check_mmap(pfe_pe_t *pe);
errno_t pfe_pe_get_fw_errors(pfe_pe_t *pe);
errno_t pfe_pe_mem_lock(pfe_pe_t *pe);
errno_t pfe_pe_mem_unlock(pfe_pe_t *pe);
#endif /* PFE_PE_H_ */

/** @}*/
/** @}*/
