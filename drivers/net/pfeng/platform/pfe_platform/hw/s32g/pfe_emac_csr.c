// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup  dxgr_PFE_EMAC
 * @{
 *
 * @file		s32g/pfe_emac_csr.c
 * @brief		The EMAC module low-level API (s32g)(Synopsys EQoS).
 * @details
 *
 */

#include "oal.h"
#include "hal.h"
#include "pfe_cbus.h"
#include "pfe_emac_csr.h"

/* Mode conversion table */
static const char_t *phy_mode[] = {
	"GMII_MII", "RGMII", "SGMII",  "TBI",	  "RMII",
	"RTBI",	    "SMII",  "RevMII", "INVALID",
};

static inline uint32_t
crc32_reversed(const u8 *const data, const uint32_t len)
{
	const u32 poly = 0xEDB88320U;
	u32 res = 0xffffffffU;
	u32 ii, jj;

	for (ii = 0U; ii < len; ii++) {
		res ^= (uint32_t)data[ii];

		for (jj = 0; jj < 8; jj++) {
			if ((res & 0x1U) != 0U) {
				res = res >> 1;
				res = (uint32_t)(res ^ poly);
			} else {
				res = res >> 1;
			}
		}
	}

	return res;
}

/**
 * @brief		Convert EMAC mode to string
 * @details		Helper function for statistics to convert phy mode to string.
 * @param[in]	mode	phy mode
 * @return		pointer to string
 */
static inline const char_t *
phy_mode_to_str(uint32_t mode)
{
	/* Initialize to invalid */
	u8 index = (ARRAY_SIZE(phy_mode)) - 1;

	if ((ARRAY_SIZE(phy_mode)) > mode) {
		index = mode;
	}

	return phy_mode[index];
}

/**
 * @brief		Convert EMAC speed to string
 * @details		Helper function for statistics to convert emac speed to string.
 * @param[in]	speed	emac speed
 * @return		pointer to string
 */
static const char *
emac_speed_to_str(pfe_emac_speed_t speed)
{
	switch (speed) {
	default:
	case EMAC_SPEED_10_MBPS:
		return "10 Mbps";
	case EMAC_SPEED_100_MBPS:
		return "100 Mbps";
	case EMAC_SPEED_1000_MBPS:
		return "1 Gbps";
	case EMAC_SPEED_2500_MBPS:
		return "2.5 Gbps";
	}
}

/**
 * @brief		HW-specific initialization function
 * @details		This function is called during HW initialization routine and should
 * 				ensure that all necessary values are correctly configured before
 * 				the MAC is enabled.
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	mode MII mode to be configured @see pfe_emac_mii_mode_t
 * @param[in]	speed Speed to be configured @see pfe_emac_speed_t
 * @param[in]	duplex Duplex type to be configured @see pfe_emac_duplex_t
 * @return		EOK if success, error code if invalid configuration is detected
 */
errno_t
pfe_emac_cfg_init(void *base_va, pfe_emac_mii_mode_t mode,
		  pfe_emac_speed_t speed, pfe_emac_duplex_t duplex)
{
	uint32_t reg;

	/*
	 	 0. Ensure the TX/RX is disabled (MAC_CONFIGURATION)
	*/
	hal_write32(0U, (addr_t)base_va + MAC_CONFIGURATION);

	/*
		1. Set MAC address (MAC_ADDRESS0_HIGH, MAC_ADDRESS0_LOW)
	*/
	/*	TODO */
	hal_write32(0x8000ffeeU, (addr_t)base_va + MAC_ADDRESS0_HIGH);
	hal_write32(0xddccbbaaU, (addr_t)base_va + MAC_ADDRESS0_LOW);

	/*
		2. Configure packet filter (MAC_PACKET_FILTER)
	*/
	hal_write32(
		0U | DROP_NON_TCP_UDP(0U) | L3_L4_FILTER_ENABLE(0U) |
			VLAN_TAG_FILTER_ENABLE(0U) |
			HASH_OR_PERFECT_FILTER(
				1U) /* Match either perfect filtering or hash filtering (see HMC/HUC) */
			| SA_FILTER(0U) | SA_INVERSE_FILTER(0U) |
			PASS_CONTROL_PACKETS(FORWARD_ALL_EXCEPT_PAUSE) |
			DISABLE_BROADCAST_PACKETS(0U) | PASS_ALL_MULTICAST(0U) |
			DA_INVERSE_FILTER(0U) |
			HASH_MULTICAST(
				1U) /* Multicast packets are accepted when passed perfect OR hash MAC filter */
 |
			HASH_UNICAST(
				1U) /* Unicast packets are accepted when passed perfect OR hash MAC filter */
			| PROMISCUOUS_MODE(0U),
		(addr_t)base_va + MAC_PACKET_FILTER);

	/*
		3. Configure flow control (MAC_Q0_TX_FLOW_CTRL)
	*/
	reg = hal_read32((addr_t)base_va + MAC_Q0_TX_FLOW_CTRL);
	reg &= ~TX_FLOW_CONTROL_ENABLE(1U); /* No flow control engaged */
	hal_write32(reg, (addr_t)base_va + MAC_Q0_TX_FLOW_CTRL);

	/*
		4. Disable interrupts (MAC_INTERRUPT_ENABLE)
	*/
	hal_write32(0U, (addr_t)base_va + MAC_INTERRUPT_ENABLE);

	/*
		5. Set initial MAC configuration (MAC_CONFIGURATION)
	*/
	hal_write32(
		0U | ARP_OFFLOAD_ENABLE(0U) |
			SA_INSERT_REPLACE_CONTROL(CTRL_BY_SIGNALS) |
			CHECKSUM_OFFLOAD(1U) | INTER_PACKET_GAP(0U) |
			GIANT_PACKET_LIMIT_CONTROL(1U) |
			SUPPORT_2K_PACKETS(0U) | CRC_STRIPPING_FOR_TYPE(1U) |
			AUTO_PAD_OR_CRC_STRIPPING(1U) | WATCHDOG_DISABLE(1U) |
			PACKET_BURST_ENABLE(0U) | JABBER_DISABLE(1U) |
			JUMBO_PACKET_ENABLE(0U) |
			PORT_SELECT(
				0U) /* To be set up by pfe_emac_cfg_set_speed() */
 |
			SPEED(0U) /* To be set up by pfe_emac_cfg_set_speed() */
 |
			DUPLEX_MODE(
				1U) /* To be set up by pfe_emac_cfg_set_duplex() */
			| LOOPBACK_MODE(0U) | CARRIER_SENSE_BEFORE_TX(0U) |
			DISABLE_RECEIVE_OWN(0) | DISABLE_CARRIER_SENSE_TX(0U) |
			DISABLE_RETRY(0U) | BACK_OFF_LIMIT(MIN_N_10) |
			DEFERRAL_CHECK(0U) | PREAMBLE_LENGTH_TX(PREAMBLE_7B) |
			TRANSMITTER_ENABLE(0U) | RECEIVER_ENABLE(0U),
		(addr_t)base_va + MAC_CONFIGURATION);

	hal_write32(0U | FORWARD_ERROR_PACKETS(1U),
		    (addr_t)base_va + MTL_RXQ0_OPERATION_MODE);

	hal_write32(0U, (addr_t)base_va + MTL_TXQ0_OPERATION_MODE);

	hal_write32(GIANT_PACKET_SIZE_LIMIT(0x3000U),
		    (addr_t)base_va + MAC_EXT_CONFIGURATION);

	/*	Enable Data Path Parity protection */
	hal_write32(0x1U, (addr_t)base_va + MTL_DPP_CONTROL);

	hal_write32(0U | ENABLE_TIMESTAMP(1U) | INITIALIZE_TIMESTAMP(1U) |
			    ENABLE_TIMESTAMP_FOR_All(1U) |
			    ENABLE_PTP_PROCESSING(1U) | SELECT_PTP_PACKETS(3U),
		    (addr_t)base_va + MAC_TIMESTAMP_CONTROL);

	hal_write32(0x140000U, (addr_t)base_va + MAC_SUB_SECOND_INCREMENT);

	/*
		6. Configure MAC mode
	*/
	/*	Set speed */
	if (pfe_emac_cfg_set_speed(base_va, speed) != EOK) {
		return EINVAL;
	}

	/*	Set MII mode */
	if (pfe_emac_cfg_set_mii_mode(base_va, mode) != EOK) {
		return EINVAL;
	}

	/*	Set duplex */
	if (pfe_emac_cfg_set_duplex(base_va, duplex) != EOK) {
		return EINVAL;
	}

	/*	Now the MAC is initialized and disabled */
	return EOK;
}

/**
 * @brief		Set MAC duplex
 * @param[in]	base_va Base address to be written
 * @param[in]	duplex Duplex type to be configured @see pfe_emac_duplex_t
 * @return		EOK if success, error code when invalid configuration is requested
 */
errno_t
pfe_emac_cfg_set_duplex(void *base_va, pfe_emac_duplex_t duplex)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION) &
		       ~(DUPLEX_MODE(1U));

	switch (duplex) {
	case EMAC_DUPLEX_HALF: {
		reg |= DUPLEX_MODE(0U);
		break;
	}

	case EMAC_DUPLEX_FULL: {
		reg |= DUPLEX_MODE(1U);
		break;
	}

	default:
		return EINVAL;
	}

	hal_write32(reg, (addr_t)base_va + MAC_CONFIGURATION);

	return EOK;
}

