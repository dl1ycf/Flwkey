#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>

#include "support.h"
#include "debug.h"
#include "gettext.h"
#include "wkey_io.h"
#include "dialogs.h"
#include "wkey_dialogs.h"
#include "status.h"
#include "logbook.h"

using namespace std;

Cserial WKEY_serial;

//=============================================================================
// WinKey command sequences
//=============================================================================
// ADMIN MODE
const char ADMIN		= '\x00';
const char CALIBRATE	= '\x00';
const char RESET		= '\x01';
const char HOST_OPEN	= '\x02';
const char HOST_CLOSE	= '\x03';
const char ECHO_TEST	= '\x04';
const char PADDLE_A2D	= '\x05';
const char SPEED_A2D	= '\x06';
const char GET_VALUES	= '\x07';
const char GET_CAL		= '\x09';
const char WK1_MODE		= '\x0A';
const char WK2_MODE		= '\x0B';
const char DUMP_EEPROM	= '\x0C';
const char LOAD_EEPROM	= '\x0D';

const char *SEND_MSG_NBR	= "\x0E";

// HOST MODE
const char *SIDETONE		= "\x01";	// N see table page 6 Interface Manual
const char *SET_WPM		= "\x02";	// 5 .. N .. 99 in WPM
const char *SET_WEIGHT		= "\x03";	// 10 .. N .. 90 %
const char *SET_PTT_LT		= "\x04";	// A - lead time, B - tail time
										// both 0..250 in 10 msec steps
										// "0x04<01><A0> = 10 msec lead, 1.6 sec tail
const char *SET_SPEED_POT	= "\x05";	// A = min, B = range, C anything
const char *PAUSE			= "\x06";	// 1 = pause, 0 = release
const char *GET_SPEED_POT 	= "\x07"; 	// return values as per page 7/8
const char *BACKSPACE		= "\x08";
const char *SET_PIN_CONFIG	= "\x09";	// N as per tables page 8
const char *CLEAR_BUFFER	= "\x0A";
const char *KEY_IMMEDIATE	= "\x0B";	// 0 = keyup, 1 = keydown
const char *HSCW			= "\x0C";	// N = lpm / 100
const char *FARNS_WPM		= "\x0D";	// 10 .. N .. 99
const char *SET_WK2_MODE	= "\x0E";	// N as per table page 9
const char *LOAD_DEFAULTS	= "\x0F";
	// A = MODE REGISTER	B = SPEED IN WPM		C = SIDETONE FREQ
	// D = WEIGHT			E = LEAD-IN TIME		F = TAIL TIME
	// G = MIN_WPM			H = WPM RANGE			I = 1ST EXTENSION
	// J = KEY COMPENSATION	K = FARNSWORTH WPM		L = PADDLE SETPOINT
	// M = DIT/DAH RATIO	N = PIN_CONFIGURATION	O = DONT CARE ==> zero
const char *FIRST_EXT		= "\x10";		// see page 10/11
const char *SET_KEY_COMP	= "\x11";	// see page 11
const char *NULL_CMD		= "\023\023\023";
const char *PADDLE_SW_PNT	= "\x12";	// 10 .. N .. 90%
const char *SOFT_PADDLE		= "\x14";	// 0 - up, 1 - dit, 2 - dah, 3 - both
const char *GET_STATUS		= "\x15";	// request status byte, see page 12
const char *POINTER_CMD		= "\x16";	// see page 12
const char *SET_DIT_DAH		= "\x17";	// 33 .. N .. 66 N = ratio * 50 / 3

// BUFFERED COMMANDS
const char *PTT_ON_OFF		= "\x18";	// 1 = on 0 = off
const char *KEY_BUFFERED	= "\x19";	// 0 .. N .. 99 seconds
const char *WAIT			= "\x1A";	// 0 .. N .. 99 seconds
const char *MERGE			= "\x1B";	// merger CD into prosign, ie AR, SK etc
const char *CHANGE_BFR_SPD	= "\x1C";	// 5 .. N .. 99
const char *CHANGE_HSCW_SPD	= "\x1D";	// N = lpm / 100
const char *CANCEL_BFR_SPD	= "\x1E";
const char *BUFFER_NOP		= "\x1F";

