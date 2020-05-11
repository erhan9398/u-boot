/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup	dxgr_PFE_PHY_IF PFE Physical Interface
 * @brief		PFE physical interface representation module
 * @details		This is the software representation of PFE physical interfaces. It is
 *				intended to abstract various types of Ethernet interfaces provided by
 * 				the PFE (EMAC, HIF channel) and enable driver and rest of the PFE software
 * 				to manage them via single API.
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
 * @addtogroup  dxgr_PFE_PHY_IF
 * @{
 *
 * @file		pfe_phy_if.h
 * @brief		The PFE Physical Interface module header file.
 * @details		This file contains PFE Physical Interface-related API.
 *
 */

#ifndef PUBLIC_PFE_PHY_IF_H_
#define PUBLIC_PFE_PHY_IF_H_

#include "oal_types.h"
#include "pfe_ct.h"	  /* common (fw/host) types */
#include "pfe_emac.h"	  /* pfe_emac_t, pfe_mac_addr_t */
#include "pfe_hif_chnl.h" /* pfe_hif_chnl_t */
#include "pfe_class.h"	  /* pfe_class_t */

/**
 * @brief	Interface callback reasons
 */
typedef enum {
	PHY_IF_EVT_MAC_ADDR_UPDATE, /*!< PHY_IF_EVT_MAC_ADDR_UPDATE */
	PHY_IF_EVT_INVALID	    /*!< PHY_IF_EVT_INVALID */
} pfe_phy_if_event_t;

typedef struct __pfe_phy_if_tag pfe_phy_if_t;

#include "pfe_log_if.h" /* pfe_log_if_t, needs pfe_phy_if_t */

/**
 * @brief	Interface callback type
 */
typedef void (*pfe_phy_if_cbk_t)(pfe_phy_if_t *iface, pfe_phy_if_event_t event,
				 void *arg);

pfe_phy_if_t *pfe_phy_if_create(pfe_class_t *class, pfe_ct_phy_if_id_t id,
				char_t *name);
bool_t pfe_phy_if_has_log_if(pfe_phy_if_t *iface, pfe_log_if_t *log_if);
errno_t pfe_phy_if_bind_emac(pfe_phy_if_t *iface, pfe_emac_t *emac);
pfe_emac_t *pfe_phy_if_get_emac(pfe_phy_if_t *iface);
errno_t pfe_phy_if_bind_hif(pfe_phy_if_t *iface, pfe_hif_chnl_t *hif);
pfe_hif_chnl_t *pfe_phy_if_get_hif(pfe_phy_if_t *iface);
pfe_ct_phy_if_id_t pfe_phy_if_get_id(pfe_phy_if_t *iface) __attribute__((pure));
char_t *pfe_phy_if_get_name(pfe_phy_if_t *iface) __attribute__((pure));
errno_t pfe_phy_if_destroy(pfe_phy_if_t *iface);
pfe_class_t *pfe_phy_if_get_class(pfe_phy_if_t *iface) __attribute__((pure));
errno_t pfe_phy_if_set_block_state(pfe_phy_if_t *iface,
				   pfe_ct_block_state_t block_state);
errno_t pfe_phy_if_get_block_state(pfe_phy_if_t *iface,
				   pfe_ct_block_state_t *block_state);
pfe_ct_if_op_mode_t pfe_phy_if_get_op_mode(pfe_phy_if_t *iface);
errno_t pfe_phy_if_set_op_mode(pfe_phy_if_t *iface, pfe_ct_if_op_mode_t mode);
bool_t pfe_phy_if_is_enabled(pfe_phy_if_t *iface);
errno_t pfe_phy_if_enable(pfe_phy_if_t *iface);
errno_t pfe_phy_if_disable(pfe_phy_if_t *iface);
bool_t pfe_phy_if_is_promisc(pfe_phy_if_t *iface);
errno_t pfe_phy_if_promisc_enable(pfe_phy_if_t *iface);
errno_t pfe_phy_if_promisc_disable(pfe_phy_if_t *iface);
errno_t pfe_phy_if_add_mac_addr(pfe_phy_if_t *iface, pfe_mac_addr_t addr);
errno_t pfe_phy_if_del_mac_addr(pfe_phy_if_t *iface, pfe_mac_addr_t addr);
errno_t pfe_phy_if_get_mac_addr(pfe_phy_if_t *iface, pfe_mac_addr_t addr);
uint32_t pfe_phy_if_get_text_statistics(pfe_phy_if_t *iface, char_t *buf,
					u32 buf_len, uint8_t verb_level);
errno_t pfe_phy_if_set_mirroring(pfe_phy_if_t *iface,
				 pfe_ct_phy_if_id_t mirror);
pfe_ct_phy_if_id_t pfe_phy_if_get_mirroring(pfe_phy_if_t *iface);

#endif /* PUBLIC_PFE_PHY_IF_H_ */

/** @}*/
/** @}*/
