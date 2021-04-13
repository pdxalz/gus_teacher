#ifndef __GUI_H
#define __GUI_H

#include <zephyr.h>

typedef enum {GUI_EVT_SCAN, GUI_EVT_IDENTIFY} gui_event_type_t;

typedef struct
{
	gui_event_type_t evt_type;
	uint8_t element;
} gui_event_t;

typedef enum  {GUI_MSG_UPDATE_LIST, GUI_MSG_PROGRESS} gui_msg_type_t;

typedef struct
{
	gui_msg_type_t type;
	void *p_data;
	union 
	{
		uint16_t param;
		uint8_t progress;
	} params;
} gui_message_t;


typedef void (*gui_callback_t)(gui_event_t *event);

typedef struct
{
	gui_callback_t event_callback;
} gui_config_t;

void gui_init(gui_config_t * config);

void gui_update(void);
void gui_update_namelist(void);
void gui_update_progress(uint8_t progress);

#endif
