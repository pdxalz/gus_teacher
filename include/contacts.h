#ifndef CONTACTS_H_
#define CONTACTS_H_

//void init_contacts(void);
uint16_t get_total_contacts(void);
uint16_t get_start_time(int index);
uint16_t get_end_time(int index);
uint16_t get_distance_squared(int index);
uint8_t get_contact_badgeA(int index);
uint8_t get_contact_badgeB(int index);
uint16_t final_time(void);
void simulate_contacts( uint8_t rows, uint8_t space);
void add_contact(uint16_t badgeA, uint16_t badgeB, uint32_t start_time, uint32_t end_time, double distance);
void reset_proximity_contacts(void);
void add_proximity_contact(uint16_t badgeA, uint16_t badgeB, int8_t rssi);
#endif /* CONTACTS_H_ */
