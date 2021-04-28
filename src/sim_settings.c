#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include "sim_settings.h"


#include <lvgl.h>

struct sim_config {
    bool proximity;
    uint8_t rows;
    uint8_t spacing;
    uint8_t rate;
};
struct sim_config sim_config;

bool get_sim_proximity(void) 
{ 
    return sim_config.proximity;
}

void set_sim_proximity(bool prox) 
{  
    sim_config.proximity = prox;
}

uint8_t get_sim_rows(void) 
{   
    return sim_config.rows;
}

void set_sim_rows(uint8_t rows) 
{
    sim_config.rows = rows;
}

uint8_t get_sim_spacing(void) 
{ 
    return sim_config.spacing;
}

void set_sim_spacing(uint8_t spacing) 
{
    sim_config.spacing = spacing;
}

uint8_t get_sim_rate(void) 
{ 
    return sim_config.rate;
}

void set_sim_rate(uint8_t rate) 
{
    sim_config.rate = rate;
}

void init_sim_settings(void)
{
    sim_config.proximity = false;
    sim_config.rows = 3;
    sim_config.spacing = 4;
    sim_config.rate = 12;
}