/**
 * @brief		Set MAC MII mode
 * @param[in]	base_va Base address to be written
 * @param[in]	mode MII mode to be configured @see pfe_emac_mii_mode_t
 * @return		EOK if success, error code when invalid configuration is requested
 */
errno_t
pfe_emac_cfg_set_mii_mode(void *base_va, pfe_emac_mii_mode_t mode)
{
	/*
	 	 The PHY mode selection is done using a HW interface. See the "phy_intf_sel" signal.
	*/
	NXP_LOG_INFO(
		"The PHY mode selection is done using a HW interface. See the 'phy_intf_sel' signal.\n");
	(void)base_va;
	(void)mode;

	return EOK;
}

/**
 * @brief		Set MAC speed
 * @param[in]	base_va Base address to be written
 * @param[in]	speed Speed to be configured @see pfe_emac_speed_t
 * @return		EOK if success, error code when invalid configuration is requested
 */
errno_t
pfe_emac_cfg_set_speed(void *base_va, pfe_emac_speed_t speed)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION) &
		       ~(PORT_SELECT(1U) | SPEED(1U));

	switch (speed) {
	case EMAC_SPEED_10_MBPS: {
		reg |= PORT_SELECT(1U);
		reg |= SPEED(0U);
		break;
	}

	case EMAC_SPEED_100_MBPS: {
		reg |= PORT_SELECT(1U);
		reg |= SPEED(1U);
		break;
	}

	case EMAC_SPEED_1000_MBPS: {
		reg |= PORT_SELECT(0);
		reg |= SPEED(0);
		break;
	}

	case EMAC_SPEED_2500_MBPS: {
		reg |= PORT_SELECT(0);
		reg |= SPEED(1);
		break;
	}

	default:
		return EINVAL;
	}

	hal_write32(reg, (addr_t)base_va + MAC_CONFIGURATION);

	return EOK;
}

