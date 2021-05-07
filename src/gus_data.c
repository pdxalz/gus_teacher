#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gus_config.h"
#include "gus_data.h"
#include "contacts.h"
#include "sim_settings.h"
#include "model_handler.h"
#include "gui.h"

#include <lvgl.h>

#define INFECTION_THRESHOLD 200

struct gus_node {
    char name[MAX_NAME_LENGTH];
    uint16_t addr;
    bool patient_zero;
    bool infected;
    bool mask;
    bool vaccine;
    uint32_t exposure;
};

struct gus_node gus_nodes[MAX_GUS_NODES];
uint16_t node_count;


static void update_node_health_state(uint8_t index, bool initializing)
{
    enum bt_mesh_gus_state state = BT_MESH_GUS_HEALTHY;

    state = has_vaccine(index) ? 
                (has_mask(index) ? 
                    (get_infected(index) ? BT_MESH_GUS_VACCINATED_MASKED_INFECTED
                                         : BT_MESH_GUS_VACCINATED_MASKED) :
                    (get_infected(index) ? BT_MESH_GUS_VACCINATED_INFECTED
                                         : BT_MESH_GUS_VACCINATED) ) :
                (has_mask(index) ? 
                    (get_infected(index) ? BT_MESH_GUS_MASKED_INFECTED
                                         : BT_MESH_GUS_MASKED) :
                    (get_infected(index) ? BT_MESH_GUS_INFECTED
                                         : BT_MESH_GUS_HEALTHY) );
                       
    if (initializing && (state == BT_MESH_GUS_HEALTHY)) {
        return;
    }
    printk("update node %d %d %d\n", index, state, initializing);
    k_sleep(K_MSEC(120));
    model_handler_set_state(get_address(index), state);
}


bool is_patient_zero(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].patient_zero;
}

void set_patient_zero(int index, bool patient_zero)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_nodes[index].patient_zero =  patient_zero;
}

bool get_infected(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].infected;
}

void set_infected(int index, bool infected)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_nodes[index].infected =  infected;
}

bool has_mask(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].mask;
}

void set_masked(int index, bool mask)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_nodes[index].mask =  mask;
}


bool has_vaccine(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].vaccine;
}

void set_vaccine(int index, bool vaccine)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_nodes[index].vaccine =  vaccine;
}

uint32_t get_exposure(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].exposure;
}

void set_exposure(int index, uint32_t exposure)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_nodes[index].exposure =  exposure;
}


void gd_add_exposure(int index, uint32_t exposure, bool update)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_nodes[index].exposure +=  exposure;

    if (gus_nodes[index].exposure > INFECTION_THRESHOLD) {
        set_infected(index, true);
        if (update) {
            update_node_health_state(index, false);
        }
    }
}





uint16_t get_address(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].addr;
}



char * status_symbol(int index)
{
    if (gus_nodes[index].patient_zero) {
        return LV_SYMBOL_SETTINGS;
    }
    if (gus_nodes[index].vaccine) {
        return LV_SYMBOL_PLUS;
    }
    if (gus_nodes[index].mask) {
        return LV_SYMBOL_WIFI;
    }
    return "  ";
}


void gd_init(void)
{
    init_sim_settings();
}


int cmpfunc (const void * a, const void * b) 
{
    return strcmp(((struct gus_node * )a)->name,((struct gus_node *)b)->name);
//   return ( *(int*)a - *(int*)b );
}

void gd_add_node(const char * name, uint16_t addr, bool patient_zero, bool mask, bool vaccine)
{
    int i;

    if (node_count >= MAX_GUS_NODES) {
        return;
    }

    for (i=0; i<node_count; ++i) {
        if (gus_nodes[i].addr == addr) {
            break;
        }
    }

    if (i==node_count) {
        ++node_count;
    }

    strncpy(gus_nodes[i].name, name, MAX_NAME_LENGTH);
    gus_nodes[i].addr = addr;
    gus_nodes[i].patient_zero = patient_zero;
    gus_nodes[i].infected = patient_zero;
    gus_nodes[i].mask = mask;
    gus_nodes[i].vaccine = vaccine;

    qsort(gus_nodes, node_count, sizeof(struct gus_node), cmpfunc);
    gui_update_namelist();
}

char * get_name(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].name;
}

void set_name(int index, const char * name)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
     strncpy(gus_nodes[index].name, name, MAX_NAME_LENGTH);
}


bool everyone_infected(void) {
    for (int i=0; i<node_count; ++i) {
        if (!gus_nodes[i].infected) {
            return false;
        }
    }
    return true;
}

// fills buf with list of names separated by '\n'
void gd_get_namelist(char * buf, int length)
{
    int pos = 0;
    buf[0] = '\0';
    for (int i=0; i<node_count && i<MAX_GUS_NODES; ++i) {
        if (gus_nodes[i].addr == 0) {
            break;
        }
        strncpy(&buf[pos], status_symbol(i), length - pos); 
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        strncpy(&buf[pos], gus_nodes[i].name, length - pos);
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        if (i != node_count-1) {
            strncpy(&buf[pos], "\n", length - pos); 
            pos = strlen(buf);
            __ASSERT_NO_MSG(pos < length);
        }
    }
}

uint16_t gd_get_node_count(void) 
{
    __ASSERT_NO_MSG(node_count < MAX_BADGES);
    return node_count;
}

void reset_exposures(bool update)
{

    // publish health to all
    if (update) {
        model_handler_set_state(0, BT_MESH_GUS_HEALTHY);
    }
printk("All green\n");
    for (int i=0; i < gd_get_node_count(); ++i) {
        set_exposure(i, 0);
        set_infected(i, is_patient_zero(i));
        if (update) {
printk("patient zero %d\n", i);
            update_node_health_state(i, true);
        }
    }
}

