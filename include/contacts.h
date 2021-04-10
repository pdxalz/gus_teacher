#ifndef CONTACTS_H_
#define CONTACTS_H_

//void init_contacts(void);
uint16_t get_total_contacts(void);
uint16_t get_start_time(int index);
uint16_t get_end_time(int index);
uint16_t get_distance_squared(int index);
uint8_t get_contact_badgeA(int index);
uint8_t get_contact_badgeB(int index);
void simulate_contacts(void);
#endif /* CONTACTS_H_ */