errno_t
pfe_emac_cfg_get_link_config(void *base_va, pfe_emac_speed_t *speed,
			     pfe_emac_duplex_t *duplex)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION);

	/* speed */
	switch (GET_LINE_SPEED(reg)) {
	default:
	case 0x0U:
		*speed = EMAC_SPEED_1000_MBPS;
		break;
	case 0x01U:
		*speed = EMAC_SPEED_2500_MBPS;
		break;
	case 0x02U:
		*speed = EMAC_SPEED_10_MBPS;
		break;
	case 0x03U:
		*speed = EMAC_SPEED_100_MBPS;
		break;
	}

	/* duplex */
	*duplex = (GET_DUPLEX_MODE(reg) == 1U) ? EMAC_DUPLEX_FULL :
						 EMAC_DUPLEX_HALF;

	return EOK;
}

/**
 * @brief		Get MAC configured link parameters
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[out]	clock_speed Currently configured link speed
 * @param[out]	duplex Currently configured Duplex type @see pfe_emac_duplex_t
 * @param[out]	link Current link state
 * @return		EOK if success
 */
errno_t
pfe_emac_cfg_get_link_status(void *base_va, pfe_emac_link_speed_t *link_speed,
			     pfe_emac_duplex_t *duplex, bool *link)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_PHYIF_CONTROL_STATUS);

	/* speed */
	switch (LNKSPEED(reg)) {
	default:
	case 0x0U:
		*link_speed = EMAC_LINK_SPEED_2_5_MHZ;
		break;
	case 0x01U:
		*link_speed = EMAC_LINK_SPEED_25_MHZ;
		break;
	case 0x02U:
		*link_speed = EMAC_LINK_SPEED_125_MHZ;
		break;
	case 0x03U:
		*link_speed = EMAC_LINK_SPEED_INVALID;
		break;
	}

	/* duplex */
	*duplex = (LNKMOD(reg) == 1U) ? EMAC_DUPLEX_FULL : EMAC_DUPLEX_HALF;

	/* link */
	*link = LNKSTS(reg) == 1U;

	return EOK;
}

/**
 * @brief		Set maximum frame length
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	len The new maximum frame length
 * @return		EOK if success, error code if invalid value is requested
 */
