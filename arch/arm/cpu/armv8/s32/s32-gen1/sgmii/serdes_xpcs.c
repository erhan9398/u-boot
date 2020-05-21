// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2020 NXP
 *
 * The SerDes module source file.
 */

#include <serdes_regs.h>
#include <serdes_xpcs_regs.h>
#include <stdio.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <asm/io.h>


/**
 * @brief	Variables for XPCS indirect access
 */
typedef struct __serdes_xpcs_access_vars_tag {
	u32 ofsleft;
	u32 ofsright;
	u32 addr1;
	u32 data1;
	u32 addr2;
	u32 data2;
} serdes_xpcs_access_vars_t;

/**
 * @brief		Get variables needed for indirect XPCS access
 */
static int serdes_get_xpcs_access_vars(u32 xpcs, u32 reg,
					   serdes_xpcs_access_vars_t *vars)
{
	vars->ofsleft = (reg >> 8) & 0xffffU;
	vars->ofsright = (reg & 0xffU);
	vars->data1 = vars->ofsleft;

	if (SERDES_XPCS_0_BASE == xpcs) {
		vars->addr1 = SERDES_XPCS_0_ADDR1;
		vars->addr2 = SERDES_XPCS_0_ADDR2 + (vars->ofsright * 4U);
	} else if (SERDES_XPCS_1_BASE == xpcs) {
		vars->addr1 = SERDES_XPCS_1_ADDR1;
		vars->addr2 = SERDES_XPCS_1_ADDR2 + (vars->ofsright * 4U);
	} else {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief			Read XPCS register
 * @param[in]		base SerDes base address
 * @param[in]		xpcs XPCS offset within SerDes memory space
 * @param[in]		reg XPCS register address
 * @param[in,out]	val The XPCS register value
 * @return		0 if success, error code otherwise
 */
static void serdes_xpcs_reg_read(void *base, u32 xpcs, u32 reg,
				 volatile u16 *val)
{
	int ret;
	serdes_xpcs_access_vars_t vars = {0U};

	ret = serdes_get_xpcs_access_vars(xpcs, reg, &vars);
	if (ret)
		pr_warn("Can't read XPCS register (0x%x)\n", reg);

	writel(vars.data1, (phys_addr_t)base + vars.addr1);
	*val = readl((phys_addr_t)base + vars.addr2) & 0xffffU;
}

/**
 * @brief	Write XPCS register
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	reg XPCS register address
 * @param[in]	val The XPCS register value
 * @return	0 if success, error code otherwise
 */
static void serdes_xpcs_reg_write(void *base, u32 xpcs, u32 reg, u16 val)
{
	int ret;
	serdes_xpcs_access_vars_t vars = {0U};

	ret = serdes_get_xpcs_access_vars(xpcs, reg, &vars);
	if (ret)
		pr_warn("Can't write XPCS register (0x%x)\n", reg);

	writel(vars.data1, (phys_addr_t)base + vars.addr1);
	writel(val, (phys_addr_t)base + vars.addr2);
}

/**
 * @brief	Set SerDes functional mode
 * @param[in]	base SerDes base address
 * @param[in]	mode Mode
 * @return	0 if success, error code otherwise
 */
int serdes_set_mode(void *base, serdes_mode_t mode)
{
	u32 regval;

	if (mode > SERDES_MODE_MAX)
		return -EINVAL;

	regval = readl(base + SS_SS_RW_REG_0);
	regval &= ~(SUBSYS_MODE_VALUE(0xff));
	writel(regval | mode, base + SS_SS_RW_REG_0);

	return 0;
}

/**
 * @brief	Wait until XPCS power-up sequence state isn't "Power_Good"
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @return	0 Power-up sequence state is "Power_Good"
 */
int serdes_xpcs_wait_for_power_good(void *base, u32 xpcs)
{
	volatile u16 reg16;
	u8 pseq;
	u32 timeout = 1000U;

	do {
		serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_STS, &reg16);
		pseq = (reg16 >> 2) & 0x7U;
		if (0x4U == pseq)
			break;
		timeout--;
		udelay(1000U);
	} while (timeout > 0U);

	if (timeout > 0U)
		return 0;

	return -ETIMEDOUT;
}

/**
 * @brief		Set SGMII speed
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	mbps Speed in [Mbps]
 * @param[in]	duplex Full duplex = TRUE, Half duplex = FALSE
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_set_sgmii_speed(void *base, u32 xpcs, u32 mbps,
				    bool fduplex)
{
	u16 reg16;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	/*	Update control register (+ disable AN) */
	serdes_xpcs_reg_read(base, xpcs, SR_MII_CTRL, &reg16);
	reg16 &= ~(MII_CTRL_SS13 | MII_CTRL_SS6 | MII_CTRL_DUPLEX_MODE
		   | MII_CTRL_AN_ENABLE);

	switch (mbps) {
	case 10U:
		break;

	case 100U:
		reg16 |= MII_CTRL_SS13;
		break;

	case 1000U:
		reg16 |= MII_CTRL_SS6;
		break;

	default:
		/*	Unsupported value */
		return -EINVAL;
	}

	if (fduplex)
		reg16 |= MII_CTRL_DUPLEX_MODE;

	/*	Write the control register */
	serdes_xpcs_reg_write(base, xpcs, SR_MII_CTRL, reg16);

	return 0;
}

