#include <zephyr.h>
#include <stdlib.h>
#include <math.h>
#include "errors.h"
#include "contacts.h"
#include "gus_data.h"
#include "gus_config.h"

#define MAX_CONTACTS 5000
//const uint8_t rows = 1;
//const uint8_t desk_spacing = 4*12;


const uint8_t safe_distance = 100;
const uint16_t create_interval = (1*60*10);  //todo 

struct contact {
    uint8_t     badgeA;
    uint8_t     badgeB;
    uint16_t    start_time;
    uint16_t    end_time;
    uint8_t     distance;
};

static uint16_t _final_time;
static uint16_t _total_contacts;
static struct contact contact_list[MAX_CONTACTS];

uint16_t get_total_contacts(void) 
{
    return _total_contacts;
}

uint16_t get_start_time(int index)
{
    return contact_list[index].start_time;
}

uint16_t get_end_time(int index)
{
    return contact_list[index].end_time;
}

uint16_t get_distance_squared(int index)
{
    uint16_t dist = contact_list[index].distance;
    return  dist * dist;
}

uint8_t get_contact_badgeA(int index)
{
    return contact_list[index].badgeA;
}

uint8_t get_contact_badgeB(int index)
{
    return contact_list[index].badgeB;
}

uint16_t final_time(void)
{
    return _final_time;
}

void add_contact(uint16_t badgeA, uint16_t badgeB, uint32_t start_time, uint32_t end_time, double distance)
{
    __ASSERT(_total_contacts < MAX_CONTACTS, ERR_BAD_PARAM);

    contact_list[_total_contacts].badgeA = badgeA;
    contact_list[_total_contacts].badgeB = badgeB;
    contact_list[_total_contacts].start_time = start_time;
    contact_list[_total_contacts].end_time = end_time;

    __ASSERT(distance != 0, ERR_BAD_PARAM);
    contact_list[_total_contacts].distance = distance;
    _total_contacts++;
}

static uint8_t calc_distance(int badgeA, int badgeB, uint8_t rows, uint8_t space)
{
    double distance;

    double x = (double)(badgeA % rows) - (double)(badgeB % rows);
    double y = (double)(badgeA / rows) - (double)(badgeB / rows);
    distance = space * 12.0 * sqrt(x*x +y*y);
    if (distance > 254)
        return 255;
    return (uint8_t) distance;
}

// create contacts for non-proximity simulation
void simulate_contacts(uint8_t rows, uint8_t space)
{
    _total_contacts = 0;
    uint16_t time = 0;
    uint8_t distance;
    uint8_t node_count = gd_get_node_count();

    for (uint16_t i=1; i<10; ++i) {  //todo until contacts full
        for (uint8_t badgeA=0; badgeA<node_count; ++badgeA) {
            for (uint8_t badgeB=0; badgeB<gd_get_node_count(); ++badgeB) {
                if (badgeA==badgeB) continue;
            
                distance = calc_distance(badgeA, badgeB, rows, space);
                if (distance < safe_distance) {
                    add_contact(badgeA, badgeB, time, time+create_interval, distance);
                }
            }
        }
        time += create_interval;
    }
    _final_time = time;
}