errno_t
pfe_emac_cfg_set_max_frame_length(void *base_va, uint32_t len)
{
	uint32_t reg, maxlen = 0U;
	bool_t je, s2kp, gpslce, edvlp;

	/*
		In this case the function just performs check whether the requested length
	 	is supported by the current MAC configuration. When change is needed then
	 	particular parameters (JE, S2KP, GPSLCE, DVLP, and GPSL must be changed).
	*/

	reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION);
	je = !!(reg & JUMBO_PACKET_ENABLE(1U));
	s2kp = !!(reg & SUPPORT_2K_PACKETS(1U));
	gpslce = !!(reg & GIANT_PACKET_LIMIT_CONTROL(1U));

	reg = hal_read32((addr_t)base_va + MAC_VLAN_TAG_CTRL);
	edvlp = !!(reg & ENABLE_DOUBLE_VLAN(1U));

	if (je && edvlp) {
		maxlen = 9026U;
	}

	if (!je && s2kp) {
		maxlen = 2000U;
	}

	if (!je && !s2kp && gpslce && edvlp) {
		reg = hal_read32((addr_t)base_va + MAC_EXT_CONFIGURATION);
		maxlen = reg & GIANT_PACKET_SIZE_LIMIT((uint32_t)-1);
		maxlen += 8U;
	}

	if (!je && !s2kp && !gpslce && edvlp) {
		maxlen = 1526U;
	}

	if (je && !edvlp) {
		maxlen = 9022U;
	}

	if (!je && !s2kp && gpslce && !edvlp) {
		reg = hal_read32((addr_t)base_va + MAC_EXT_CONFIGURATION);
		maxlen = reg & GIANT_PACKET_SIZE_LIMIT((uint32_t)-1);
		maxlen += 4U;
	}

	if (!je && !s2kp && !gpslce && !edvlp) {
		maxlen = 1522U;
	}

	if (maxlen == 0U) {
		return EINVAL;
	}

	if (len > maxlen) {
		return EINVAL;
	}

	return EOK;
}

/**
 * @brief		Write MAC address to a specific individual address slot
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	addr The MAC address to be written
 * @param[in]	slot Index of slot where the address shall be written
 * @note		Maximum number of slots is given by EMAC_CFG_INDIVIDUAL_ADDR_SLOTS_COUNT
 */
void
pfe_emac_cfg_write_addr_slot(void *base_va, pfe_mac_addr_t addr, uint8_t slot)
{
	u32 bottom = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) |
			  (addr[0] << 0);
	uint32_t top = (addr[5] << 8) | (addr[4] << 0);

	/*	All-zeros MAC address is special case (invalid entry) */
	if ((top != 0U) || (bottom != 0U)) {
		/*	Write the Address Enable flag */
		top |= 0x80000000U;
	}

	/*	Double synchronization (see MAC manual) */

	/*	Write HIGH register FIRST */
	hal_write32(top, (addr_t)base_va + MAC_ADDRESS_HIGH(slot));

	/*	Write LOW register NOW */
	hal_write32(bottom, (addr_t)base_va + MAC_ADDRESS_LOW(slot));

	/*	Due to sync at least 4 clock cycles ((G)MII) should be spent here */
	oal_time_usleep(100);

	/*	Write LOW register AGAIN */
	hal_write32(bottom, (addr_t)base_va + MAC_ADDRESS_LOW(slot));
}

/**
 * @brief		Convert MAC address to its hash representation
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	addr The MAC address to compute the hash for
 * @retval		The hash value as represented/used by the HW
 */
uint32_t
pfe_emac_cfg_get_hash(void *base_va, pfe_mac_addr_t addr)
{
	/*
	  	 - The EQoS is using the CRC32 - See IEEE 802.3, Section 3.2.8
	  	 - The hash table has 64 entries so hash is 6-bits long
	  	 - THe hash table is addressed by UPPER 6-bits of the CRC32
	  	 	 - Most significant bits (1) are addressing hash table register
	  	 	 - Remaining (5) bits are addressing position within the (32-bit long) register
	*/
	(void)base_va;
	(void)addr;

	return crc32_reversed((uint8_t *)&addr, 6U);
}

/**
 * @brief		Enable/Disable individual address group defined by 'hash'
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	hash The hash value
 * @param[in]	en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_uni_group(void *base_va, int32_t hash, bool_t en)
{
	uint32_t reg;
	uint32_t val = (uint32_t)(hash & 0xfc000000U) >> 26;
	uint8_t hash_table_idx = (val & 0x40U) >> 6;
	uint8_t pos = (val & 0x1fU);

	reg = hal_read32((addr_t)base_va + MAC_HASH_TABLE_REG(hash_table_idx));

	if (en) {
		reg |= (uint32_t)(1U << pos);
	} else {
		reg &= (uint32_t)~(1U << pos);
	}

	/*	Double synchronization (see MAC manual) */
	hal_write32(reg, (addr_t)base_va + MAC_HASH_TABLE_REG(hash_table_idx));
	/*	Wait at least 4 clock cycles ((G)MII) */
	oal_time_usleep(100);
	hal_write32(reg, (addr_t)base_va + MAC_HASH_TABLE_REG(hash_table_idx));
}