//=============================================================================
// loop for serial i/o thread
// runs continuously until program is closed
// only accesses the serial port if it has been successfully opened
//=============================================================================

void version_(int);
void echo_test(int);
void status_(int);
void speed_(int);
void echo_(int);
void eeprom_(int);

bool bypass_serial_thread_loop = true;
bool run_serial_thread = true;

bool PTT = false;
int  powerlevel = 0;

string str_out;
bool get_version = false;
bool test_echo = false;
bool host_is_up = false;
bool wk2_version = false;
bool read_EEPROM = false;

int  wkeyer_ready = true;

void upcase(string &s)
{
	for (size_t n = 0; n < s.length(); n++) s[n] = toupper(s[n]);
}

void noctrl(string &s)
{
	for (size_t n = 0; n < s.length(); n++)
		if (s[n] < ' ' || s[n] > 'Z') s[n] = ' ';
}

enum {NOTHING, WAIT_ECHO, WAIT_VERSION};
void sendCommand(string &cmd, int what = NOTHING)
{
	int cnt = 101;
	while (cnt-- && !str_out.empty()) MilliSleep(1);
	if (!str_out.empty())
		LOG_ERROR("output string not cleared!");
	pthread_mutex_lock(&mutex_serial);
        //
        // no "upcase" when sending a command. This may have disastrous results
        // e.g. modifying the mode_register in load_defaults()
	// upcase(cmd);
	str_out = cmd;
	switch (what) {
		case WAIT_ECHO : 
			test_echo = true;
			break;
		case WAIT_VERSION :
			get_version = true;
			break;
		default: ;
	}
	pthread_mutex_unlock(&mutex_serial);
}

void sendText(string &cmd)
{
	int cnt = 101;
	while (cnt-- && !str_out.empty()) MilliSleep(1);
	if (!str_out.empty())
		LOG_ERROR("output string not cleared!");
	pthread_mutex_lock(&mutex_serial);
	upcase(cmd);
	noctrl(cmd);
	str_out = cmd;
	pthread_mutex_unlock(&mutex_serial);
}

bool echo_ok = true;

void send_char()
{
	if (!btn_send->value() || !wkeyer_ready)
		return;

	char c;
	c = txt_to_send->nextChar();
	if (c > -1) {
		c = toupper(c);
		if (c < ' ') c = ' ';
		if (c == '0' && progStatus.cut_zeronine) c = 'T';
		if (c == '9' && progStatus.cut_zeronine) c = 'N';
		str_out = c;
		echo_ok = false;
	}
}

static string zt_off;
void show_time(void *)
{
	txt_time_off->value(zt_off.c_str());
}

void * serial_thread_loop(void *d)
{
Fl::awake(show_time);

unsigned char byte;
	for(;;) {
		if (!run_serial_thread) break;

		MilliSleep(progStatus.serloop_timing);
		zt_off = szTime(0);
		if (zt_off != txt_time_off->value()) Fl::awake(show_time);

		if (bypass_serial_thread_loop ||
			!WKEY_serial.IsOpen()) goto serial_bypass_loop;

		pthread_mutex_lock(&mutex_serial);
// process outgoing
			if (!str_out.empty()) {
				sendString(str_out, true);
				str_out.clear();
			} else if (echo_ok)
				send_char();

			if (WKEY_serial.ReadByte(byte)) {
				if ((byte == 0xA5 || read_EEPROM))
					eeprom_(byte);
				else if ((byte & 0xC0) == 0xC0)
					status_(byte);
				else if ((byte & 0xC0) == 0x80)
					speed_(byte);
				else if (test_echo)
					echo_test(byte);
				else if (get_version)
					version_(byte);
				else
					echo_(byte);
				echo_ok = true;
			}
		pthread_mutex_unlock(&mutex_serial);
serial_bypass_loop: ;
	}
	return NULL;
}

void display_byte(void *d)
{
	long lch = (long)d;
	char ch = (char)lch;
	txt_sent->add(ch);
}

void display_chars(void *d)
{
	char *sz_chars = (char *)d;
	txt_sent->add(sz_chars);
}

