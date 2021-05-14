#include <zephyr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lvgl.h>
#include "gus_data.h"
#include "gus_config.h"
#include "contacts.h"
#include "sim_settings.h"
#include "model_handler.h"
#include "gui.h"

////////////////////////
// declarations
////////////////////////
struct gus_node
{
    char name[MAX_NAME_LENGTH];
    uint16_t addr;
    bool patient_zero;
    bool infected;
    bool mask;
    bool vaccine;
    uint32_t exposure;
};

////////////////////
// static variables
////////////////////
struct gus_node _gus_badges[MAX_GUS_BADGES];
uint16_t _badge_count;

/////////////////////
// Static functions
/////////////////////

// updates the state of the badge to reflect the current health state
// index: index of the badge
// initializing: used to avoid unnecessary healthy state messages.
static void update_badge_health_state(uint8_t index, bool initializing)
{
    enum bt_mesh_gus_state state = BT_MESH_GUS_HEALTHY;

    state = get_vaccine(index) ? (get_mask(index) ? (get_infected(index) ? BT_MESH_GUS_VACCINATED_MASKED_INFECTED
                                                                         : BT_MESH_GUS_VACCINATED_MASKED)
                                                  : (get_infected(index) ? BT_MESH_GUS_VACCINATED_INFECTED
                                                                         : BT_MESH_GUS_VACCINATED))
                               : (get_mask(index) ? (get_infected(index) ? BT_MESH_GUS_MASKED_INFECTED
                                                                         : BT_MESH_GUS_MASKED)
                                                  : (get_infected(index) ? BT_MESH_GUS_INFECTED
                                                                         : BT_MESH_GUS_HEALTHY));

    if (initializing && (state == BT_MESH_GUS_HEALTHY))
    {
        return;
    }
    //    printk("update node %d %d %d\n", index, state, initializing);
    //    k_sleep(K_MSEC(120));
    model_handler_set_state(get_badge_address(index), state);
}

// returns a special character used to indicate infected, masked, or vaccinated
// in the namelist roller control.
char *status_symbol(int index)
{
    if (_gus_badges[index].patient_zero)
    {
        return LV_SYMBOL_SETTINGS;
    }
    if (_gus_badges[index].vaccine)
    {
        return LV_SYMBOL_PLUS;
    }
    if (_gus_badges[index].mask)
    {
        return LV_SYMBOL_WIFI;
    }
    return "  ";
}

// string compare function, used to sort the gus data array
int cmpfunc(const void *a, const void *b)
{
    return strcmp(((struct gus_node *)a)->name, ((struct gus_node *)b)->name);
    //   return ( *(int*)a - *(int*)b );
}

/////////////////////
// global functions
/////////////////////
uint16_t get_badge_count(void)
{
    __ASSERT_NO_MSG(_badge_count < MAX_GUS_BADGES);
    return _badge_count;
}

bool get_patient_zero(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].patient_zero;
}

void set_patient_zero(int index, bool patient_zero)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    _gus_badges[index].patient_zero = patient_zero;
}

bool get_mask(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].mask;
}

void set_mask(int index, bool mask)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    _gus_badges[index].mask = mask;
}

bool get_vaccine(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].vaccine;
}

void set_vaccine(int index, bool vaccine)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    _gus_badges[index].vaccine = vaccine;
}

uint16_t get_badge_address(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].addr;
}

uint16_t get_badge_index_from_address(uint16_t addr)
{
    for (int i = 0; i < _badge_count; ++i)
    {
        if (addr == _gus_badges[i].addr)
        {
            return i;
        }
    }
    __ASSERT_NO_MSG(1);
    return 0;
}

bool get_infected(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].infected;
}

void set_infected(int index, bool infected)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    _gus_badges[index].infected = infected;
}

bool everyone_infected(void)
{
    for (int i = 0; i < _badge_count; ++i)
    {
        if (!_gus_badges[i].infected)
        {
            return false;
        }
    }
    return true;
}

uint16_t total_infections(void)
{
    uint16_t count = 0;
    for (int i = 0; i < _badge_count; ++i)
    {
        if (_gus_badges[i].infected)
        {
            ++count;
        }
    }
    return count;
}

char *get_badge_name(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].name;
}

void set_badge_name(int index, const char *name)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    strncpy(_gus_badges[index].name, name, MAX_NAME_LENGTH);
}

// fills buf with list of names separated by '\n'
void get_badge_namelist(char *buf, int length)
{
    int pos = 0;
    buf[0] = '\0';
    for (int i = 0; i < _badge_count && i < MAX_GUS_BADGES; ++i)
    {
        if (_gus_badges[i].addr == 0)
        {
            break;
        }
        strncpy(&buf[pos], status_symbol(i), length - pos);
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        strncpy(&buf[pos], _gus_badges[i].name, length - pos);
        pos = strlen(buf);
        __ASSERT_NO_MSG(pos < length);

        if (i != _badge_count - 1)
        {
            strncpy(&buf[pos], "\n", length - pos);
            pos = strlen(buf);
            __ASSERT_NO_MSG(pos < length);
        }
    }
}

void add_new_badge(const char *name, uint16_t addr, bool patient_zero, bool mask, bool vaccine)
{
    int i;

    if (_badge_count >= MAX_GUS_BADGES)
    {
        return;
    }

    for (i = 0; i < _badge_count; ++i)
    {
        if (_gus_badges[i].addr == addr)
        {
            break;
        }
    }

    if (i == _badge_count)
    {
        ++_badge_count;
    }

    strncpy(_gus_badges[i].name, name, MAX_NAME_LENGTH);
    _gus_badges[i].addr = addr;
    _gus_badges[i].patient_zero = patient_zero;
    _gus_badges[i].infected = patient_zero;
    _gus_badges[i].mask = mask;
    _gus_badges[i].vaccine = vaccine;

    qsort(_gus_badges, _badge_count, sizeof(struct gus_node), cmpfunc);
    gui_update_namelist();
}

void clear_badge_list(void)
{
    _badge_count = 0;
}

uint32_t get_exposure(int index)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    return _gus_badges[index].exposure;
}

void set_exposure(int index, uint32_t exposure)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    _gus_badges[index].exposure = exposure;
}

void add_exposure_to_badge(int index, uint32_t exposure, bool update)
{
    __ASSERT_NO_MSG(index < MAX_GUS_BADGES);
    _gus_badges[index].exposure += exposure;

    if (_gus_badges[index].exposure > INFECTION_THRESHOLD)
    {
        set_infected(index, true);
        if (update)
        {
            update_badge_health_state(index, false);
        }
    }
}

void reset_exposures(bool update)
{

    // publish health to all
    if (update)
    {
        model_handler_set_state(0, BT_MESH_GUS_HEALTHY);
    }
    printk("All green\n");
    for (int i = 0; i < get_badge_count(); ++i)
    {
        set_exposure(i, 0);
        set_infected(i, get_patient_zero(i));
        if (update)
        {
            printk("patient zero %d\n", i);
            update_badge_health_state(i, true);
        }
    }
}

void init_badge_data(void)
{
    init_sim_settings();
}
