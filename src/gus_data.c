#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include "gus_config.h"
#include "gus_data.h"
#include <lvgl.h>

#define ELEMENT_NOT_CONNECTED 0xffff

struct gus_node {
    char name[MAX_NAME_LENGTH];
    uint16_t element;
    bool virus;
    bool mask;
    bool vaccine;
};

struct gus_node gus_list[MAX_GUS_NODES];

struct sim_config {
    bool proximity;
    uint8_t rows;
    uint8_t spacing;
    uint8_t rate;
};
struct sim_config sim_config;

bool get_sim_proximity(void) { return sim_config.proximity;}
void set_sim_proximity(bool prox) {sim_config.proximity = prox;}
uint8_t get_sim_rows(void) { return sim_config.rows;}
void set_sim_rows(uint8_t rows) {sim_config.rows = rows;}
uint8_t get_sim_spacing(void) { return sim_config.spacing;}
void set_sim_spacing(uint8_t spacing) {sim_config.spacing = spacing;}
uint8_t get_sim_rate(void) { return sim_config.rate;}
void set_sim_rate(uint8_t rate) {sim_config.rate = rate;}


bool has_virus(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_list[index].virus;
}

void set_infected(int index, bool virus)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_list[index].virus =  virus;
}

bool has_mask(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_list[index].mask;
}

void set_masked(int index, bool mask)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_list[index].mask =  mask;
}


bool has_vaccine(int index) 
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_list[index].vaccine;
}

void set_vaccine(int index, bool vaccine)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    gus_list[index].vaccine =  vaccine;
}


uint16_t get_element(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);
    return gus_list[index].element;
}



char * status_symbol(int index)
{
    if (gus_list[index].virus) {
        return LV_SYMBOL_SETTINGS;
    }
    if (gus_list[index].vaccine) {
        return LV_SYMBOL_PLUS;
    }
    if (gus_list[index].mask) {
        return LV_SYMBOL_WIFI;
    }
    return "  ";
}


void gd_init(void)
{
    sim_config.proximity = false;
    sim_config.rows = 3;
    sim_config.spacing = 4;
    sim_config.rate = 12;

    for (int i=0; i<MAX_GUS_NODES; ++i) {
        gd_add_node(i, "", ELEMENT_NOT_CONNECTED, false, false, false);
    }

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

void gd_add_node(int index, char * name, uint16_t element, bool virus, bool mask, bool vaccine)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);

    strncpy(gus_list[index].name, name, MAX_NAME_LENGTH);
    gus_list[index].element = element;
    gus_list[index].virus = virus;
    gus_list[index].mask = mask;
    gus_list[index].vaccine = vaccine;
}

// fills buf with list of names separated by '\n'
void gd_get_namelist(char * buf, int length)
{
    int pos = 0;
    buf[0] = '\0';
    for (int i=0; i<MAX_GUS_NODES; ++i) {
        if (gus_list[i].element == ELEMENT_NOT_CONNECTED) {
            break;
        }
        strncpy(&buf[pos], status_symbol(i), length - pos); 
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        strncpy(&buf[pos], gus_list[i].name, length - pos);
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        strncpy(&buf[pos], "\n", length - pos); 
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);
    }
}