/**
 * @brief		Enable/Disable multicast address group defined by 'hash'
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	hash The hash value
 * @param[in]	en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_multi_group(void *base_va, int32_t hash, bool_t en)
{
	/*	Both, uni- and multi-cast addresses go to the same hash table */
	pfe_emac_cfg_set_uni_group(base_va, hash, en);
}

/**
 * @brief		Enable/Disable loopback mode
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param		en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_loopback(void *base_va, bool_t en)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION) &
		       ~(LOOPBACK_MODE(1));

	reg |= LOOPBACK_MODE(en);

	hal_write32(reg, (addr_t)base_va + MAC_CONFIGURATION);
}

/**
 * @brief		Enable/Disable promiscuous mode
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param		en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_promisc_mode(void *base_va, bool_t en)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_PACKET_FILTER) &
		       ~(PROMISCUOUS_MODE(1));

	reg |= PROMISCUOUS_MODE(en);

	hal_write32(reg, (addr_t)base_va + MAC_PACKET_FILTER);
}

/**
 * @brief		Enable/Disable broadcast reception
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param		en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_broadcast(void *base_va, bool_t en)
{
	u32 reg = hal_read32((addr_t)base_va + MAC_PACKET_FILTER) &
		       ~(DISABLE_BROADCAST_PACKETS(1));

	reg |= DISABLE_BROADCAST_PACKETS(!en);

	hal_write32(reg, (addr_t)base_va + MAC_PACKET_FILTER);
}

/**
 * @brief		Enable/Disable the Ethernet controller
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param		en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_enable(void *base_va, bool_t en)
{
	uint32_t reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION);

	reg &= ~(TRANSMITTER_ENABLE(1) | RECEIVER_ENABLE(1));
	reg |= TRANSMITTER_ENABLE(en) | RECEIVER_ENABLE(en);

	hal_write32(reg, (addr_t)base_va + MAC_CONFIGURATION);
}

/**
 * @brief		Enable/Disable the flow control
 * @details		Once enabled the MAC shall process PAUSE frames
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param		en TRUE means ENABLE, FALSE means DISABLE
 */
void
pfe_emac_cfg_set_flow_control(void *base_va, bool_t en)
{
	u32 reg, ii = 0U;

	do {
		reg = hal_read32((addr_t)base_va + MAC_Q0_TX_FLOW_CTRL);
		oal_time_usleep(100U);
		ii++;
	} while ((reg & BUSY_OR_BACKPRESSURE_ACTIVE(1)) && (ii < 10U));

	if (ii >= 10U) {
		NXP_LOG_ERROR("Flow control is busy, exiting...\n");
		return;
	}

	reg &= ~(TX_FLOW_CONTROL_ENABLE(1));
	reg |= TX_FLOW_CONTROL_ENABLE(en);

	hal_write32(reg, (addr_t)base_va + MAC_Q0_TX_FLOW_CTRL);
}

/**
 * @brief		Read value from the MDIO bus using Clause 22
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	pa Address of the PHY to read (5-bit)
 * @param[in]	ra Address of the register in the PHY to be read (5-bit)
 * @param[out]	val If success the the read value is written here (16 bit)
 * @retval		EOK Success
 */
errno_t
pfe_emac_cfg_mdio_read22(void *base_va, uint8_t pa, uint8_t ra, uint16_t *val)
{
	uint32_t reg;
	uint32_t timeout = 500U;
	reg = GMII_BUSY(1U) | CLAUSE45_ENABLE(0U) |
	      GMII_OPERATION_CMD(GMII_READ) |
	      SKIP_ADDRESS_PACKET(0U)
	      /*	Select according to real CSR clock frequency. S32G: CSR_CLK = XBAR_CLK = 400MHz */
	      | CSR_CLOCK_RANGE(CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204) |
	      NUM_OF_TRAILING_CLOCKS(0U) | REG_DEV_ADDR(ra) |
	      PHYS_LAYER_ADDR(pa) | BACK_TO_BACK(0U) | PREAMBLE_SUPPRESSION(0U);

	/*	Start a read operation */
	hal_write32(reg, base_va + MAC_MDIO_ADDRESS);
	/*	Wait for completion */
	while (GMII_BUSY(1) == ((reg = hal_read32(base_va + MAC_MDIO_ADDRESS)) &
				GMII_BUSY(1))) {
		if (timeout-- == 0U) {
			return ETIME;
		}
		oal_time_usleep(10);
	}

	/*	Get the data */
	reg = hal_read32(base_va + MAC_MDIO_DATA);
	*val = GMII_DATA(reg);

	return EOK;
}

