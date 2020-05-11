/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup	dxgr_PFE_LOG_IF PFE Logical Interface
 * @brief		PFE logical interface representation module
 * @details		This is the software representation of PFE logical interfaces.
 *
 *				[TODO: Future extension]
 *				Interface properties are shared between PFE driver and PFE firmware. The
 *				PFE firmware is using the interfaces within traffic processing chain and
 *				thus needs to keep the interface-related properties in PFE internal memory.
 *				So the PFE interface SW module must ensure that every change done on
 *				interface in host domain (property/state change) will be propagated to data
 *				structures describing the interface in PFE domain. For instance, if port-based
 *				VLAN feature is implemented then once host changes the default port VLAN ID,
 *				this change must be announced to PFE firmware too.
 *
 * @addtogroup  dxgr_PFE_LOG_IF
 * @{
 *
 * @file		pfe_log_if.h
 * @brief		The PFE Logical Interface module header file.
 * @details		This file contains PFE Logical Interface-related API.
 *
 */

#ifndef PUBLIC_PFE_LOG_IF_H_
#define PUBLIC_PFE_LOG_IF_H_

#include "pfe_ct.h" /* common (fw/host) types */

/**
 * @brief	Interface callback reasons
 */
typedef enum
{
	LOG_IF_EVT_MAC_ADDR_UPDATE,	/*!< LOG_IF_EVT_MAC_ADDR_UPDATE */
	LOG_IF_EVT_INVALID         	/*!< LOG_IF_EVT_INVALID */
} pfe_log_if_event_t;

typedef struct __pfe_log_if_tag pfe_log_if_t;

#include "pfe_phy_if.h" /* pfe_phy_if_t, needs pfe_log_if_t */

/**
 * @brief	Interface callback type
 */
typedef void (* pfe_log_if_cbk_t)(pfe_log_if_t *iface, pfe_log_if_event_t event, void *arg);

pfe_log_if_t *pfe_log_if_create(pfe_phy_if_t *parent, char_t *name);
uint8_t pfe_log_if_get_id(pfe_log_if_t *iface) __attribute__((pure));
__attribute__((pure)) pfe_phy_if_t *pfe_log_if_get_parent(pfe_log_if_t *iface);
errno_t pfe_log_if_set_next_dmem_ptr(pfe_log_if_t *iface, addr_t next_dmem_ptr);
errno_t pfe_log_if_get_next_dmem_ptr(pfe_log_if_t *iface, addr_t *next_dmem_ptr);
errno_t pfe_log_if_get_dmem_base(pfe_log_if_t *iface, addr_t *dmem_base);
void pfe_log_if_destroy(pfe_log_if_t *iface);
errno_t pfe_log_if_set_match_or(pfe_log_if_t *iface);
errno_t pfe_log_if_set_match_and(pfe_log_if_t *iface);
bool_t pfe_log_if_is_match_or(pfe_log_if_t *iface);
errno_t pfe_log_if_set_match_rules(pfe_log_if_t *iface, pfe_ct_if_m_rules_t rules, pfe_ct_if_m_args_t *args);
errno_t pfe_log_if_get_match_rules(pfe_log_if_t *iface, pfe_ct_if_m_rules_t *rules, pfe_ct_if_m_args_t *args);
errno_t pfe_log_if_add_match_rule(pfe_log_if_t *iface, pfe_ct_if_m_rules_t rule, void *arg, uint32_t arg_len);
errno_t pfe_log_if_del_match_rule(pfe_log_if_t *iface, pfe_ct_if_m_rules_t rule);
errno_t pfe_log_if_set_mac_addr(pfe_log_if_t *iface, pfe_mac_addr_t addr);
errno_t pfe_log_if_get_mac_addr(pfe_log_if_t *iface, pfe_mac_addr_t addr);
errno_t pfe_log_if_get_egress_ifs(pfe_log_if_t *iface, uint32_t *egress);
errno_t pfe_log_if_set_egress_ifs(pfe_log_if_t *iface, uint32_t egress);
errno_t pfe_log_if_add_egress_if(pfe_log_if_t *iface, pfe_phy_if_t *phy_if);
errno_t pfe_log_if_del_egress_if(pfe_log_if_t *iface, pfe_phy_if_t *phy_if);
errno_t pfe_log_if_clear_mac_addr(pfe_log_if_t *iface);
errno_t pfe_log_if_enable(pfe_log_if_t *iface);
errno_t pfe_log_if_disable(pfe_log_if_t *iface);
bool_t pfe_log_if_is_enabled(pfe_log_if_t *iface) __attribute__((pure));
errno_t pfe_log_if_promisc_enable(pfe_log_if_t *iface);
errno_t pfe_log_if_promisc_disable(pfe_log_if_t *iface);
bool_t pfe_log_if_is_promisc(pfe_log_if_t *iface) __attribute__((pure));
char_t *pfe_log_if_get_name(pfe_log_if_t *iface) __attribute__((pure));
errno_t pfe_log_if_discard_enable(pfe_log_if_t *iface);
errno_t pfe_log_if_discard_disable(pfe_log_if_t *iface);
bool_t pfe_log_if_is_discard(pfe_log_if_t *iface);
uint32_t pfe_log_if_get_text_statistics(pfe_log_if_t *iface, char_t *buf, uint32_t buf_len, uint8_t verb_level);

#endif /* PUBLIC_PFE_LOG_IF_H_ */

/** @}*/
/** @}*/