void echo_(int byte)
{
	if (WKEY_DEBUG)
		LOG_WARN("%2x", byte & 0xFF);
	Fl::awake(display_byte, (void*)byte);
}

void echo_test(int byte)
{
	if (WKEY_DEBUG)
		LOG_WARN("%02X", byte & 0xFF);
	if (byte == 'U') {
		if (WKEY_DEBUG)
			LOG_WARN("passed %c", byte & 0xFF);
	} else
		LOG_ERROR("failed %c", byte & 0xFF);
	test_echo = false;
}

void version_(int byte)
{
	if (WKEY_DEBUG)
		LOG_WARN("%02X", byte & 0xFF);
	static char ver[200];
	snprintf(ver, sizeof(ver), "Flwkey version: %s\nWkeyer version %d\n", 
		PACKAGE_VERSION, byte & 0xFF);
	host_is_up = true;
	get_version = false;
	Fl::awake(display_chars, ver);
	if (byte >= 20) wk2_version = true;
}

void show_status_change(void *d)
{
	int byte = (int)(reinterpret_cast<long>(d));

	box_wait->color((byte & 0x10) == 0x10 ? FL_RED : FL_BACKGROUND2_COLOR);
	box_wait->redraw();

	box_keydown->color((byte & 0x08) == 0x08 ? FL_RED : FL_BACKGROUND2_COLOR);
	box_keydown->redraw();

	box_busy->color((byte & 0x04) == 0x04 ? FL_RED : FL_BACKGROUND2_COLOR);
	box_busy->redraw();

	box_break_in->color((byte & 0x02) == 0x02 ? FL_RED : FL_BACKGROUND2_COLOR);
	box_break_in->redraw();

	box_xoff->color((byte & 0x01) == 0x01 ? FL_RED : FL_BACKGROUND2_COLOR);
	box_xoff->redraw();
}

void status_(int byte)
{
	if (WKEY_DEBUG)
		LOG_WARN("%02X", byte & 0xFF);
	if ((byte & 0x04)== 0x04) wkeyer_ready = false;
	else wkeyer_ready = true;
	if (WKEY_DEBUG)
		LOG_WARN("Wait %c, Keydown %c, Busy %c, Breakin %c, Xoff %c", 
			byte & 0x10 ? 'T' : 'F',
			byte & 0x08 ? 'T' : 'F',
			byte & 0x04 ? 'T' : 'F',
			byte & 0x02 ? 'T' : 'F',
			byte & 0x01 ? 'T' : 'F');
	Fl::awake(show_status_change, (void *)byte);
}

void show_speed_change(void *d)
{
	long wpm = (long)d;
	int iwpm = (int)wpm;
	static char szwpm[8];
	snprintf(szwpm, sizeof(szwpm), "%3d", iwpm);
	txt_wpm->value(szwpm);
	txt_wpm->redraw();
	if (!progStatus.use_pot) return;
	string cmd = SET_WPM;
	cmd += iwpm;
	sendCommand(cmd);
}

void speed_(int byte)
{
	if (WKEY_DEBUG)
		LOG_WARN("%02X", byte & 0xFF);
	int val = (byte & 0x3F) + progStatus.min_wpm;
	Fl::awake(show_speed_change, (void *)(val));
	if (WKEY_DEBUG)
		LOG_WARN("wpm: %d", val);
}

void set_wpm()
{
	int wpm = (int)cntr_wpm->value();
	progStatus.speed_wpm = wpm;
	if (progStatus.use_pot) return;
	string cmd = SET_WPM;
	cmd += wpm;
	sendCommand(cmd);
}

void use_pot_changed()
{
	progStatus.use_pot = btn_use_pot->value();
	if (progStatus.use_pot) {
		string cmd = GET_SPEED_POT;
		sendCommand(cmd);
	} else {
		string cmd = SET_WPM;
		cmd += (int)cntr_wpm->value();
		sendCommand(cmd);
	}
}

