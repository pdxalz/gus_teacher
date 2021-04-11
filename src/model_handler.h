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

#ifdef __cplusplus
extern "C" {
#endif

typedef enum gus_state {
    GUS_ST_IDENTIFY = 0x0000, 
    GUS_ST_HEALTHY = 0x2000, 
    GUS_ST_INFECTED = 0x4000,
    GUS_ST_MASKED = 0x6000, 
    GUS_ST_VACCINE = 0x8000} gus_state_t;

const struct bt_mesh_comp *model_handler_init(void);
void model_handler_provision(void);
void model_handler_set_state(uint16_t index, gus_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
