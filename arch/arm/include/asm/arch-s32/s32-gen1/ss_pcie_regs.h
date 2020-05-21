// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 */

#ifndef S32GEN1_SS_PCIE_REGS_H
#define S32GEN1_SS_PCIE_REGS_H

#define PCIE_CTRL_BASEADDRESS(dbi_base) ((uint64_t)(dbi_base))

/* Status and Command Register. */

#define PCIE_CTRL_TYPE1_STATUS_COMMAND_REG(dbi_base) \
		(PCIE_CTRL_BASEADDRESS(dbi_base) + 0x4)

/* Class Code and Revision ID Register. */

#define PCIE_CTRL_TYPE1_CLASS_CODE_REV_ID_REG(dbi_base) \
		(PCIE_CTRL_BASEADDRESS(dbi_base) + 0x8)

/* Field definitions for TYPE1_STATUS_COMMAND_REG */

#define PCIE_IO_EN_VALUE(x)        (((x) & 0x00000001) << 0)
#define PCIE_IO_EN_BIT             (0)
#define PCIE_IO_EN                 ((1) << (PCIE_IO_EN_BIT))

#define PCIE_MSE_VALUE(x)          (((x) & 0x00000001) << 1)
#define PCIE_MSE_BIT               (1)
#define PCIE_MSE                   ((1) << (PCIE_MSE_BIT))

#define PCIE_BME_VALUE(x)          (((x) & 0x00000001) << 2)
#define PCIE_BME_BIT               (2)
#define PCIE_BME                   ((1) << (PCIE_BME_BIT))

/* Instance PCIE_SPCIE_CAP_HEADER */

#define PCIE_SPCIE_CAP_HEADER_BASEADDRESS(dbi_base) \
	(PCIE_CTRL_BASEADDRESS(dbi_base) + 0x148)

/* Lane Equalization Control Register for lanes 1 and 0. */

#define PCIE_SPCIE_CAP_SPCIE_CAP_0CH_REG(dbi_base) \
	(PCIE_SPCIE_CAP_HEADER_BASEADDRESS(dbi_base) + 0xC)

/* Instance PCIE_CAP */

#define PCIE_CAP_BASEADDRESS(dbi_base) ((uint64_t)(dbi_base) + 0x0070)

/* Device Control and Status Register. */

#define PCIE_CAP_DEVICE_CONTROL_DEVICE_STATUS(dbi_base) \
	(PCIE_CAP_BASEADDRESS(dbi_base) + 0x8)

/* Link Capabilities Register */
#define PCIE_CAP_LINK_CAP(dbi_base)	(PCIE_CAP_BASEADDRESS(dbi_base) + 0xC)

/* Link Control 2 and Status 2 Register. */

#define PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG(dbi_base) \
	(PCIE_CAP_BASEADDRESS(dbi_base) + 0x30)

/* Field definitions for DEVICE_CONTROL_DEVICE_STATUS */

#define PCIE_CAP_MAX_PAYLOAD_SIZE_CS_VALUE(x) (((x) & 0x00000007) << 5)
#define PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MSB (7)
#define PCIE_CAP_MAX_PAYLOAD_SIZE_CS_LSB (5)
#define PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MASK (0x00000007)
#define PCIE_CAP_MAX_PAYLOAD_SIZE_CS ((PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MASK) << \
		(PCIE_CAP_MAX_PAYLOAD_SIZE_CS_LSB))

#define PCIE_CAP_MAX_READ_REQ_SIZE_VALUE(x) (((x) & 0x00000007) << 12)
#define PCIE_CAP_MAX_READ_REQ_SIZE_MSB (14)
#define PCIE_CAP_MAX_READ_REQ_SIZE_LSB (12)
#define PCIE_CAP_MAX_READ_REQ_SIZE_MASK (0x00000007)
#define PCIE_CAP_MAX_READ_REQ_SIZE ((PCIE_CAP_MAX_READ_REQ_SIZE_MASK) << \
		(PCIE_CAP_MAX_READ_REQ_SIZE_LSB))

/* Field definitions for PCIE_CAP_LINK_CAP */

