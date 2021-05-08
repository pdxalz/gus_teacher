#ifndef __SIMULATE_H
#define __SIMULATE_H

#include <zephyr.h>

typedef enum {SIM_MSG_RESTART, SIM_MSG_NEXT, SIM_MSG_ADD_CONTACT} sim_msg_type_t;

typedef struct
{
	sim_msg_type_t type;
	void *p_data;
	union 
	{
            struct  {
		uint8_t rows;
		uint8_t space;
                uint8_t rate;
                bool    tag_mode;
                };
            struct {
                uint16_t badgeA;
                uint16_t badgeB;
                int8_t rssi;
               };
	} params;
} sim_message_t;

void sim_msg_restart(uint8_t rows, uint8_t space, uint8_t rate, bool tag_mode);
void sim_msg_next(bool tag_mode);
void sim_msg_add_contact(uint16_t badgeA, uint16_t badgeB, int8_t rssi);

#endif  //__SIMULATE_H
