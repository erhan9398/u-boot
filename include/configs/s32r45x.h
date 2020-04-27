/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for all the Freescale S32R45X boards.
 */

#ifndef __S32R45X_H
#define __S32R45X_H

#define CONFIG_S32R45X

#include <configs/s32-gen1.h>

#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR

#ifdef CONFIG_BOOTARGS_LOGLEVEL
#undef CONFIG_BOOTARGS_LOGLEVEL
#endif

#define CONFIG_BOOTARGS_LOGLEVEL	" loglevel=7 "

#ifdef CONFIG_BOOTDELAY
#undef CONFIG_BOOTDELAY
#endif

#define CONFIG_BOOTDELAY	-1

#endif /* CONFIG_TARGET_TYPE_S32GEN1_EMULATOR */

#endif
