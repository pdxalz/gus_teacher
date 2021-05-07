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
K_MSGQ_DEFINE(m_sim_cmd_queue, sizeof(sim_message_t), 8, 4);

const int step_interval = 60;  // 1 minute intervals
static uint16_t step_time;
static uint8_t  rate = 4;
static uint16_t time_of_last_exposure;
static uint16_t time_to_complete;
static uint16_t _current_step_index;
static bool _first_step;

static uint32_t exposed_in_period(int index, uint16_t t0, uint16_t t1) 
{
    t0 = MAX(t0, get_start_time(index));
    t1 = MIN(t1, get_end_time(index));
    uint32_t exposure = (t1-t0) * 1000 / get_distance_squared(index);
//    printk("exposed: %d %d %d\n", t0, t1, get_distance_squared(index));
//    k_sleep(K_MSEC(50));

    return (t0 < t1 ) ? exposure : 0;
}

// Adds to the exposure of a healthy badge if they are exposed to the infection.
// Also, the exposure is reduce by 1/3 if the healthy badge wears a mask,
// by 1/10 if the infected badge has a mask, and by 1/50 if the healthy badge
// has been vaccinated.
static void add_exposure(int index, uint32_t exposure, bool update)
{
    int badgeA = get_contact_badgeA(index);
    int badgeB = get_contact_badgeB(index);
//printk("add ex: %d %d %d\n", badgeA, badgeB, exposure);
    // if either but not both infected, add the exposure to the non-infected
    if (!get_infected(badgeA) && get_infected(badgeB)) {
        exposure /= (has_mask(badgeA) ? 3 : 1);
        exposure /= (has_mask(badgeB) ? 10 : 1);
        exposure /= (has_vaccine(badgeA) ? 50 : 1);
        gd_add_exposure(badgeA, exposure, update);
    }
    else if (get_infected(badgeA) && !get_infected(badgeB)) {
        exposure /= (has_mask(badgeB) ? 2 : 1);
        exposure /= (has_mask(badgeA) ? 10 : 1);
        exposure /= (has_vaccine(badgeB) ? 50 : 1);
        gd_add_exposure(badgeB, exposure, update);
    }
}


static void calculate_exposures(uint16_t t0, uint16_t t1, bool update) 
{
//printk("calc exp: total=%d\n, ", get_total_contacts());
    time_of_last_exposure = 0;
    for (uint16_t i=0; i < get_total_contacts(); ++i) {
        uint32_t exposure = exposed_in_period(i, t0, t1) * (uint32_t)rate / 10L;
        if (exposure > 0) {
           add_exposure(i, exposure, update);
           time_of_last_exposure = t1;
        }
    }
}

void print_infections(void) 
{
    for (int i=0; i<gd_get_node_count(); ++i) {
        if (get_infected(i)) {
            printk(" %s,", get_name(i));
        }
    }
    printk("\n");
}



static void calc_time_to_complete(void)
{
    step_time=0;
    reset_exposures(false);
    
    while (step_time < final_time() && !everyone_infected()) {     
        calculate_exposures(step_time, step_time + step_interval, false);
         step_time += step_interval;
    }
    time_to_complete = step_time;
}


#define RECORDS_PER_STEP 2

static void apply_infections_next_step(void)
{
    if (_first_step) {
        reset_exposures(true);
        _first_step = false;
    }
    for (int i=0; i<RECORDS_PER_STEP; ++i)
    {
        if (_current_step_index >= get_total_contacts()) {
            break;
        }
        uint32_t exposure = 100000 / get_distance_squared(_current_step_index);
        printk("e-> %d  %d\n", _current_step_index, exposure);
        add_exposure(_current_step_index, exposure, true);
        ++_current_step_index;
    printk("exposure: %d %d %d %d \n", _current_step_index, get_exposure(1), get_exposure(2), get_exposure(3));

        uint8_t progress = MIN(100, _current_step_index * 100 / get_total_contacts());
        gui_update_progress(progress, total_infections());
    }
}


static void show_number_of_infections(void)
{
    reset_exposures(false);
    for (int i=0; i<get_total_contacts(); ++i)
    {
        uint32_t exposure = 100000 / get_distance_squared(i);
        add_exposure(i, exposure, false);
    }
    uint8_t progress = MIN(100, total_infections() * 100 / gd_get_node_count());
    gui_update_progress(progress, total_infections());
}

static void rewind_sim(void)
{
    _first_step = true;
    _current_step_index = 0;
    time_to_complete = final_time();
    printk("complete: %d\n", time_to_complete);
    reset_exposures(false);
    step_time = get_start_time(0);
    gui_update_progress(0, 0);    
}

static void restart_sim( uint8_t rows, uint8_t space)
{
    simulate_contacts(rows, space);
    calc_time_to_complete();
printk("complete: %d\n", time_to_complete);
    reset_exposures(true);
    step_time = 0;
    gui_update_progress(0, 0);
}

static void next_analysis_point(void)
{
    calculate_exposures(step_time, step_time + step_interval, true);
    print_infections();
    step_time += step_interval;   
    
    printk("exposure: %d %d %d %d \n", step_time, get_exposure(1), get_exposure(2), get_exposure(3));
    if (time_to_complete != 0) {
        uint8_t progress = MIN(100, step_time * 100 / time_to_complete);

        gui_update_progress(progress, total_infections());
    }
}


void sim_msg_restart(uint8_t rows, uint8_t space, uint8_t rate, bool tag_mode)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_RESTART;
    msg.params.tag_mode = tag_mode;
    msg.params.rows = rows;
    msg.params.space = space;
    msg.params.rate = rate;

    k_msgq_put(&m_sim_cmd_queue, &msg, K_NO_WAIT);
}

void sim_msg_next(bool tag_mode)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_NEXT;
    msg.params.tag_mode = tag_mode;
    k_msgq_put(&m_sim_cmd_queue, &msg, K_NO_WAIT);
}

void sim_msg_add_contact(uint16_t badgeA, uint16_t badgeB, int8_t rssi)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_ADD_CONTACT;
    msg.params.badgeA = badgeA;
    msg.params.badgeB = badgeB;
    msg.params.rssi = rssi;

    k_msgq_put(&m_sim_cmd_queue, &msg, K_NO_WAIT);
}



static void process_sim_msg_queue(void)
{
    sim_message_t sim_message;
    while(k_msgq_get(&m_sim_cmd_queue, &sim_message, K_NO_WAIT) == 0){
        // Process incoming commands depending on type
        switch(sim_message.type){
            case SIM_MSG_RESTART:
                rate = sim_message.params.rate;
                if (sim_message.params.tag_mode) {
                    rewind_sim();
                }
                else {
                    restart_sim(sim_message.params.rows, sim_message.params.space);
                }
                break;
            case SIM_MSG_NEXT:
                if (sim_message.params.tag_mode) {
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


static void sim_run(void)
{
	while(1){
		process_sim_msg_queue();
		k_sleep(K_MSEC(20));
	}
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(sim_thread, 4096, sim_run, NULL, NULL, NULL, 7, 0, 0);