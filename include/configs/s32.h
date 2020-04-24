/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for all the Freescale S32 boards.
 */

#ifndef __S32_H
#define __S32_H

#define CONFIG_S32

#include <asm/arch-s32/imx-regs.h>
#include <generated/autoconf.h>

#undef CONFIG_RUN_FROM_IRAM_ONLY

#define CONFIG_REMAKE_ELF

/* u-boot uses just DDR0 */
#define CONFIG_RUN_FROM_DDR0
#undef CONFIG_RUN_FROM_DDR1

#define CONFIG_MACH_TYPE		4146

/* Config CACHE */
#define CONFIG_CMD_CACHE

/* Enable DCU QoS fix */
#define CONFIG_DCU_QOS_FIX

/* Flat device tree definitions */
#define CONFIG_OF_FDT
#define CONFIG_OF_BOARD_SETUP

/* System Timer */
/* #define CONFIG_SYS_PIT_TIMER */

#define CONFIG_LOADADDR		LOADADDR

/* Init CSE3 from u-boot */
#ifdef CONFIG_FSL_CSE3

#define CONFIG_ARCH_MISC_INIT
#define KIA_BASE		(IRAM_BASE_ADDR + 0x5000UL)
/* Secure Boot */
#ifdef CONFIG_SECURE_BOOT
#define SECURE_BOOT_KEY_ID	0x4UL
#endif
/* start address and size of firmware+keyimage binary blob */
#define CSE_BLOB_BASE		(IRAM_BASE_ADDR + 0x1000UL)
#define CSE_BLOB_SIZE		0x00004500UL

#define CONFIG_FSL_CSE3_SETTINGS \
	"cse_addr=" __stringify(CSE_BLOB_BASE) "\0" \
	"cse_file=cse.bin\0" \
	"sec_boot_key_id=" __stringify(SECURE_BOOT_KEY_ID) "\0" \

#else
#define CONFIG_FSL_CSE3_SETTINGS
#endif /* CONFIG_FSL_CSE3 */

/* DDR chips on S32V234 boards have 32 bits cells */
#define RAM_CELL_SIZE		32

#define CONFIG_SKIP_LOWLEVEL_INIT

/* Enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG

/* SMP definitions */
#define CONFIG_MAX_CPUS			(4)
#define SECONDARY_CPU_BOOT_PAGE		(CONFIG_SYS_SDRAM_BASE)
#define CPU_RELEASE_ADDR		SECONDARY_CPU_BOOT_PAGE
#define CONFIG_FSL_SMP_RELEASE_ALL
#ifndef CONFIG_XEN_SUPPORT
#define CONFIG_ARMV8_SWITCH_TO_EL1
#endif

/* SMP Spin Table Definitions */
#define CONFIG_MP

/* Ramdisk name */
#define RAMDISK_NAME		rootfs.uimg

/* Increase image size */
#define CONFIG_SYS_BOOTM_LEN    (64 << 20)

#ifdef CONFIG_RUN_FROM_DDR0

/* Flat device tree definitions */
#  ifdef CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR
#      define FDT_ADDR		0x82000000
#   else
#      define FDT_ADDR		0x83E00000
#endif

/*Kernel image load address */
#  define LOADADDR		0x80080000

/* Ramdisk load address */
#  define RAMDISK_ADDR		0x84000000
#else
/* Flat device tree definitions */
#  define FDT_ADDR		0xC3E00000

/*Kernel image load address */
#  define LOADADDR		0xC007FFC0

/* Ramdisk load address */
#  define RAMDISK_ADDR		0xC4000000
#endif

#if defined(CONFIG_SPI_FLASH) && defined(CONFIG_FSL_QSPI)

/* Flash Size and Num need to be updated according to the board's flash type */
#define FSL_QSPI_FLASH_SIZE            SZ_128M
#define FSL_QSPI_FLASH_NUM             2

#define QSPI0_BASE_ADDR                QSPI_BASE_ADDR
#define QSPI0_AMBA_BASE                CONFIG_SYS_FSL_FLASH0_BASE

#else

#ifndef CONFIG_S32_GEN1
/* S32V234 flash driver */
#define CONFIG_S32_FLASH
#endif

/* QSPI/hyperflash configs */

/* Debug stuff for qspi/hyperflash */
#undef CONFIG_DEBUG_S32_QSPI_QSPI

/* Flash comand disabled until implemented */
#undef CONFIG_CMD_FLASH

