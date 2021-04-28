/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
/**
 * @file
 * @brief Model handler for the light switch.
 *
 * Instantiates a Generic lvl Client model for each button on the devkit, as
 * well as the standard Config and Health Server models. Handles all application
 * behavior related to the models.
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"
#include <bluetooth/mesh/access.h>
#include "gus_config.h"
#include "gus_data.h"
#include "gus_cli.h"


#define STATE_CACHE_SIZE 10


static struct k_delayed_work attention_blink_work;

static void attention_blink(struct k_work *work)
{
	static int idx;
	const uint8_t pattern[] = {
		BIT(0) | BIT(1),
		BIT(1) | BIT(2),
		BIT(2) | BIT(3),
		BIT(3) | BIT(0),
	};

	dk_set_leds(pattern[idx++ % ARRAY_SIZE(pattern)]);
	k_delayed_work_submit(&attention_blink_work, K_MSEC(30));
}

static void attention_on(struct bt_mesh_model *mod)
{
	k_delayed_work_submit(&attention_blink_work, K_NO_WAIT);
}

static void attention_off(struct bt_mesh_model *mod)
{
	k_delayed_work_cancel(&attention_blink_work);
	dk_set_leds(DK_NO_LEDS_MSK);
}


static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
    if ((pressed & changed & BIT(0)))
    {
        model_handler_set_state(3, 1);
    } else  if ((pressed & changed & BIT(1)))
    {
        model_handler_set_state(7, 1);
    } else  if ((pressed & changed & BIT(2)))
    {
        //model_handler_set_state(4, 2);
        model_scan_for_badges();
    } else  if ((pressed & changed & BIT(3)))
    {
        model_handler_set_state(0xc000, 3);
    }

}
//////////////////////////////
//  Health server
//////////////////////////////
static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);



// ******************************************************************************
// ***************************** GUS model setup *******************************
// ******************************************************************************

struct state_cache {
	uint16_t addr;
	enum bt_mesh_gus_cli_state state;
};


//static struct state_cache state_cache[ STATE_CACHE_SIZE];


//static bool address_is_local(struct bt_mesh_model *mod, uint16_t addr)
//{
//	return bt_mesh_model_elem(mod)->addr == addr;
//}

//static bool address_is_unicast(uint16_t addr)
//{
//	return (addr > 0) && (addr <= 0x7FFF);
//}


//static bool presence_cache_entry_check_and_update(uint16_t addr,
//				       enum bt_mesh_chat_cli_presence presence)

static void handle_gus_start(struct bt_mesh_gus_cli *gus)
{
    printk("started gus\n");
	
}

static void handle_gus_set_state(struct bt_mesh_gus_cli *gus,
				 struct bt_mesh_msg_ctx *ctx,
				 enum bt_mesh_gus_cli_state state)
{

}


static void handle_gus_sign_in_reply(struct bt_mesh_gus_cli *gus,
				 struct bt_mesh_msg_ctx *ctx,
				 const uint8_t *msg)
{
    gd_add_node(msg, ctx->addr, false, false, false);
}


static const struct bt_mesh_gus_cli_handlers gus_handlers = {
	.start = handle_gus_start,
	.set_state = handle_gus_set_state,
        .sign_in_reply = handle_gus_sign_in_reply,
};

static struct bt_mesh_gus_cli gus = {
	.handlers = &gus_handlers,
};


static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(
		1,
		BT_MESH_MODEL_LIST(
			BT_MESH_MODEL_CFG_SRV,
			BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub)),
		BT_MESH_MODEL_LIST(BT_MESH_MODEL_GUS_CLI(&gus))),
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};





void model_handler_set_state(uint16_t addr, enum bt_mesh_gus_cli_state state)
{
    int err;

//    printk("identify %d\n", addr);

    if (addr == 0) {
        addr = 0xc000;   //todo
    }

    err = bt_mesh_gus_cli_state_set(&gus, addr, state);
    if (err) {
        printk("error set state %d %d failed %d\n", addr,state, err);
    }
}



void model_scan_for_badges(void)
{
    int err;
    err = bt_mesh_gus_cli_sign_in(&gus);
    if (err) {
        printk("sign in failed %d\n", err);
    }
}

void model_handler_provision(void)
{
    model_scan_for_badges();
}

void model_set_name(uint16_t addr, const uint8_t *name)
{
    int err;
    err = bt_mesh_gus_cli_name_set(&gus, addr, name);
    if (err) {
        printk("set name %d %s failed %d\n", addr, name, err);
    }
}


/******************************************************************************/
/******************************** Public API **********************************/
/******************************************************************************/
const struct bt_mesh_comp *model_handler_init(void)
{
	static struct button_handler button_handler = {
		.cb = button_handler_cb,
	};

	dk_button_handler_add(&button_handler);
	k_delayed_work_init(&attention_blink_work, attention_blink);

	return &comp;
}

