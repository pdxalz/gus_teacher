#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include "sim_settings.h"

#include <lvgl.h>


////////////////////////
// declarations
////////////////////////
struct _sim_config
{
    bool proximity;
    uint8_t rows;
    uint8_t spacing;
    uint8_t rate;
};

static struct _sim_config _sim_config;

/////////////////////
// global functions
/////////////////////
bool get_sim_proximity(void)
{
    return _sim_config.proximity;
}

void set_sim_proximity(bool prox)
{
    _sim_config.proximity = prox;
}

uint8_t get_sim_rows(void)
{
    return _sim_config.rows;
}

void set_sim_rows(uint8_t rows)
{
    _sim_config.rows = rows;
}

uint8_t get_sim_spacing(void)
{
    return _sim_config.spacing;
}

void set_sim_spacing(uint8_t spacing)
{
    _sim_config.spacing = spacing;
}

uint8_t get_sim_rate(void)
{
    return _sim_config.rate;
}

void set_sim_rate(uint8_t _infection_rate)
{
    _sim_config.rate = _infection_rate;
}

void init_sim_settings(void)
{
    _sim_config.proximity = false;
    _sim_config.rows = 3;
    _sim_config.spacing = 4;
    _sim_config.rate = 12;
}
