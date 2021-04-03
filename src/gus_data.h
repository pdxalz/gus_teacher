#ifndef __GUS_DATA_H
#define __GUS_DATA_H

//#include <zephyr.h>

void gd_init(void);
void gd_add_node(int index, char * name, uint16_t addr, bool virus, bool mask, bool vaccine);
void gd_get_namelist(char * buf, int length);
#endif  //__GUS_DATA_H