#define FLASH_BASE_ADR2		(CONFIG_SYS_FSL_FLASH0_BASE + 0x4000000)

#endif

#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_FSL_FLASH0_BASE

/* Flash booting */
#define UBOOT_FLASH_ADDR		CONFIG_SYS_FSL_FLASH0_BASE + 0x0
#define KERNEL_FLASH_ADDR		CONFIG_SYS_FSL_FLASH0_BASE + 0x100000
#define KERNEL_FLASH_MAXSIZE		0xA00000
#define FDT_FLASH_ADDR			CONFIG_SYS_FSL_FLASH0_BASE + 0xB00000
#define FDT_FLASH_MAXSIZE		0x100000
#define RAMDISK_FLASH_ADDR		CONFIG_SYS_FSL_FLASH0_BASE + 0xC00000
#define RAMDISK_FLASH_MAXSIZE		0x2000000

/* Generic Timer Definitions */
#if defined(CONFIG_SYS_ARCH_TIMER)
/* COUNTER_FREQUENCY value will be used at startup but will be replaced
 * if an older chip version is determined at runtime.
 */
#if defined(CONFIG_S32_GEN1)
/* FXOSC_CLK; this will be further divided by "GPR00[26:24] + 1"
 * Note: CONFIG_TARGET_S32G274AEVB is a per-board configuration, as the value of
 * FXOSC_CLK itself is board-specific.
 */
#define COUNTER_FREQUENCY		(40 * 1000 * 1000)
#elif defined(CONFIG_S32V234)
#define COUNTER_FREQUENCY               (10000000)     /* 10MHz*/
#define COUNTER_FREQUENCY_CUT1          (12000000)     /* 12MHz*/
#elif defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
#define COUNTER_FREQUENCY				(1000)		    /* 1Khz */
#endif
#elif defined(CONFIG_SYS_PIT_TIMER)
#define COUNTER_FREQUENCY               (133056128)     /* 133MHz */
#else
#error "Unknown System Timer"
#endif

/* Size of malloc() pool */
#ifdef CONFIG_RUN_FROM_IRAM_ONLY
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1 * 1024 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)
#endif

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_UART_PORT		(1)

#undef CONFIG_CMD_IMLS

/* Regarding S32G, some of these are already controlled (read: duplicated)
 * in the defconfig; others are unused throughout the arch, board or
 * platform code; others yet are still unnecessary because we only plan to
 * enable them later (e.g CONFIG_FEC_MXC/MII or CONFIG_CMD_I2C)
 */
#ifndef CONFIG_S32_GEN1
#ifndef CONFIG_FLASH_BOOT
#define CONFIG_SD_BOOT
#endif

#define CONFIG_SYS_FSL_ESDHC_NUM	1

#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC

/* Ethernet config */
#define CONFIG_FEC_MXC
#define CONFIG_MII

#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
#define CONFIG_FEC_MXC_PHYADDR (0x484a53)
#define CONFIG_BCM_DUPLEX_MODE	DUPLEX_FULL
#endif

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1	/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2	/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3	/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED	100000
#define CONFIG_SYS_I2C_SLAVE	0x8
#define CONFIG_SYS_SPD_BUS_NUM	0

#endif

#undef CONFIG_BOOTDELAY

#define CONFIG_BOOTDELAY	3

#define CONFIG_BOOTARGS_LOGLEVEL ""

#undef CONFIG_BOOTARGS
#define CONFIG_BOOTARGS		\
	"console=ttyLF" __stringify(CONFIG_FSL_LINFLEX_MODULE) "," __stringify(CONFIG_BAUDRATE) \
	" root=/dev/ram rw" CONFIG_BOOTARGS_LOGLEVEL " earlycon " \
	CONFIG_EXTRA_KERNEL_BOOT_ARGS
#define CONFIG_CMD_ENV

#define CONFIG_HWCONFIG

#ifdef CONFIG_CMD_BOOTI

/*
 * Enable CONFIG_USE_BOOTI if the u-boot enviroment variables
 * specific for boot method have to be defined for booti by default.
 */
#define CONFIG_USE_BOOTI
#ifdef CONFIG_USE_BOOTI
#define IMAGE_NAME Image
#define BOOT_MTD booti
#else
#define IMAGE_NAME uImage
#define BOOT_MTD bootm
#endif

#endif

#ifndef CONFIG_BOARD_EXTRA_ENV_SETTINGS
#define CONFIG_BOARD_EXTRA_ENV_SETTINGS	""
#endif

