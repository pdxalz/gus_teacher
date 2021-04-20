#ifndef __SIMULATE_H
#define __SIMULATE_H

#include <zephyr.h>

typedef enum {SIM_MSG_RESTART, SIM_MSG_NEXT} sim_msg_type_t;

typedef struct
{
	sim_msg_type_t type;
	void *p_data;
	struct // union 
	{
		uint8_t rows;
		uint8_t space;
                uint8_t rate;
	} params;
} sim_message_t;

void sim_msg_restart(uint8_t rows, uint8_t space, uint8_t rate);
void sim_msg_next(void);
#endif  //__SIMULATE_H
