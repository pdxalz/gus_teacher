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

// initialize the model handler
const struct bt_mesh_comp *model_handler_init(void);

// Sends a message to the badge to set its state
// addr - node address of the badge
// state - new gus state
void model_handler_set_state(uint16_t addr, enum bt_mesh_gus_state state);

// Sends a sign-in message to all badges to retreive active names and addresses
void model_scan_for_badges(void);

// SEnds a message to assign a new name to a badge
void model_set_name(uint16_t addr, const uint8_t *name);

// Sends a message to request a contact report from a badge
// addr - node address of the badge
void model_report_request(uint16_t addr);


#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
