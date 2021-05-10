#ifndef __GUS_DATA_H
#define __GUS_DATA_H

// returns: number of badges that responded to scan
uint16_t get_badge_count(void);

// index: index of the badge
// returns: true is badge is initially infected
bool get_patient_zero(int index);

// index: index of the badge
// patient_zero: true if initially infected
void set_patient_zero(int index, bool patient_zero);

// index: index of the badge
// returns: true if the badge is wearing a mask
bool get_mask(int index);

// index: index of the badge
// masked: true if the badge is wearing a mask
void set_mask(int index, bool masked);

// index: index of the badge
// returns: true if badge has the vaccine
bool get_vaccine(int index);

// index: index of the badge
// vaccine: true if the badge has the vaccine
void set_vaccine(int index, bool vaccine);

// index: index of the badge
// returns: the node address associated with the badge
uint16_t get_badge_address(int index);

// addr: a node address
// returns: the index of the badge that is att address addr
uint16_t get_badge_index_from_address(uint16_t addr);

// index: index of the badge
// returns: true if the badge is infected
bool get_infected(int index);

// index: index of the badge
// infected: set the badge infection to infected
void set_infected(int index, bool infected);

// returns: true if all badges are infected
bool everyone_infected(void);

// returns: the total number of infections
uint16_t total_infections(void);

// index: index of the badge
// returns: the name associated with the badge
char *get_badge_name(int index);

// index: index of the badge
// name: string containing the new name for the badge
void set_badge_name(int index, const char *name);

// Takes all the names and puts them in a single string and separates
// them with \n.  Used to initialize the roller control.
// string to fill with the list of names
// length: length of the buffer
void get_badge_namelist(char *buf, int length);

// adds a new badge record to the gus data and sort the list by name.
// name: name of badge
// addr: node address of badge
// patient_zero: true if
void add_new_badge(const char *name, uint16_t addr, bool patient_zero, bool mask, bool vaccine);

// empties the node list
void clear_badge_list(void);

// index: index of the badge
// returns:
uint32_t get_exposure(int index);

// index: index of the badge
// exposure: the amount of exposure the badge has to other badges
void set_exposure(int index, uint32_t exposure);

// adds to the nodes total exposure
// index: index of the badge
// exposure: the amount of exposure the badge has to other badges
// update: if true, sends a set state message to the badge
void add_exposure_to_badge(int index, uint32_t exposure, bool update);

// initializes the exposure of all nodes to zero
// update: if true, publishes a set state healthy message to all badges
//      then sends a set state message infected to all patient zero badges
void reset_exposures(bool update);

// initializes the simulation settings
void init_badge_data(void);

#endif //__GUS_DATA_H
