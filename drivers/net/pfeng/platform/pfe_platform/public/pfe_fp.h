/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2019 NXP
 */
#ifndef PFE_FP_H
#define PFE_FP_H

#include "pfe_class.h"

void pfe_fp_init(void);
uint32_t pfe_fp_create_table(pfe_class_t *class, uint8_t rules_count);
uint32_t pfe_fp_table_write_rule(pfe_class_t *class, uint32_t table_address, pfe_ct_fp_rule_t *rule, uint8_t position);
void pfe_fp_destroy_table(pfe_class_t *class, uint32_t table_address);

#endif

