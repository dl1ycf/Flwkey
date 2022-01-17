// =====================================================================
//
// xmlrpc_log.cxx
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

XmlRpcClient *log_client = (XmlRpcClient *)0;

bool test_connection()
{
#ifndef NO_XML
	XmlRpcValue result;
	if (log_client->execute("system.listMethods", XmlRpcValue(), result))
		return true;
	LOG_ERROR("Cannot connect to %s, %s", 
			progStatus.log_address.c_str(),
			progStatus.log_port.c_str());

#endif
	return false;
}

void xml_get_record(const char *callsign)
{
#ifndef NO_XML
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
	if (log_client->execute("log.get_record", oneArg, result)) {
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
#endif
}

void xml_add_record()
{
#ifndef NO_XML
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

	char *szt = szTime(0);
	char *szdt = szDate(0x86);
	char sznbr[6];
	string call = txt_sta->value();
	string freq = xcvr_freq->strval();
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
	log_client->execute("log.add_record", oneArg, result);
//	std::cout << "log.add_record result " << result << "\n\n";
#endif
}

void xml_check_dup()
{
#ifndef NO_XML
	Fl_Color call_clr = FL_BACKGROUND2_COLOR;
	XmlRpcValue sixargs, result;
	sixargs[0] = txt_sta->value();
	sixargs[1] = "CW";
	sixargs[2] = "0";
	sixargs[3] = "0";
	sixargs[4] = "0";
	sixargs[5] = "0";
	if (log_client->execute("log.check_dup", sixargs, result)) {
		string res = std::string(result);
		if (res == "true")
			call_clr = fl_rgb_color( 255, 110, 180);
	}
	txt_sta->color(call_clr);
	txt_sta->redraw();
#endif
}

void connect_to_server()
{
#ifndef NO_XML
	if (log_client) delete log_client;
	if (mnu_log_client->value()) {
		try {
			log_client = new XmlRpcClient(
					progStatus.log_address.c_str(), 
					atol(progStatus.log_port.c_str()));
			mnu_log_client->set();
		} catch (...) {
			LOG_ERROR("Cannot create %s, %s", 
						progStatus.log_address.c_str(),
						progStatus.log_port.c_str());
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
			return;
		}
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
			fl_alert2(_("Cannot connect to logbook server!\nUsing internal logbook"));
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
		}
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
#endif
}