/**
* @brief		Read value from the MDIO bus using Clause 45
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	pa  Address of the PHY to read (5-bit)
 * @param[in]	dev Selects the device in the PHY to read (5-bit)
 * @param[in]	ra  Register address in the device to read  (16-bit)
 * @param[out]	val If success the the read value is written here (16-bit)
 * @retval		EOK Success
 */
errno_t
pfe_emac_cfg_mdio_read45(void *base_va, uint8_t pa, uint8_t dev, uint16_t ra,
			 uint16_t *val)
{
	uint32_t reg;
	uint32_t timeout = 500U;

	/* Set the register addresss to read */
	reg = GMII_REGISTER_ADDRESS(ra);
	hal_write32(reg, base_va + MAC_MDIO_DATA);

	reg = GMII_BUSY(1U) | CLAUSE45_ENABLE(1U) |
	      GMII_OPERATION_CMD(GMII_READ) |
	      SKIP_ADDRESS_PACKET(0U)
	      /*	Select according to real CSR clock frequency. S32G: CSR_CLK = XBAR_CLK = 400MHz */
	      | CSR_CLOCK_RANGE(CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204) |
	      NUM_OF_TRAILING_CLOCKS(0U) | REG_DEV_ADDR(dev) |
	      PHYS_LAYER_ADDR(pa) | BACK_TO_BACK(0U) | PREAMBLE_SUPPRESSION(0U);

	/*	Start a read operation */
	hal_write32(reg, base_va + MAC_MDIO_ADDRESS);

	/*	Wait for completion */
	while (GMII_BUSY(1) ==
	       (hal_read32(base_va + MAC_MDIO_ADDRESS) & GMII_BUSY(1))) {
		if (timeout-- == 0U) {
			return ETIME;
		}

		oal_time_usleep(10);
	}

	/*	Get the data */
	reg = hal_read32(base_va + MAC_MDIO_DATA);
	*val = GMII_DATA(reg);

	return EOK;
}

/**
 * @brief		Write value to the MDIO bus using Clause 22
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	pa Address of the PHY to be written (5-bit)
 * @param[in]	ra Address of the register in the PHY to be written (5-bit)
 * @param[in]	val Value to be written into the register (16 bit)
 * @retval		EOK Success
 */
errno_t
pfe_emac_cfg_mdio_write22(void *base_va, uint8_t pa, uint8_t ra, uint16_t val)
{
	uint32_t reg;
	uint32_t timeout = 500U;

	/*	Write data register first */
	reg = GMII_DATA(val);
	hal_write32(reg, (addr_t)base_va + MAC_MDIO_DATA);

	reg = GMII_BUSY(1U) | CLAUSE45_ENABLE(0U) |
	      GMII_OPERATION_CMD(GMII_WRITE) |
	      SKIP_ADDRESS_PACKET(0U)
	      /*	Select according to real CSR clock frequency. S32G: CSR_CLK = XBAR_CLK = 400MHz */
	      | CSR_CLOCK_RANGE(CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204) |
	      NUM_OF_TRAILING_CLOCKS(0U) | REG_DEV_ADDR(ra) |
	      PHYS_LAYER_ADDR(pa) | BACK_TO_BACK(0U) | PREAMBLE_SUPPRESSION(0U);

	/*	Start a write operation */
	hal_write32(reg, (addr_t)base_va + MAC_MDIO_ADDRESS);
	/*	Wait for completion */
	while (GMII_BUSY(1) == (hal_read32((addr_t)base_va + MAC_MDIO_ADDRESS) &
				GMII_BUSY(1))) {
		if (timeout-- == 0U) {
			return ETIME;
		}
		oal_time_usleep(10);
	}

	return EOK;
}

/**
* @brief		Write value to the MDIO bus using Clause 45
 * @param[in]	base_va Base address of MAC register space (virtual)
 * @param[in]	pa  Address of the PHY to be written (5-bit)
 * @param[in]   dev Device in the PHY to be written (5-bit)
 * @param[in]	ra  Address of the register in the device to be written (16-bit)
 * @param[in]	val Value to be written (16-bit)
 * @retval		EOK Success
 */
