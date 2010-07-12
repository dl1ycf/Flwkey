#ifndef FL_LOGBOOK_H
#define FL_LOGBOOK_H


#include <cstring>

#include "flwkey.h"

#include "lgbook.h"
#include "logsupport.h"

extern std::string log_checksum;

extern pthread_t logbook_thread;
extern pthread_mutex_t logbook_mutex;

extern void start_logbook();
extern void close_logbook();


#endif
