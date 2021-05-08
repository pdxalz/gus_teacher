/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <bluetooth/mesh.h>
#include "gus_cli.h"
#include "mesh/net.h"
#include "mesh/transport.h"
#include <string.h>
#include <logging/log.h>
#include <drivers/gpio.h>

LOG_MODULE_DECLARE(gus);

BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_GUS_OP_SET_NAME,
				   CONFIG_BT_MESH_GUS_NAME_LENGTH) <=
		    BT_MESH_RX_SDU_MAX,
	     "The message must fit inside an application SDU.");
BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_GUS_OP_SET_NAME,
				   CONFIG_BT_MESH_GUS_NAME_LENGTH) <=
		    BT_MESH_TX_SDU_MAX,
	     "The message must fit inside an application SDU.");
BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_GUS_OP_REPORT_REPLY,
				   BT_MESH_GUS_MSG_LEN_REPORT_REPLY) <=
		    BT_MESH_RX_SDU_MAX,
	     "The report reply message must fit inside an application SDU.");
BUILD_ASSERT(BT_MESH_MODEL_BUF_LEN(BT_MESH_GUS_OP_REPORT_REPLY,
				   BT_MESH_GUS_MSG_LEN_REPORT_REPLY) <=
		    BT_MESH_TX_SDU_MAX,
	     "The report reply message must fit inside an application SDU.");


static const uint8_t *extract_name(struct net_buf_simple *buf)
{
	buf->data[buf->len - 1] = '\0';
	return net_buf_simple_pull_mem(buf, buf->len);
}


////////////////////
// message handlers
///////////////////

static void handle_sign_in_reply(struct bt_mesh_model *model,
			   struct bt_mesh_msg_ctx *ctx,
			   struct net_buf_simple *buf)
{
	struct bt_mesh_gus *gus = model->user_data;
	const uint8_t *name;

	name = extract_name(buf);

	if (gus->handlers->sign_in_reply) {
		gus->handlers->sign_in_reply(gus, ctx, name);
	}
}

static void handle_report_reply(struct bt_mesh_model *model,
				  struct bt_mesh_msg_ctx *ctx,
				  struct net_buf_simple *buf)
{
	struct bt_mesh_gus *gus = model->user_data;
	const uint8_t *msg;

	msg = extract_name(buf);  //todo extract report

	if (gus->handlers->report_reply) {
		gus->handlers->report_reply(gus, ctx, msg);
	}
}

////////////////////
// message handler table
///////////////////


const struct bt_mesh_model_op _bt_mesh_gus_cli_op[] = {
	{
		BT_MESH_GUS_OP_SIGN_IN_REPLY,
		BT_MESH_GUS_MSG_MINLEN_MESSAGE,
		handle_sign_in_reply
	},
	{
		BT_MESH_GUS_OP_REPORT_REPLY,
		BT_MESH_GUS_MSG_MINLEN_MESSAGE,
		handle_report_reply
	},
	BT_MESH_MODEL_OP_END,
};





//todo hopefully not needed

// static int bt_mesh_chat_cli_update_handler(struct bt_mesh_model *model)
// {
	// struct bt_mesh_chat_cli *chat = model->user_data;

	// /* Continue publishing current presence. */
	// encode_presence(model->pub->msg, chat->presence);

	// return 0;
// }


#ifdef CONFIG_BT_SETTINGS
static int bt_mesh_gus_cli_settings_set(struct bt_mesh_model *model,
					 const char *name,
					 size_t len_rd,
					 settings_read_cb read_cb,
					 void *cb_arg)
{
	struct bt_mesh_gus *gus = model->user_data;

	if (name) {
		return -ENOENT;
	}

	ssize_t bytes = read_cb(cb_arg, &gus->state,
				sizeof(gus->state));
	if (bytes < 0) {
		return bytes;
	}

	if (bytes != 0 && bytes != sizeof(gus->state)) {
		return -EINVAL;
	}

	return 0;
}
#endif



static int bt_mesh_gus_cli_init(struct bt_mesh_model *model)
{
	struct bt_mesh_gus *gus = model->user_data;

	gus->model = model;

        net_buf_simple_init_with_data(&gus->pub_msg, gus->buf,
				      sizeof(gus->buf));

	gus->pub.msg = &gus->pub_msg;
	gus->pub.update = NULL; //bt_mesh_gus_cli_update_handler;

	return 0;
}


static int bt_mesh_gus_cli_start(struct bt_mesh_model *model)
{
	struct bt_mesh_gus *gus = model->user_data;

	if (gus->handlers->start) {
		gus->handlers->start(gus);
	}

	return 0;
}


static void bt_mesh_gus_cli_reset(struct bt_mesh_model *model)
{
	struct bt_mesh_gus *gus = model->user_data;

	gus->state = BT_MESH_GUS_HEALTHY;

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		(void) bt_mesh_model_data_store(model, true, NULL, NULL, 0);
	}
}



