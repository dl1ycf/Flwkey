// =====================================================================
//
// xmlrpc_io.cxx
//
// connect to logbook xmlrpc server
//
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
#include "lgbook.h"
#include "icons.h"
#include "gettext.h"
#include "debug.h"
#include "util.h"
#include "date.h"
#include "logbook.h"

using namespace XmlRpc;

XmlRpcClient log_client("localhost", 8421);

bool test_connection()
{
	XmlRpcValue result;
	if (log_client.execute("system.listMethods", XmlRpcValue(), result))
		return true;
	return false;
}

void xml_get_record(const char *callsign)
{
	XmlRpcValue oneArg, result;
	if (!test_connection()) {
		fl_alert2(_("Logbook server down!\nUsing internal logbook"));
		txt_name->value("");
		mnu_log_client->clear();
		mnu_display_log->activate();
		mnu_new_log->activate();
		mnu_open_logbook->activate();
		mnu_save_logbook->activate();
		mnu_merge_logbook->activate();
		mnu_export_adif->activate();
		mnu_export_logbook_text->activate();
		mnu_export_logbook_csv->activate();
		mnu_export_cabrillo->activate();
		start_logbook();
	}
	oneArg[0] = callsign;
	if (log_client.execute("log.get_record", oneArg, result)) {
		string adifline = std::string(result);
		size_t pos1 = adifline.find("<NAME:");
		if (pos1 == std::string::npos) {
			txt_name->value("");
			return;
		}
		pos1 = adifline.find(">", pos1) + 1;
		size_t pos2 = adifline.find("<", pos1);
		string name = adifline.substr(pos1, pos2 - pos1);
		txt_name->value(name.c_str());
	} else {
		txt_name->value("");
	}
//  else
//    std::cout << "Error calling 'log.get_record'\n\n";
}

void xml_add_record()
{
	if (txt_sta->value()[0] == 0) return;

	if (!test_connection()) {
		fl_alert2(_("Logbook server down!\nUsing internal logbook"));
		mnu_log_client->clear();
		mnu_display_log->activate();
		mnu_new_log->activate();
		mnu_open_logbook->activate();
		mnu_save_logbook->activate();
		mnu_merge_logbook->activate();
		mnu_export_adif->activate();
		mnu_export_logbook_text->activate();
		mnu_export_logbook_csv->activate();
		mnu_export_cabrillo->activate();
		start_logbook();
	}

	char *szt = szTime(2);
	char *szdt = szDate(0x86);
	char sznbr[6];
	string call = txt_sta->value();
	string freq = txt_freq->value();
	string name = txt_name->value();
	string xin = txt_xchg->value();
	snprintf(sznbr, sizeof(sznbr), "%d", progStatus.serial_nbr);

	XmlRpcValue oneArg, result;
	char adifrec[200];
	snprintf(adifrec, sizeof(adifrec), "\
<FREQ:%d>%s\
<CALL:%d>%s\
<NAME:%d>%s\
<MODE:2>CW\
<QSO_DATE:8>%s<TIME_ON:4>%s<TIME_OFF:4>%s\
<STX:%d>%s\
<STX_STRING:%d>%s\
<SRX_STRING:%d>%s\
<RST_RCVD:3>599<RST_SENT:3>599<EOR>",
		freq.length(), freq.c_str(),
		call.length(), call.c_str(),
		name.length(), name.c_str(),
		szdt, szt, szt,
		strlen(sznbr), sznbr,
		progStatus.xout.length(), progStatus.xout.c_str(),
		xin.length(), xin.c_str());
	oneArg[0] = adifrec;
	log_client.execute("log.add_record", oneArg, result);
//	std::cout << "log.add_record result " << result << "\n\n";
}

void xml_check_dup()
{
	Fl_Color call_clr = FL_BACKGROUND2_COLOR;
	XmlRpcValue fourargs, result;
	fourargs[0] = txt_sta->value();
	fourargs[1] = "CW";
	fourargs[2] = "0";
	fourargs[3] = "0";
	if (log_client.execute("log.check_dup", fourargs, result)) {
		string res = std::string(result);
		if (res == "true")
			call_clr = fl_rgb_color( 255, 110, 180);
	}
//	else
//		std::cout << "Error calling 'log.check_dup'\n";
	txt_sta->color(call_clr);
	txt_sta->redraw();
}

void connect_to_server()
{
	if (mnu_log_client->value())
		if (test_connection()) {
			close_logbook();
			if (dlgLogbook) dlgLogbook->hide();
			mnu_display_log->deactivate();
			mnu_new_log->deactivate();
			mnu_open_logbook->deactivate();
			mnu_save_logbook->deactivate();
			mnu_merge_logbook->deactivate();
			mnu_export_adif->deactivate();
			mnu_export_logbook_text->deactivate();
			mnu_export_logbook_csv->deactivate();
			mnu_export_cabrillo->deactivate();
		} else {
			progStatus.xml_logbook = false;
			mnu_log_client->clear();
			mnu_display_log->activate();
			mnu_new_log->activate();
			mnu_open_logbook->activate();
			mnu_save_logbook->activate();
			mnu_merge_logbook->activate();
			mnu_export_adif->activate();
			mnu_export_logbook_text->activate();
			mnu_export_logbook_csv->activate();
			mnu_export_cabrillo->activate();
			start_logbook();
	} else {
		mnu_display_log->activate();
		mnu_new_log->activate();
		mnu_open_logbook->activate();
		mnu_save_logbook->activate();
		mnu_merge_logbook->activate();
		mnu_export_adif->activate();
		mnu_export_logbook_text->activate();
		mnu_export_logbook_csv->activate();
		mnu_export_cabrillo->activate();
		start_logbook();
	}
}
