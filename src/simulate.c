#include <zephyr.h>
#include <stdio.h>
#include <string.h>

#include "simulate.h"
#include "contacts.h"
#include "gus_config.h"
#include "gus_data.h"
#include "gui.h"


// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(m_sim_cmd_queue, sizeof(sim_message_t), 8, 4);

const int step_interval = 60;  // 1 minute intervals
static uint16_t time;
static uint8_t  rate;
static uint16_t time_of_last_exposure;
static uint16_t time_to_complete;


static uint32_t exposed_in_period(int index, uint16_t t0, uint16_t t1) 
{
    t0 = MAX(t0, get_start_time(index));
    t1 = MIN(t1, get_end_time(index));

    return (t0 < t1 ) ? (t1-t0) * 1000 / get_distance_squared(index) : 0;
}


static void add_exposure(int index, uint32_t exposure, bool update)
{
    int badgeA = get_contact_badgeA(index);
    int badgeB = get_contact_badgeB(index);

    // if either but not both infected, add the exposure to the non-infected
    if (!get_infected(badgeA) && get_infected(badgeB)) {
        gd_add_exposure(badgeA, exposure, update);
    }
    else if (get_infected(badgeA) && !get_infected(badgeB)) {
        gd_add_exposure(badgeB, exposure, update);
    }
}


static void calculate_exposures(uint16_t t0, uint16_t t1, bool update) 
{
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
}



static void calc_time_to_complete(void)
{
    time=0;
    reset_exposures(false);
    
    while (time < final_time() && !everyone_infected()) {     
        calculate_exposures(time, time + step_interval, false);
         time += step_interval;
    }
    time_to_complete = time;
}

static void restart_sim( uint8_t rows, uint8_t space)
{
    simulate_contacts(rows, space);
    calc_time_to_complete();
    reset_exposures(true);
    time = 0;
    gui_update_progress(0);
}

static void next_analysis_point(void)
{
    calculate_exposures(time, time + step_interval, true);
    print_infections();
    time += step_interval;   
    
    uint8_t progress = MIN(100, time * 100 / time_to_complete);
    gui_update_progress(progress);
}


void sim_msg_restart(uint8_t rows, uint8_t space, uint8_t rate)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_RESTART;
    msg.params.rows = rows;
    msg.params.space = space;
    msg.params.rate = rate;

    k_msgq_put(&m_sim_cmd_queue, &msg, K_NO_WAIT);
}

void sim_msg_next(void)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_NEXT;
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
                restart_sim(sim_message.params.rows, sim_message.params.space);
                break;
            case SIM_MSG_NEXT:
                next_analysis_point();
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