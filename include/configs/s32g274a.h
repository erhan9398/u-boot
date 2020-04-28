/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2017-2020 NXP
 */

/*
 * Configuration settings for all the Freescale S32G274A boards.
 */

#ifndef __S32G274A_H
#define __S32G274A_H

#define CONFIG_S32G274A

#include <configs/s32-gen1.h>

#ifndef CONFIG_PRAM
#define CONFIG_PRAM	2048	/* 2MB */
#endif

#ifdef CONFIG_TARGET_S32G274AEVB
#define CONFIG_USB_EHCI_MX6
#define CONFIG_MXC_USB_PORTSC        PORT_PTS_ULPI
#endif

#endif

