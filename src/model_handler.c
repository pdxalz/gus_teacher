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

/* Light switch behavior */

/** Context for a single light switch. */
struct button {
	/** Current light status of the corresponding server. */
	uint16_t status;
	/** Generic lvl client instance for this switch. */
	struct bt_mesh_lvl_cli client;
};

static void status_handler(struct bt_mesh_lvl_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_lvl_status *status);

static struct button buttons[MAX_GUS_NODES] = {
	[0 ... MAX_GUS_NODES-1] = { .client = BT_MESH_LVL_CLI_INIT(&status_handler) },
};


static int gus_badge_count(void)
{
int count = 0;
    for (int i=0; i<10; ++i) {
        printk(" %d, ", buttons[i].client.pub.addr);

        if (buttons[i].client.pub.addr != 0) {
            ++count;
        }
    }    return count;
}



static void status_handler(struct bt_mesh_lvl_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_lvl_status *status)
{
	struct button *button =
		CONTAINER_OF(cli, struct button, client);
	int index = button - &buttons[0];

	button->status = status->current;
	dk_set_led(index, status->current);

	printk("Button %d: Received response: %s\n", index + 1,
	       status->current ? "on" : "off");
}
uint16_t count=0;
static void button_handler_cb(uint32_t pressed, uint32_t changed)
{
	if (!bt_mesh_is_provisioned()) {
		return;
	}

	for (int i = 0; i < 4; ++i) {
		if (!(pressed & changed & BIT(i))) {
			continue;
		}

                if (i==3) {
                    printk("mesh count %d\n", gus_badge_count());
                }

              count+= 0x2000;
		struct bt_mesh_lvl_set set = {
//			.lvl = !buttons[i].status,
			.lvl = count,
		};
 //               printk("lvl=%x  #elem %d\n",set.lvl, bt_mesh_elem_count());

		int err;

		/* As we can't know how many nodes are in a group, it doesn't
		 * make sense to send acknowledged messages to group addresses -
		 * we won't be able to make use of the responses anyway.
		 */
		if (bt_mesh_model_pub_is_unicast(buttons[i].client.model)) {
			err = bt_mesh_lvl_cli_set(&buttons[i].client, NULL,
						    &set, NULL);
		} else {
			err = bt_mesh_lvl_cli_set_unack(&buttons[i].client,
							  NULL, &set);
			if (!err) {
				/* There'll be no response status for the
				 * unacked message. Set the state immediately.
				 */
				buttons[i].status = set.lvl;
				dk_set_led(i, set.lvl);
			}
		}

		if (err) {
			printk("lvl %d set failed: %d\n", i + 1, err);
		}
	}
}

/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */
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

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);

static struct bt_mesh_elem elements[MAX_GUS_NODES] = {
	BT_MESH_ELEM(1,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_CFG_SRV,
			     BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
			     BT_MESH_MODEL_LVL_CLI(&buttons[0].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(2,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[1].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(3,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[2].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(4,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[3].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(5,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[4].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(6,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[5].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(7,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[6].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(8,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[7].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(9,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[8].client)),
		     BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(10,
		     BT_MESH_MODEL_LIST(
			     BT_MESH_MODEL_LVL_CLI(&buttons[9].client)),
		     BT_MESH_MODEL_NONE),

};

static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	static struct button_handler button_handler = {
		.cb = button_handler_cb,
	};

	dk_button_handler_add(&button_handler);
	k_delayed_work_init(&attention_blink_work, attention_blink);

	return &comp;
}