errno_t
pfe_emac_cfg_mdio_write45(void *base_va, uint8_t pa, uint8_t dev, uint16_t ra,
			  uint16_t val)
{
	uint32_t reg;
	uint32_t timeout = 500U;

	/*	Write data register first */
	reg = GMII_DATA(val) | GMII_REGISTER_ADDRESS(ra);
	hal_write32(reg, base_va + MAC_MDIO_DATA);

	reg = GMII_BUSY(1U) | CLAUSE45_ENABLE(1U) |
	      GMII_OPERATION_CMD(GMII_WRITE) |
	      SKIP_ADDRESS_PACKET(0U)
	      /*	Select according to real CSR clock frequency. S32G: CSR_CLK = XBAR_CLK = 400MHz */
	      | CSR_CLOCK_RANGE(CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204) |
	      NUM_OF_TRAILING_CLOCKS(0U) | REG_DEV_ADDR(dev) |
	      PHYS_LAYER_ADDR(pa) | BACK_TO_BACK(0U) | PREAMBLE_SUPPRESSION(0U);

	/*	Start a write operation */
	hal_write32(reg, base_va + MAC_MDIO_ADDRESS);
	/*	Wait for completion */
	while (GMII_BUSY(1) ==
	       (hal_read32(base_va + MAC_MDIO_ADDRESS) & GMII_BUSY(1))) {
		if (timeout-- == 0U) {
			return ETIME;
		}

		oal_time_usleep(10);
	}

	return EOK;
}

/**
 * @brief		Get EMAC statistics in text form
 * @details		This is a HW-specific function providing detailed text statistics
 * 				about the EMAC block.
 * @param[in]	base_va 	Base address of EMAC register space (virtual)
 * @param[in]	buf 		Pointer to the buffer to write to
 * @param[in]	size 		Buffer length
 * @param[in]	verb_level 	Verbosity level
 * @return		Number of bytes written to the buffer
 */
