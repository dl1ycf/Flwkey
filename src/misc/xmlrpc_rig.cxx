// =====================================================================
//
// xmlrpc_rig.cxx
//
// connect to flrig xmlrpc server
//
// Copyright (C) 2007-2009
//		Dave Freese, W1HKJ
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// =====================================================================

#include <iostream>
#include <cmath>
#include <cstring>
#include <stdlib.h>

#include <FL/Fl.H>
#include <FL/filename.H>
#include <FL/fl_ask.H>

#include "XmlRpc.h"

#include "support.h"
#include "wkey_dialogs.h"
#include "config.h"
#include "icons.h"
#include "gettext.h"
#include "debug.h"
#include "util.h"
#include "date.h"
#include "threads.h"

#include "xmlrpc_rig.h"

using namespace XmlRpc;
using namespace std;

int xmlrpc_verbosity = 0;

//======================================================================
// flrig xmlrpc client
//======================================================================

void movFreq(Fl_Widget *w, void *d);
void show_freq(void *);
void flrig_get_vfo();
void flrig_get_frequency();

void post_mode(void *);
void flrig_get_mode();
void post_modes(void *);
void flrig_get_modes();

void post_bw(void *);
void flrig_get_bw();
void post_bws(void *);
void flrig_get_bws();

void flrig_get_notch();
void flrig_set_notch(int);

void set_flrig_ab(int n);

bool flrig_get_xcvr();
void flrig_connection();

void set_flrig_ptt(int on);
void flrig_set_freq(long int fr);
void set_flrig_mode(const char *md);
void set_flrig_bw(int bw1, int bw2);

void connect_to_flrig();

XmlRpcClient *flrig_client = (XmlRpcClient *)0;

bool bws_posted = false;
bool bw_posted = false;
bool mode_posted = false;
bool modes_posted = false;
bool freq_posted = true;

string xcvr_name;
string str_freq;
string mode_result;
#ifndef NO_XML
XmlRpcValue modes_result;
XmlRpcValue bws_result;
XmlRpcValue bw_result;
XmlRpcValue notch_result;
#endif

bool connected_to_flrig = false;

//==============================================================================
// UI controls changed
//==============================================================================
void cb_vfoAB()
{
#ifndef NO_XML
	set_flrig_ab(btn_vfoA->value());
#endif
}

void cb_frequency()
{
#ifndef NO_XML
	flrig_set_freq(xcvr_freq->value());
#endif
}

void cb_mode()
{
#ifndef NO_XML
	set_flrig_mode(opMODE->value());
#endif
}

void cb_bw()
{
#ifndef NO_XML
	set_flrig_bw(opBW1->index(), opBW2->index());
#endif
}

//======================================================================
// set / get pairs
//======================================================================

//----------------------------------------------------------------------
// push to talk
//----------------------------------------------------------------------
bool wait_ptt = false; // wait for transceiver to respond
int  wait_ptt_timeout = 5; // 5 polls and then disable wait
int  ptt_state = 0;

void set_flrig_ptt(int on) {
#ifndef NO_XML
	if (!connected_to_flrig) return;

	XmlRpcValue val, result;
	val = int(on);

	guard_lock flrig_lock(&mutex_flrig);
	if (flrig_client->execute("rig.set_ptt", val, result)) {
		wait_ptt = true;
		wait_ptt_timeout = 10;
		ptt_state = on;
	} else {
		wait_ptt = false;
		wait_ptt_timeout = 0;
		LOG_ERROR("%s", "rig.set_vfo failed");
	}
	return;
#endif
}

pthread_mutex_t mutex_flrig_ptt = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_show_ptt(void *data)
{
//	guard_lock flrig_lock(&mutex_flrig_ptt);
//	int on = reinterpret_cast<long>(data);
//	if (wf) wf->xmtrcv->value(on);
}

void flrig_get_ptt()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	XmlRpcValue result;
	if (flrig_client->execute("rig.get_ptt", XmlRpcValue(), result) ) {
		int val = int(result);
		if (!wait_ptt && (val != ptt_state)) {
			ptt_state = val;
			guard_lock flrig_lock(&mutex_flrig_ptt);
			Fl::awake(xmlrpc_rig_show_ptt, reinterpret_cast<void*>(val) );
		} else if (wait_ptt && (val == ptt_state)) {
			wait_ptt = false;
			wait_ptt_timeout = 0;
		} else if (wait_ptt_timeout == 0) {
			wait_ptt = false;
		} else if (wait_ptt_timeout) {
			--wait_ptt_timeout;
		}
	} else {
		connected_to_flrig = false;
		wait_ptt = false;
		wait_ptt_timeout = 5;
	}
