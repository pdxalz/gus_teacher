#ifndef __SIMULATE_H
#define __SIMULATE_H

#include <zephyr.h>

typedef enum {SIM_MSG_RESTART, SIM_MSG_NEXT} sim_msg_type_t;

typedef struct
{
	sim_msg_type_t type;
	void *p_data;
	union 
	{
		uint16_t param;
		bool led_state;
	} params;
} sim_message_t;

void sim_msg_restart(void);
void sim_msg_next(void);
#endif  //__SIMULATE_H
