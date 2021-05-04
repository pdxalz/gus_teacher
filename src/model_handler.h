/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/**
 * @file
 * @brief Model handler
 */

#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <bluetooth/mesh.h>
#include "gus_cli.h"
#ifdef __cplusplus
extern "C" {
#endif


const struct bt_mesh_comp *model_handler_init(void);
void model_handler_provision(void);
void model_handler_set_state(uint16_t index, enum bt_mesh_gus_cli_state state);
void model_scan_for_badges(void);
void model_set_name(uint16_t addr, const uint8_t *name);
void model_report_request(uint16_t addr);


#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
