#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include "gus_config.h"
#include "gus_data.h"
#include "contacts.h"
#include "sim_settings.h"
#include "model_handler.h"

#include <lvgl.h>

#define ELEMENT_NOT_CONNECTED 0xffff
#define INFECTION_THRESHOLD 200

struct gus_node {
    char name[MAX_NAME_LENGTH];
    uint16_t element;
    bool patient_zero;
    bool infected;
    bool mask;
    bool vaccine;
    uint32_t exposure;
};

struct gus_node gus_nodes[MAX_GUS_NODES];
uint16_t node_count;


static void update_node_health_state(uint8_t index)
{
    gus_state_t state = GUS_ST_HEALTHY;

    if (get_infected(index)) {
        state = GUS_ST_INFECTED;
    } else if (has_mask(index)) {
        state = GUS_ST_MASKED;        
    } else if (has_vaccine(index)) {
        state = GUS_ST_VACCINE;        
    }
    model_handler_set_state(get_element(index), state);
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
            update_node_health_state(index);
        }
    }
}





uint16_t get_element(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_nodes[index].element;
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

#if 0 // test data
    gd_add_node(0, "Alan", 1, true, false, false);
    gd_add_node(1, "Ally", 2, false, true, false);
    gd_add_node(2, "Brenda", 3, false, false, true);
    gd_add_node(3, "Bryan", 4, false, false, false);
    gd_add_node(4, "Carol", 5, false, false, false);
    gd_add_node(5, "Craig", 6, false, false, false);
    gd_add_node(6, "Dalene", 7, false, false, false);
    gd_add_node(7, "Darrell", 8, false, false, false);
    gd_add_node(8, "Eric", 9, false, false, false);
#endif
}

void gd_add_node(int index, char * name, uint16_t element, bool patient_zero, bool mask, bool vaccine)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);

    strncpy(gus_nodes[index].name, name, MAX_NAME_LENGTH);
    gus_nodes[index].element = element;
    gus_nodes[index].patient_zero = patient_zero;
    gus_nodes[index].infected = patient_zero;
    gus_nodes[index].mask = mask;
    gus_nodes[index].vaccine = vaccine;

    ++node_count;
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
    for (int i=0; i<MAX_GUS_NODES; ++i) {
        if (gus_nodes[i].element == ELEMENT_NOT_CONNECTED) {
            break;
        }
        strncpy(&buf[pos], status_symbol(i), length - pos); 
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        strncpy(&buf[pos], gus_nodes[i].name, length - pos);
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        strncpy(&buf[pos], "\n", length - pos); 
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);
    }
}

uint16_t gd_get_node_count(void) 
{
    __ASSERT_NO_MSG(node_count < MAX_BADGES);
    return node_count;
}

void reset_exposures(bool update)
{
    for (int i=0; i < gd_get_node_count(); ++i) {
        set_exposure(i, 0);
        set_infected(i, is_patient_zero(i));
        if (update) {
            update_node_health_state(i);
        }
    }
}