#define PCIE_MAX_LINK_SPEED_MASK	0xf
#define PCIE_MAX_LINK_SPEED_OFF		0
#define PCIE_MAX_LINK_WIDTH_MASK	0x3f0
#define PCIE_MAX_LINK_WIDTH_OFF		4

/* Field definitions for LINK_CONTROL2_LINK_STATUS2_REG */

#define PCIE_CAP_TARGET_LINK_SPEED_VALUE(x) (((x) & 0x0000000F) << 0)
#define PCIE_CAP_TARGET_LINK_SPEED_MSB (3)
#define PCIE_CAP_TARGET_LINK_SPEED_LSB (0)
#define PCIE_CAP_TARGET_LINK_SPEED_MASK (0x0000000F)
#define PCIE_CAP_TARGET_LINK_SPEED ((PCIE_CAP_TARGET_LINK_SPEED_MASK) << \
		(PCIE_CAP_TARGET_LINK_SPEED_LSB))

/* Symbol Timer Register and Filter Mask 1 Register (SYMBOL_TIMER_FILTER_1) */

#define PCIE_SYMBOL_TIMER_FILTER_1(dbi_base) \
	(PCIE_CTRL_BASEADDRESS(dbi_base) + 0x71c)

/* Instance PCIE_IATU */

#define PCIE_DBI_IATU_OFFSET	0x60000
#define PCIE_IATU_BASEADDRESS(dbi_base) \
	((uint64_t)(dbi_base) + PCIE_DBI_IATU_OFFSET)

/* iATU Region Control 1 Register. */

#define PCIE_IATU_REGION_CTRL_1_OUTBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x0)

/* iATU Region Control 2 Register. */

#define PCIE_IATU_REGION_CTRL_2_OUTBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x4)

/* iATU Lower Base Address Register. */

#define PCIE_IATU_LWR_BASE_ADDR_OUTBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x8)

/* iATU Upper Base Address Register. */

#define PCIE_IATU_UPPER_BASE_ADDR_OUTBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0xC)

/* iATU Limit Address Register. */

#define PCIE_IATU_LIMIT_ADDR_OUTBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x10)

/* iATU Lower Target Address Register. */

#define PCIE_IATU_LWR_TARGET_ADDR_OUTBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x14)

/* iATU Upper Target Address Register. */

#define PCIE_IATU_UPPER_TARGET_ADDR_OUTBOUND_0(dbi_base) \
		(PCIE_IATU_BASEADDRESS(dbi_base) + 0x18)

/* iATU Region Control 1 Register. */

#define PCIE_IATU_REGION_CTRL_1_INBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x100)

/* iATU Region Control 2 Register. */

#define PCIE_IATU_REGION_CTRL_2_INBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x104)

/* iATU Lower Base Address Register. */

#define PCIE_IATU_LWR_BASE_ADDR_INBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x108)

/* iATU Upper Base Address Register. */

#define PCIE_IATU_UPPER_BASE_ADDR_INBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x10C)

/* iATU Limit Address Register. */

#define PCIE_IATU_LIMIT_ADDR_INBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x110)

/* iATU Lower Target Address Register. */

#define PCIE_IATU_LWR_TARGET_ADDR_INBOUND_0(dbi_base) \
	(PCIE_IATU_BASEADDRESS(dbi_base) + 0x114)

/* iATU Upper Target Address Register. */

#define PCIE_IATU_UPPER_TARGET_ADDR_INBOUND_0(dbi_base) \
		(PCIE_IATU_BASEADDRESS(dbi_base) + 0x118)


/* Field definitions for IATU_REGION_CTRL_2_OUTBOUND_0 */

#define PCIE_CFG_SHIFT_MODE_VALUE(x) (((x) & 0x00000001) << 28)
#define PCIE_CFG_SHIFT_MODE_BIT    (28)
#define PCIE_CFG_SHIFT_MODE        ((1) << (PCIE_CFG_SHIFT_MODE_BIT))

#define PCIE_REGION_EN_VALUE(x)    (((x) & 0x00000001) << 31)
#define PCIE_REGION_EN_BIT         (31)
#define PCIE_REGION_EN             ((1) << (PCIE_REGION_EN_BIT))