char eeprom_image[256];
int eeprom_ptr = 0;
void eeprom_(int byte)
{
	if (WKEY_DEBUG)
		LOG_WARN("%02X", byte & 0xFF);
	if (byte == 0xA5) {
		memset( eeprom_image, 0, 256);
		eeprom_ptr = 0;
		read_EEPROM = true;
	}
	if (eeprom_ptr < 256)
		eeprom_image[eeprom_ptr++] = byte;
	if (eeprom_ptr == 256) {
		read_EEPROM = false;
		if (WKEY_DEBUG)
			LOG_WARN("\n%s", str2hex(eeprom_image, 256));
		eeprom_ptr = 0;
	}
}

void cbExit()
{
// shutdown threads
	run_serial_thread = false;
	run_flrig_thread = false;

	//
	// do NOT use pthread_join, we might hang
	// if the threads have lost connection etc.
	//
        usleep(100000);

// close host and close down the serial port
	if (host_is_up) {
		string cmd = " ";
		cmd += HOST_CLOSE;
		cmd[0] = ADMIN;
		sendString(cmd, true);
	}

	WKEY_serial.ClosePort();

	progStatus.saveLastState();

	close_logbook();

	exit(0);
}


void open_wkeyer()
{
	int cnt = 0;
	string cmd = NULL_CMD;
	sendCommand(cmd);

	cmd = ADMIN;
	cmd += ECHO_TEST;
	cmd += 'U';
	sendCommand(cmd, WAIT_ECHO);

	cnt = 200;
	while (test_echo == true && cnt) {
		MilliSleep(10);
		cnt--;
	}

	if (test_echo) {
		debug::show();
		LOG_ERROR("%s", "Winkeyer not responding");
		test_echo = false;
		pthread_mutex_lock(&mutex_serial);
			bypass_serial_thread_loop = true;
		pthread_mutex_unlock(&mutex_serial);
		WKEY_serial.ClosePort();
		progStatus.serial_port_name = "NONE";
		selectCommPort->value(progStatus.serial_port_name.c_str());
		return;
	}

/*
	cmd = ADMIN;
	cmd += DUMP_EEPROM;
	sendCommand(cmd);
	read_EEPROM = true;

	cnt = 4000;
	while (read_EEPROM == true && cnt) {
		MilliSleep(10);
		cnt--;
	}
	if (WKEY_DEBUG)
		LOG_WARN("EEprom read time %.2f sec", (4000 - cnt) * 0.01);
*/

	cmd = " ";
	cmd += HOST_OPEN;
	cmd[0] = ADMIN;
	sendCommand(cmd, WAIT_VERSION);

	cnt = 200;
	while (get_version == true && cnt) {
		MilliSleep(10);
		cnt--;
	}

	cntr_wpm->minimum(progStatus.min_wpm);
	cntr_wpm->maximum(progStatus.rng_wpm + progStatus.min_wpm);
	btn_use_pot->value(progStatus.use_pot);

	load_defaults();

	cmd = GET_SPEED_POT;
	sendCommand(cmd);

	cmd = SET_WPM;
	cmd += progStatus.speed_wpm;
	sendCommand(cmd);

	if (wk2_version) {
		cmd = ADMIN;
		cmd += WK2_MODE;
		sendCommand(cmd);
	}

	cmd = SET_SPEED_POT;
	cmd += progStatus.min_wpm;
	cmd += progStatus.rng_wpm;
	cmd += 0xFF;
	sendCommand(cmd);

	cmd = GET_SPEED_POT;
	sendCommand(cmd);

}

//
// read_parameters()
//
// Read WinKey EEPROM (all 256 bytes), set progStatus according
// to the first 16 bytes therein
//
void read_parameters()
{
  string cmd;

  if (host_is_up) {
    cmd = " ";
    cmd += HOST_CLOSE;
    cmd[0] = ADMIN;
    sendString(cmd, true);
    host_is_up=false;
  }

  cmd = " ";
  cmd += DUMP_EEPROM;
  cmd[0] = ADMIN;
  sendString(cmd, true);
  read_EEPROM=true;

  int cnt = 1000;
  while (read_EEPROM == true && cnt) {
    MilliSleep(10);
    cnt--;
  }

  cmd = " ";
  cmd += HOST_OPEN;
  cmd[0] = ADMIN;
  sendCommand(cmd, WAIT_VERSION);

  if (!read_EEPROM) {
    LOG_INFO("EEPROM read.");
    progStatus.mode_register     = eeprom_image[ 1];
    // ignore speed
    progStatus.sidetone          = eeprom_image[ 3];
    progStatus.weight            = eeprom_image[ 4];
    progStatus.lead_in_time      = eeprom_image[ 5]*10;
    progStatus.tail_time         = eeprom_image[ 6]*10;
    progStatus.min_wpm           = eeprom_image[ 7];
    progStatus.rng_wpm           = eeprom_image[ 8];
    progStatus.first_extension   = eeprom_image[ 9];
    progStatus.key_compensation  = eeprom_image[10];
    progStatus.farnsworth_wpm    = eeprom_image[11];
    progStatus.paddle_setpoint   = eeprom_image[12];
    progStatus.dit_dah_ratio     = eeprom_image[13];
    progStatus.pin_configuration = eeprom_image[14];
    progStatus.dont_care         = eeprom_image[15];
  }
  load_defaults();
}

