#include <zephyr.h>
#include <stdlib.h>
#include <math.h>
#include "errors.h"
#include "contacts.h"
#include "gus_data.h"
#include "gus_config.h"

////////////////////////
// declarations
////////////////////////
struct contact
{
    uint8_t badgeA;
    uint8_t badgeB;
    uint16_t start_time;
    uint16_t end_time;
    uint8_t distance;
};

////////////////////
// static variables
////////////////////
static uint16_t _final_time;
static uint16_t _total_contacts;
static uint32_t prox_start_time;
static struct contact _contact_list[MAX_CONTACTS];

/////////////////////
// Static functions
/////////////////////

// adds a new contact recored
static void add_contact(uint16_t badgeA, uint16_t badgeB, uint32_t start_time, uint32_t end_time, double distance)
{
    __ASSERT(_total_contacts < MAX_CONTACTS, ERR_BAD_PARAM);

    _contact_list[_total_contacts].badgeA = badgeA;
    _contact_list[_total_contacts].badgeB = badgeB;
    _contact_list[_total_contacts].start_time = start_time;
    _contact_list[_total_contacts].end_time = end_time;

    __ASSERT(distance != 0, ERR_BAD_PARAM);
    _contact_list[_total_contacts].distance = distance;
    _total_contacts++;
}

// calcuates the distance between two badges for classroom mode which assumes
// badges are in alphabetical order, in a number of row specified by "rows",
// and separated in distance by "space".
static uint8_t calc_distance(int badgeA, int badgeB, uint8_t rows, uint8_t space)
{
    double distance;

    double x = (double)(badgeA % rows) - (double)(badgeB % rows);
    double y = (double)(badgeA / rows) - (double)(badgeB / rows);
    distance = space * 12.0 * sqrt(x * x + y * y);
    if (distance > 254)
        return 255;
    return (uint8_t)distance;
}


/////////////////////
// non-static functions
/////////////////////
uint16_t get_total_contacts(void)
{
    return _total_contacts;
}

uint16_t get_start_time(int index)
{
    return _contact_list[index].start_time;
}

uint16_t get_end_time(int index)
{
    return _contact_list[index].end_time;
}

uint16_t get_distance_squared(int index)
{
    uint16_t dist = _contact_list[index].distance;
    return dist * dist;
}

uint8_t get_contact_badgeA(int index)
{
    return _contact_list[index].badgeA;
}

uint8_t get_contact_badgeB(int index)
{
    return _contact_list[index].badgeB;
}

uint16_t final_time(void)
{
    return _final_time;
}


// create contacts for non-proximity simulation
void simulate_contacts(uint8_t rows, uint8_t space)
{
    _total_contacts = 0;
    uint16_t time = 0;
    uint8_t distance;
    uint8_t node_count = gd_get_node_count();

    for (uint16_t i = 1; i < 10; ++i)
    { 
        for (uint8_t badgeA = 0; badgeA < node_count; ++badgeA)
        {
            for (uint8_t badgeB = 0; badgeB < gd_get_node_count(); ++badgeB)
            {
                if (badgeA == badgeB)
                    continue;

                distance = calc_distance(badgeA, badgeB, rows, space);
                if (distance < SAFE_DISTANCE)
                {
                    add_contact(badgeA, badgeB, time, time + TIME_PER_CONTACT, distance);
                }
            }
        }
        time += TIME_PER_CONTACT;
    }
    _final_time = time;
}

void reset_proximity_contacts(void)
{
    prox_start_time = k_uptime_get_32();
    _total_contacts = 0;
}


void add_proximity_contact(uint16_t badgeA, uint16_t badgeB, int8_t rssi)
{
    // time in seconds since proximity checks were restarted
    uint32_t current_time = (k_uptime_get_32() - prox_start_time) / 1000;
    uint32_t interval = gd_get_node_count();

    if (rssi > -RSSI_TUNE_CONSTANT)
    {
        double distance = (rssi + RSSI_TUNE_CONSTANT) * 2.0;
        add_contact(badgeA, badgeB, current_time, current_time + interval, distance);
        printk("contact %d-%d: %d\n", badgeA, badgeB, (rssi + RSSI_TUNE_CONSTANT) * 2);
    }
}