#ifndef CONFIG_DCU_EXTRA_ENV_SETTINGS
#define CONFIG_DCU_EXTRA_ENV_SETTINGS	""
#endif

/*
 * Enable CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT if u-boot should use a ramdisk
 * for nfsbooting.
 */
#ifdef CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT
#define NFSRAMFS_ADDR "${ramdisk_addr}"
#define NFSRAMFS_TFTP_CMD "run loadtftpramdisk; "
#else
#define NFSRAMFS_ADDR "-"
#define NFSRAMFS_TFTP_CMD ""
#endif

#ifdef CONFIG_DWC_ETH_QOS_S32CC
#define GMAC_EXTRA_ENV_SETTINGS \
	"s32cc_gmac_mode=enable\0"
#else
#define GMAC_EXTRA_ENV_SETTINGS ""
#endif

#ifdef CONFIG_FSL_PFENG
#define PFENG_EXTRA_BOOT_ARGS " nohz=off coherent_pool=64M "
#ifdef CONFIG_TARGET_S32G274AEVB
#define PFENG_MODE "enable,none,rgmii,rgmii"
#define PFENG_EMAC "1"
#endif
#ifdef CONFIG_TARGET_S32G274ARDB
#define PFENG_MODE "enable,sgmii,none,rgmii"
#define PFENG_EMAC "2"
#endif
#define PFE_EXTRA_ENV_SETTINGS \
	"pfeng_mode=" PFENG_MODE "\0" \
	"pfeaddr=00:01:be:be:ef:11\0" \
	"pfe1addr=00:01:be:be:ef:22\0" \
	"pfe2addr=00:01:be:be:ef:33\0" \
	"ethact=eth_pfeng\0" \
	"pfengemac=" PFENG_EMAC "\0"
#define PFE_INIT_CMD "pfeng stop; "
#else
#define PFENG_EXTRA_BOOT_ARGS ""
#define PFE_EXTRA_ENV_SETTINGS ""
#define PFE_INIT_CMD ""
#endif

#if defined(CONFIG_PCIE_S32GEN1) || defined(CONFIG_FSL_PFENG)
#define PCIE_EXTRA_ENV_SETTINGS \
	"hwconfig=" CONFIG_S32GEN1_HWCONFIG "\0"
#else
#define PCIE_EXTRA_ENV_SETTINGS ""
#endif

#define CONFIG_FLASHBOOT_RAMDISK " ${ramdisk_addr} "

#ifdef CONFIG_XEN_SUPPORT
#ifdef CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR
#define XEN_LOAD_FILES "setenv filesize a00000; "
#else
#define XEN_LOAD_FILES \
	"fatload mmc 0 ${fdt_addr} ${fdt_file}; " \
	"fatload mmc 0 ${loadaddr} xen; " \
	"fatload mmc 0 ${dom0_addr} Image; "
#endif
#define XEN_EXTRA_ENV_SETTINGS \
	"dom0_addr=0xbe800000\0" \
	"bootargs=dom0_mem=384M bootscrub=0\0" \
	"updatexenfdt=fdt addr ${fdt_addr} 0x40000; fdt resize; fdt chosen; " \
		"fdt set /chosen \\\\\#address-cells <1>; " \
		"fdt set /chosen \\\\\#size-cells <1>; " \
		"fdt mknod /chosen module@0; " \
		"fdt set /chosen/module@0 compatible \"xen,linux-zimage\" \"xen,multiboot-module\"; " \
		"fdt set /chosen/module@0 reg <${dom0_addr} 0x${filesize} >; " \
		"fdt set /chosen/module@0 bootargs \"console=ttyLF0,115200 root=/dev/mmcblk0p2 rootwait rw\" \0" \
	"bootcmd=" XEN_LOAD_FILES "run updatexenfdt; fdt rm /chosen stdout-path; " \
		"fdt rm /chosen linux,initrd-start; fdt rm /chosen linux,initrd-end; " \
		"booti ${loadaddr} - ${fdt_addr}\0"
#else
#define XEN_EXTRA_ENV_SETTINGS  ""
#endif