uint32_t
pfe_emac_cfg_get_text_stat(void *base_va, char_t *buf, uint32_t size,
			   uint8_t verb_level)
{
	uint32_t len = 0U;
	uint32_t reg;
	pfe_emac_speed_t speed;
	pfe_emac_duplex_t duplex;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!base_va)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return 0U;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Get version */
	reg = hal_read32((addr_t)base_va + MAC_VERSION);
	len += oal_util_snprintf(buf + len, size - len,
				 "SNPVER                    : 0x%x\n",
				 reg & 0xffU);
	len += oal_util_snprintf(buf + len, size - len,
				 "USERVER                   : 0x%x\n",
				 (reg >> 8) & 0xffU);

	reg = hal_read32((addr_t)base_va + RX_PACKETS_COUNT_GOOD_BAD);
	len += oal_util_snprintf(buf + len, size - len,
				 "RX_PACKETS_COUNT_GOOD_BAD : 0x%x\n", reg);
	reg = hal_read32((addr_t)base_va + TX_PACKET_COUNT_GOOD_BAD);
	len += oal_util_snprintf(buf + len, size - len,
				 "TX_PACKET_COUNT_GOOD_BAD  : 0x%x\n", reg);

	pfe_emac_cfg_get_link_config(base_va, &speed, &duplex);
	reg = hal_read32((addr_t)base_va + MAC_CONFIGURATION);
	len += oal_util_snprintf(
		buf + len, size - len,
		"MAC_CONFIGURATION         : 0x%x [speed: %s]\n", reg,
		emac_speed_to_str(speed));

	reg = (hal_read32((addr_t)base_va + MAC_HW_FEATURE0) >> 28U) & 0x07U;
	len += oal_util_snprintf(buf + len, size - len,
				 "ACTPHYSEL(MAC_HW_FEATURE0): %s\n",
				 phy_mode_to_str(reg));

	/* Error debugging */
	if (verb_level >= 8) {
		reg = hal_read32((addr_t)base_va + TX_UNDERFLOW_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_UNDERFLOW_ERROR_PACKETS        : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_SINGLE_COLLISION_GOOD_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_SINGLE_COLLISION_GOOD_PACKETS  : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_MULTIPLE_COLLISION_GOOD_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_MULTIPLE_COLLISION_GOOD_PACKETS: 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_DEFERRED_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_DEFERRED_PACKETS               : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_LATE_COLLISION_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_LATE_COLLISION_PACKETS         : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_EXCESSIVE_COLLISION_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_EXCESSIVE_COLLISION_PACKETS    : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_CARRIER_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_CARRIER_ERROR_PACKETS          : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_EXCESSIVE_DEFERRAL_ERROR);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_EXCESSIVE_DEFERRAL_ERROR       : 0x%x\n", reg);

		reg = hal_read32((addr_t)base_va + TX_OSIZE_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_OSIZE_PACKETS_GOOD             : 0x%x\n", reg);

		reg = hal_read32((addr_t)base_va + RX_CRC_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_CRC_ERROR_PACKETS              : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_ALIGNMENT_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_ALIGNMENT_ERROR_PACKETS        : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_RUNT_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_RUNT_ERROR_PACKETS             : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_JABBER_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_JABBER_ERROR_PACKETS           : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_LENGTH_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_LENGTH_ERROR_PACKETS           : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_OUT_OF_RANGE_TYPE_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_OUT_OF_RANGE_TYPE_PACKETS      : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_FIFO_OVERFLOW_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_FIFO_OVERFLOW_PACKETS          : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_RECEIVE_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_RECEIVE_ERROR_PACKETS          : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_RECEIVE_ERROR_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_RECEIVE_ERROR_PACKETS          : 0x%x\n", reg);
	}

	/* Cast/vlan/flow control */
	if (verb_level >= 3) {
		reg = hal_read32((addr_t)base_va + TX_UNICAST_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_UNICAST_PACKETS_GOOD_BAD       : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_BROADCAST_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_BROADCAST_PACKETS_GOOD         : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_BROADCAST_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_BROADCAST_PACKETS_GOOD_BAD     : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_MULTICAST_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_MULTICAST_PACKETS_GOOD         : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_MULTICAST_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_MULTICAST_PACKETS_GOOD_BAD     : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_VLAN_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_VLAN_PACKETS_GOOD              : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_PAUSE_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_PAUSE_PACKETS                  : 0x%x\n", reg);
	}

	if (verb_level >= 4) {
		reg = hal_read32((addr_t)base_va + RX_UNICAST_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_UNICAST_PACKETS_GOOD           : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_BROADCAST_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_BROADCAST_PACKETS_GOOD         : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_MULTICAST_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_MULTICAST_PACKETS_GOOD         : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_VLAN_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_VLAN_PACKETS_GOOD_BAD          : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_PAUSE_PACKETS);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_PAUSE_PACKETS                  : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_CONTROL_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_CONTROL_PACKETS_GOOD           : 0x%x\n", reg);
	}

	if (verb_level >= 1) {
		reg = hal_read32((addr_t)base_va + TX_OCTET_COUNT_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_OCTET_COUNT_GOOD                : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + TX_OCTET_COUNT_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_OCTET_COUNT_GOOD_BAD            : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_64OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_64OCTETS_PACKETS_GOOD_BAD       : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_65TO127OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_65TO127OCTETS_PACKETS_GOOD_BAD  : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_128TO255OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_128TO255OCTETS_PACKETS_GOOD_BAD : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_256TO511OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_256TO511OCTETS_PACKETS_GOOD_BAD : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_512TO1023OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_512TO1023OCTETS_PACKETS_GOOD_BAD: 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 TX_1024TOMAXOCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_1024TOMAXOCTETS_PACKETS_GOOD_BAD: 0x%x\n", reg);
	}

	if (verb_level >= 5) {
		reg = hal_read32((addr_t)base_va + TX_OSIZE_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_OSIZE_PACKETS_GOOD              : 0x%x\n", reg);
	}

	if (verb_level >= 2) {
		reg = hal_read32((addr_t)base_va + RX_OCTET_COUNT_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_OCTET_COUNT_GOOD                : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_OCTET_COUNT_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_OCTET_COUNT_GOOD_BAD            : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_64OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_64OCTETS_PACKETS_GOOD_BAD       : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_65TO127OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_65TO127OCTETS_PACKETS_GOOD_BAD  : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_128TO255OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_128TO255OCTETS_PACKETS_GOOD_BAD : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_256TO511OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_256TO511OCTETS_PACKETS_GOOD_BAD : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_512TO1023OCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_512TO1023OCTETS_PACKETS_GOOD_BAD: 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va +
				 RX_1024TOMAXOCTETS_PACKETS_GOOD_BAD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_1024TOMAXOCTETS_PACKETS_GOOD_BAD: 0x%x\n", reg);
	}

	if (verb_level >= 5) {
		reg = hal_read32((addr_t)base_va + RX_OVERSIZE_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"TX_OSIZE_PACKETS_GOOD              : 0x%x\n", reg);
		reg = hal_read32((addr_t)base_va + RX_UNDERSIZE_PACKETS_GOOD);
		len += oal_util_snprintf(
			buf + len, size - len,
			"RX_UNDERSIZE_PACKETS_GOOD          : 0x%x\n", reg);
	}

	return len;
}

/** @}*/
