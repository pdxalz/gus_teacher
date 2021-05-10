#ifndef __SIM_SETTINGS_H
#define __SIM_SETTINGS_H

//////////////////////////////////////////////////////////////////
// This module maintains a list of properties used to configure
// the simulation
//////////////////////////////////////////////////////////////////

// settings properties get and set functions
bool get_sim_proximity(void);
void set_sim_proximity(bool prox);
uint8_t get_sim_rows(void);
void set_sim_rows(uint8_t rows);
uint8_t get_sim_spacing(void);
void set_sim_spacing(uint8_t spacing);
uint8_t get_sim_rate(void);
void set_sim_rate(uint8_t _infection_rate);

// initialize the settings properties
void init_sim_settings(void);
#endif  //__SIM_SETTINGS_H
