#include <math.h>
#include <string>

#include <FL/Fl.H>

#include "flwkey.h"
#include "support.h"
#include "util.h"
#include "debug.h"
#include "status.h"
#include "wkey_io.h"

using namespace std;

extern bool test;

const char *nuline = "\n";

static int iBaudRates[] = { 300, 600, 1200, 2400, 4800, 9600,
	19200, 38400, 57600, 115200, 230400, 460800 };
const char *szBaudRates[] = { "300", "600", "1200", "2400", "4800", "9600",
	"19200", "38400", "57600", "115200", "230400", "460800", NULL };

int BaudRate(int n)
{
	if (n > (int)sizeof(iBaudRates)) return 1200;
	return (iBaudRates[n]);
}

bool start_wkey_serial()
{
//	bypass_serial_thread_loop = true;
// setup commands for serial port

	WKEY_serial.ClosePort();

	if (progStatus.serial_port_name == "NONE") return false;

	WKEY_serial.Device(progStatus.serial_port_name);
	WKEY_serial.Baud(1200);
	WKEY_serial.Stopbits(2);
	WKEY_serial.Retries(1);
	WKEY_serial.Timeout(50);
	WKEY_serial.RTSptt(false);
	WKEY_serial.DTRptt(false);
	WKEY_serial.RTSCTS(false);
	WKEY_serial.RTS(false);
	WKEY_serial.DTR(true);

	if (!WKEY_serial.OpenPort()) {
		LOG_ERROR("Cannot access %s", progStatus.serial_port_name.c_str());
		return false;
	} else if (debug::level == debug::DEBUG_LEVEL) {
		LOG_DEBUG("\n\
Serial port:\n\
  Port     : %s\n\
  Baud     : %d\n\
  Stopbits : %d\n\
  Timeout  : %d\n\
  DTR      : %s\n\
  RTS/CTS  : %s",
	progStatus.serial_port_name.c_str(),
	WKEY_serial.Baud(),
	WKEY_serial.Stopbits(),
	WKEY_serial.Timeout(),
	WKEY_serial.DTR() ? "true" : "false",
	WKEY_serial.RTSCTS() ? "true" : "false");
	}

	MilliSleep(400);	// to allow WK1 to wake up
	WKEY_serial.FlushBuffer();

	return true;
}

#define RXBUFFSIZE 512
char replybuff[RXBUFFSIZE+1];
string replystr;

bool readByte(unsigned char &byte)
{
	return WKEY_serial.ReadByte(byte);
}

int readString()
{
	int numread = 0;
	size_t n;
	memset(replybuff, 0, RXBUFFSIZE + 1);
	while (numread < RXBUFFSIZE) {
		if ((n = WKEY_serial.ReadBuffer(&replybuff[numread], RXBUFFSIZE - numread)) == 0) break;
		numread += n;
	}
	replystr.append(replybuff);
	return numread;
}

int sendString (string &s, bool b)
{
	if (WKEY_serial.IsOpen() == false) {
		LOG_DEBUG("command: %s", b ? str2hex(s.data(), s.length()) : s.c_str());
		return 0;
	}
	int numwrite = (int)s.length();
	if (WKEY_DEBUG)
		LOG_WARN("%s", b ? str2hex(s.data(), numwrite) : s.c_str());

	WKEY_serial.WriteBuffer(s.c_str(), numwrite);
	return numwrite;
}

void clearSerialPort()
{
	if (WKEY_serial.IsOpen() == false) return;
	WKEY_serial.FlushBuffer();
}

