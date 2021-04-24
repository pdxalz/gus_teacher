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
    if ((pressed & changed & BIT(1)))
    {
        model_handler_set_state(3, 1);
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


static struct state_cache state_cache[ STATE_CACHE_SIZE];


static bool address_is_local(struct bt_mesh_model *mod, uint16_t addr)
{
	return bt_mesh_model_elem(mod)->addr == addr;
}

static bool address_is_unicast(uint16_t addr)
{
	return (addr > 0) && (addr <= 0x7FFF);
}


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
    printk("set state:  %x %d\n", ctx->addr, state);

    /*
	if (address_is_local(chat->model, ctx->addr)) {
		if (address_is_unicast(ctx->recv_dst)) {
			shell_print(chat_shell, "<you> are %s",
				    presence_string[presence]);
		}
	} else {
		if (address_is_unicast(ctx->recv_dst)) {
			shell_print(chat_shell, "<0x%04X> is %s", ctx->addr,
				    presence_string[presence]);
		} else if (presence_cache_entry_check_and_update(ctx->addr,
								 presence)) {
			shell_print(chat_shell, "<0x%04X> is now %s",
				    ctx->addr,
				    presence_string[presence]);
		}
	}
        */
}


static const struct bt_mesh_gus_cli_handlers gus_handlers = {
	.start = handle_gus_start,
	.set_state = handle_gus_set_state,
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





void model_handler_set_state(uint16_t addr, gus_state_t state)
{
    int err;

    enum bt_mesh_gus_cli_state gus_state = state;
        printk("identify %d\n", addr);
    err = bt_mesh_gus_cli_state_set(&gus, addr, gus_state);
    if (err) {
        printk("identify %d %d failed %d\n", addr,state, err);
    }
}





void model_handler_provision(void)
{
    char * names[] = {
    "Alan",
    "Ally", 
    "Brenda",
    "Bryan",
    "Carol",
    "Craig",
    "Dalene",
    "Darrell",
    "Eric"
    };
    // Gus provisioning isn't working yet.  This function is a placeholder. 
    // For now it just adds an entry in the gus data for each node. 
    gd_init();
    uint16_t svr_addr = 2;
    for (int i=0; i<3; ++i) {
       gd_add_node(i, names[i], svr_addr, i==0, false, false);
       ++svr_addr;
        
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

#if 0
/* Light switch behavior */

/** Context for a single badge. */
struct button {
	/** Current light status of the corresponding server. */
	uint16_t status;
	/** Generic lvl client instance for this switch. */
};

//struct bt_mesh_lvl_cli client = BT_MESH_LVL_CLI_INIT(&status_handler);
struct bt_mesh_gus_cli client = BT_MESH_LVL_CLI_INIT(&status_handler);


static void status_handler(struct bt_mesh_lvl_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_lvl_status *status);

static struct button button;

static int gus_badge_count(void)
{
    return 4;
}



static void status_handler(struct bt_mesh_lvl_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_lvl_status *status)
{

}


uint16_t count=0;
static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
/*
	if (!bt_mesh_is_provisioned()) {
		return;
	}


        if (!(pressed & changed & BIT(1))) {
                return;
        }

        struct bt_mesh_lvl_set set = {
                .lvl = count,
        };
        int err;


        if (bt_mesh_model_pub_is_unicast(button.client.model)) {
                err = bt_mesh_lvl_cli_set(&button.client, NULL,
                                            &set, NULL);
        } else {
                err = bt_mesh_lvl_cli_set_unack(&button.client,
                                                  NULL, &set);
                if (!err) {
                        
                        button.status = set.lvl;
                       
                            dk_set_led(1, set.lvl);
                        
                }
        }

        if (err) {
                printk("lvl set failed: %d\n", err);
        }
*/
}

/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */


static struct bt_mesh_elem elements[1] = {
	BT_MESH_ELEM(1,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_CFG_SRV,
			     BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
			     BT_MESH_MODEL_LVL_CLI(&button.client)),
		     BT_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = 1,
};



#endif