/* Field definitions for IATU_REGION_CTRL_2_INBOUND_0 */

#define PCIE_BAR_NUM_VALUE(x)      (((x) & 0x00000007) << 8)
#define PCIE_BAR_NUM_MSB           (10)
#define PCIE_BAR_NUM_LSB           (8)
#define PCIE_BAR_NUM_MASK          (0x00000007)
#define PCIE_BAR_NUM               ((PCIE_BAR_NUM_MASK) << (PCIE_BAR_NUM_LSB))

#define PCIE_MATCH_MODE_VALUE(x)   (((x) & 0x00000001) << 30)
#define PCIE_MATCH_MODE_BIT        (30)
#define PCIE_MATCH_MODE            ((1) << (PCIE_MATCH_MODE_BIT))


/* Instance PCIE_PORT_LOGIC */

#define PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) \
	(PCIE_CTRL_BASEADDRESS(dbi_base) + 0x0700)

/* Port Force Link Register. */

#define PCIE_PORT_LOGIC_PORT_FORCE(dbi_base) \
	(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x8)

/* Port Link Control Register. */

#define PCIE_PORT_LOGIC_PORT_LINK_CTRL(dbi_base) \
	(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x10)

/* Timer Control and Max Function Number Register. */

#define PCIE_PORT_LOGIC_TIMER_CTRL_MAX_FUNC_NUM(dbi_base) \
		(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x18)

/* Link Width and Speed Change Control Register. */

#define PCIE_PORT_LOGIC_GEN2_CTRL(dbi_base) \
		(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x10C)

/* Gen3 Control Register. */

#define PCIE_PORT_LOGIC_GEN3_RELATED(dbi_base) \
	(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x190)

/* Gen3 EQ Control Register. */

#define PCIE_PORT_LOGIC_GEN3_EQ_CONTROL(dbi_base) \
		(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x1A8)

/* DBI Read-Only Write Enable Register. */

#define PCIE_PORT_LOGIC_MISC_CONTROL_1(dbi_base) \
		(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x1BC)

/* ACE Cache Coherency Control Register 3 */

#define PCIE_PORT_LOGIC_COHERENCY_CONTROL_3(dbi_base) \
		(PCIE_PORT_LOGIC_BASEADDRESS(dbi_base) + 0x1E8)

/* Field definitions for PORT_FORCE */

#define PCIE_LINK_NUM_VALUE(x)     (((x) & 0x000000FF) << 0)
#define PCIE_LINK_NUM_MSB          (7)
#define PCIE_LINK_NUM_LSB          (0)
#define PCIE_LINK_NUM_MASK         (0x000000FF)
#define PCIE_LINK_NUM              ((PCIE_LINK_NUM_MASK) << \
		(PCIE_LINK_NUM_LSB))

#define PCIE_FORCED_LTSSM_VALUE(x) (((x) & 0x0000000F) << 8)
#define PCIE_FORCED_LTSSM_MSB      (11)
#define PCIE_FORCED_LTSSM_LSB      (8)
#define PCIE_FORCED_LTSSM_MASK     (0x0000000F)
#define PCIE_FORCED_LTSSM          ((PCIE_FORCED_LTSSM_MASK) << \
		(PCIE_FORCED_LTSSM_LSB))

#define PCIE_FORCE_EN_VALUE(x)     (((x) & 0x00000001) << 15)
#define PCIE_FORCE_EN_BIT          (15)
#define PCIE_FORCE_EN              ((1) << (PCIE_FORCE_EN_BIT))

#define PCIE_LINK_STATE_VALUE(x)   (((x) & 0x0000003F) << 16)
#define PCIE_LINK_STATE_MSB        (21)
#define PCIE_LINK_STATE_LSB        (16)
#define PCIE_LINK_STATE_MASK       (0x0000003F)
#define PCIE_LINK_STATE            ((PCIE_LINK_STATE_MASK) << \
		(PCIE_LINK_STATE_LSB))

/* Field definitions for PORT_LINK_CTRL */

