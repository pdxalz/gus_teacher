/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

//////////////////////////////////////////////////////////////////////////////
// GUS Client model - API for the Bluetooth Mesh Gus Client model.
// This code originated from the Nordic Bluetooth mesh chat sample.
//
// Messages sent:
// Sign-in - publishes a message to all nodes requesting the node reply with their
//     name and it provides a way for the teacher to discover the node
//     addresses of all active badges in the mesh network.
// Set state - sends a message or to the whole group to change the state
// Set name - allows the teacher to rename the badge
// Report - sends a message to request a report of all contacts a badge has had
// 
// Message handlers
// Sign-in reply gets the name and node address of a badge
// Report reply gets a list of contacts a badge has had recently
//////////////////////////////////////////////////////////////////////////////

#ifndef BT_MESH_GUS_CLI_H__
#define BT_MESH_GUS_CLI_H__

#include <bluetooth/mesh.h>
#include <bluetooth/mesh/model_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONFIG_BT_MESH_GUS_NAME_LENGTH 12   // max length of a name
#define NUM_PROXIMITY_REPORTS 6             // number of proximity records
                                            // sent in a message

/** Company ID of the Bluetooth Mesh Gus Client model. */
#define BT_MESH_GUS_VENDOR_COMPANY_ID    0xFFFF  // not a real company

/** Model ID of the Bluetooth Mesh Gus Client model. */
#define BT_MESH_GUS_VENDOR_MODEL_ID      0x0042 // answer to life, universe & everything

/** Sign in opcode. */
#define BT_MESH_GUS_OP_SIGN_IN BT_MESH_MODEL_OP_3(0x04, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

/** Sign in reply opcode. */
#define BT_MESH_GUS_OP_SIGN_IN_REPLY BT_MESH_MODEL_OP_3(0x05, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

/** Set State opcode. */
#define BT_MESH_GUS_OP_SET_STATE BT_MESH_MODEL_OP_3(0x06, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

/** Set name opcode. */
#define BT_MESH_GUS_OP_SET_NAME BT_MESH_MODEL_OP_3(0x07, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

/**  Report opcode. */
#define BT_MESH_GUS_OP_REPORT BT_MESH_MODEL_OP_3(0x08, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

/** Report reply opcode. */
#define BT_MESH_GUS_OP_REPORT_REPLY BT_MESH_MODEL_OP_3(0x09, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

/** Check proximity opcode. */
#define BT_MESH_GUS_OP_CHECK_PROXIMITY BT_MESH_MODEL_OP_3(0x0A, \
				       BT_MESH_GUS_VENDOR_COMPANY_ID)

struct gus_report_data {
    uint16_t addr;
    int8_t rssi;
    }; 


#define BT_MESH_GUS_MSG_MINLEN_MESSAGE 1
#define BT_MESH_GUS_MSG_MAXLEN_MESSAGE (\
				     CONFIG_BT_MESH_GUS_NAME_LENGTH \
				     + 1) /* + \0 */
#define BT_MESH_GUS_MSG_LEN_SIGN_IN_REPLY (CONFIG_BT_MESH_GUS_NAME_LENGTH+1)
#define BT_MESH_GUS_MSG_LEN_SET_STATE 1
#define BT_MESH_GUS_MSG_LEN_REPORT_REPLY (NUM_PROXIMITY_REPORTS*sizeof(struct gus_report_data)+1)
#define BT_MESH_GUS_MSG_LEN_REQUEST 0


/** Bluetooth Mesh Gus state values. */
enum bt_mesh_gus_state {
	BT_MESH_GUS_IDENTIFY,
	BT_MESH_GUS_HEALTHY,
	BT_MESH_GUS_INFECTED,
	BT_MESH_GUS_MASKED,
	BT_MESH_GUS_MASKED_INFECTED,
	BT_MESH_GUS_VACCINATED,
	BT_MESH_GUS_VACCINATED_INFECTED,
	BT_MESH_GUS_VACCINATED_MASKED,
	BT_MESH_GUS_VACCINATED_MASKED_INFECTED,	
        BT_MESH_GUS_OFF,
};

/* Forward declaration of the Bluetooth Mesh Gus model context. */
struct bt_mesh_gus;

											
/** @def BT_MESH_MODEL_GUS_CLI
 *
 * @brief Bluetooth Mesh Gus Client model composition data entry.
 *
 * @param[in] _gus Pointer to a @ref bt_mesh_gus_cli instance.
 */
