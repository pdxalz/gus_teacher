#include <zephyr.h>
#include <stdio.h>
#include <string.h>

#include "model_handler.h"
#include "simulate.h"
#include "contacts.h"
#include "gus_config.h"
#include "gus_data.h"
#include "gui.h"
#include "contacts.h"

// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(_sim_cmd_queue, sizeof(sim_message_t), 8, 4);

const int _step_interval = 60; // 1 minute intervals
static uint16_t _step_time;
static uint8_t _infection_rate;
static uint16_t _time_of_last_exposure;
static uint16_t _time_to_complete;
static uint16_t _current_step_index;
static bool _first_step;


// calculates the amount of exposure for a time range
// index: index in the contact data array
// t0: start of time range
// t1: end of time range
static uint32_t exposed_in_period(int index, uint16_t t0, uint16_t t1)
{
    t0 = MAX(t0, get_start_time(index));
    t1 = MIN(t1, get_end_time(index));
    uint32_t exposure = (t1 - t0) * 1000 / get_distance_squared(index);
    //    printk("exposed: %d %d %d\n", t0, t1, get_distance_squared(index));
    //    k_sleep(K_MSEC(50));

    return (t0 < t1) ? exposure : 0;
}

// Adds to the exposure of a healthy badge if they are exposed to the infection.
// Also, the exposure is reduce by 1/3 if the healthy badge wears a mask,
// by 1/10 if the infected badge has a mask, and by 1/50 if the healthy badge
// has been vaccinated.
// index: index in the contact data array
// exposure: the amount of the exposure
// update: if true, infections are shown in the badge eyes
static void add_exposure(int index, uint32_t exposure, bool update)
{
    int badgeA = get_contact_badgeA(index);
    int badgeB = get_contact_badgeB(index);
    //printk("add ex: %d %d %d\n", badgeA, badgeB, exposure);
    // if either but not both infected, add the exposure to the non-infected
    if (!get_infected(badgeA) && get_infected(badgeB))
    {
        exposure /= (get_mask(badgeA) ? 3 : 1);
        exposure /= (get_mask(badgeB) ? 10 : 1);
        exposure /= (get_vaccine(badgeA) ? 50 : 1);
        add_exposure_to_badge(badgeA, exposure, update);
    }
    else if (get_infected(badgeA) && !get_infected(badgeB))
    {
        exposure /= (get_mask(badgeB) ? 2 : 1);
        exposure /= (get_mask(badgeA) ? 10 : 1);
        exposure /= (get_vaccine(badgeB) ? 50 : 1);
        add_exposure_to_badge(badgeB, exposure, update);
    }
}

// check for any exposures in a time range and add them to the badge data
// t0: start of time range
// t1: end of time range
// update: if true, infections are shown in the badge eyes
static void calculate_exposures(uint16_t t0, uint16_t t1, bool update)
{
    //printk("calc exp: total=%d\n, ", get_total_contacts());
    _time_of_last_exposure = 0;
    for (uint16_t i = 0; i < get_total_contacts(); ++i)
    {
        uint32_t exposure = exposed_in_period(i, t0, t1) * (uint32_t)_infection_rate / 10L;
        if (exposure > 0)
        {
            add_exposure(i, exposure, update);
            _time_of_last_exposure = t1;
        }
    }
}

// for debugging, output current infections to the console
void print_infections(void)
{
    for (int i = 0; i < get_badge_count(); ++i)
    {
        if (get_infected(i))
        {
            printk(" %s,", get_badge_name(i));
        }
    }
    printk("\n");
}

// for classroom mode, determine at what step time is everyone infected.
static void calc_time_to_complete(void)
{
    _step_time = 0;
    reset_exposures(false);

    while (_step_time < final_time() && !everyone_infected())
    {
        calculate_exposures(_step_time, _step_time + _step_interval, false);
        _step_time += _step_interval;
    }
    _time_to_complete = _step_time;
}


// Takes another step in the analysis in classroom mode.  Determines how many
// infections have occurred to this point.  Updates the GUI progress with
// the percentage of how far we have stepped through the simulation.
static void apply_infections_next_step(void)
{
    if (_first_step)
    {
        reset_exposures(true);
        _first_step = false;
    }
    for (int i = 0; i < RECORDS_PER_STEP; ++i)
    {
        if (_current_step_index >= get_total_contacts())
        {
            break;  // reached end of contacts, quit
        }
        uint32_t exposure = 100000 / get_distance_squared(_current_step_index);
        add_exposure(_current_step_index, exposure, true);
        ++_current_step_index;
 
        uint8_t progress = MIN(100, _current_step_index * 100 / get_total_contacts());
        gui_update_progress(progress, total_infections());
    }
}