int serdes_xpcs_set_loopback(void *base, u32 xpcs, bool enable)
{
	u16 reg16;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	serdes_xpcs_reg_read(base, xpcs, SR_MII_CTRL, &reg16);

	/*	Update control register (+ manage LBE) */
	if (enable)
		reg16 |= R2TLBE;
	else
		reg16 &= ~R2TLBE;

	/*	Write the control register */
	serdes_xpcs_reg_write(base, xpcs, SR_MII_CTRL, reg16);

	return 0;
}

/**
 * @brief	Wait for PCS link
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	timeout Timeout in [s]
 * @return	0 link is up, error code otherwise
 */
int serdes_wait_for_link(void *base, u32 xpcs, u8 timeout)
{
	/*	Number of 100ms periods */
	u32 tout = (1000U * timeout) / 100U;
	u16 reg16;

	do {
		serdes_xpcs_reg_read(base, xpcs, SR_MII_STS, &reg16);
		if (0U != (reg16 & MII_STS_LINK_STS))
			break;
		tout--;
		udelay(100000U);
	} while (tout > 0U);

	if (0U == tout)
		return -ETIMEDOUT;

	return 0;
}


/**
 * @brief	Configure XPCS to 1G mode with respect to reference clock
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	ext_ref If reference clock is taken via pads then this shall be
 *		TRUE. If internal reference clock is used then use FALSE.
 * @param[in]	ref_mhz Reference clock frequency in [MHz]. 100 or 125.
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_set_1000_mode(void *base, u32 xpcs, bool ext_ref,
				  u8 ref_mhz)
{
	int retval;
	u16 reg16, use_pad = 0U;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	if ((100U != ref_mhz) && (125U != ref_mhz))
		return -EINVAL;

	if (ext_ref)
		/*	Using external clock reference */
		use_pad = REF_CLK_CTRL_REF_USE_PAD;

	/*	Wait for XPCS power up */
	retval = serdes_xpcs_wait_for_power_good(base, xpcs);
	if (retval)
		/*	XPCS power-up failed */
		return retval;

	/*	Compatibility check */
	serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID1, &reg16);
	if (0x7996U != reg16)
		/*	Unexpected XPCS ID */
		return -EINVAL;

	serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID2, &reg16);
	if (0xced0U != reg16)
		/*	Unexpected XPCS ID */
		return -EINVAL;

	/*	(Switch to 1G mode: #1) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, EN_VSMMD1);
	/*	(Switch to 1G mode: #2) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DBG_CTRL, 0U);
	/*	(Switch to 1G mode: #3) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL,
			      MPLL_CMN_CTRL_MPLL_EN_0);

	if (100U == ref_mhz) {
		/*	RefClk = 100MHz */
		/*	(Switch to 1G mode: #4) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
				      REF_CLK_CTRL_REF_RANGE(3U)
				      | use_pad
				      | REF_CLK_CTRL_REF_CLK_EN);
		/*	(Switch to 1G mode: #5) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
				      MPLLA_MULTIPLIER_VALUE(25U));
	} else {
		/*	RefClk = 125MHz */
		/*	(Switch to 1G mode: #4) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
				      REF_CLK_CTRL_REF_RANGE(2U)
				      | use_pad
				      | REF_CLK_CTRL_REF_MPLLA_DIV2
				      | REF_CLK_CTRL_REF_CLK_DIV2
				      | REF_CLK_CTRL_REF_CLK_EN);
		/*	(Switch to 1G mode: #5) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
				      MPLLA_MULTIPLIER_VALUE(80U));
	}

	/*	(Switch to 1G mode: #6) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL1, 0U);

	/*	(Switch to 1G mode: #7) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL2,
			      MPLLA_TX_CLK_DIV(1U) | MPLLA_DIV10_CLK_EN);

	if (100U == ref_mhz) {
		/*	RefClk = 100MHz */
		/*	(Switch to 1G mode: #8) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL3,
				      357U);
		/*	(Switch to 1G mode: #9) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_VCO_CAL_LD0, 1350U);
		/*	(Switch to 1G mode: #10) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0,
				      27U);
	} else {
		/*	RefClk = 125MHz */
		/*	(Switch to 1G mode: #8) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL3,
				      43U);
		/*	(Switch to 1G mode: #9) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_VCO_CAL_LD0, 1360U);
		/*	(Switch to 1G mode: #10) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0,
				      17U);
	}

	/*	(Switch to 1G mode: #11) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
			      0x2U); /* b010 */
	/*	(Switch to 1G mode: #12) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
			      0x3U); /* b11 */
	/*	(Switch to 1G mode: #13) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_CDR_CTRL,
			      0x1U); /* VCO_LOW_FREQ_0=0 + CDR_TRACK_ENA=1 */
	/*	(Switch to 1G mode: #14) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			      MPPLB_CAL_DISABLE|0x7dU);
			      /* CAL_DISABLE=1, MPPLB_MULTIPLIER=default */

	/*	(Switch to 1G mode: #15) */
	serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, reg16 | VR_RST);
			      /* Issue reset */

	/*	(Switch to 1G mode: #16) */
	do
		serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
	while (0U != (reg16 & VR_RST));

	/*	Wait for XPCS power up */
	pr_debug("Waiting for XPCS power-up\n");
	if (serdes_xpcs_wait_for_power_good(base, xpcs)) {
		pr_err("XPCS power-up failed\n");
		return -EXIT_FAILURE;
	}