#endif
}

//----------------------------------------------------------------------
// transceiver radio frequency
//----------------------------------------------------------------------

bool wait_freq = false; // wait for transceiver to respond
int  wait_freq_timeout = 5; // 5 polls and then disable wait
long int  x_freq = 0;

void flrig_set_freq(long int fr)
{
#ifndef NO_XML
	if (!connected_to_flrig) return;

	guard_lock flrig_lock(&mutex_flrig);

	XmlRpcValue val, result;
	val = double(fr);
	if (!flrig_client->execute("rig.set_vfo", val, result)) {
		LOG_ERROR("%s", "rig.set_vfo failed");
		wait_freq = false;
		wait_freq_timeout = 0;
	} else {
		wait_freq = true;
		wait_freq_timeout = 5;
	}
#endif
}

pthread_mutex_t mutex_flrig_freq = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_show_freq(void * fr)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_freq);
	long freq = reinterpret_cast<long>(fr);
	xcvr_freq->value(freq);
	x_freq = freq;
#endif
}

void flrig_get_frequency()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	XmlRpcValue result;
	if (!freq_posted) return;
	if (flrig_client->execute("rig.get_vfo", XmlRpcValue(), result) ) {
		str_freq = string(result);
		int fr = atoi(str_freq.c_str());

		if (!wait_freq && (fr != x_freq)) {
			guard_lock flrig_lock(&mutex_flrig_freq);
			Fl::awake(xmlrpc_rig_show_freq, reinterpret_cast<void*>(fr));
		} else if (wait_freq && (fr == x_freq)) {
			wait_freq = false;
			wait_freq_timeout = 0;
		} else if (wait_freq_timeout == 0) {
			wait_freq = false;
		} else if (wait_freq_timeout)
			--wait_freq_timeout;
	} else {
		connected_to_flrig = false;
		wait_freq = false;
		wait_freq_timeout = 5;
	}
#endif
}

//----------------------------------------------------------------------
// transceiver set / get mode
// transceiver get modes (mode table)
//----------------------------------------------------------------------

bool wait_mode = false; // wait for transceiver to respond
int  wait_mode_timeout = 5; // 5 polls and then disable wait
string posted_mode = "";

void set_flrig_mode(const char *md)
{
#ifndef NO_XML
	if (!connected_to_flrig) return;

	XmlRpcValue val, result;
	val = string(md);

	guard_lock flrig_lock(&mutex_flrig);
	if (!flrig_client->execute("rig.set_mode", val, result)) {
		LOG_ERROR("%s", "rig.set_mode failed");
		wait_mode = false;
		wait_mode_timeout = 5;
	} else {
		posted_mode = md;
		wait_mode = true;
		wait_mode_timeout = 5;
	}
#endif
}

pthread_mutex_t mutex_flrig_mode = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_post_mode(void *data)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_mode);
	if (!opMODE) return;
	string *s = reinterpret_cast<string *>(data);
	opMODE->value(s->c_str());
	bws_posted = false;
#endif
}

void flrig_get_mode()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	XmlRpcValue res;
	if (flrig_client->execute("rig.get_mode", XmlRpcValue(), res) ) {
		static string md;
		md = string(res);
		bool posted = (md == posted_mode);
		if (!wait_mode && !posted) {
			posted_mode = md;
			guard_lock flrig_lock(&mutex_flrig_mode);
			Fl::awake(xmlrpc_rig_post_mode, reinterpret_cast<void*>(&md));
		} else if (wait_mode && posted) {
			wait_mode = false;
			wait_mode_timeout = 0;
		} else if (wait_mode_timeout == 0) {
			wait_mode = false;
		} else if (wait_mode_timeout)
			--wait_mode_timeout;
	} else {
		connected_to_flrig = false;
		wait_mode = false;
		wait_freq_timeout = 0;
	}
#endif
}

pthread_mutex_t mutex_flrig_modes = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_post_modes(void *)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_modes);
	if (!opMODE) return;

	int nargs = modes_result.size();

	opMODE->clear();

	if (nargs == 0) {
		opMODE->add("");
		opMODE->index(0);
		opMODE->deactivate();
		return;
	}

	for (int i = 0; i < nargs; i++)
		opMODE->add(string(modes_result[i]).c_str());

	opMODE->index(0);
	opMODE->activate();

	modes_posted = true;
