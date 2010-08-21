/* -----------------------------------------------------------------------------
 * status structure / methods
 *
 * A part of "rig", a rig control program compatible with fldigi / xmlrpc i/o
 *
 * copyright Dave Freese 2009, w1hkj@w1hkj.com
 *
*/

#include <iostream>
#include <fstream>
#include <string>

#include <FL/Fl_Preferences.H>
#include <FL/Fl_Progress.H>

#include "status.h"
#include "util.h"
#include "flwkey.h"
#include "support.h"
#include "config.h"
#include "wkey_dialogs.h"
#include "debug.h"

#include "logsupport.h"

string last_xcvr_used = "none";

status progStatus = {
	50,			// int mainX;
	50,			// int mainY;
	"NONE",		// string serial_port_name;
	1200,		// int comm_baudrate;
	2,			// int stopbits;
	2,			// int comm_retries;
	5,			// int comm_wait;
	50,			// int comm_timeout;
	false,		// bool comm_echo;
	5,			// int  serloop_timing;
// wkeyer defaults
	0xC4,		// unsigned char mode_register;
	18,			// unsigned char speed_wpm;
	6,			// unsigned char sidetone;
	50,			// unsigned char weight;
	0,			// unsigned char lead_in_time;
	0,			// unsigned char tail_time;
	10,			// unsigned char min_wpm;
	25,			// unsigned char max_wpm;
	0,			// unsigned char first_extension;
	0,			// unsigned char key_compensation;
	0,			// unsigned char farnsworth_wpm;
	50,			// unsigned char paddle_setpoint;
	50,			// unsigned char dit_dah_ratio;
	7,			// unsigned char pin_configuration;
	255,		// unsigned char dont_care;

	false,		// bool cut_zeronine;
	18,			// unsigned char cmd_wpm;
	true,		// bool use_pot
	
// message store
	"CQ",	// string	label_1;
	"CQ CQ CQ DE <CLL> <CLL> K ",	// string	edit_msg1;
	"call",	// string	label_2;
	"<STA> DE <CLL> <CLL> K",			// string	edit_msg2;
	"m 3",	// string	label_3;
	"",			// string	edit_msg3;
	"m 4",	// string	label_4;
	"",			// string	edit_msg4;
	"Xout",	// string	label_5;
	"R <#> AL",			// string	edit_msg5;
	"Xlog",	// string	label_6;
	"<LOG><+>",			// string	edit_msg6;
	"X--",	// string	label_7;
	"<->",			// string	edit_msg7;
	"X++",	// string	label_8;
	"<+>",			// string	edit_msg8;
	"LOG",	// string	label_9;
	"<LOG>",			// string	edit_msg9;
	"m 10",	// string	label_10;
	"",			// string	edit_msg10;
	"m 11",	// string	label_11;
	"",			// string	edit_msg11;
	"m 12",	// string	label_12;
	"",			// string	edit_msg12;

	"",			//string	tag_call;
	"",			//string	tag_qth;
	"",			//string	tag_loc;
	"",			//string	tag_op;

	"",			// string	logbookfilename

	1,			// int	serial_nbr;
	0,			// int	time_span;
	true,		// bool	zeros;
	false		// bool	dups;

};

void status::saveLastState()
{
	Fl_Preferences spref(WKeyHomeDir.c_str(), "w1hkj.com", PACKAGE_TARNAME);

	int mX = mainwindow->x();
	int mY = mainwindow->y();
	if (mX >= 0 && mX >= 0) {
		mainX = mX;
		mainY = mY;
	}

	spref.set("version", PACKAGE_VERSION);
	spref.set("mainx", mX);
	spref.set("mainy", mY);

	spref.set("serial_port_name", serial_port_name.c_str());

	spref.set("label1", label_1.c_str()); 
	spref.set("msg1", edit_msg1.c_str());
	spref.set("label2", label_2.c_str()); 
	spref.set("msg2", edit_msg2.c_str());
	spref.set("label3", label_3.c_str()); 
	spref.set("msg3", edit_msg3.c_str());
	spref.set("label4", label_4.c_str()); 
	spref.set("msg4", edit_msg4.c_str());
	spref.set("label5", label_5.c_str()); 
	spref.set("msg5", edit_msg5.c_str());
	spref.set("label6", label_6.c_str()); 
	spref.set("msg6", edit_msg6.c_str());
	spref.set("label7", label_7.c_str()); 
	spref.set("msg7", edit_msg7.c_str());
	spref.set("label8", label_8.c_str()); 
	spref.set("msg8", edit_msg8.c_str());
	spref.set("label9", label_9.c_str()); 
	spref.set("msg9", edit_msg9.c_str());
	spref.set("label10", label_10.c_str()); 
	spref.set("msg10", edit_msg10.c_str());
	spref.set("label11", label_11.c_str()); 
	spref.set("msg11", edit_msg11.c_str());
	spref.set("label12", label_12.c_str()); 
	spref.set("msg12", edit_msg12.c_str());

	spref.set("mode_register", mode_register);
	spref.set("speed_wpm", speed_wpm);
	spref.set("cmd_wpm", cmd_wpm);
	spref.set("sidetone", sidetone);
	spref.set("weight", weight);
	spref.set("lead_in_time", lead_in_time);
	spref.set("tail_time", tail_time);
	spref.set("min_wpm", min_wpm);
	spref.set("rng_wpm", rng_wpm);
	spref.set("1st_ext", first_extension);
	spref.set("key_comp", key_compensation);
	spref.set("farnsworth", farnsworth_wpm);
	spref.set("paddle_set", paddle_setpoint);
	spref.set("dit_dah_ratio", dit_dah_ratio);
	spref.set("pin_config", pin_configuration);

	spref.set("tag_cll", tag_cll.c_str());
	spref.set("tag_qth", tag_qth.c_str());
	spref.set("tag_loc", tag_loc.c_str());
	spref.set("tag_opr", tag_opr.c_str());

	spref.set("logbook_filename", logbookfilename.c_str());
	spref.set("logbook_ser_nbr", serial_nbr);
	spref.set("logbook_time_span", time_span);
	spref.set("logbook_zeros", zeros);
	spref.set("logbook_dups", dups);

}