#ifdef CONFIG_S32_GEN1
#define LIMIT_DDR " run limit_ddr; "
#define LIMIT_DDR_CMD "limit_ddr=fdt addr ${fdt_addr} && fdt set /memory_DDR1 reg <0 0xc0000000 0 0x20000000>;\0"
#else
#define LIMIT_DDR ""
#define LIMIT_DDR_CMD ""
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_BOARD_EXTRA_ENV_SETTINGS  \
	CONFIG_DCU_EXTRA_ENV_SETTINGS \
	LIMIT_DDR_CMD \
	"ipaddr=10.0.0.100\0" \
	"serverip=10.0.0.1\0" \
	"netmask=255.255.255.0\0" \
	"nfsbootargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs rw " \
		"ip=${ipaddr}:${serverip}::${netmask}::eth0:off " \
		"nfsroot=${serverip}:/tftpboot/rfs,nolock,v3,tcp " \
		"earlycon " CONFIG_EXTRA_KERNEL_BOOT_ARGS \
		PFENG_EXTRA_BOOT_ARGS "\0" \
	"loadtftpimage=tftp ${loadaddr} ${image};\0" \
	"loadtftpramdisk=tftp ${ramdisk_addr} ${ramdisk};\0" \
	"loadtftpfdt=tftp ${fdt_addr} ${fdt_file};" LIMIT_DDR "\0" \
	"nfsboot=echo Booting from net using tftp and nfs...; " \
		"run nfsbootargs;"\
		"run loadtftpimage; " NFSRAMFS_TFTP_CMD "run loadtftpfdt;"\
		"${boot_mtd} ${loadaddr} " NFSRAMFS_ADDR " ${fdt_addr};\0" \
	"script=boot.scr\0" \
	"boot_mtd=" __stringify(BOOT_MTD) "\0" \
	"image=" __stringify(IMAGE_NAME) "\0" \
	"ramdisk=" __stringify(RAMDISK_NAME) "\0"\
	"console=ttyLF" __stringify(CONFIG_FSL_LINFLEX_MODULE) "\0" \
	"fdt_high=0xa0000000\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file="  __stringify(FDT_FILE) "\0" \
	"fdt_addr=" __stringify(FDT_ADDR) "\0" \
	"ramdisk_addr=" __stringify(RAMDISK_ADDR) "\0" \
	"boot_fdt=try\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(CONFIG_MMC_PART) "\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	CONFIG_FSL_CSE3_SETTINGS \
	"update_sd_firmware_filename=u-boot.s32\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x8 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " CONFIG_BOOTARGS_LOGLEVEL \
		" root=${mmcroot} earlycon " \
		CONFIG_EXTRA_KERNEL_BOOT_ARGS \
		PFENG_EXTRA_BOOT_ARGS "\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadramdisk=fatload mmc ${mmcdev}:${mmcpart} ${ramdisk_addr} ${ramdisk}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file};" LIMIT_DDR "\0" \
	"jtagboot=echo Booting using jtag...; " \
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"jtagsdboot=echo Booting loading Linux with ramdisk from SD...; " \
		"run loadimage; run loadramdisk; run loadfdt;"\
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"run loadimage; if run loadfdt; then " \
			"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp " \
		"earlycon " CONFIG_EXTRA_KERNEL_BOOT_ARGS \
		PFENG_EXTRA_BOOT_ARGS "\0" \
	"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"${boot_mtd}; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"${boot_mtd}; " \
		"fi;\0" \
	"flashbootargs=setenv bootargs console=${console}" \
		CONFIG_BOOTARGS_LOGLEVEL " root=/dev/ram rw earlycon " \
		CONFIG_EXTRA_KERNEL_BOOT_ARGS \
		PFENG_EXTRA_BOOT_ARGS ";" \
		"setexpr uboot_flashaddr " __stringify(UBOOT_FLASH_ADDR) ";" \
		"setexpr kernel_flashaddr " __stringify(KERNEL_FLASH_ADDR) ";" \
		"setenv kernel_maxsize " __stringify(KERNEL_FLASH_MAXSIZE) ";" \
		"setexpr fdt_flashaddr " __stringify(FDT_FLASH_ADDR) ";" \
		"setenv fdt_maxsize " __stringify(FDT_FLASH_MAXSIZE) ";" \
		"setexpr ramdisk_flashaddr " \
				__stringify(RAMDISK_FLASH_ADDR) ";" \
		"setenv ramdisk_maxsize " \
				__stringify(RAMDISK_FLASH_MAXSIZE) ";\0" \
	"flashboot=echo Booting from flash...; " \
		"run flashbootargs;"\
		"cp.b ${kernel_flashaddr} ${loadaddr} ${kernel_maxsize};"\
		"cp.b ${fdt_flashaddr} ${fdt_addr} ${fdt_maxsize};"\
		LIMIT_DDR\
		"cp.b ${ramdisk_flashaddr} ${ramdisk_addr} ${ramdisk_maxsize};"\
		"${boot_mtd} ${loadaddr}" CONFIG_FLASHBOOT_RAMDISK \
		"${fdt_addr};\0" \
	XEN_EXTRA_ENV_SETTINGS \
	GMAC_EXTRA_ENV_SETTINGS \
	PFE_EXTRA_ENV_SETTINGS \
	PCIE_EXTRA_ENV_SETTINGS