#endif
}

void flrig_get_modes()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	if (flrig_client->execute("rig.get_modes", XmlRpcValue(), modes_result) ) {
		guard_lock flrig_lock(&mutex_flrig_modes);
		Fl::awake(xmlrpc_rig_post_modes);
	}
#endif
}

//----------------------------------------------------------------------
// transceiver get / set bandwidth
// transceiver get bandwidth table
//----------------------------------------------------------------------
bool wait_bw = false; // wait for transceiver to respond
int  wait_bw_timeout = 5; // 5 polls and then disable wait
string  posted_bw = "";

void set_flrig_bw(int bw1, int bw2)
{
#ifndef NO_XML
	if (!connected_to_flrig) return;

	XmlRpcValue val, result;
	val = 256*bw2 + bw1;

	guard_lock flrig_lock(&mutex_flrig);
	if (!flrig_client->execute("rig.set_bw", val, result)) {
		LOG_ERROR("%s", "rig.set_bw failed");
		wait_bw = false;
		wait_bw_timeout = 0;
	} else {
		wait_bw = true;
		wait_bw_timeout = 5;
	}
#endif
}

pthread_mutex_t mutex_flrig_bw = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_post_bw(void *data)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_bw);
	std::string *s = reinterpret_cast<string*>(data);
	size_t p = s->find("|");
	std::string s1 = s->substr(0, p);
	std::string s2 = s->substr(p+1);
	opBW1->value(s1.c_str());
	opBW2->value(s2.c_str());
	opBW1->redraw();
	opBW2->redraw();
#endif
}

void flrig_get_bw()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	XmlRpcValue res;
	if (flrig_client->execute("rig.get_bw", XmlRpcValue(), res) ) {
		static string s;

		s = string(res[0]);
		s.append("|").append(string(res[1]));

		bool posted = ((s == posted_bw));
		if (!wait_bw && !posted) {
			posted_bw = s;
			guard_lock flrig_lock(&mutex_flrig_bw);
			Fl::awake(xmlrpc_rig_post_bw, reinterpret_cast<void*>(&s));
		} else if (wait_bw && !posted) {
			wait_bw = false;
			wait_bw_timeout = 0;
		} else if (wait_bw_timeout == 0) {
			wait_bw = false;
		} else if (wait_bw_timeout)
			--wait_bw_timeout;
	} else {
		connected_to_flrig = false;
		wait_bw = false;
		wait_bw_timeout = 0;
	}
#endif
}

pthread_mutex_t mutex_flrig_bws = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_post_bws(void *)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_bws);
	int nargs;
	opBW1->clear();
	opBW1->value("");
	opBW1->label("                ");
	opBW2->clear();
	opBW2->value("");
	opBW2->label("                ");

	try {
		nargs = bws_result[0].size();
		if (nargs > 1) {
			opBW1->label(std::string(bws_result[0][0]).c_str());
			for (int i = 1; i < nargs; i++) {
				opBW1->add(std::string(bws_result[0][i]).c_str());
			}
		}
		opBW1->activate();
	} catch (XmlRpcException err) {
		opBW1->deactivate();
	}
	try {
		nargs = bws_result[1].size();
		if (nargs > 1) {
			opBW2->label(std::string(bws_result[1][0]).c_str());
			for (int i = 1; i < nargs; i++) {
				opBW2->add(std::string(bws_result[1][i]).c_str());
			}
			opBW2->activate();
		} else
			opBW2->add("");
	} catch (XmlRpcException err) {
		opBW2->deactivate();
	}
	opBW1->redraw_label();
	opBW1->redraw();
	opBW2->redraw_label();
	opBW2->redraw();
	bws_posted = true;
#endif
}

void flrig_get_bws()
{
#ifndef NO_XML
	if (bws_posted) return;
	XmlRpcValue result;
	if (flrig_client->execute("rig.get_bws", XmlRpcValue(), bws_result) ) {
		bws_posted = false;
		wait_bw = true;
		wait_bw_timeout = 5;
		posted_bw.clear();
		guard_lock flrig_lock(&mutex_flrig_bws);
		Fl::awake(xmlrpc_rig_post_bws);
	}
#endif
}

//----------------------------------------------------------------------
// transceiver get / set vfo A / B
//----------------------------------------------------------------------

