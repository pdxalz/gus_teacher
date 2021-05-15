#ifndef __SIMULATE_H
#define __SIMULATE_H

#include <zephyr.h>

// message types for simulator message queue
typedef enum
{
    SIM_MSG_RESTART,
    SIM_MSG_NEXT,
    SIM_MSG_ADD_CONTACT
} sim_msg_type_t;

// message parameters
typedef struct
{
    sim_msg_type_t type;
    void *p_data;
    union
    {
        struct
        {
            uint8_t rows;
            uint8_t space;
            uint8_t infection_rate;
            uint8_t tag_mode;
        };
        struct
        {
            uint16_t badgeA;
            uint16_t badgeB;
            int8_t rssi;
        };
    } params;
} sim_message_t;


// set live_mode.  
// live_mode: if true health status is sent to the badges during recording
void set_live_mode(bool live_mode);

// Sends a message to the simalation message queue to restart the simulation. 
// Basicly a rewind of the sim playback.
// rows: number of row the class is organized into, for classroom mode
// space: distance between the rows, for classroom mode
// infection_rate: rate of the spread of infection, both modes 
// tag_mode: GUS tag mode
void sim_msg_restart(uint8_t rows, uint8_t space, uint8_t infection_rate, uint8_t tag_mode);

// Sends a message to the simalation message queue to process the next step
// of the simulation
// tag_mode: true if GUS tag mode, false if classroom mode
void sim_msg_next(bool tag_mode);

// Sends a message to the simalation message queue to add a new contact record.
// Occurs when the model handles a report request reply from a mesh node.
// badgeA: node address of the badge that sent the reply
// badgeB: node address of the badge that badgeA was in contact with
// rssi: the RSSI value when the contact occured. Used to estimate distance
void sim_msg_add_contact(uint16_t badgeA, uint16_t badgeB, int8_t rssi);

#endif //__SIMULATE_H
