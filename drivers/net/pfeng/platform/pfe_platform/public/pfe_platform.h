/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @defgroup	dxgrPFE_PLATFORM PFE Platform
 * @brief		The PFE Platform
 * @details		This SW module is intended to provide full PFE HW abstraction layer. Using
 * 				this module's API one is able to configure, control and encapsulate particular
 * 				PFE HW components like classifier, GPI or HIF as a single object.
 * 				
 * 				Module can be used to create and maintain various PFE instances depending on
 * 				currently used HW platform or HW configuration without need to modify upper
 * 				or application-level SW layers.
 * 				
 * 				Module consists of following components:
 * 				- pfe_pe - A generic processing engine representation
 * 				- pfe_class - The CLASS block
 * 				- pfe_rtable - The routing table
 * 				- pfe_tmu - The TMU block
 * 				- pfe_util - The UTIL block
 * 				- pfe_bmu - The BMU block
 * 				- pfe_gpi - The GPI block
 * 				- pfe_util - The UTIL block
 * 				- pfe_emac - The EMAC block
 * 				- pfe_hif - The HIF block
 * 				- pfe_hif_chnl - The HIF channel
 * 				- pfe_hif_ring - The HIF BD ring
 *				- pfe_wdt - The WDT block
 * 				
 * 				These components can be used to synthesize SW representation of a PPFE HW block
 * 				using hierarchical structure where top level is the PFE platform object. 
 * 
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 * 
 * @file		pfe_platform.h
 * @brief		The PFE platform management
 * @details		Header for the HW-specific code
 *
 */

#ifndef SRC_PFE_PLATFORM_H_
#define SRC_PFE_PLATFORM_H_

#include "pfe_platform_cfg.h"
#include "pfe_pe.h"
#include "pfe_gpi.h"
#include "pfe_bmu.h"
#include "pfe_class.h"
#if defined(PFE_CFG_RTABLE_ENABLE)
#include "pfe_rtable.h"
#endif /* PFE_CFG_RTABLE_ENABLE */
#include "pfe_tmu.h"
#include "pfe_util.h"
#include "pfe_hif.h"
#include "pfe_hif_nocpy.h"
#if !defined(TARGET_OS_UBOOT)
#include "pfe_safety.h"
#endif
#include "pfe_emac.h"
#if defined(PFE_CFG_L2BRIDGE_ENABLE)
#include "pfe_l2br_table.h"
#include "pfe_l2br.h"
#endif /* PFE_CFG_L2BRIDGE_ENABLE */
#include "pfe_phy_if.h"
#include "pfe_log_if.h"
#include "pfe_hif_drv.h"
#include "pfe_if_db.h"
#include "pfe_wdt.h"

#define GEMAC0_MAC                                       \
	{                                                \
		0x00U, 0x0AU, 0x0BU, 0x0CU, 0x0DU, 0x0EU \
	}
#define GEMAC1_MAC                                       \
	{                                                \
		0x00U, 0x1AU, 0x1BU, 0x1CU, 0x1DU, 0x1EU \
	}
#define GEMAC2_MAC                                       \
	{                                                \
		0x00U, 0x2AU, 0x2BU, 0x2CU, 0x2DU, 0x2EU \
	}

typedef enum {
	POLLER_STATE_DISABLED,
	POLLER_STATE_ENABLED,
	POLLER_STATE_STOPPED
} pfe_poller_state_t;

typedef struct
/*	The PFE firmware data type */
{
	char_t *version;     /* free text: version */
	char_t *source;	     /* free text: filename, filepath, ... etc */
	void *class_data;    /* The CLASS fw data buffer */
	u32 class_size; /* The CLASS fw data size */
	void *tmu_data;	     /* The TMU fw data buffer */
	u32 tmu_size;   /* The TMU fw data size */
	void *util_data;     /* The UTIL fw data buffer */
	u32 util_size;  /* The UTIL fw data size */
} pfe_fw_t;

/*	The PFE platform config */
typedef struct {
	addr_t cbus_base;	/* PFE control bus base address */
	addr_t cbus_len;	/* PFE control bus size */
	char_t *fw_name;	/* FW name */
	pfe_fw_t *fw;		/* Required firmware, embedded */
	bool_t common_irq_mode; /* True if FPGA specific common irq is used */
	u32 irq_vector_global; /* Global IRQ number */
	u32 irq_vector_bmu;    /* BMU IRQ number */

	pfe_hif_chnl_id_t
		hif_chnls_mask; /* The bitmap list of the requested HIF channels */
	uint32_t irq_vector_hif_chnls
		[HIF_CFG_MAX_CHANNELS]; /* HIF channels IRQ number */
	uint32_t irq_vector_hif_nocpy;	/* HIF nocopy channel IRQ number */
	u32 irq_vector_upe_gpt;	/* UPE + GPT IRQ number */
	u32 irq_vector_safety;	/* Safety IRQ number */
	bool_t enable_util;		/* Shall be UTIL enabled? */
} pfe_platform_config_t;