void set_flrig_ab(int n)
{
#ifndef NO_XML
	if (!connected_to_flrig) return;

	XmlRpcValue val = string(n == 1 ? "A" : "B");
	XmlRpcValue result;

	guard_lock flrig_lock(&mutex_flrig);
	if (!flrig_client->execute("rig.set_AB", val, result))
		printf("rig.set_AB failed\n");
	return;
#endif
}

void show_A(void *)
{
#ifndef NO_XML
	btn_vfoA->set();
	btn_vfoB->clear();
#endif
}

void show_B(void *)
{
#ifndef NO_XML
	btn_vfoA->clear();
	btn_vfoB->set();
#endif
}

void flrig_get_vfo()
{
#ifndef NO_XML
	XmlRpcValue result;
	if (flrig_client->execute("rig.get_AB", XmlRpcValue(), result) ) {
		std::string str_vfo = string(result);
		if (str_vfo[0] == 'A' && !btn_vfoA->value()) Fl::awake(show_A);
		else if (str_vfo[0] == 'B' && !btn_vfoB->value()) Fl::awake(show_B);
	} else {
		connected_to_flrig = false;
	}
#endif
}

//==============================================================================
// transceiver set / get notch
//==============================================================================
bool wait_notch = false; // wait for transceiver to respond
int  wait_notch_timeout = 5; // 5 polls and then disable wait
int  xcvr_notch = 0;

void set_flrig_notch()
{
#ifndef NO_XML
/*
	if (!connected_to_flrig) return;

	guard_lock flrig_lock(&mutex_flrig);

	XmlRpcValue val, result;
	val = (double)(notch_frequency);
	if (flrig_client->execute("rig.set_notch", val, result)) {
		wait_notch = true;
		wait_notch_timeout = 5;
		xcvr_notch = notch_frequency;
	} else {
		LOG_ERROR("%s", "rig.set_notch failed");
		wait_notch = 0;
		wait_notch_timeout = 0;
		xcvr_notch = 0;
	}
*/
#endif
}

void flrig_get_notch()
{
#ifndef NO_XML
/*
	guard_lock flrig_lock(&mutex_flrig);
	if (flrig_client->execute("rig.get_notch", XmlRpcValue(), notch_result) ) {
		int nu_notch = (int)(notch_result);
		if (nu_notch != notch_frequency) {
			notch_frequency = nu_notch;
		}

		bool posted = (nu_notch == xcvr_notch);

		if (!wait_notch && !posted) {
			xcvr_notch = nu_notch;
			notch_frequency = nu_notch;
		} else if (wait_notch && posted) {
			wait_notch = false;
			wait_notch_timeout = 0;
		} else if (wait_notch_timeout == 0) {
			wait_notch = false;
		} else if (wait_notch_timeout)
			--wait_notch_timeout;
	} else {
		wait_notch = false;
		wait_notch_timeout = 0;
	}
*/
#endif
}

//==============================================================================
// transceiver get smeter
// transceiver get power meter
//==============================================================================
/*
pthread_mutex_t mutex_flrig_smeter = PTHREAD_MUTEX_INITIALIZER;
static void xmlrpc_rig_set_smeter(void *data)
{
	guard_lock flrig_lock(&mutex_flrig_smeter);
	if (!smeter && !pwrmeter) return;

	if (smeter && progStatus.meters) {
		if (!smeter->visible()) {
			pwrmeter->hide();
			smeter->show();
		}
		int val = reinterpret_cast<long>(data);
		smeter->value(val);
	}
}

void flrig_get_smeter()
{
#ifndef NO_XML
	XmlRpcValue val, result;
	if (flrig_client->execute("rig.get_smeter", val, result)) {
		int val = (int)(result);
		guard_lock flrig_lock(&mutex_flrig_smeter);
		Fl::awake(xmlrpc_rig_set_smeter, reinterpret_cast<void*>(val));
	}
#endif
}

pthread_mutex_t mutex_flrig_pwrmeter = PTHREAD_MUTEX_INITIALIZER;
static void xmlrpc_rig_set_pwrmeter(void *data)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_pwrmeter);
	if (!smeter && !pwrmeter) return;

	if (pwrmeter && progStatus.meters) {
		if (!pwrmeter->visible()) {
			smeter->hide();
			pwrmeter->show();
		}
		int val = reinterpret_cast<long>(data);
		pwrmeter->value(val);
	}
#endif
}

void flrig_get_pwrmeter()
{
#ifndef NO_XML
	XmlRpcValue val, result;
	if (flrig_client->execute("rig.get_pwrmeter", val, result)) {
		int val = (int)(result);
		guard_lock flrig_lock(&mutex_flrig_pwrmeter);
		Fl::awake(xmlrpc_rig_set_pwrmeter, reinterpret_cast<void*>(val));
	}
#endif
}
*/

