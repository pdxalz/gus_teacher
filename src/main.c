/*
 * Copyright (c) 2018 Jan Van Winkel <jan.van_winkel@dxplore.eu>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <device.h>
#include <drivers/display.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <bluetooth/mesh/dk_prov.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"
#include "gui.h"
#include "simulate.h"

//#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
//#include <logging/log.h>
//LOG_MODULE_REGISTER(app);

static void bt_ready(int err)
{
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	dk_leds_init();
	dk_buttons_init(NULL);

	err = bt_mesh_init(bt_mesh_dk_prov_init(), model_handler_init());
	if (err)
	{
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS))
	{
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

void on_gui_event(gui_event_t *event)
{
	switch (event->evt_type)
	{
	case GUI_EVT_SCAN:
		model_scan_for_badges();
		gui_update_namelist();
		update_checkboxes();
		break;
	case GUI_EVT_IDENTIFY:
		model_handler_set_state(event->addr, BT_MESH_GUS_IDENTIFY);
		break;
	case GUI_EVT_RECORD:
		model_report_request(event->addr);
		break;
	}
}

void main(void)
{
	int err;

	gui_config_t gui_config = {.event_callback = on_gui_event};
	gui_init(&gui_config);

	printk("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
	}

	while (1)
	{
		k_sleep(K_MSEC(1000));
	}
}
