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
#include "simulate.h"
#include "gui.h"

#define STATE_CACHE_SIZE 10

static struct k_delayed_work attention_blink_work;

// Blink leds in response to provisioning
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

// Receives button events.
// Not used, but useful to software testing
static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
    if ((pressed & changed & BIT(0)))
    {
        //        model_report_request(gui_get_selected_addr());
        //        model_handler_set_state(3, 1);
    }
    else if ((pressed & changed & BIT(1)))
    {
        //        model_handler_set_state(gui_get_selected_addr(), 6);
    }
    else if ((pressed & changed & BIT(2)))
    {
        //model_handler_set_state(4, 2);
        //       model_handler_set_state(gui_get_selected_addr(), 2);
    }
    else if ((pressed & changed & BIT(3)))
    {
        //        model_handler_set_state(0xc000, 3);
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

////////////////////
// GUS model setup 
////////////////////

struct state_cache
{
    uint16_t addr;
    enum bt_mesh_gus_state state;
};

// todo
//static struct state_cache state_cache[ STATE_CACHE_SIZE];

//static bool presence_cache_entry_check_and_update(uint16_t addr,
//				       enum bt_mesh_chat_cli_presence presence)

// handle start message
static void handle_gus_start(struct bt_mesh_gus *gus)
{
    printk("started gus\n");
}

// handle sign in reply - save name and address in gus data
static void handle_gus_sign_in_reply(struct bt_mesh_gus *gus,
                                     struct bt_mesh_msg_ctx *ctx,
                                     const uint8_t *msg)
{
    add_new_badge(msg, ctx->addr, false, false, false);
}

// handle report reply
// saves to the list of contacts the addresses of the reporting node
// and the node of the contact, and also the rssi signal level when
// the contact happened
static void handle_gus_report_reply(struct bt_mesh_gus *gus,
                                    struct bt_mesh_msg_ctx *ctx,
                                    const uint8_t *msg)
{
    struct gus_report_data *dd = (struct gus_report_data *)msg;
    uint16_t badgeA = ctx->addr;
    for (int i = 0; i < NUM_PROXIMITY_REPORTS; ++i)
    {
        uint16_t badgeB = dd[i + 0].addr;
        int8_t rssi = dd[i + 0].rssi;

        if (rssi > -90)
        {
            sim_msg_add_contact(badgeA, badgeB, rssi);
        }
    }
}

static const struct bt_mesh_gus_handlers gus_handlers = {
    .start = handle_gus_start,
    .sign_in_reply = handle_gus_sign_in_reply,
    .report_reply = handle_gus_report_reply,
};

static struct bt_mesh_gus gus = {
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


/////////////////////
// global functions
/////////////////////
void model_handler_set_state(uint16_t addr, enum bt_mesh_gus_state state)
{
    int err;

    // if addr is 0, the message is published to all badges
    if (addr == 0)
    {
        addr = 0xc000; //todo need to find the address of the group
    }

    err = bt_mesh_gus_cli_state_set(&gus, addr, state);
    if (err)
    {
        printk("error set state %d %d failed %d\n", addr, state, err);
    }
}

void model_scan_for_badges(void)
{
    int err;
    err = bt_mesh_gus_cli_sign_in(&gus);
    if (err)
    {
        printk("sign in failed %d\n", err);
    }
}

void model_set_name(uint16_t addr, const uint8_t *name)
{
    int err;
    err = bt_mesh_gus_cli_name_set(&gus, addr, name);
    if (err)
    {
        printk("set name %d %s failed %d\n", addr, name, err);
    }
}

void model_report_request(uint16_t addr)
{
    int err;

    err = bt_mesh_gus_cli_report_request(&gus, addr);
    if (err)
    {
        printk("report request %d failed %d\n", addr, err);
    }
}

const struct bt_mesh_comp *model_handler_init(void)
{
    static struct button_handler button_handler = {
        .cb = button_handler_cb,
    };

    dk_button_handler_add(&button_handler);
    k_delayed_work_init(&attention_blink_work, attention_blink);

    return &comp;
}
