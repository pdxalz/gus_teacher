#include <zephyr.h>
#include <stdio.h>
#include <string.h>
#include "gus_config.h"
#include "gus_data.h"
#include <lvgl.h>

struct gus_node {
    char name[MAX_NAME_LENGTH];
    uint16_t addr;
    bool virus;
    bool mask;
    bool vaccine;
};

struct gus_node gus_list[MAX_GUS_NODES];

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
    for (int i=0; i<MAX_GUS_NODES; ++i) {
        gd_add_node(i, "", 0, false, false, false);
    }

// test data
    gd_add_node(0, "Alan", 1, true, false, false);
    gd_add_node(1, "Ally", 2, false, true, false);
    gd_add_node(2, "Brenda", 3, false, false, true);
    gd_add_node(3, "Bryan", 4, false, false, false);
    gd_add_node(4, "Carol", 5, false, false, false);
    gd_add_node(5, "Craig", 6, false, false, false);
    gd_add_node(6, "Dalene", 7, false, false, false);
    gd_add_node(7, "Darrell", 8, false, false, false);
    gd_add_node(8, "Eric", 9, false, false, false);

}

void gd_add_node(int index, char * name, uint16_t addr, bool virus, bool mask, bool vaccine)
{
    __ASSERT_NO_MSG(index < MAX_GUS_NODES);

    strncpy(gus_list[index].name, name, MAX_NAME_LENGTH);
    gus_list[index].addr = addr;
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
        if (gus_list[i].addr == 0) {
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