#define PCIE_RESET_ASSERT_VALUE(x) (((x) & 0x00000001) << 3)
#define PCIE_RESET_ASSERT_BIT      (3)
#define PCIE_RESET_ASSERT          ((1) << (PCIE_RESET_ASSERT_BIT))

#define PCIE_DLL_LINK_EN_VALUE(x)  (((x) & 0x00000001) << 5)
#define PCIE_DLL_LINK_EN_BIT       (5)
#define PCIE_DLL_LINK_EN           ((1) << (PCIE_DLL_LINK_EN_BIT))

#define PCIE_LINK_DISABLE_VALUE(x) (((x) & 0x00000001) << 6)
#define PCIE_LINK_DISABLE_BIT      (6)
#define PCIE_LINK_DISABLE          ((1) << (PCIE_LINK_DISABLE_BIT))

#define PCIE_FAST_LINK_MODE_VALUE(x) (((x) & 0x00000001) << 7)
#define PCIE_FAST_LINK_MODE_BIT    (7)
#define PCIE_FAST_LINK_MODE        ((1) << (PCIE_FAST_LINK_MODE_BIT))

#define PCIE_LINK_RATE_VALUE(x)    (((x) & 0x0000000F) << 8)
#define PCIE_LINK_RATE_MSB         (11)
#define PCIE_LINK_RATE_LSB         (8)
#define PCIE_LINK_RATE_MASK        (0x0000000F)
#define PCIE_LINK_RATE             ((PCIE_LINK_RATE_MASK) << \
		(PCIE_LINK_RATE_LSB))

#define PCIE_LINK_CAPABLE_VALUE(x) (((x) & 0x0000003F) << 16)
#define PCIE_LINK_CAPABLE_MSB      (21)
#define PCIE_LINK_CAPABLE_LSB      (16)
#define PCIE_LINK_CAPABLE_MASK     (0x0000003F)
#define PCIE_LINK_CAPABLE          ((PCIE_LINK_CAPABLE_MASK) << \
		(PCIE_LINK_CAPABLE_LSB))

/* Field definitions for TIMER_CTRL_MAX_FUNC_NUM */

#define PCIE_MAX_FUNC_NUM_VALUE(x) (((x) & 0x000000FF) << 0)
#define PCIE_MAX_FUNC_NUM_MSB      (7)
#define PCIE_MAX_FUNC_NUM_LSB      (0)
#define PCIE_MAX_FUNC_NUM_MASK     (0x000000FF)
#define PCIE_MAX_FUNC_NUM          ((PCIE_MAX_FUNC_NUM_MASK) << \
		(PCIE_MAX_FUNC_NUM_LSB))

#define PCIE_FAST_LINK_SCALING_FACTOR_VALUE(x) (((x) & 0x00000003) << 29)
#define PCIE_FAST_LINK_SCALING_FACTOR_MSB (30)
#define PCIE_FAST_LINK_SCALING_FACTOR_LSB (29)
#define PCIE_FAST_LINK_SCALING_FACTOR_MASK (0x00000003)
#define PCIE_FAST_LINK_SCALING_FACTOR (PCIE_FAST_LINK_SCALING_FACTOR_MASK << \
		(PCIE_FAST_LINK_SCALING_FACTOR_LSB))

/* Field definitions for GEN2_CTRL */

#define PCIE_NUM_OF_LANES_VALUE(x) (((x) & 0x0000001F) << 8)
#define PCIE_NUM_OF_LANES_MSB      (12)
#define PCIE_NUM_OF_LANES_LSB      (8)
#define PCIE_NUM_OF_LANES_MASK     (0x0000001F)
#define PCIE_NUM_OF_LANES          ((PCIE_NUM_OF_LANES_MASK) << \
		(PCIE_NUM_OF_LANES_LSB))

#define PCIE_DIRECT_SPEED_CHANGE_VALUE(x) (((x) & 0x00000001) << 17)
#define PCIE_DIRECT_SPEED_CHANGE_BIT  (17)
#define PCIE_DIRECT_SPEED_CHANGE      ((1) << (PCIE_DIRECT_SPEED_CHANGE_BIT))

/* Field definitions for GEN3_RELATED */

