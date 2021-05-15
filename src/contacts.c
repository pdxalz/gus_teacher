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
    __ASSERT(_total_contacts < MAX_CONTACTS, "bad param");

    _contact_list[_total_contacts].badgeA = badgeA;
    _contact_list[_total_contacts].badgeB = badgeB;
    _contact_list[_total_contacts].start_time = start_time;
    _contact_list[_total_contacts].end_time = end_time;

    __ASSERT(distance != 0, "bad param");
    _contact_list[_total_contacts].distance = distance;
//    printk("added %4d %4d %4d %4d %4d\n", _total_contacts, badgeA, start_time, end_time, (int)distance); 
    _total_contacts++;

}

// calcuates the distance between two badges for classroom mode which assumes
// badges are in alphabetical order, in a number of row specified by "rows",
// and separated in distance by "space".
static uint8_t calc_distance(int badgeA, int badgeB, uint8_t rows, uint8_t space)
{
    double distance;
    uint8_t result;
    double x = (double)(badgeA % rows) - (double)(badgeB % rows);
    double y = (double)(badgeA / rows) - (double)(badgeB / rows);
    distance = space * 12.0 * sqrt(x * x + y * y);
    if (distance > 254)
        return 255;
    result = MAX(MIN(distance,255.0), 1.0);
    return result;
}


/////////////////////
// global functions
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
    uint8_t _badge_count = get_badge_count();

    for (uint16_t i = 1; i < 10; ++i)
    { 
        for (uint8_t badgeA = 0; badgeA < _badge_count; ++badgeA)
        {
            for (uint8_t badgeB = 0; badgeB < get_badge_count(); ++badgeB)
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
printk("reset prox contacts\n");
    prox_start_time = k_uptime_get_32();
    _total_contacts = 0;
}


void add_proximity_contact(uint16_t addr1, uint16_t addr2, int8_t rssi)
{
    // time in seconds since proximity checks were restarted
    uint32_t current_time = (k_uptime_get_32() - prox_start_time) / 1000;
    uint32_t interval = get_badge_count();

    uint16_t badgeA = get_badge_index_from_address(addr1);
    uint16_t badgeB = get_badge_index_from_address(addr2);
    if (rssi > -RSSI_TUNE_CONSTANT)
    {
        current_time *= DEMO_VIDEO_ACCEL;
        interval *= DEMO_VIDEO_ACCEL;
        double distance = 500.0 / (rssi + RSSI_TUNE_CONSTANT);
//        printk("contact %d-%d: %d %d\n", badgeA, badgeB, rssi, 500 / (rssi + RSSI_TUNE_CONSTANT));
//        if (distance<1.0) distance = 1.0;
        add_contact(badgeA, badgeB, current_time, current_time + interval, distance);
        _final_time = current_time + interval;
    }
}
