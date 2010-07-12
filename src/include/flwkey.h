#ifndef _Rig_H
#define _Rig_H

#include <string>
#include <sys/types.h>
#include <pthread.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Enumerations.H>

#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif

#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/fl_draw.H>

#include "config.h"

#include "serial.h"
#include "support.h"

extern Fl_Double_Window *mainwindow;
extern string WKeyHomeDir;

extern string defFileName;
extern string title;

extern pthread_t *serial_thread;
extern pthread_mutex_t mutex_serial;

extern bool WKEY_DEBUG;

#endif