const struct bt_mesh_model_cb _bt_mesh_gus_cli_cb = {
	.init = bt_mesh_gus_cli_init,
	.start = bt_mesh_gus_cli_start,
#ifdef CONFIG_BT_SETTINGS
	.settings_set = bt_mesh_gus_cli_settings_set,
#endif
	.reset = bt_mesh_gus_cli_reset,
};


/////////////////////////////
// public access functions
/////////////////////////////

int bt_mesh_gus_cli_sign_in(struct bt_mesh_gus *gus)
{
	struct net_buf_simple *buf = gus->model->pub->msg;
	bt_mesh_model_msg_init(buf, BT_MESH_GUS_OP_SIGN_IN);
	return bt_mesh_model_publish(gus->model);	
}

int bt_mesh_gus_cli_sign_in_reply(struct bt_mesh_gus *gus, 
                                    struct bt_mesh_msg_ctx *ctx, 
                                    const uint8_t * name)
{
	BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_GUS_OP_SIGN_IN_REPLY,
				 BT_MESH_GUS_MSG_LEN_SIGN_IN_REPLY);
	bt_mesh_model_msg_init(&msg, BT_MESH_GUS_OP_SIGN_IN_REPLY);

        size_t len = strlen(name);
        net_buf_simple_add_mem(&msg, name, len);
       	net_buf_simple_add_u8(&msg, '\0');

	return bt_mesh_model_send(gus->model, ctx, &msg, NULL, NULL);
}

int bt_mesh_gus_cli_state_set(struct bt_mesh_gus *gus,
				  uint16_t addr,
				  enum bt_mesh_gus_state state)
{				  
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = gus->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = false,  //todo if true, causes No matching TX context warning
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_GUS_OP_SET_STATE,
				 BT_MESH_GUS_MSG_LEN_SET_STATE);
	bt_mesh_model_msg_init(&buf, BT_MESH_GUS_OP_SET_STATE);
	net_buf_simple_add_u8(&buf, state);

	return bt_mesh_model_send(gus->model, &ctx, &buf, NULL, NULL);
}

int bt_mesh_gus_cli_name_set(struct bt_mesh_gus *gus,
				  uint16_t addr,
				  const uint8_t *name)
{
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = gus->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = false,  //todo
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_GUS_OP_SET_NAME,
				 CONFIG_BT_MESH_GUS_NAME_LENGTH);
	bt_mesh_model_msg_init(&buf, BT_MESH_GUS_OP_SET_NAME);

	net_buf_simple_add_mem(&buf, name, strlen(name));
			       //strnlen(name,
				      // CONFIG_BT_MESH_GUS_CLI_MESSAGE_LENGTH));
	net_buf_simple_add_u8(&buf, '\0');

	return bt_mesh_model_send(gus->model, &ctx, &buf, NULL, NULL);  
}


int bt_mesh_gus_cli_report_request(struct bt_mesh_gus *gus,
				  uint16_t addr)
{
	struct bt_mesh_msg_ctx ctx = {
		.addr = addr,
		.app_idx = gus->model->keys[0],
		.send_ttl = BT_MESH_TTL_DEFAULT,
		.send_rel = false,  //todo
	};

	BT_MESH_MODEL_BUF_DEFINE(buf, BT_MESH_GUS_OP_REPORT,
				 BT_MESH_GUS_MSG_LEN_REQUEST);
	bt_mesh_model_msg_init(&buf, BT_MESH_GUS_OP_REPORT);

	return bt_mesh_model_send(gus->model, &ctx, &buf, NULL, NULL);	
}

int bt_mesh_gus_cli_report_reply(struct bt_mesh_gus *gus,
				  struct bt_mesh_msg_ctx *ctx, 
                                  const uint8_t *report)
{
    BT_MESH_MODEL_BUF_DEFINE(msg, BT_MESH_GUS_OP_REPORT_REPLY,
                             BT_MESH_GUS_MSG_LEN_REPORT_REPLY);
    bt_mesh_model_msg_init(&msg, BT_MESH_GUS_OP_REPORT_REPLY);

    net_buf_simple_add_mem(&msg, report, BT_MESH_GUS_MSG_LEN_REPORT_REPLY);

    return bt_mesh_model_send(gus->model, ctx, &msg, NULL, NULL);
}


int bt_mesh_gus_cli_check_proximity(struct bt_mesh_gus *gus)
{
//todo	set_tx_power(BT_HCI_VS_LL_HANDLE_TYPE_ADV, 0, -8);

	struct net_buf_simple *buf = gus->model->pub->msg;
	bt_mesh_model_msg_init(buf, BT_MESH_GUS_OP_CHECK_PROXIMITY);
        gus->model->pub->ttl = 0;
        gus->model->pub->send_rel = false;
	return bt_mesh_model_publish(gus->model);	
}
