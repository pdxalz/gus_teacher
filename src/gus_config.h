#ifndef __GUS_CONFIG_H
#define __GUS_CONFIG_H


#define MAX_CONTACTS 5000       // Maximum number of contact records allowed
#define SAFE_DISTANCE 100       // Contact records not added if over this
#define TIME_PER_CONTACT (1 * 60 * 10)  // time between calculated records
#define RSSI_TUNE_CONSTANT 80   // constant used to convert RSSI values to
                                // reasonable distances between badges.
#define MAX_GUS_NODES 10        // maximum number of badges allowed.  10 have
                                // been test with no noticable performance
                                // issues.       
#define MAX_NAME_LENGTH 12      // maximum length of a students name

#define PLAY_TIMER_VALUE K_SECONDS(2)   // Analyze playback speed
#define RECORD_TIMER_VALUE K_SECONDS(5) // Speed badge proximity is checked

#endif  //__GUS_CONFIG_H
