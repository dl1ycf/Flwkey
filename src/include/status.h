#ifndef _status_H
#define _status_H

#include <string>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

#include "flwkey.h"

using namespace std;

struct status {
	int		mainX;
	int		mainY;
	string	serial_port_name;
	int		comm_baudrate;
	int		stopbits;
	int		comm_retries;
	int		comm_wait;
	int		comm_timeout;
	bool	comm_echo;
//	bool	comm.rtscts;
//	bool	comm.rts;
//	bool	comm.dtr;

	int		serloop_timing;

// wkeyer values
	unsigned char mode_register;
	unsigned char speed_wpm;
	unsigned char sidetone;
	unsigned char weight;
	unsigned char lead_in_time;
	unsigned char tail_time;
	unsigned char min_wpm;
	unsigned char rng_wpm;
	unsigned char first_extension;
	unsigned char key_compensation;
	unsigned char farnsworth_wpm;
	unsigned char paddle_setpoint;
	unsigned char dit_dah_ratio;
	unsigned char pin_configuration;
	unsigned char dont_care;

	bool	cut_zeronine;
	unsigned char cmd_wpm;
	bool	use_pot;

// message store
	string	label_1;
	string	edit_msg1;
	string	label_2;
	string	edit_msg2;
	string	label_3;
	string	edit_msg3;
	string	label_4;
	string	edit_msg4;
	string	label_5;
	string	edit_msg5;
	string	label_6;
	string	edit_msg6;
	string	label_7;
	string	edit_msg7;
	string	label_8;
	string	edit_msg8;
	string	label_9;
	string	edit_msg9;
	string	label_10;
	string	edit_msg10;
	string	label_11;
	string	edit_msg11;
	string	label_12;
	string	edit_msg12;

// message tags
	string	tag_cll;
	string	tag_qth;
	string	tag_loc;
	string	tag_opr;

// logbook entries
	string logbookfilename;
	bool	xml_logbook;

// contest data
	int		serial_nbr;
	int		time_span;
	bool	band;
	bool	zeros;
	bool	dups;
	string	xout;

	void saveLastState();
	void loadLastState();
};

extern status progStatus;

#endif
