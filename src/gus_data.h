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
uint16_t get_address(int index);
void gd_add_node(const char * name, uint16_t addr, bool patient_zero, bool mask, bool vaccine);
uint16_t gd_get_node_count(void);
void gd_get_namelist(char * buf, int length);
bool get_infected(int index);
void set_infected(int index, bool infected);
uint32_t get_exposure(int index);
void set_exposure(int index, uint32_t exposure);
void gd_add_exposure(int index, uint32_t exposure, bool update);
char * get_name(int index);
void set_name(int index, const char * name);
void reset_exposures(bool update);
bool everyone_infected(void);
uint16_t total_infections(void);
uint16_t get_badge_from_address(uint16_t addr);

#endif  //__GUS_DATA_H