#if 0
	/*	TODO: Clause 37 Auto-negotiation here: */
	/*	Link timer */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_LINK_TIMER_CTRL, 0x20);
	/*	AN CTRL: SGMII + 4-bit MII, MAC side */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_AN_CTRL,
			      MII_AN_CTRL_PCS_MODE(PCS_MODE_SGMII)
			      | MII_AN_CTRL_MII_CTRL);
	/*	Enable auto speed mode change */
	serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
			      reg16 | MAC_AUTO_SW | EN_VSMMD1);
	/*	Enable and restart AN */
	serdes_xpcs_reg_read(base, xpcs, SR_MII_CTRL, &reg16);
	serdes_xpcs_reg_write(base, xpcs, SR_MII_CTRL,
			      reg16 | MII_CTRL_RESTART_AN | MII_CTRL_AN_ENABLE);

	pr_debug("waiting for AN...\n");
	/*	Wait for AN complete */
	do
		serdes_xpcs_reg_read(base, xpcs, VR_MII_AN_INTR_STS, &reg16);
	while (0U == (reg16 & MII_AN_INTR_STS_CL37_ANCMPLT_INTR));

	/*	TODO: W1clear the MII_AN_INTR_STS_CL37_ANCMPLT_INTR */
#endif

	return 0;
}

