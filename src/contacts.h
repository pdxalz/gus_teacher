#ifndef CONTACTS_H_
#define CONTACTS_H_

/////////////////////////////////////////////////////////////////////////////////
// This module manages the list of contacts between pairs of badges
// Contacts tracks when two badges are in close proximity.
// A new contact is added when two badges come near each other.
// A contact record contains the mesh node address of each of the badges,
// the start and end time of the contact and the distance of the contact.
// Contacts can be created in Classroom mode where distances are calculated, or
// in GUS_Tag mode where distances are approximated using the signal strength
// between badges.
// time is in seconds from when the contact calculation begins or when recording
// is started.
// badge addresses are limited to the range of 1-255
/////////////////////////////////////////////////////////////////////////////////

// Contact record Get access functions

// returns: total number of contact records
uint16_t get_total_contacts(void);

// index : index of the contact record
// returns : time the contact started
uint16_t get_start_time(int index);

// index : index of the contact record
// returns : time the contact ended
uint16_t get_end_time(int index);

// index : index of the contact record
// returns : square of the distance between the badges
uint16_t get_distance_squared(int index);

// index : index of the contact record
// returns : node address of first badge
uint8_t get_contact_badgeA(int index);

// index : index of the contact record
// returns : node address of second badge
uint8_t get_contact_badgeB(int index);

// returns : the time of the last contact
uint16_t final_time(void);

// Creates the list of contacts for classroom mode.  Assumes badges are in order
// of badge names and organized in one or more rows defined by the row parameter.
// rows : number of rows badges
// space : the distance between two adjacent badges.
void simulate_contacts( uint8_t rows, uint8_t space);

// resets the contact list and sets the start time
void reset_proximity_contacts(void);

// Adds a new contact between two badges.  The distance is calculated based on
// the rssi value
// addr1 : node address of the first badge
// addr2 : node address of the second badge
void add_proximity_contact(uint16_t addr1, uint16_t addr2, int8_t rssi);

#endif /* CONTACTS_H_ */