/*	The PFE platform data type */
typedef struct {
	volatile bool_t
		probed; /* Flag indicating that instance has been successfully probed */
	void *cbus_baseaddr;
	void *bmu_buffers_va;
	addr_t bmu_buffers_size;
	void *rtable_va;
	addr_t rtable_size;
	oal_irq_t *irq_global;
#if defined(PFE_CFG_GLOB_ERR_POLL_WORKER)
	oal_thread_t *poller; /* Global poller thread */
#endif			      /* PFE_CFG_GLOB_ERR_POLL_WORKER */
	pfe_poller_state_t poller_state;
	oal_irq_t *irq_bmu;	 /* BMU IRQ */
	u32 hif_chnl_count; /* Number of HIF channels */
#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
	pfe_hif_nocpy_t *hif_nocpy; /* The HIF_NOCPY block */
	oal_irq_t *irq_hif_nocpy;   /* HIF nocopy channel IRQ */
#endif				    /* PFE_CFG_HIF_NOCPY_SUPPORT */
	u32 emac_count;	    /* Number of EMAC blocks */
	u32 gpi_count;	    /* Number of GPI blocks */
	u32 etgpi_count;	    /* Number of ETGPI blocks */
	u32 hgpi_count;	    /* Number of HGPI blocks */
	u32 bmu_count;	    /* Number of BMU blocks */
	u32 class_pe_count;    /* Number of CLASS PEs */
	u32 util_pe_count;	    /* Number of UTIL PEs */
	u32 tmu_pe_count;	    /* Number of TMU PEs */
	pfe_fw_t *fw;
#if defined(PFE_CFG_RTABLE_ENABLE)
	pfe_rtable_t *rtable; /* The routing table */
#endif			      /* PFE_CFG_RTABLE_ENABLE */
#if defined(PFE_CFG_L2BRIDGE_ENABLE)
	pfe_l2br_table_t *mactab;  /* The MAC table */
	pfe_l2br_table_t *vlantab; /* The VLAN table */
	pfe_l2br_t *l2_bridge;	   /* The L2 bridge */
#endif				   /* PFE_CFG_L2BRIDGE_ENABLE */
	pfe_class_t *classifier;   /* The classifier block */
	pfe_tmu_t *tmu;		   /* The TMU block */
	pfe_util_t *util;	   /* The UTIL block */
	pfe_bmu_t **bmu;	   /* The BMU blocks */
	pfe_gpi_t **gpi;	   /* The GPI blocks */
	pfe_gpi_t **etgpi;	   /* The ETGPI blocks */
	pfe_gpi_t **hgpi;	   /* The HGPI blocks */
	pfe_hif_t *hif;		   /* The HIF block */
	pfe_emac_t **emac;	   /* The EMAC blocks */
#if !defined(TARGET_OS_UBOOT)
	pfe_safety_t *safety; /* The SAFETY block */
	pfe_wdt_t *wdt;	      /* The WDT block */
#endif
	pfe_if_db_t *phy_if_db; /* The PFE physical interfaces */
	pfe_if_db_t *log_if_db; /* The PFE logical interfaces */
	bool_t fci_created;
} pfe_platform_t;

pfe_fw_t *pfe_fw_load(char_t *class_fw_name, char_t *util_fw_name);
errno_t pfe_platform_init(pfe_platform_config_t *config);
errno_t pfe_platform_soft_reset(pfe_platform_t *platform);
errno_t pfe_platform_remove(void);
void pfe_platform_print_versions(pfe_platform_t *platform);
pfe_platform_t *pfe_platform_get_instance(void);
errno_t pfe_platform_register_log_if(pfe_platform_t *platform,
				     pfe_log_if_t *log_if);
errno_t pfe_platform_unregister_log_if(pfe_platform_t *platform,
				       pfe_log_if_t *log_if);
pfe_log_if_t *pfe_platform_get_log_if_by_id(pfe_platform_t *platform,
					    uint8_t id);
pfe_log_if_t *pfe_platform_get_log_if_by_name(pfe_platform_t *platform,
					      char_t *name);
pfe_phy_if_t *pfe_platform_get_phy_if_by_id(pfe_platform_t *platform,
					    pfe_ct_phy_if_id_t id);

#endif /* SRC_PFE_PLATFORM_H_ */

/** @}*/
