#include <zephyr.h>
#include <stdio.h>
#include <string.h>

#include "sim.h"

void sim_run(void)
{

	while(1){
//		process_cmd_msg_queue();
//		lv_task_handler();
		k_sleep(K_MSEC(20));
	}
}

// Define our GUI thread, using a stack size of 4096 and a priority of 7
K_THREAD_DEFINE(sim_thread, 4096, sim_run, NULL, NULL, NULL, 7, 0, 0);