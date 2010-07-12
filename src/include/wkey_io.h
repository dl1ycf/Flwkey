#ifndef _WKEY_IO_H
#define _WKEY_IO_H

#include <cstring>
#include <cmath>

#include <FL/Fl.H>

using namespace std;

enum BAUDS {
	BR300, BR600, BR1200, BR2400, BR4800, BR9600, BR19200, 
	BR38400, BR57600, BR115200, BR230400, BR460800 };

extern const char *szBaudRates[];

extern bool start_wkey_serial();

extern int readString();
extern unsigned char readByte();
extern int sendString(string &s, bool loghex = false);

extern void clearSerialPort();
extern char replybuff[];
extern string replystr;

#endif