//
// write_parameters()
//
// Load all 256 WinKey EEPROM bytes. The first 16 bytes are
// updated from progStatus, the others are either zeroed
// (if there was no read_parameters() before) or left
// unchanged
//
void write_parameters()
{
  string cmd;

  eeprom_image[ 0] = 0xA5;  // magic byte
  eeprom_image[ 1] = progStatus.mode_register;
  eeprom_image[ 2] = progStatus.speed_wpm;
  eeprom_image[ 3] = progStatus.sidetone;
  eeprom_image[ 4] = progStatus.weight;
  eeprom_image[ 5] = (progStatus.lead_in_time+5)/10;
  eeprom_image[ 6] = (progStatus.tail_time+5)/10;
  eeprom_image[ 7] = progStatus.min_wpm;
  eeprom_image[ 8] = progStatus.rng_wpm;
  eeprom_image[ 9] = progStatus.first_extension;
  eeprom_image[10] = progStatus.key_compensation;
  eeprom_image[11] = progStatus.farnsworth_wpm;
  eeprom_image[12] = progStatus.paddle_setpoint;
  eeprom_image[13] = progStatus.dit_dah_ratio;
  eeprom_image[14] = progStatus.pin_configuration;
  eeprom_image[15] = progStatus.dont_care;

  //
  // If this is a "virgin" EEPROM image (without CW messages stored),
  // set the data at addresses 16 through 23 to a reasonable value
  // and clear message area
  //
  if ((int) (eeprom_image[17] & 0xFF) < 24) {
    eeprom_image[16] = 15;   // default CmdWPM
    eeprom_image[17] = 24;   // default FreePtr;
    eeprom_image[18] = 0;    // no messages stored
    eeprom_image[19] = 0;
    eeprom_image[20] = 0;
    eeprom_image[21] = 0;
    eeprom_image[22] = 0;
    eeprom_image[23] = 0;
    for (int i=24; i<256; i++) eeprom_image[i]=0;
  }

  if (host_is_up) {
    cmd = " ";
    cmd += HOST_CLOSE;
    cmd[0] = ADMIN;
    sendString(cmd, true);
    host_is_up=false;
  }
  cmd = " ";
  cmd += LOAD_EEPROM;
  cmd[0] = ADMIN;
  sendString(cmd, true);

  //
  // Sending the EEPROM image in one large string
  // overflows the serial buffer of the Winkey device.
  // At 1200 baud, each character takes up to 8.3 msec,
  // therfore we send 1 char every 12 msec.
  // So writing the EEPROM takes about 3 seconds (!).
  //
  for (int i=0; i<256; i++) {
    cmd = " ";
    cmd[0] = eeprom_image[i];
    MilliSleep(12);
    sendString(cmd, true);
  }

  cmd = " ";
  cmd += HOST_OPEN;
  cmd[0] = ADMIN;
  sendCommand(cmd, WAIT_VERSION);

  load_defaults();

  LOG_INFO("EEPROM written.");
}

