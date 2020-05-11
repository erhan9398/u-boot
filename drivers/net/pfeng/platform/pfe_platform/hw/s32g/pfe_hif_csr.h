/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup  dxgr_PFE_HIF
 * @{
 *
 * @file		pfe_hif_csr.h
 * @brief		The HIF module registers definition file (S32G).
 * @details		This is the HW-specific part of the HIF module. Applicable for
 *				IP versions listed below.
 *
 */

#ifndef PFE_HIF_CSR_H_
#define PFE_HIF_CSR_H_

#include "pfe_hif.h"

#ifndef PFE_CBUS_H_
#error Missing cbus.h
#endif /* PFE_CBUS_H_ */

/*	Supported IPs. Defines are validated within pfe_cbus.h. */
#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4) && \
	(PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_NPU_7_14)
#error Unsupported IP version
#endif /* PFE_CFG_IP_VERSION */

#define HIF_VERSION		    (0x00U)
#define HIF_TX_POLL_CTRL	    (0x04U)
#define HIF_RX_POLL_CTRL	    (0x08U)
#define HIF_MISC		    (0x0cU)
#define HIF_TIMEOUT_REG		    (0x10U)
#define HIF_SOFT_RESET		    (0x14U)
#define HIF_INT_SRC		    (0x40U)
#define HIF_ERR_INT_SRC		    (0x68U)
#define HIF_ERR_INT_EN		    (0x6cU)
#define HIF_TX_FIFO_ERR_INT_SRC	    (0x70U)
#define HIF_TX_FIFO_ERR_INT_EN	    (0x74U)
#define HIF_RX_FIFO_ERR_INT_SRC	    (0x78U)
#define HIF_RX_FIFO_ERR_INT_EN	    (0x7cU)
#define HIF_TX_STATE		    (0x80U)
#define HIF_TX_ACTV		    (0x84U)
#define HIF_TX_CURR_CH_NO	    (0x88U)
#define HIF_DXR_TX_FIFO_CNT	    (0x8cU)
#define HIF_TX_CTRL_WORD_FIFO_CNT1  (0x90U)
#define HIF_TX_CTRL_WORD_FIFO_CNT2  (0x94U)
#define HIF_TX_BVALID_FIFO_CNT	    (0x98U)
#define HIF_TX_PKT_CNT1		    (0x9cU) /* TODO: STAT */
#define HIF_TX_PKT_CNT2		    (0xa0U) /* TODO: STAT */
#define HIF_RX_STATE		    (0xa4U)
#define HIF_RX_ACTV		    (0xa8U)
#define HIF_RX_CURR_CH_NO	    (0xacU)
#define HIF_DXR_RX_FIFO_CNT	    (0xb0U)
#define HIF_RX_CTRL_WORD_FIFO_CNT   (0xb4U)
#define HIF_RX_BVALID_FIFO_CNT	    (0xb8U)
#define HIF_RX_PKT_CNT1		    (0xbcU) /* TODO: STAT */
#define HIF_RX_PKT_CNT2		    (0xc0U) /* TODO: STAT */
#define HIF_DMA_BASE_ADDR	    (0xc4U)
#define HIF_DMA_BURST_SIZE_ADDR	    (0xc8U)
#define HIF_RX_QUEUE_MAP_CH_NO_ADDR (0xccU)
#define HIF_LTC_PKT_CTRL_ADDR	    (0xd0U)

#define HIF_CTRL_CH0			(0x100U)
#define HIF_RX_BDP_WR_LOW_ADDR_CH0	(0x104U)
#define HIF_RX_BDP_WR_HIGH_ADDR_CH0	(0x108U)
#define HIF_RX_BDP_RD_LOW_ADDR_CH0	(0x10cU)
#define HIF_RX_BDP_RD_HIGH_ADDR_CH0	(0x110U)
#define HIF_TX_BDP_WR_LOW_ADDR_CH0	(0x114U)
#define HIF_TX_BDP_WR_HIGH_ADDR_CH0	(0x118U)
#define HIF_TX_BDP_RD_LOW_ADDR_CH0	(0x11cU)
#define HIF_TX_BDP_RD_HIGH_ADDR_CH0	(0x120U)
#define HIF_RX_WRBK_BD_CH0_BUFFER_SIZE	(0x124U)
#define HIF_RX_CH0_START		(0x128U)
#define HIF_TX_WRBK_BD_CH0_BUFFER_SIZE	(0x12cU)
#define HIF_TX_CH0_START		(0x130U)
#define HIF_CH0_INT_SRC			(0x160U)
#define HIF_CH0_INT_EN			(0x164U)
#define HIF_TX_RD_CURR_BD_LOW_ADDR_CH0	(0x180U)
#define HIF_TX_RD_CURR_BD_HIGH_ADDR_CH0 (0x184U)
#define HIF_TX_WR_CURR_BD_LOW_ADDR_CH0	(0x188U)
#define HIF_TX_WR_CURR_BD_HIGH_ADDR_CH0 (0x18cU)
#define HIF_BDP_CH0_TX_FIFO_CNT		(0x190U)
#define HIF_TX_DMA_STATUS_0_CH0		(0x194U)
#define HIF_TX_STATUS_0_CH0		(0x198U)
#define HIF_TX_STATUS_1_CH0		(0x19cU)
#define HIF_TX_PKT_CNT0_CH0		(0x1a0U) /* TODO: STAT */
#define HIF_TX_PKT_CNT1_CH0		(0x1a4U) /* TODO: STAT */
#define HIF_TX_PKT_CNT2_CH0		(0x1a8U) /* TODO: STAT */
#define HIF_RX_RD_CURR_BD_LOW_ADDR_CH0	(0x1c0U)
#define HIF_RX_RD_CURR_BD_HIGH_ADDR_CH0 (0x1c4U)
#define HIF_RX_WR_CURR_BD_LOW_ADDR_CH0	(0x1c8U)
#define HIF_RX_WR_CURR_BD_HIGH_ADDR_CH0 (0x1ccU)
#define HIF_BDP_CH0_RX_FIFO_CNT		(0x1d0U) /* TODO: STAT */
#define HIF_RX_DMA_STATUS_0_CH0		(0x1d4U)
#define HIF_RX_STATUS_0_CH0		(0x1d8U)
#define HIF_RX_PKT_CNT0_CH0		(0x1dcU) /* TODO: STAT */
#define HIF_RX_PKT_CNT1_CH0		(0x1e0U) /* TODO: STAT */
#define HIF_LTC_MAX_PKT_CH0_ADDR	(0x1e4U)
#define HIF_ABS_INT_TIMER_CH0		(0x1e8U)
#define HIF_ABS_FRAME_COUNT_CH0		(0x1ecU)
#define HIF_INT_COAL_EN_CH0		(0x1f0U)

#define HIF_CTRL_CHn(n) ((((n) & 0x3U) * 0x100U) + HIF_CTRL_CH0)
#define HIF_RX_BDP_WR_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_BDP_WR_LOW_ADDR_CH0)
#define HIF_RX_BDP_WR_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_BDP_WR_HIGH_ADDR_CH0)
#define HIF_RX_BDP_RD_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_BDP_RD_LOW_ADDR_CH0)
#define HIF_RX_BDP_RD_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_BDP_RD_HIGH_ADDR_CH0)
#define HIF_TX_BDP_WR_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_BDP_WR_LOW_ADDR_CH0)
#define HIF_TX_BDP_WR_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_BDP_WR_HIGH_ADDR_CH0)
#define HIF_TX_BDP_RD_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_BDP_RD_LOW_ADDR_CH0)
#define HIF_TX_BDP_RD_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_BDP_RD_HIGH_ADDR_CH0)
#define HIF_RX_WRBK_BD_CHn_BUFFER_SIZE(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_WRBK_BD_CH0_BUFFER_SIZE)
#define HIF_RX_CHn_START(n) ((((n) & 0x3U) * 0x100U) + HIF_RX_CH0_START)
#define HIF_TX_WRBK_BD_CHn_BUFFER_SIZE(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_WRBK_BD_CH0_BUFFER_SIZE)
#define HIF_TX_CHn_START(n) ((((n) & 0x3U) * 0x100U) + HIF_TX_CH0_START)
#define HIF_CHn_INT_SRC(n)  ((((n) & 0x3U) * 0x100U) + HIF_CH0_INT_SRC)
#define HIF_CHn_INT_EN(n)   ((((n) & 0x3U) * 0x100U) + HIF_CH0_INT_EN)
#define HIF_TX_RD_CURR_BD_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_RD_CURR_BD_LOW_ADDR_CH0)
#define HIF_TX_RD_CURR_BD_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_RD_CURR_BD_HIGH_ADDR_CH0)
#define HIF_TX_WR_CURR_BD_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_WR_CURR_BD_LOW_ADDR_CH0)
#define HIF_TX_WR_CURR_BD_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_WR_CURR_BD_HIGH_ADDR_CH0)
#define HIF_BDP_CHn_TX_FIFO_CNT(n) \
	((((n) & 0x3U) * 0x100U) + HIF_BDP_CH0_TX_FIFO_CNT)
#define HIF_TX_DMA_STATUS_0_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_DMA_STATUS_0_CH0)
#define HIF_TX_STATUS_0_CHn(n) ((((n) & 0x3U) * 0x100U) + HIF_TX_STATUS_0_CH0)
#define HIF_TX_STATUS_1_CHn(n) ((((n) & 0x3U) * 0x100U) + HIF_TX_STATUS_1_CH0)
#define HIF_TX_PKT_CNT0_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_PKT_CNT0_CH0) /* TODO: STAT */
#define HIF_TX_PKT_CNT1_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_PKT_CNT1_CH0) /* TODO: STAT */
#define HIF_TX_PKT_CNT2_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_TX_PKT_CNT2_CH0) /* TODO: STAT */
#define HIF_RX_RD_CURR_BD_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_RD_CURR_BD_LOW_ADDR_CH0)
#define HIF_RX_RD_CURR_BD_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_RD_CURR_BD_HIGH_ADDR_CH0)
#define HIF_RX_WR_CURR_BD_LOW_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_WR_CURR_BD_LOW_ADDR_CH0)
#define HIF_RX_WR_CURR_BD_HIGH_ADDR_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_WR_CURR_BD_HIGH_ADDR_CH0)
#define HIF_BDP_CHn_RX_FIFO_CNT(n) \
	((((n) & 0x3U) * 0x100U) + HIF_BDP_CH0_RX_FIFO_CNT) /* TODO: STAT */
#define HIF_RX_DMA_STATUS_0_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_DMA_STATUS_0_CH0)
#define HIF_RX_STATUS_0_CHn(n) ((((n) & 0x3U) * 0x100U) + HIF_RX_STATUS_0_CH0)
#define HIF_RX_PKT_CNT0_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_PKT_CNT0_CH0) /* TODO: STAT */
#define HIF_RX_PKT_CNT1_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_RX_PKT_CNT1_CH0) /* TODO: STAT */
#define HIF_LTC_MAX_PKT_CHn_ADDR(n) \
	((((n) & 0x3U) * 0x100U) + HIF_LTC_MAX_PKT_CH0_ADDR)
#define HIF_ABS_INT_TIMER_CHn(n) ((((n) & 0x3U) * 0x100U) + HIF_ABS_INT_TIMER_CH0)
#define HIF_ABS_FRAME_COUNT_CHn(n) \
	((((n) & 0x3U) * 0x100U) + HIF_ABS_FRAME_COUNT_CH0)
#define HIF_INT_COAL_EN_CHn(n) ((((n) & 0x3U) * 0x100U) + HIF_INT_COAL_EN_CH0)

/*	HIF_MISC bitfields */
#define SEQ_NUM_CHECK_EN     BIT(0)
#define BDPRD_AXI_WRITE_DONE BIT(1)
#define DBPWR_AXI_WRITE_DONE BIT(2)
#define RXDXR_AXI_WRITE_DONE BIT(3)
#define TXDXR_AXI_WRITE_DONE BIT(4)
#define HIF_TIMEOUT_EN	     BIT(5)
#define BD_START_SEQ_NUM(x)  (((x) & 0xffffU) << 16)

/*	HIF_CTRL_CHn bitfields */
#define TX_DMA_ENABLE	    BIT(0)
#define RX_DMA_ENABLE	    BIT(16)
#define TX_BDP_POLL_CNTR_EN BIT(1)
#define RX_BDP_POLL_CNTR_EN BIT(17)

/*	HIF_RX_CHn_START bitfields */
#define RX_BDP_CH_START BIT(0)

/*	HIF_TX_CHn_START bitfields */
#define TX_BDP_CH_START BIT(0)

/*	HIF_CHn_INT_EN bitfields */
#define HIF_CH_INT_EN			BIT(0)
#define BDP_CSR_RX_CBD_CH_INT_EN	BIT(1)
#define BDP_CSR_RX_PKT_CH_INT_EN	BIT(2)
#define BDP_CSR_TX_CBD_CH_INT_EN	BIT(3)
#define BDP_CSR_TX_PKT_CH_INT_EN	BIT(4)
#define BDP_RD_CSR_RX_TIMEOUT_CH_INT_EN BIT(5)
#define BDP_WR_CSR_RX_TIMEOUT_CH_INT_EN BIT(6)
#define BDP_RD_CSR_TX_TIMEOUT_CH_INT_EN BIT(7)
#define BDP_WD_CSR_TX_TIMEOUT_CH_INT_EN BIT(8)
#define DXR_CSR_RX_TIMEOUT_CH_INT_EN	BIT(9)
#define DXR_CSR_TX_TIMEOUT_CH_INT_EN	BIT(10)

/*	HIF_CHn_INT_SRC bitfields */
#define HIF_CH_INT		     BIT(0)
#define BDP_CSR_RX_CBD_CH_INT	     BIT(1)
#define BDP_CSR_RX_PKT_CH_INT	     BIT(2)
#define BDP_CSR_TX_CBD_CH_INT	     BIT(3)
#define BDP_CSR_TX_PKT_CH_INT	     BIT(4)
#define BDP_RD_CSR_RX_TIMEOUT_CH_INT BIT(5)
#define BDP_WR_CSR_RX_TIMEOUT_CH_INT BIT(6)
#define BDP_RD_CSR_TX_TIMEOUT_CH_INT BIT(7)
#define BDP_WR_CSR_TX_TIMEOUT_CH_INT BIT(8)
#define DXR_CSR_RX_TIMEOUT_CH_INT    BIT(9)
#define DXR_CSR_TX_TIMEOUT_CH_INT    BIT(10)

/*HIF_INT_SRC/ HIF_INT_ENABLE control bits */
#define HIF_INT	      BIT(0)
#define HIF_RXBD_INT  BIT(1)
#define HIF_RXPKT_INT BIT(2)
#define HIF_TXBD_INT  BIT(3)
#define HIF_TXPKT_INT BIT(4)

/*HIF_TX_CTRL bits */
#define HIF_CTRL_DMA_EN		   BIT(0)
#define HIF_CTRL_BDP_POLL_CTRL_EN  BIT(1)
#define HIF_CTRL_BDP_CH_START_WSTB BIT(2)

/*HIF_INT_ENABLE bits */
#define HIF_INT_EN	 BIT(0)
#define HIF_RXBD_INT_EN	 BIT(1)
#define HIF_RXPKT_INT_EN BIT(2)
#define HIF_TXBD_INT_EN	 BIT(3)
#define HIF_TXPKT_INT_EN BIT(4)

/*HIF_POLL_CTRL bits*/
#define HIF_RX_POLL_CTRL_CYCLE 0x0400U
#define HIF_TX_POLL_CTRL_CYCLE 0x0400U

/*HIF_INT_COAL bits*/
#define HIF_INT_COAL_ENABLE BIT(31)

#define BDP_CSR_RX_DMA_ACTV BIT(16)

/*	HIF_INT_SRC */
#define HIF_INT_SRC_HIF_CH0_INT		BIT(0)
#define HIF_INT_SRC_HIF_CH1_INT		BIT(1)
#define HIF_INT_SRC_HIF_CH2_INT		BIT(2)
#define HIF_INT_SRC_HIF_CH3_INT		BIT(3)
#define HIF_INT_SRC_HIF_ERR_INT		BIT(16)
#define HIF_INT_SRC_HIF_TX_FIFO_ERR_INT BIT(17)
#define HIF_INT_SRC_HIF_RX_FIFO_ERR_INT BIT(18)

/*	HIF_ERR_INT_SRC, HIF_ERR_INT_EN bits */
#define HIF_ERR_INT		   BIT(0)
#define DXR_CSR_TX_PKT_LEN_ERR_INT BIT(1)
#define DXR_CSR_TX_SOF_ERR_INT	   BIT(2)
#define DXR_CSR_TX_DATA_ERR_INT	   BIT(3)
#define DXR_CSR_TX_EOF_ERR_INT	   BIT(4)
#define DXR_CSR_RX_PKT_LEN_ERR_INT BIT(5)
#define DXR_CSR_RX_SOF_ERR_INT	   BIT(6)
#define DXR_CSR_RX_DATA_ERR_INT	   BIT(7)
#define DXR_CSR_RX_EOF_ERR_INT	   BIT(8)
#define BDP_CSR_TX_RD_AXI_ERR_INT  BIT(9)
#define BDP_CSR_TX_WR_AXI_ERR_INT  BIT(10)
#define BDP_CSR_RX_RD_AXI_ERR_INT  BIT(11)
#define BDP_CSR_RX_WR_AXI_ERR_INT  BIT(12)
#define BDP_CSR_TX_AXI_ERR_INT	   BIT(13)
#define BDP_CSR_RX_AXI_ERR_INT	   BIT(14)

/*	HIF_TX_FIFO_ERR_INT_SRC, HIF_TX_FIFO_ERR_INT_EN bits */
#define HIF_TX_FIFO_ERR_INT			    BIT(0)
#define BDP_CSR_TX_RD_FIFO_OVERRUN_INT		    BIT(1)
#define BDP_CSR_TX_WR_FIFO_OVERRUN_INT		    BIT(2)
#define DXR_CSR_TX_FIFO_OVERRUN_INT		    BIT(3)
#define DXR_CSR_TX_LBUF_OVERRUN_INT		    BIT(4)
#define DXR_CSR_TX_SOF_CTRL_WORD_FIFO_OVERRUN_INT   BIT(5)
#define BDP_DXR_CSR_TX_BD_CTRL_FIFO_OVERRUN_INT	    BIT(6)
#define DXR_CSR_TX_SAD_FIFO_OVERRUN_INT		    BIT(7)
#define BDP_CSR_TX_BVALID_FIFO_OVERRUN_INT	    BIT(8)
#define HIF_AXI_BDP_CSR_TX_BVALID_FIFO_OVERRUN_INT  BIT(9)
#define BDP_CSR_TX_RD_FIFO_UNDERRUN_INT		    BIT(10)
#define BDP_CSR_TX_WR_FIFO_UNDERRUN_INT		    BIT(11)
#define DXR_CSR_TX_FIFO_UNDERRUN_INT		    BIT(12)
#define DXR_CSR_TX_LBUF_UNDERRUN_INT		    BIT(13)
#define DXR_CSR_TX_SOF_CTRL_WORD_FIFO_UNDERRUN_INT  BIT(14)
#define BDP_DXR_CSR_TX_BD_CTRL_FIFO_UNDERRUN_INT    BIT(15)
#define DXR_CSR_TX_SAD_FIFO_UNDERRUN_INT	    BIT(16)
#define BDP_CSR_TX_BVALID_FIFO_UNDERRUN_INT	    BIT(17)
#define HIF_AXI_BDP_CSR_TX_BVALID_FIFO_UNDERRUN_INT BIT(18)

/*	HIF_RX_FIFO_ERR_INT_SRC, HIF_RX_FIFO_ERR_INT_EN bits */
#define HIF_RX_FIFO_ERR_INT			    BIT(0)
#define BDP_CSR_RX_RD_FIFO_OVERRUN_INT		    BIT(1)
#define BDP_CSR_RX_WR_FIFO_OVERRUN_INT		    BIT(2)
#define DXR_CSR_RX_FIFO_OVERRUN_INT		    BIT(3)
#define DXR_CSR_RX_LBUF_OVERRUN_INT		    BIT(4)
#define DXR_CSR_RX_SOF_CTRL_WORD_FIFO_OVERRUN_INT   BIT(5)
#define DXR_CSR_RX_EOF_CTRL_WORD_FIFO_OVERRUN_INT   BIT(6)
#define BDP_CSR_RX_BVALID_FIFO_OVERRUN_INT	    BIT(7)
#define HIF_AXI_BDP_CSR_RX_BVALID_FIFO_OVERRUN_INT  BIT(8)
#define DXR_CSR_RX_BVALID_FIFO_OVERRUN_INT	    BIT(9)
#define HIF_AXI_DXR_CSR_RX_BVALID_FIFO_OVERRUN_INT  BIT(10)
#define BDP_CSR_RX_RD_FIFO_UNDERRUN_INT		    BIT(11)
#define BDP_CSR_RX_WR_FIFO_UNDERRUN_INT		    BIT(12)
#define DXR_CSR_RX_FIFO_UNDERRUN_INT		    BIT(13)
#define DXR_CSR_RX_LBUF_UNDERRUN_INT		    BIT(14)
#define DXR_CSR_RX_SOF_CTRL_WORD_FIFO_UNDERRUN_INT  BIT(15)
#define DXR_CSR_RX_EOF_CTRL_WORD_FIFO_UNDERRUN_INT  BIT(16)
#define BDP_CSR_RX_BVALID_FIFO_UNDERRUN_INT	    BIT(17)
#define HIF_AXI_BDP_CSR_RX_BVALID_FIFO_INDERRUN_INT BIT(18)
#define DXR_CSR_RX_BVALID_FIFO_UNDERRUN_INT	    BIT(19)
#define HIF_AXI_DXR_CSR_RX_BVALID_FIFO_UNDERRUN_INT BIT(20)

errno_t pfe_hif_cfg_isr(void *base_va);
void pfe_hif_cfg_irq_mask(void *base_va);
void pfe_hif_cfg_irq_unmask(void *base_va);
errno_t pfe_hif_chnl_cfg_isr(void *base_va, uint32_t channel_id,
			     pfe_hif_chnl_event_t *events);
errno_t pfe_hif_chnl_cfg_init(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_fini(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_irq_mask(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_rx_irq_mask(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_tx_irq_mask(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_irq_unmask(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_rx_irq_unmask(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_tx_irq_unmask(void *base_va, uint32_t channel_id);
errno_t pfe_hif_cfg_init(void *base_va);
void pfe_hif_cfg_fini(void *base_va);
uint32_t pfe_hif_cfg_get_tx_fifo_fill_level(void *base_va);
void pfe_hif_chnl_cfg_tx_enable(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_tx_disable(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_rx_enable(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_rx_disable(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_rx_dma_start(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_tx_dma_start(void *base_va, uint32_t channel_id);
void pfe_hif_chnl_cfg_set_rx_bd_ring_addr(void *base_va, uint32_t channel_id,
					  void *rx_ring_pa);
void pfe_hif_chnl_cfg_set_tx_bd_ring_addr(void *base_va, uint32_t channel_id,
					  void *tx_ring_pa);
void pfe_hif_chnl_cfg_set_rx_wb_table(void *base_va, uint32_t channel_id,
				      void *wb_tbl_pa, uint32_t tbl_len);
void pfe_hif_chnl_cfg_set_tx_wb_table(void *base_va, uint32_t channel_id,
				      void *wb_tbl_pa, uint32_t tbl_len);
bool_t pfe_hif_chnl_cfg_is_rx_dma_active(void *base_va, uint32_t channel_id);
bool_t pfe_hif_chnl_cfg_is_tx_dma_active(void *base_va, uint32_t channel_id);
uint16_t pfe_hif_chnl_cfg_get_rx_seqnum(void *base_va, uint32_t channel_id);
uint16_t pfe_hif_chnl_cfg_get_tx_seqnum(void *base_va, uint32_t channel_id);
bool_t pfe_hif_chnl_cfg_is_rx_bdp_fifo_empty(void *base_va,
					     uint32_t channel_id);
bool_t pfe_hif_chnl_cfg_is_tx_bdp_fifo_empty(void *base_va,
					     uint32_t channel_id);
uint32_t pfe_hif_chnl_cfg_get_text_stat(void *base_va, uint32_t channel_id,
					char_t *buf, uint32_t size,
					uint8_t verb_level);
uint32_t pfe_hif_cfg_get_text_stat(void *base_va, char_t *buf, uint32_t size,
				   uint8_t verb_level);

#endif /* PFE_HIF_CSR_H_ */

/** @}*/