// processes the list of all contacts and finds the total number of infections
// The progress on the gui is updated with the percentage of badges
// that have been infected.  Used in GUS tag mode.
static void show_number_of_infections(void)
{
    reset_exposures(false);
    for (int i = 0; i < get_total_contacts(); ++i)
    {
        uint32_t exposure = 100000 / get_distance_squared(i);
        add_exposure(i, exposure, false);
    }
    uint8_t progress = MIN(100, total_infections() * 100 / get_badge_count());
    gui_update_progress(progress, total_infections());
}

// Restarts the simulation for GUS tag mode
// The list of contacts is emptied
static void rewind_sim(void)
{
    _first_step = true;
    _current_step_index = 0;
    _time_to_complete = final_time();
    printk("complete: %d\n", _time_to_complete);
    reset_exposures(false);
    _step_time = get_start_time(0);
    gui_update_progress(0, 0);
}

// Restarts the simulation for the classroom mode.
// A fake list of contacts is generates based on number of rows and space
static void restart_sim(uint8_t rows, uint8_t space)
{
    simulate_contacts(rows, space);
    calc_time_to_complete();
 //   printk("complete: %d\n", _time_to_complete);
    reset_exposures(true);
    _step_time = 0;
    gui_update_progress(0, 0);
}

// handle the next step message
static void next_analysis_point(void)
{
    calculate_exposures(_step_time, _step_time + _step_interval, true);
    print_infections();
    _step_time += _step_interval;

//    printk("exposure: %d %d %d %d \n", _step_time, get_exposure(1), get_exposure(2), get_exposure(3));
    if (_time_to_complete != 0)
    {
        uint8_t progress = MIN(100, _step_time * 100 / _time_to_complete);

        gui_update_progress(progress, total_infections());
    }
}

// processes any messages in the simulation message queue 
static void process_sim_msg_queue(void)
{
    sim_message_t sim_message;
    while (k_msgq_get(&_sim_cmd_queue, &sim_message, K_NO_WAIT) == 0)
    {
        // Process incoming commands depending on type
        switch (sim_message.type)
        {
        case SIM_MSG_RESTART:
            _infection_rate = sim_message.params.infection_rate;
            if (sim_message.params.tag_mode)
            {
                rewind_sim();
            }
            else
            {
                restart_sim(sim_message.params.rows, sim_message.params.space);
            }
            break;
        case SIM_MSG_NEXT:
            if (sim_message.params.tag_mode)
            {
                apply_infections_next_step();
            }
            else
            {
                next_analysis_point();
            }

            break;
        case SIM_MSG_ADD_CONTACT:
            add_proximity_contact(sim_message.params.badgeA,
                                  sim_message.params.badgeB,
                                  sim_message.params.rssi);
            show_number_of_infections();

            break;
        }
    }
}

// checks the message queue
static void sim_run(void)
{
    while (1)
    {
        process_sim_msg_queue();
        k_sleep(K_MSEC(20));
    }
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(sim_thread, 4096, sim_run, NULL, NULL, NULL, 7, 0, 0);

/////////////////////
// global functions
/////////////////////
void sim_msg_restart(uint8_t rows, uint8_t space, uint8_t infection_rate, bool tag_mode)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_RESTART;
    msg.params.tag_mode = tag_mode;
    msg.params.rows = rows;
    msg.params.space = space;
    msg.params.infection_rate = infection_rate;

    k_msgq_put(&_sim_cmd_queue, &msg, K_NO_WAIT);
}

void sim_msg_next(bool tag_mode)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_NEXT;
    msg.params.tag_mode = tag_mode;
    k_msgq_put(&_sim_cmd_queue, &msg, K_NO_WAIT);
}

void sim_msg_add_contact(uint16_t badgeA, uint16_t badgeB, int8_t rssi)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_ADD_CONTACT;
    msg.params.badgeA = badgeA;
    msg.params.badgeB = badgeB;
    msg.params.rssi = rssi;

    k_msgq_put(&_sim_cmd_queue, &msg, K_NO_WAIT);
}