/**
 * @brief	Configure XPCS to 2.5G mode with respect to reference clock
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	ext_ref If reference clock is taken via pads then this
 *		shall be TRUE. If internal ref clock is used then use FALSE.
 * @param[in]	ref_mhz Reference clock frequency in [MHz]. Valid is only 125.
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_set_2500_mode(void *base, u32 xpcs, bool ext_ref,
				  u8 ref_mhz)
{
	int retval;
	u16 reg16, use_pad = 0U;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	if (125U != ref_mhz)
		return -EINVAL;

	if (ext_ref)
		/*	Using external clock reference */
		use_pad = REF_CLK_CTRL_REF_USE_PAD;

	/*	Wait for XPCS power up */
	retval = serdes_xpcs_wait_for_power_good(base, xpcs);
	if (retval)
		/*	XPCS power-up failed */
		return retval;

	/*	Compatibility check */
	serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID1, &reg16);
	if (0x7996U != reg16)
		/*	Unexpected XPCS ID */
		return -EINVAL;

	serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID2, &reg16);
	if (0xced0U != reg16)
		/*	Unexpected XPCS ID */
		return -EINVAL;

	/*	(Switch to 2.5G mode: #1) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
			      EN_VSMMD1 | EN_2_5G_MODE);
	/*	(Switch to 2.5G mode: #2) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DBG_CTRL, 0U);
	/*	(Switch to 2.5G mode: #3) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL,
			      MPLL_CMN_CTRL_MPLL_EN_0
			      | MPLL_CMN_CTRL_MPLLB_SEL_0);
	/*	(Switch to 2.5G mode: #4) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			      REF_CLK_CTRL_REF_MPLLB_DIV2
			      | use_pad
			      | REF_CLK_CTRL_REF_RANGE(2U)
			      | REF_CLK_CTRL_REF_CLK_DIV2);
	/*	(Switch to 2.5G mode: #5) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			      MPLLB_MULTIPLIER(125U));
	/*	(Switch to 2.5G mode: #6) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL1, 0U);
	/*	(Switch to 2.5G mode: #7) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL2,
			      MPLLB_CTRL2_MPLLB_TX_CLK_DIV(5U)
			      | MPLLB_CTRL2_MPLLB_DIV10_CLK_EN);
	/*	(Switch to 2.5G mode: #8) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL3, 68U);
	/*	(Switch to 2.5G mode: #9) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_VCO_CAL_LD0,
			      1350U);
	/*	(Switch to 2.5G mode: #10) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0, 27U);
	/*	(Switch to 2.5G mode: #11) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_TX_RATE_CTRL, 0U);
	/*	(Switch to 2.5G mode: #12) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
			      0x1U); /* b01 */
	/*	(Switch to 2.5G mode: #13) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_CDR_CTRL,
			      CDR_CTRL_VCO_LOW_FREQ_0 | 0x1U);
			      /* +CDR_TRACKING_ENABLE=1 */
	/*	(Switch to 2.5G mode: #14) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
			      MPLLA_CAL_DISABLE | 0x50U);
			      /* MPPLA_MULTIPLIER=default */
	/*	(Switch to 2.5G mode: #15) */
	serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, reg16 | VR_RST);
		/* Issue reset */

	/*	(Switch to 2.5G mode: #16) */
	do
		serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
	while (0U != (reg16 & VR_RST));

	/*	Wait for XPCS power up */
	pr_debug("Waiting for XPCS power-up\n");
	if (serdes_xpcs_wait_for_power_good(base, xpcs)) {
		pr_err("XPCS power-up failed\n");
		return -EXIT_FAILURE;
	}

	return 0;
}