#define BT_MESH_MODEL_GUS_CLI(_gus)                               \
		BT_MESH_MODEL_VND_CB(BT_MESH_GUS_VENDOR_COMPANY_ID,   \
			BT_MESH_GUS_VENDOR_MODEL_ID,                      \
			_bt_mesh_gus_cli_op, &(_gus)->pub,                    \
			BT_MESH_MODEL_USER_DATA(struct bt_mesh_gus,       \
						_gus),                        \
			&_bt_mesh_gus_cli_cb)
									   

/** Bluetooth Mesh Gus Client model handlers. */
struct bt_mesh_gus_handlers {
	/** @brief Called after the node has been provisioned, or after all
	 * mesh data has been loaded from persistent storage.
	 *
	 * @param[in] cli Gus Client instance that has been started.
	 */
	void (*const start)(struct bt_mesh_gus *gus);

	/** @brief Handler for a sign in reply.
	 *
	 * @param[in] cli Gus client instance that received the reply.
	 * @param[in] ctx Context of the incoming message.
	 * @param[in] msg Pointer to a name terminated with
	 * a null character, '\0'.
	 */
	void (*const sign_in_reply)(struct bt_mesh_gus *gus,
				    struct bt_mesh_msg_ctx *ctx,
				      const uint8_t *msg);

	/** @brief Handler for a reply on a report request.
	 *
	 * @param[in] cli Gus client instance that received the reply.
	 * @param[in] ctx Context of the incoming message.
	 * @param[in] msg Pointer to a proximity report structure
	 * a null character, '\0'.	 */
	void (*const report_reply)(struct bt_mesh_gus *gus,
				    struct bt_mesh_msg_ctx *ctx,
				      const uint8_t *msg);
};


/**
 * Bluetooth Mesh Gus Client model context.
 */
struct bt_mesh_gus {
	/** Access model pointer. */
	struct bt_mesh_model *model;
	/** Publish parameters. */
	struct bt_mesh_model_pub pub;
	/** Publication message. */
	struct net_buf_simple pub_msg;
	/** badge name. */
	uint8_t buf[BT_MESH_MODEL_BUF_LEN(BT_MESH_GUS_OP_SET_NAME,
					  BT_MESH_GUS_MSG_MAXLEN_MESSAGE)];
        uint8_t name[CONFIG_BT_MESH_GUS_NAME_LENGTH+1];
	/** Handler function structure. */
	const struct bt_mesh_gus_handlers *handlers;
	/** Current Presence value. */
	enum bt_mesh_gus_state state;
};

/** @brief Publish the sign in request to the mesh network.
 *
 * @param[in] gus     Gus Client model instance to sign into.
 *
 * @retval 0 Successfully set the preceive and sent the message.
 * @retval -EADDRNOTAVAIL Publishing is not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_gus_cli_sign_in(struct bt_mesh_gus *gus);

/** @brief Set the client state.
 *
 * @param[in] gus     Gus Client model instance to set presence on.
 * @param[in] addr    Address of the gus client to set the state of.
 * @param[in] state   New client state.
 *
 * @retval 0 Successfully set the preceive and sent the message.
 * @retval -EADDRNOTAVAIL Publishing is not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_gus_cli_state_set(struct bt_mesh_gus *gus,
				  uint16_t addr,
				  enum bt_mesh_gus_state state);

/** @brief Set the client name.
 *
 * @param[in] cli Gus Client model instance.
 * @param[in] addr    Address of the gus client to set the state of.
 * @param[in] name Pointer to a name. Must be terminated with
 * a null character, '\0'.
 *
 * @retval 0 Successfully sent the message.
 * @retval -EADDRNOTAVAIL Publishing is not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_gus_cli_name_set(struct bt_mesh_gus *gus,
				  uint16_t addr,
				  const uint8_t *name);

/** @brief Request a proximity report.
 *
 * @param[in] gus     Gus Client model instance to sign into.
 * @param[in] addr    Address of the gus client.
 *
 * @retval 0 Successfully set the preceive and sent the message.
 * @retval -EADDRNOTAVAIL Publishing is not configured.
 * @retval -EAGAIN The device has not been provisioned.
 */
int bt_mesh_gus_cli_report_request(struct bt_mesh_gus *gus,
				  uint16_t addr);

/** @cond INTERNAL_HIDDEN */
extern const struct bt_mesh_model_op _bt_mesh_gus_cli_op[];
extern const struct bt_mesh_model_cb _bt_mesh_gus_cli_cb;
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* BT_MESH_GUS_CLI_H__ */

/** @} */
