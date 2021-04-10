#ifndef __GUS_DATA_H
#define __GUS_DATA_H

//#include <zephyr.h>

#define NAMESIZE 14
#define MAX_BADGES 32


void gd_init(void);

bool is_patient_zero(int index);
void set_patient_zero(int index, bool patient_zero);
bool has_mask(int index);
void set_masked(int index, bool masked);
bool has_vaccine(int index);
void set_vaccine(int index, bool vaccine);
uint16_t get_element(int index);
void gd_add_node(int index, char * name, uint16_t addr, bool virus, bool mask, bool vaccine);
uint16_t gd_get_node_count(void);
void gd_get_namelist(char * buf, int length);
bool get_infected(int index);
void set_infected(int index, bool infected);
uint32_t get_exposure(int index);
void set_exposure(int index, uint32_t exposure);
void gd_add_exposure(int index, uint32_t exposure);
char * get_name(int index);
void reset_exposures(void);

#endif  //__GUS_DATA_H