#define PCIE_EQ_PHASE_2_3_VALUE(x) (((x) & 0x00000001) << 9)
#define PCIE_EQ_PHASE_2_3_BIT      (9)
#define PCIE_EQ_PHASE_2_3          ((1) << (PCIE_EQ_PHASE_2_3_BIT))

#define PCIE_EQ_EIEOS_CNT_VALUE(x) (((x) & 0x00000001) << 10)
#define PCIE_EQ_EIEOS_CNT_BIT      (10)
#define PCIE_EQ_EIEOS_CNT          ((1) << (PCIE_DSP_EQ_EIEOS_CNT_BIT))

#define PCIE_EQ_REDO_VALUE(x)      (((x) & 0x00000001) << 11)
#define PCIE_EQ_REDO_BIT           (11)
#define PCIE_EQ_REDO               ((1) << (PCIE_DSP_EQ_REDO_BIT))

#define PCIE_RXEQ_PH01_EN_VALUE(x) (((x) & 0x00000001) << 12)
#define PCIE_RXEQ_PH01_EN_BIT      (12)
#define PCIE_RXEQ_PH01_EN          ((1) << (PCIE_RXEQ_PH01_EN_BIT))

/* Field definitions for GEN3_EQ_CONTROL */

#define PCIE_GEN3_EQ_FB_MODE_VALUE(x) (((x) & 0x0000000F) << 0)
#define PCIE_GEN3_EQ_FB_MODE_MSB   (3)
#define PCIE_GEN3_EQ_FB_MODE_LSB   (0)
#define PCIE_GEN3_EQ_FB_MODE_MASK  (0x0000000F)
#define PCIE_GEN3_EQ_FB_MODE       ((PCIE_GEN3_EQ_FB_MODE_MASK) << \
		(PCIE_GEN3_EQ_FB_MODE_LSB))

#define PCIE_GEN3_EQ_PSET_REQ_VEC_VALUE(x) (((x) & 0x0000FFFF) << 8)
#define PCIE_GEN3_EQ_PSET_REQ_VEC_MSB (23)
#define PCIE_GEN3_EQ_PSET_REQ_VEC_LSB (8)
#define PCIE_GEN3_EQ_PSET_REQ_VEC_MASK (0x0000FFFF)
#define PCIE_GEN3_EQ_PSET_REQ_VEC  ((PCIE_GEN3_EQ_PSET_REQ_VEC_MASK) << \
		(PCIE_GEN3_EQ_PSET_REQ_VEC_LSB))

/* Field definitions for MISC_CONTROL_1 */

#define PCIE_DBI_RO_WR_EN_VALUE(x) (((x) & 0x00000001) << 0)
#define PCIE_DBI_RO_WR_EN_BIT      (0)
#define PCIE_DBI_RO_WR_EN          ((1) << (PCIE_DBI_RO_WR_EN_BIT))

/* Field definitions for COHERENCY_CONTROL_3 */

#define PCIE_CFG_MSTR_ARDOMAIN_MODE_VALUE(x) (((x) & 0x00000003) << 0)
#define PCIE_CFG_MSTR_ARDOMAIN_MODE_MSB (1)
#define PCIE_CFG_MSTR_ARDOMAIN_MODE_LSB (0)
#define PCIE_CFG_MSTR_ARDOMAIN_MODE_MASK (0x00000003)
#define PCIE_CFG_MSTR_ARDOMAIN_MODE ((PCIE_CFG_MSTR_ARDOMAIN_MODE_MASK) << \
		(PCIE_CFG_MSTR_ARDOMAIN_MODE_LSB))

#define PCIE_CFG_MSTR_ARCACHE_MODE_VALUE(x) (((x) & 0x0000000F) << 3)
#define PCIE_CFG_MSTR_ARCACHE_MODE_MSB (6)
#define PCIE_CFG_MSTR_ARCACHE_MODE_LSB (3)
#define PCIE_CFG_MSTR_ARCACHE_MODE_MASK (0x0000000F)
#define PCIE_CFG_MSTR_ARCACHE_MODE ((PCIE_CFG_MSTR_ARCACHE_MODE_MASK) << \
		(PCIE_CFG_MSTR_ARCACHE_MODE_LSB))

