#ifndef __GUI_H
#define __GUI_H

#include <zephyr.h>

// callback function declarations
typedef enum {GUI_EVT_SCAN, GUI_EVT_IDENTIFY, GUI_EVT_RECORD} gui_event_type_t;

typedef struct
{
	gui_event_type_t evt_type;
	uint16_t addr;
} gui_event_t;

typedef void (*gui_callback_t)(gui_event_t *event);

typedef struct
{
	gui_callback_t event_callback;
} gui_config_t;

// GUI message queue types and message structure
typedef enum  {GUI_MSG_UPDATE_LIST, GUI_MSG_PROGRESS} gui_msg_type_t;

typedef struct
{
	gui_msg_type_t type;
	void *p_data;
	struct 
	{
		uint16_t param;
		uint8_t progress;
	} params;
} gui_message_t;


// initialize the GUI
// config : event callback function
void gui_init(gui_config_t * config);

// Add a GUI message to reload the scroller namelist control
void gui_update_namelist(void);

// Add a GUI message to update the state of the config checkboxes
void update_checkboxes(void);

// Add a GUI message to update the analyze progress bar
// progress: percent progress complete, or percentage of infections if gus-tag mode
void gui_update_progress(uint8_t progress, uint16_t infections);

// returns the node address of the selected item
uint16_t gui_get_selected_addr(void);


#endif
