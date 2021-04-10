#include <zephyr.h>
#include <stdio.h>
#include <string.h>

#include "simulate.h"
#include "contacts.h"
#include "gus_config.h"
#include "gus_data.h"

// Create a message queue for handling external GUI commands
K_MSGQ_DEFINE(m_sim_cmd_queue, sizeof(sim_message_t), 8, 4);

static uint32_t time;



static void restart_sim(void)
{
    reset_exposures();
    simulate_contacts();
    time = 0;
}


static uint32_t exposed_in_period(int index, uint16_t t0, uint16_t t1) 
{
printk("t= %d %d %d %d %d\n", index, t0, t1, get_start_time(index), get_end_time(index));
    t0 = MAX(t0, get_start_time(index));
    t1 = MIN(t1, get_end_time(index));

    return (t0 < t1 ) ? (t1-t0) * 1000 / get_distance_squared(index) : 0;
}


static void add_exposure(int index, uint32_t exposure)
{
    int badgeA = get_contact_badgeA(index);
    int badgeB = get_contact_badgeB(index);

    // if either but not both infected, add the exposure to the non-infected
    if (!get_infected(badgeA) && get_infected(badgeB)) {
        gd_add_exposure(badgeA, exposure);
    }
    else if (get_infected(badgeA) && !get_infected(badgeB)) {
        gd_add_exposure(badgeB, exposure);
    }
}


static void calculate_exposures(uint16_t t0, uint16_t t1) 
{
    for (uint16_t i=0; i < get_total_contacts(); ++i) {
        uint32_t exposure = exposed_in_period(i, t0, t1);
        if (exposure > 0) {
            add_exposure(i, exposure);
        }
    }
}

void print_infections(void) 
{
printk("\n-->");
    for (int i=0; i<gd_get_node_count(); ++i) {
        if (get_infected(i)) {
            printk(" %s,", get_name(i));
        }
        printk("exp: %d %6d\n", i, get_exposure(i));
    }
}


static void next_analysis_point(void)
{
printk("time %d dur %d\n", time, DURATION);
    calculate_exposures(time, time + DURATION);
    print_infections();
    time += DURATION;    
}

void sim_msg_restart(void)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_RESTART;
    k_msgq_put(&m_sim_cmd_queue, &msg, K_NO_WAIT);
}

void sim_msg_next(void)
{
    static sim_message_t msg;
    msg.type = SIM_MSG_NEXT;
    k_msgq_put(&m_sim_cmd_queue, &msg, K_NO_WAIT);
}


static void process_cmd_msg_queue(void)
{
    sim_message_t cmd_message;
    while(k_msgq_get(&m_sim_cmd_queue, &cmd_message, K_NO_WAIT) == 0){
        // Process incoming commands depending on type
        switch(cmd_message.type){
            case SIM_MSG_RESTART:
                restart_sim();
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
		process_cmd_msg_queue();
		k_sleep(K_MSEC(20));
	}
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(sim_thread, 4096, sim_run, NULL, NULL, NULL, 7, 0, 0);