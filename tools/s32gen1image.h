/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2020 NXP */

#ifndef S32GEN1IMAGE_H
#define S32GEN1IMAGE_H

#include <asm/types.h>
#include <generated/autoconf.h>

#define BCW_BOOT_SEQ			(1 << 3)
#define BCW_SWT				(1 << 2)
#define BCW_BOOT_TARGET_M7_0		(0)
#define BCW_BOOT_TARGET_A53_0		(1 << 0)

#define LCCW_IN_FIELD			(1 << 1)
#define LCCW_OEM_PROD			(1 << 0)

#define DCD_WRITE_COMMAND_TAG		0xcc
#define DCD_CHECK_COMMAND_TAG		0xcf
#define DCD_NOP_COMMAND_TAG		0xc0

#define DCD_COMMAND_PARAMS_DATA_SET	(1 << 4)
#define DCD_COMMAND_PARAMS_DATA_MASK	(1 << 3)

#define IVT_TAG				0xd1
#define IVT_VERSION			0x60
#define DCD_TAG				0xd2
#define DCD_VERSION			0x60
#define APPLICATION_BOOT_CODE_TAG	0xd5
#define APPLICATION_BOOT_CODE_VERSION	0x60

#define SRAM_RESERVED_0_START		0x34008050
#define SRAM_RESERVED_0_END		0x34008200
#define SRAM_RESERVED_1_START		0x38008050
#define SRAM_RESERVED_1_END		0x38008200

#define DCD_MAXIMUM_SIZE		8192

#define S32GEN1_QSPI_PARAMS_SIZE	0x200

enum dcd_command_type {
	INVALID = -1,
	WRITE_DATA,
	WRITE_SET_BITMASK,
	WRITE_CLEAR_BITMASK,
	CHECK_BITS_ARE_SET,
	CHECK_BITS_ARE_CLEAR,
	CHECK_ANY_BIT_IS_SET,
	CHECK_ANY_BIT_IS_CLEAR,
	NOP,
};

struct dcd_command {
	__u8	tag;
	__u16	length;
	__u8	params;
	union specific {
		struct check_command_specific {
			__u32	address;
			__u32	mask;
			__u32	count;
		} check;
		struct write_command_specific {
			__u32	addr;
			__u32	data;
		} write[0];
	} s;
} __attribute__((packed));

struct dcd {
	__u8	tag;
	__u16	length;
	__u8	version;
	__u8	commands[DCD_MAXIMUM_SIZE - 4 - 16];
	__u8	gmac[16];
} __attribute__((packed));

struct ivt {
	__u8		tag;
	__u16		length;
	__u8		version;
	__u8		reserved1[4];
	__u32		self_test_dcd_pointer;
	__u32		self_test_dcd_pointer_backup;
	__u32		dcd_pointer;
	__u32		dcd_pointer_backup;
	__u32		hse_h_firmware_pointer;
	__u32		hse_h_firmware_pointer_backup;
	__u32		application_boot_code_pointer;
	__u32		application_boot_code_pointer_backup;
	__u32		boot_configuration_word;
	__u32		lifecycle_configuration_word;
	__u8		reserved2[4];
	__u8		reserved_for_hse_h_fw[32];
	__u8		reserved3[156];
	__u32		gmac[4];
} __attribute__((packed));

struct application_boot_code {
	__u8		tag;
	__u8		reserved1[2];
	__u8		version;
	__u32		ram_start_pointer;
	__u32		ram_entry_pointer;
	__u32		code_length;
	__u32		auth_mode;
	__u8		reserved2[44];
	__u8		code[0];
} __attribute__((packed));

struct image_comp {
	size_t offset;
	size_t size;
	size_t alignment;
	uint8_t *data;
};

#ifdef CONFIG_FLASH_BOOT
struct flash_write {
	union {
		struct {
			__u32 opcode:8;
			__u32 reserved:8;
			__u32 pad:2;
			__u32 addr_size:6;
			__u32 cdata_size:7;

			__u32 valid_addr:1;
		} config;
		__u32 _config;
	};
	__u32 addr;
	__u32 data;
} __packed;

struct qspi_params {
	__u32 header;
	__u32 mcr;
	__u32 flshcr;
	__u32 bufgencr;
	__u32 dllcr;
	__u32 paritycr;
	__u32 sfacr;
	__u32 smpr;
	__u32 dlcr;
	__u32 sflash_1_size;
	__u32 sflash_2_size;
	__u32 dlpr;
	__u32 sfar;
	__u32 ipcr;
	__u32 tbdr;
	__u8 dll_bypass_en;
	__u8 dll_slv_upd_en;
	__u8 dll_auto_upd_en;
	__u8 ipcr_trigger_en;
	__u8 sflash_clk_freq;
	__u8 reserved[3];
	__u32 command_seq[80];
	struct flash_write writes[10];
};
#endif

struct program_image {
	struct image_comp ivt;
#ifdef CONFIG_FLASH_BOOT
	struct image_comp qspi_params;
#endif
	struct image_comp dcd;
	struct image_comp app_code;
	__u8 *header;
};

#endif /* S32GEN1IMAGE_H */