#define PCIE_CFG_MSTR_AWDOMAIN_MODE_VALUE(x) (((x) & 0x00000003) << 8)
#define PCIE_CFG_MSTR_AWDOMAIN_MODE_MSB (9)
#define PCIE_CFG_MSTR_AWDOMAIN_MODE_LSB (8)
#define PCIE_CFG_MSTR_AWDOMAIN_MODE_MASK (0x00000003)
#define PCIE_CFG_MSTR_AWDOMAIN_MODE ((PCIE_CFG_MSTR_AWDOMAIN_MODE_MASK) << \
		(PCIE_CFG_MSTR_AWDOMAIN_MODE_LSB))


#define PCIE_CFG_MSTR_AWCACHE_MODE_VALUE(x) (((x) & 0x0000000F) << 11)
#define PCIE_CFG_MSTR_AWCACHE_MODE_MSB (14)
#define PCIE_CFG_MSTR_AWCACHE_MODE_LSB (11)
#define PCIE_CFG_MSTR_AWCACHE_MODE_MASK (0x0000000F)
#define PCIE_CFG_MSTR_AWCACHE_MODE ((PCIE_CFG_MSTR_AWCACHE_MODE_MASK) << \
		(PCIE_CFG_MSTR_AWCACHE_MODE_LSB))

#define PCIE_CFG_MSTR_ARDOMAIN_VALUE_VALUE(x) (((x) & 0x00000003) << 16)
#define PCIE_CFG_MSTR_ARDOMAIN_VALUE_MSB (17)
#define PCIE_CFG_MSTR_ARDOMAIN_VALUE_LSB (16)
#define PCIE_CFG_MSTR_ARDOMAIN_VALUE_MASK (0x00000003)
#define PCIE_CFG_MSTR_ARDOMAIN_VALUE ((PCIE_CFG_MSTR_ARDOMAIN_VALUE_MASK) << \
		(PCIE_CFG_MSTR_ARDOMAIN_VALUE_LSB))

#define PCIE_CFG_MSTR_ARCACHE_VALUE_VALUE(x) (((x) & 0x0000000F) << 19)
#define PCIE_CFG_MSTR_ARCACHE_VALUE_MSB (22)
#define PCIE_CFG_MSTR_ARCACHE_VALUE_LSB (19)
#define PCIE_CFG_MSTR_ARCACHE_VALUE_MASK (0x0000000F)
#define PCIE_CFG_MSTR_ARCACHE_VALUE ((PCIE_CFG_MSTR_ARCACHE_VALUE_MASK) << \
		(PCIE_CFG_MSTR_ARCACHE_VALUE_LSB))


#define PCIE_CFG_MSTR_AWDOMAIN_VALUE_VALUE(x) (((x) & 0x00000003) << 24)
#define PCIE_CFG_MSTR_AWDOMAIN_VALUE_MSB (25)
#define PCIE_CFG_MSTR_AWDOMAIN_VALUE_LSB (24)
#define PCIE_CFG_MSTR_AWDOMAIN_VALUE_MASK (0x00000003)
#define PCIE_CFG_MSTR_AWDOMAIN_VALUE ((PCIE_CFG_MSTR_AWDOMAIN_VALUE_MASK) << \
		(PCIE_CFG_MSTR_AWDOMAIN_VALUE_LSB))

#define PCIE_CFG_MSTR_AWCACHE_VALUE_VALUE(x) (((x) & 0x0000000F) << 27)
#define PCIE_CFG_MSTR_AWCACHE_VALUE_MSB (30)
#define PCIE_CFG_MSTR_AWCACHE_VALUE_LSB (27)
#define PCIE_CFG_MSTR_AWCACHE_VALUE_MASK (0x0000000F)
#define PCIE_CFG_MSTR_AWCACHE_VALUE ((PCIE_CFG_MSTR_AWCACHE_VALUE_MASK) << \
		(PCIE_CFG_MSTR_AWCACHE_VALUE_LSB))

#endif  /* S32GEN1_SS_PCIE_REGS_H */