void load_defaults()
{
	string cmd = LOAD_DEFAULTS;
	cmd += progStatus.mode_register;
	cmd += '\0';                                   //progStatus.speed_wpm;
	cmd += progStatus.sidetone;
	cmd += progStatus.weight;
	cmd += (progStatus.lead_in_time+5) / 10;
	cmd += (progStatus.tail_time+5) / 10;
	cmd += progStatus.min_wpm;
	cmd += progStatus.rng_wpm;
	cmd += progStatus.first_extension;
	cmd += progStatus.key_compensation;
	cmd += progStatus.farnsworth_wpm;
	cmd += progStatus.paddle_setpoint;
	cmd += progStatus.dit_dah_ratio;
	cmd += progStatus.pin_configuration;
	cmd += progStatus.dont_care;
	sendCommand(cmd);
}

void cb_send_button()
{
	Fl::focus(txt_to_send);
}

void cb_clear_text_to_send()
{
	txt_to_send->clear();
	Fl::focus(txt_to_send);
}

void cb_cancel_transmit()
{
	string cmd = CLEAR_BUFFER;
	sendCommand(cmd);
	cb_clear_text_to_send();
	Fl::focus(txt_to_send);
}

void cb_tune()
{
	string cmd = KEY_IMMEDIATE;
	if (btn_tune->value()) cmd += '\1';
	else cmd += '\0';
	sendCommand(cmd);
	Fl::focus(txt_to_send);
}

void expand_msg(string &msg)
{
	size_t ptr;
	upcase(msg);
	while ((ptr = msg.find("<STA>")) != string::npos)
		msg.replace(ptr, 5, txt_sta->value());
	while ((ptr = msg.find("<NAM>")) != string::npos)
		msg.replace(ptr, 5, txt_name->value());
	while ((ptr = msg.find("<CLL>")) != string::npos)
		msg.replace(ptr, 5, progStatus.tag_cll);
	while ((ptr = msg.find("<QTH>")) != string::npos)
		msg.replace(ptr, 5, progStatus.tag_qth);
	while ((ptr = msg.find("<LOC>")) != string::npos)
		msg.replace(ptr, 5, progStatus.tag_loc);
	while ((ptr = msg.find("<OPR>")) != string::npos)
		msg.replace(ptr, 5, progStatus.tag_opr);
	while ((ptr = msg.find("<X>")) != string::npos)
		msg.replace(ptr, 3, progStatus.xout);

	char snbr[8] = "";
	if (progStatus.zeros && progStatus.serial_nbr < 100)
		snprintf(snbr, sizeof(snbr), "0%d", progStatus.serial_nbr);
	else
		snprintf(snbr, sizeof(snbr), "%d", progStatus.serial_nbr);
	while ((ptr = msg.find("<#>")) != string::npos)
		msg.replace(ptr, 3, snbr);

	if ((ptr = msg.find("<LOG>")) != string::npos) {
		if (mnu_log_client->value())
			xml_add_record();
		else
			AddRecord();
		msg.replace(ptr, 5, "");
	}

	while ((ptr = msg.find("<+>")) != string::npos) {
		progStatus.serial_nbr++;
		msg.replace(ptr, 3, "");
	}
	while ((ptr = msg.find("<->")) != string::npos) {
		progStatus.serial_nbr--;
		msg.replace(ptr, 3, "");
	}
	if (progStatus.serial_nbr < 1) progStatus.serial_nbr = 1;

	snprintf(snbr, sizeof(snbr), "%d", progStatus.serial_nbr);
	txt_serial_nbr->value(snbr);
	txt_serial_nbr->redraw();

}

void serial_nbr()
{
	progStatus.serial_nbr = atoi(txt_serial_nbr->value());
}

void time_span()
{
	progStatus.time_span = atoi(txt_time_span->value());
}

void zeros()
{
	progStatus.zeros = btn_zeros->value();
	check_call();
}

void dups()
{
	progStatus.dups = btn_dups->value();
	check_call();
}

void ck_band()
{
	progStatus.band = btn_ck_band->value();
	check_call();
}

void do_config_messages(void *)
{
	config_messages();
}

void send_message(string msg)
{
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		Fl::awake(do_config_messages, 0);
		return;
	}
	if (msg.empty()) return;
	expand_msg(msg);
	txt_to_send->add(msg.c_str());
}

void exec_msg1()
{
	send_message(progStatus.edit_msg1);
	Fl::focus(txt_to_send);
}