void status::loadLastState()
{
	Fl_Preferences spref(WKeyHomeDir.c_str(), "w1hkj.com", PACKAGE_TARNAME);

	if (spref.entryExists("version")) {
		char defbuffer[200];

		spref.get("mainx", mainX, mainX);
		spref.get("mainy", mainY, mainY);

		spref.get("serial_port_name", defbuffer, "NONE", 199);
		serial_port_name = defbuffer;
		if (serial_port_name.find("tty") == 0) 
			serial_port_name.insert(0, "/dev/");

		spref.get("label1", defbuffer, "msg1", 199); label_1 = defbuffer;
		spref.get("msg1", defbuffer, "", 199); edit_msg1 = defbuffer;
		spref.get("label2", defbuffer, "msg2", 199); label_2 = defbuffer;
		spref.get("msg2", defbuffer, "", 199); edit_msg2 = defbuffer;
		spref.get("label3", defbuffer, "msg3", 199); label_3 = defbuffer;
		spref.get("msg3", defbuffer, "", 199); edit_msg3 = defbuffer;
		spref.get("label4", defbuffer, "msg4", 199); label_4 = defbuffer;
		spref.get("msg4", defbuffer, "", 199); edit_msg4 = defbuffer;
		spref.get("label5", defbuffer, "msg5", 199); label_5 = defbuffer;
		spref.get("msg5", defbuffer, "", 199); edit_msg5 = defbuffer;
		spref.get("label6", defbuffer, "msg6", 199); label_6 = defbuffer;
		spref.get("msg6", defbuffer, "", 199); edit_msg6 = defbuffer;
		spref.get("label7", defbuffer, "msg7", 199); label_7 = defbuffer;
		spref.get("msg7", defbuffer, "", 199); edit_msg7 = defbuffer;
		spref.get("label8", defbuffer, "msg8", 199); label_8 = defbuffer;
		spref.get("msg8", defbuffer, "", 199); edit_msg8 = defbuffer;
		spref.get("label9", defbuffer, "msg9", 199); label_9 = defbuffer;
		spref.get("msg9", defbuffer, "", 199); edit_msg9 = defbuffer;
		spref.get("label10", defbuffer, "msg10", 199); label_10 = defbuffer;
		spref.get("msg10", defbuffer, "", 199); edit_msg10 = defbuffer;
		spref.get("label11", defbuffer, "msg11", 199); label_11 = defbuffer;
		spref.get("msg11", defbuffer, "", 199); edit_msg11 = defbuffer;
		spref.get("label12", defbuffer, "msg11", 199); label_12 = defbuffer;
		spref.get("msg12", defbuffer, "", 199); edit_msg12 = defbuffer;

		int ichar;
		spref.get("mode_register", ichar, mode_register); mode_register = ichar & 0xFF;
		spref.get("speed_wpm", ichar, speed_wpm); speed_wpm = ichar & 0xFF;
		spref.get("cmd_wpm", ichar, cmd_wpm); cmd_wpm = ichar & 0xFF;
		spref.get("sidetone", ichar, sidetone); sidetone = ichar & 0xFF;
		spref.get("weight", ichar, weight); weight = ichar & 0xFF;
		spref.get("lead_in_time", ichar, lead_in_time); lead_in_time = ichar & 0xFF;
		spref.get("tail_time", ichar, tail_time); tail_time = ichar & 0xFF;
		spref.get("min_wpm", ichar, min_wpm); min_wpm = ichar & 0xFF;
		spref.get("rng_wpm", ichar, rng_wpm); rng_wpm = ichar & 0xFF;
		spref.get("1st_ext", ichar, first_extension); first_extension = ichar & 0xFF;
		spref.get("key_comp", ichar, key_compensation); key_compensation = ichar & 0xFF;
		spref.get("farnsworth", ichar, farnsworth_wpm); farnsworth_wpm = ichar & 0xFF;
		spref.get("paddle_set", ichar, paddle_setpoint); paddle_setpoint = ichar & 0xFF;
		spref.get("dit_dah_ratio", ichar, dit_dah_ratio); dit_dah_ratio = ichar & 0xFF;
		spref.get("pin_config", ichar, pin_configuration); pin_configuration = ichar & 0xFF;

		spref.get("tag_cll", defbuffer, "", 199); tag_cll = defbuffer;
		spref.get("tag_qth", defbuffer, "", 199); tag_qth = defbuffer;
		spref.get("tag_loc", defbuffer, "", 199); tag_loc = defbuffer;
		spref.get("tag_opr", defbuffer, "", 199); tag_opr = defbuffer;

		spref.get("logbook_filename", defbuffer, "", 199); logbookfilename = defbuffer;
		spref.get("logbook_ser_nbr", serial_nbr, serial_nbr);
		spref.get("logbook_time_span", time_span, time_span);
		spref.get("logbook_zeros", ichar, ichar); zeros = (ichar == 1);
		spref.get("logbook_dups", ichar, ichar); dups = (ichar == 1);

		update_msg_labels();
	}
}