#undef CONFIG_BOOTCOMMAND

#if defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR) || \
	defined(CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR)
#  define CONFIG_BOOTCOMMAND \
		"${boot_mtd} ${loadaddr} - ${fdt_addr}"
#elif defined(CONFIG_FLASH_BOOT)
#  define CONFIG_BOOTCOMMAND \
	PFE_INIT_CMD "run flashboot"
#elif defined(CONFIG_SD_BOOT)
#  define CONFIG_BOOTCOMMAND \
	PFE_INIT_CMD "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadimage; then " \
			   "run mmcboot; " \
		   "else run netboot; " \
		   "fi; " \
	   "else run netboot; fi"
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		\
			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	(DDR_BASE_ADDR)
#define CONFIG_SYS_MEMTEST_END		(DDR_BASE_ADDR + (CONFIG_SYS_DDR_SIZE - 1))

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ				1000

#ifdef CONFIG_RUN_FROM_IRAM_ONLY
#define CONFIG_SYS_MALLOC_BASE		(DDR_BASE_ADDR)
#endif

/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(128 * 1024)	/* regular stack */

#if 0
/* Configure PXE */
#define CONFIG_CMD_PXE
#define CONFIG_BOOTP_PXE_CLIENTARCH	0x100
#endif

/* Physical memory map */
#define PHYS_SDRAM			(DDR_BASE_ADDR)
#define PHYS_SDRAM_SIZE			(CONFIG_SYS_DDR_SIZE)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#if (defined(CONFIG_FLASH_BOOT) && defined(CONFIG_SD_BOOT))
#error "CONFIG_FLASH_BOOT and CONFIG_SD_BOOT both defined"
#endif

#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_MMC_PART			1

#ifndef CONFIG_S32_GEN1
#define FLASH_SECTOR_SIZE		0x40000 /* 256 KB */
#endif

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH) || defined(CONFIG_ENV_IS_IN_FLASH)
#if defined(CONFIG_S32_GEN1) && defined(CONFIG_SPI_FLASH_MACRONIX)
#define FLASH_SECTOR_SIZE		(64 * 1024) /* 64 KB */
#define CONFIG_ENV_SECT_SIZE		FLASH_SECTOR_SIZE
#define CONFIG_ENV_SIZE 		FLASH_SECTOR_SIZE
#define CONFIG_ENV_OFFSET		(KERNEL_FLASH_ADDR - FLASH_SECTOR_SIZE)
#else /* CONFIG_S32_GEN1 && CONFIG_SPI_FLASH_MACRONIX */
#define CONFIG_ENV_SECT_SIZE		(FLASH_SECTOR_SIZE) /* 256 KB */
#define CONFIG_ENV_SIZE 		(0x2000) /* 8 KB */
#define CONFIG_ENV_OFFSET		2 * CONFIG_ENV_SECT_SIZE
#endif
#elif defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_SIZE			(0x2000) /* 8 KB */
#define CONFIG_ENV_OFFSET		(0x100000) /* 16 * 64 * 1024 */
#elif defined(CONFIG_ENV_IS_NOWHERE)
#define CONFIG_ENV_SIZE			(0x2000) /* 8 KB */
#endif

#if defined(CONFIG_FLASH_BOOT)
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT 	(0x4000000 / CONFIG_ENV_SECT_SIZE)
#endif

#define CONFIG_BOOTP_BOOTFILESIZE

#if !defined(CONFIG_S32_GEN1)
/* TODO: update S32V234 defconfigs so that definitions below to not apply
 * to all S32's or find a smarter way to make S32G and S32V PCI coexist
 */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCIE_S32V234
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#endif
#endif  /* !CONFIG_S32_GEN1 */

#define CONFIG_SYS_LDSCRIPT  "arch/arm/cpu/armv8/s32/u-boot.lds"

#endif /* __S32_H */