void exec_msg2()
{
	send_message(progStatus.edit_msg2);
	Fl::focus(txt_to_send);
}

void exec_msg3()
{
	send_message(progStatus.edit_msg3);
	Fl::focus(txt_to_send);
}

void exec_msg4()
{
	send_message(progStatus.edit_msg4);
	Fl::focus(txt_to_send);
}

void exec_msg5()
{
	send_message(progStatus.edit_msg5);
	Fl::focus(txt_to_send);
}

void exec_msg6()
{
	send_message(progStatus.edit_msg6);
	Fl::focus(txt_to_send);
}

void exec_msg7()
{
	send_message(progStatus.edit_msg7);
	Fl::focus(txt_to_send);
}

void exec_msg8()
{
	send_message(progStatus.edit_msg8);
	Fl::focus(txt_to_send);
}

void exec_msg9()
{
	send_message(progStatus.edit_msg9);
	Fl::focus(txt_to_send);
}

void exec_msg10()
{
	send_message(progStatus.edit_msg10);
	Fl::focus(txt_to_send);
}

void exec_msg11()
{
	send_message(progStatus.edit_msg11);
	Fl::focus(txt_to_send);
}

void exec_msg12()
{
	send_message(progStatus.edit_msg12);
	Fl::focus(txt_to_send);
}

int main_handler(int event)
{
	if (event != FL_SHORTCUT)
		return 0;
	Fl_Widget* w = Fl::focus();

	if (w == mainwindow || w->window() == mainwindow) {
		int key = Fl::event_key();
		int state = Fl::event_state();
		if (key == FL_Escape) return 1;
		if ((key == 't') && ((state & FL_ALT) == FL_ALT)) {
			if (btn_tune->value() == 1) btn_tune->value(0);
			else btn_tune->value(1);
			cb_tune();
			return 1;
		}
		if ((key == 's') && ((state & FL_ALT) == FL_ALT)) {
			if (btn_send->value() == 1) btn_send->value(0);
			else btn_send->value(1);
			return 1;
		}
		if ((key == 'l') && ((state & FL_ALT) == FL_ALT)) {
			if (mnu_log_client->value())
				xml_add_record();
			else
				AddRecord();
			return 1;
		}
		if ((key > FL_F) && key <= (FL_F + 12)) {
			switch (key) {
				case (FL_F + 1):
					send_message(progStatus.edit_msg1);
					break;
				case (FL_F + 2):
					send_message(progStatus.edit_msg2);
					break;
				case (FL_F + 3):
					send_message(progStatus.edit_msg3);
					break;
				case (FL_F + 4):
					send_message(progStatus.edit_msg4);
					break;
				case (FL_F + 5):
					send_message(progStatus.edit_msg5);
					break;
				case (FL_F + 6):
					send_message(progStatus.edit_msg6);
					break;
				case (FL_F + 7):
					send_message(progStatus.edit_msg7);
					break;
				case (FL_F + 8): 
					send_message(progStatus.edit_msg8);
					break;
				case (FL_F + 9): 
					send_message(progStatus.edit_msg9);
					break;
				case (FL_F + 10):
					send_message(progStatus.edit_msg10);
					break;
				case (FL_F + 11):
					send_message(progStatus.edit_msg11);
					break;
				case (FL_F + 12):
					send_message(progStatus.edit_msg12);
					break;
				default: break;
			}
			Fl::focus(txt_to_send);
			return 1;
		}
	}
	return 0;
}

void check_call()
{
	string chkcall = txt_sta->value();
	txt_sta->color(FL_BACKGROUND2_COLOR);
	txt_sta->redraw();

	if (chkcall.empty()) {
		txt_name->value("");
		return;
	}
	upcase(chkcall);
	size_t pos = txt_sta->position();

	txt_sta->value(chkcall.c_str());
	txt_sta->position(pos);

	if (strlen(txt_sta->value()) < 3) return;

	if (btn_dups->value())
		if (mnu_log_client->value())
			xml_check_dup();
		else
			DupCheck();
	else
		if (mnu_log_client->value())
			xml_get_record(txt_sta->value());
		else
			SearchLastQSO(txt_sta->value());
}

void set_time_on() {
	txt_time_on->value(txt_time_off->value());
}