//==============================================================================
// transceiver get name
//==============================================================================

pthread_mutex_t mutex_flrig_xcvr_name = PTHREAD_MUTEX_INITIALIZER;
void xmlrpc_rig_show_xcvr_name(void *)
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig_xcvr_name);
	xcvr_group->label(xcvr_name.c_str());
	xcvr_group->redraw_label();
	opMODE->clear();
	opBW1->clear();
	opBW2->clear();
	opBW1->deactivate();
	opBW2->deactivate();
#endif
}

bool flrig_get_xcvr()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	XmlRpcValue result;
	try {
		if (flrig_client->execute("rig.get_xcvr", XmlRpcValue(), result) ) {
			string nuxcvr = string(result);
			if (nuxcvr != xcvr_name) {
				xcvr_name = nuxcvr;
				guard_lock flrig_lock(&mutex_flrig_xcvr_name);
				Fl::awake(xmlrpc_rig_show_xcvr_name);
				modes_posted = false;
				bws_posted = false;
			}
			return true;
		} else {
			connected_to_flrig = false;
		}
	} catch (XmlRpcException *err) {
		connected_to_flrig = false;
	}
#endif
	return false;
}

//======================================================================
// xmlrpc read polling thread
//======================================================================
bool run_flrig_thread = true;
int poll_interval = 100; // milliseconds

//----------------------------------------------------------------------
// Set QSY to true if xmlrpc client connection is OK
//----------------------------------------------------------------------

void flrig_connection()
{
#ifndef NO_XML
	guard_lock flrig_lock(&mutex_flrig);
	XmlRpcValue noArgs, result;
	try {
		if (flrig_client->execute("system.listMethods", noArgs, result)) {
			int nargs = result.size();
			string method_str = "\nMethods:\n";
			for (int i = 0; i < nargs; i++)
				method_str.append("    ").append(result[i]).append("\n");
			LOG_INFO("%s", method_str.c_str());
			connected_to_flrig = true;
			poll_interval = 100;
		} else {
			connected_to_flrig = false;
			poll_interval = 500;
		}
	} catch (...) {}
#endif
}

void connect_to_flrig()
{
#ifndef NO_XML
	XmlRpc::setVerbosity(xmlrpc_verbosity);
	if (flrig_client) {
		delete flrig_client;
		flrig_client = (XmlRpcClient *)0;
	}
	try {
		flrig_client = new XmlRpcClient(
				"localhost",
				atol("12345"));
		flrig_connection();
	} catch (...) {
			LOG_ERROR("Cannot connect to %s, %s",
						"localhost",
						"12345");
	}
#endif
}

void * flrig_thread_loop(void *d)
{
	for(;;) {
		MilliSleep( poll_interval );

		if (!run_flrig_thread) break;

		if (!flrig_client)
			connect_to_flrig();
		if (!connected_to_flrig) flrig_connection();
		else if (flrig_get_xcvr()) {
			flrig_get_frequency();
			if (!modes_posted) flrig_get_modes();
			if (modes_posted)  flrig_get_mode();
			if (!bws_posted)   flrig_get_bws();
			if (bws_posted)    flrig_get_bw();
			flrig_get_vfo();
//			flrig_get_notch();
//			flrig_get_ptt();
//			if (trx_state == STATE_RX) flrig_get_smeter();
//			else  flrig_get_pwrmeter();
		}
	}
	return NULL;
}

void flrig_start_thread()
{
	flrig_thread = new pthread_t;
	poll_interval = 500;
	if (pthread_create(flrig_thread, NULL, flrig_thread_loop, NULL)) {
		LOG_ERROR("%s", "flrig_thread create");
		exit(EXIT_FAILURE);
	}
}

void flrig_stop_thread()
{
	pthread_mutex_lock(&mutex_flrig);
		run_flrig_thread = false;
	pthread_mutex_unlock(&mutex_flrig);
	pthread_join(*flrig_thread, NULL);
}
