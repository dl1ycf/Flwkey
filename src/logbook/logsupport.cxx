// ----------------------------------------------------------------------------
// logsupport.cxx
//
// Copyright (C) 2006-2010
//		Dave Freese, W1HKJ
// Copyright (C) 2008-2009
//		Stelios Bounanos, M0GLD
//
// This file is part of flflrig.
//
// Flflrig is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Flflrig is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with flflrig.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>

#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include "debug.h"
#include "status.h"
#include "date.h"

#include "adif_io.h"
#include "logbook.h"
#include "textio.h"

#include "logger.h"
#include "fileselect.h"
#include "icons.h"
#include "gettext.h"

#include "timeops.h"

#include "wkey_dialogs.h"


#include <FL/filename.H>
#include <FL/fl_ask.H>

using namespace std;

char *szTime(int typ)
{
	static char szDt[80];
	time_t tmptr;
	tm sTime;
	time (&tmptr);
	switch (typ) {
		case 0:
			gmtime_r(&tmptr, &sTime);
			strftime(szDt, 79, "%H%M", &sTime);
			break;
		case 1:
			localtime_r(&tmptr, &sTime);
			strftime(szDt, 79, "%H%M", &sTime);
			break;
		case 2:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H%MZ", &sTime);
			break;
		case 3:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H:%MZ", &sTime);
			break;
		case 4:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H%M UTC", &sTime);
			break;
		case 5:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H:%M UTC", &sTime);
			break;
		case 6:
			gmtime_r (&tmptr, &sTime);
			strftime(szDt, 79, "%H%M%S", &sTime);
			break;
		default:
			localtime_r(&tmptr, &sTime);
			strftime(szDt, 79, "%H%ML", &sTime);
	}
	return szDt;
}

static const char *month_name[] =
{
  "January",
  "Febuary",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};

char *szDate(int fmt)
{
	static char szDt[20];
	static char szMonth[10];

	time_t tmptr;
	tm sTime;
	time (&tmptr);
	if ((fmt & 0x80) == 0x80) {
		gmtime_r (&tmptr, &sTime);
	} else {
		localtime_r(&tmptr, &sTime);
	}
	switch (fmt & 0x7F) {
		case 1 :
			snprintf (szDt, sizeof(szDt), "%02d/%02d/%02d",
				sTime.tm_mon + 1, 
				sTime.tm_mday, 
				sTime.tm_year > 100 ? sTime.tm_year - 100 : sTime.tm_year);
			break;
		case 2 :
			snprintf (szDt, sizeof(szDt), "%4d-%02d-%02d", 
				sTime.tm_year + 1900,
				sTime.tm_mon + 1, 
				sTime.tm_mday);
			break;
		case 3 :  
			snprintf (szDt, sizeof(szDt), "%s %2d, %4d",
				month_name[sTime.tm_mon], 
				sTime.tm_mday, 
				sTime.tm_year + 1900);
			break;
		case 4 :
			strcpy (szMonth, month_name [sTime.tm_mon]);
			szMonth[3] = 0; 
			snprintf (szDt, sizeof(szDt), "%s %2d, %4d", 
				szMonth,
				sTime.tm_mday, 
				sTime.tm_year + 1900);
			break;
		case 5 :
			strcpy (szMonth, month_name [sTime.tm_mon]);
			szMonth[3] = 0;
			for (int i = 0; i < 3; i++) szMonth[i] = toupper(szMonth[i]);
			snprintf (szDt, sizeof(szDt), "%s %d", 
				szMonth, 
				sTime.tm_mday);
			break;
		case 6 :
			snprintf (szDt, sizeof(szDt), "%4d%02d%02d", 
				sTime.tm_year + 1900,
				sTime.tm_mon + 1, 
				sTime.tm_mday);
			break;
		case 0 :
		default :
			snprintf (szDt, sizeof(szDt), "%02d/%02d/%04d",
				sTime.tm_mon + 1, 
				sTime.tm_mday,
				sTime.tm_year + 1900); 
		break;
	}
	return szDt;
}

cQsoDb		qsodb;
cAdifIO		adifFile;
cTextFile	txtFile;

string		logbook_filename;

void Export_CSV()
{
	if (chkExportBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	string filters = "CSV\t*." "csv";
	const char* p = FSEL::saveas(_("Export to CSV file"), filters.c_str(),
					 "export." "csv");
	if (!p)
		return;

	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}
	txtFile.writeCSVFile(p, &qsodb);
}

void Export_TXT()
{
	if (chkExportBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	string filters = "TEXT\t*." "txt";
	const char* p = FSEL::saveas(_("Export to fixed field text file"), filters.c_str(),
					 "export." "txt");
	if (!p)
		return;

	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}
	txtFile.writeTXTFile(p, &qsodb);
}

void Export_ADIF()
{
	if (chkExportBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	string filters = "ADIF\t*." ADIF_SUFFIX;
	const char* p = FSEL::saveas(_("Export to ADIF file"), filters.c_str(),
					 "export." ADIF_SUFFIX);
	if (!p)
		return;

	for (int i = 0; i < chkExportBrowser->nitems(); i++) {
		if (chkExportBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}

	adifFile.writeFile (p, &qsodb);
}

static savetype export_to = ADIF;

void Export_log()
{
	if (export_to == ADIF) Export_ADIF();
	else if (export_to == CSV) Export_CSV();
	else Export_TXT();
}

void saveLogbook()
{
	if (!qsodb.isdirty()) return;
	if (!fl_choice2(_("Save changed Logbook?"), _("No"), _("Yes"), NULL))
		return;

	cQsoDb::reverse = false;
	qsodb.SortByDate();

	adifFile.writeLog (logbook_filename.c_str(), &qsodb);

	qsodb.isdirty(0);
	restore_sort();
}

void cb_mnuNewLogbook(){
	saveLogbook();

	logbook_filename = WKeyHomeDir;
	logbook_filename.append("newlog." ADIF_SUFFIX);
	progStatus.logbookfilename = logbook_filename;
	dlgLogbook->label(fl_filename_name(logbook_filename.c_str()));
	qsodb.deleteRecs();
	wBrowser->clear();
	clearRecord();
	activateButtons();
}

void OpenLogbook()
{
	adifFile.readFile (progStatus.logbookfilename.c_str(), &qsodb);
	qsodb.isdirty(0);
	loadBrowser();
	dlgLogbook->label(fl_filename_name(progStatus.logbookfilename.c_str()));
	activateButtons();
}

void cb_mnuOpenLogbook()
{
	const char* p = FSEL::select(_("Open logbook file"), "ADIF\t*." ADIF_SUFFIX,
					 logbook_filename.c_str());
	if (p) {
		saveLogbook();
		qsodb.deleteRecs();
		logbook_filename = p;
		progStatus.logbookfilename = logbook_filename;
		OpenLogbook();
	}
}

void cb_mnuSaveLogbook() {
	const char* p = FSEL::saveas(_("Save logbook file"), "ADIF\t*." ADIF_SUFFIX,
					 logbook_filename.c_str());
	if (p) {
		logbook_filename = p;
		dlgLogbook->label(fl_filename_name(logbook_filename.c_str()));

		cQsoDb::reverse = false;
		qsodb.SortByDate();

		adifFile.writeLog (logbook_filename.c_str(), &qsodb);

		qsodb.isdirty(0);
		restore_sort();
	}
}

void cb_mnuMergeADIF_log() {
	const char* p = FSEL::select(_("Merge ADIF file"), "ADIF\t*." ADIF_SUFFIX);
	if (p) {
		adifFile.readFile (p, &qsodb);
		loadBrowser();
		qsodb.isdirty(1);
	}
}

void cb_Export_log() {
	if (qsodb.nbrRecs() == 0) return;
	cQsoRec *rec;
	char line[80];
	chkExportBrowser->clear();
	chkExportBrowser->textfont(FL_COURIER);
	chkExportBrowser->textsize(12);
	for( int i = 0; i < qsodb.nbrRecs(); i++ ) {
		rec = qsodb.getRec (i);
		memset(line, 0, sizeof(line));
		snprintf(line,sizeof(line),"%8s  %4s  %-32s  %10s  %-s",
			rec->getField(QSO_DATE),
			rec->getField(TIME_OFF),
			rec->getField(CALL),
			rec->getField(FREQ),
			rec->getField(MODE) );
		chkExportBrowser->add(line);
	}
	wExport->show();
}

void cb_mnuExportADIF_log() {
	export_to = ADIF;
	cb_Export_log();
}

void cb_mnuExportCSV_log() {
	export_to = CSV;
	cb_Export_log();
}

void cb_mnuExportTEXT_log() {
	export_to = TEXT;
	cb_Export_log();
}


void cb_mnuShowLogbook()
{
	dlgLogbook->show();
}

enum State {VIEWREC, NEWREC};
static State logState = VIEWREC;

void activateButtons() 
{
	if (logState == NEWREC) {
		bNewSave->label(_("Save"));
		bUpdateCancel->label(_("Cancel"));
		bUpdateCancel->activate();
		bDelete->deactivate ();
		bSearchNext->deactivate ();
		bSearchPrev->deactivate ();
		inpDate_log->take_focus();
		return;
	}
	bNewSave->label(_("New"));
	bUpdateCancel->label(_("Update"));
	if (qsodb.nbrRecs() > 0) {
		bDelete->activate();
		bUpdateCancel->activate();
		bSearchNext->activate ();
		bSearchPrev->activate ();
		wBrowser->take_focus();
	} else {
		bDelete->deactivate();
		bUpdateCancel->deactivate();
		bSearchNext->deactivate();
		bSearchPrev->deactivate();
	}
}

void cb_btnNewSave(Fl_Button* b, void* d) {
	if (logState == VIEWREC) {
		logState = NEWREC;
		clearRecord();
		activateButtons();
	} else {
		saveRecord();
		qsodb.SortByDate();
		loadBrowser();
		logState = VIEWREC;
		activateButtons();
	}
}

void cb_btnUpdateCancel(Fl_Button* b, void* d) {
	if (logState == NEWREC) {
		logState = VIEWREC;
		activateButtons ();
	} else {
		updateRecord();
		wBrowser->take_focus();
	}
}

void cb_btnDelete(Fl_Button* b, void* d) {
	deleteRecord();
	wBrowser->take_focus();
}

enum sorttype {NONE, SORTCALL, SORTDATE, SORTFREQ, SORTMODE};
sorttype lastsort = SORTDATE;
bool callfwd = true;
bool datefwd = true;
bool modefwd = true;
bool freqfwd = true;

void restore_sort()
{
	switch (lastsort) {
	case SORTCALL :
		cQsoDb::reverse = callfwd;
		qsodb.SortByCall();
		break;
	case SORTDATE :
		cQsoDb::reverse = datefwd;
		qsodb.SortByDate();
		break;
	case SORTFREQ :
		cQsoDb::reverse = freqfwd;
		qsodb.SortByFreq();
		break;
	case SORTMODE :
		cQsoDb::reverse = modefwd;
		qsodb.SortByMode();
		break;
	default: break;
	}
}

void cb_SortByCall (void) {
	if (lastsort == SORTCALL)
		callfwd = !callfwd;
	else {
		callfwd = false;
		lastsort = SORTCALL;
	}
	cQsoDb::reverse = callfwd;
	qsodb.SortByCall();
	loadBrowser();
}

void cb_SortByDate (void) {
	if (lastsort == SORTDATE)
		datefwd = !datefwd;
	else {
		datefwd = false;
		lastsort = SORTDATE;
	}
	cQsoDb::reverse = datefwd;
	qsodb.SortByDate();
	loadBrowser();
}

void cb_SortByMode (void) {
	if (lastsort == SORTMODE)
		modefwd = !modefwd;
	else {
		modefwd = false;
		lastsort = SORTMODE;
	}
	cQsoDb::reverse = modefwd;
	qsodb.SortByMode();
	loadBrowser();
}

void cb_SortByFreq (void) {
	if (lastsort == SORTFREQ)
		freqfwd = !freqfwd;
	else {
		freqfwd = false;
		lastsort = SORTFREQ;
	}
	cQsoDb::reverse = freqfwd;
	qsodb.SortByFreq();
	loadBrowser();
}

void DupCheck()
{
	Fl_Color call_clr = FL_BACKGROUND2_COLOR;
	int ispn = atoi(txt_time_span->value());

	if (qsodb.nbrRecs() > 0) 
		if (qsodb.duplicate(
				txt_sta->value(),
				szDate(6), szTime(0), ispn, progStatus.time_span,
				xcvr_freq->strval().c_str(), progStatus.band,
				"", false,
				"CW", true,
				"", false ) ) {
			call_clr = fl_rgb_color( 255, 110, 180);
	}
	txt_sta->color(call_clr);
	txt_sta->redraw();
}

cQsoRec* SearchLog(const char *callsign)
{
	size_t len = strlen(callsign);
	char* re = new char[len + 3];
	snprintf(re, len + 3, "^%s$", callsign);

	int row = 0, col = 2;
	return wBrowser->search(row, col, !cQsoDb::reverse, re) ? qsodb.getRec(row) : 0;
}

void SearchLastQSO(const char *callsign)
{
	if (qsodb.nbrRecs() == 0) return;
	size_t len = strlen(callsign);
	if (!len)
		return;
	char* re = new char[len + 3];
	snprintf(re, len + 3, "^%s$", callsign);
	int row = 0, col = 2;
	if (wBrowser->search(row, col, !cQsoDb::reverse, re)) {
		wBrowser->GotoRow(row);
		txt_name->value(inpName_log->value());
		inpSearchString->value(callsign);
	} else {
		txt_name->value("");
		inpSearchString->value("");
	}
	delete [] re;
}

void cb_search(Fl_Widget* w, void*)
{
	const char* str = inpSearchString->value();
	if (!*str)
		return;

	bool rev = w == bSearchPrev;
	int col = 2, row = wBrowser->value() + (rev ? -1 : 1);
	row = WCLAMP(row, 0, wBrowser->rows() - 1);

	if (wBrowser->search(row, col, rev, str))
		wBrowser->GotoRow(row);
	wBrowser->take_focus();
}

int log_search_handler(int)
{
	if (!(Fl::event_state() & FL_CTRL))
		return 0;

	switch (Fl::event_key()) {
	case 's':
		bSearchNext->do_callback();
		break;
	case 'r':
		bSearchPrev->do_callback();
		break;
	default:
		return 0;
	}
	return 1;
}

int editNbr = 0;

void clearRecord() {
	Date tdy;
	inpCall_log->value ("");
	inpName_log->value ("");
	inpDate_log->value (tdy.szDate(2));
	inpTimeOn_log->value ("");
	inpTimeOff_log->value ("");
	inpRstR_log->value ("");
	inpRstS_log->value ("");
	inpFreq_log->value ("");
	inpMode_log->value ("");
	inpQth_log->value ("");
	inpState_log->value ("");
	inpVE_Prov_log->value ("");
	inpCountry_log->value ("");
	inpLoc_log->value ("");
	inpQSLrcvddate_log->value ("");
	inpQSLsentdate_log->value ("");
	inpSerNoOut_log->value ("");
	inpSerNoIn_log->value ("");
	inpXchgIn_log->value("");
	inpMyXchg_log->value("");
	inpNotes_log->value ("");
	inpIOTA_log->value("");
	inpDXCC_log->value("");
	inpCONT_log->value("");
	inpCQZ_log->value("");
	inpITUZ_log->value("");
	inpTX_pwr_log->value("");
	inpSearchString->value ("");
	editGroup->show();
}

void saveRecord() {

	cQsoRec rec;

	rec.putField(CALL, inpCall_log->value());
	rec.putField(NAME, inpName_log->value());
	rec.putField(QSO_DATE, inpDate_log->value());
	rec.putField(QSO_DATE_OFF, inpDateOff_log->value());
	rec.putField(TIME_ON, inpTimeOn_log->value());
	rec.putField(TIME_OFF, inpTimeOff_log->value());
	rec.putField(FREQ, inpFreq_log->value());
	rec.putField(MODE, inpMode_log->value());
	rec.putField(QTH, inpQth_log->value());
	rec.putField(STATE, inpState_log->value());
	rec.putField(VE_PROV, inpVE_Prov_log->value());
	rec.putField(COUNTRY, inpCountry_log->value());
	rec.putField(GRIDSQUARE, inpLoc_log->value());
	rec.putField(NOTES, inpNotes_log->value());
	rec.putField(QSLRDATE, inpQSLrcvddate_log->value());
	rec.putField(QSLSDATE, inpQSLsentdate_log->value());
	rec.putField(RST_RCVD, inpRstR_log->value ());
	rec.putField(RST_SENT, inpRstS_log->value ());
	rec.putField(SRX, inpSerNoIn_log->value());
	rec.putField(STX, inpSerNoOut_log->value());
	rec.putField(XCHG1, inpXchgIn_log->value());
	rec.putField(MYXCHG, inpMyXchg_log->value());
	rec.putField(IOTA, inpIOTA_log->value());
	rec.putField(DXCC, inpDXCC_log->value());
	rec.putField(CONT, inpCONT_log->value());
	rec.putField(CQZ, inpCQZ_log->value());
	rec.putField(ITUZ, inpITUZ_log->value());
	rec.putField(TX_PWR, inpTX_pwr_log->value());

	qsodb.qsoNewRec (&rec);

	cQsoDb::reverse = false;
	qsodb.SortByDate();

	adifFile.writeLog (logbook_filename.c_str(), &qsodb);

	qsodb.isdirty(0);
	restore_sort();

	loadBrowser();
	logState = VIEWREC;
	activateButtons();
}

void updateRecord() {
cQsoRec rec;

	if (qsodb.nbrRecs() == 0) return;
	rec.putField(CALL, inpCall_log->value());
	rec.putField(NAME, inpName_log->value());
	rec.putField(QSO_DATE, inpDate_log->value());
	rec.putField(QSO_DATE_OFF, inpDateOff_log->value());
	rec.putField(TIME_ON, inpTimeOn_log->value());
	rec.putField(TIME_OFF, inpTimeOff_log->value());
	rec.putField(FREQ, inpFreq_log->value());
	rec.putField(MODE, inpMode_log->value());
	rec.putField(QTH, inpQth_log->value());
	rec.putField(STATE, inpState_log->value());
	rec.putField(VE_PROV, inpVE_Prov_log->value());
	rec.putField(COUNTRY, inpCountry_log->value());
	rec.putField(GRIDSQUARE, inpLoc_log->value());
	rec.putField(NOTES, inpNotes_log->value());
	rec.putField(QSLRDATE, inpQSLrcvddate_log->value());
	rec.putField(QSLSDATE, inpQSLsentdate_log->value());
	rec.putField(RST_RCVD, inpRstR_log->value ());
	rec.putField(RST_SENT, inpRstS_log->value ());
	rec.putField(SRX, inpSerNoIn_log->value());
	rec.putField(STX, inpSerNoOut_log->value());
	rec.putField(XCHG1, inpXchgIn_log->value());
	rec.putField(MYXCHG, inpMyXchg_log->value());
	rec.putField(IOTA, inpIOTA_log->value());
	rec.putField(DXCC, inpDXCC_log->value());
	rec.putField(CONT, inpCONT_log->value());
	rec.putField(CQZ, inpCQZ_log->value());
	rec.putField(ITUZ, inpITUZ_log->value());
	rec.putField(TX_PWR, inpTX_pwr_log->value());
	qsodb.qsoUpdRec (editNbr, &rec);

	cQsoDb::reverse = false;
	qsodb.SortByDate();

	adifFile.writeLog (logbook_filename.c_str(), &qsodb);

	qsodb.isdirty(0);
	restore_sort();

	loadBrowser(true);
}

void deleteRecord () {
	if (qsodb.nbrRecs() == 0 || fl_choice2(_("Really delete record for \"%s\"?"),
						   _("Yes"), _("No"), NULL, wBrowser->valueAt(-1, 2)))
		return;

	qsodb.qsoDelRec(editNbr);

	cQsoDb::reverse = false;
	qsodb.SortByDate();

	adifFile.writeLog (logbook_filename.c_str(), &qsodb);

	qsodb.isdirty(0);
	restore_sort();

	loadBrowser(true);
}

void EditRecord( int i )
{
	cQsoRec *editQSO = qsodb.getRec (i);
	if( !editQSO )
		return;

	inpCall_log->value (editQSO->getField(CALL));
	inpName_log->value (editQSO->getField(NAME));
	inpDate_log->value (editQSO->getField(QSO_DATE));
	inpDateOff_log->value (editQSO->getField(QSO_DATE_OFF));
	inpTimeOn_log->value (editQSO->getField(TIME_ON));
	inpTimeOff_log->value (editQSO->getField(TIME_OFF));
	inpRstR_log->value (editQSO->getField(RST_RCVD));
	inpRstS_log->value (editQSO->getField(RST_SENT));
	inpFreq_log->value (editQSO->getField(FREQ));
	inpMode_log->value (editQSO->getField(MODE));
	inpState_log->value (editQSO->getField(STATE));
	inpVE_Prov_log->value (editQSO->getField(VE_PROV));
	inpCountry_log->value (editQSO->getField(COUNTRY));
	inpQth_log->value (editQSO->getField(QTH));
	inpLoc_log->value (editQSO->getField(GRIDSQUARE));
	inpQSLrcvddate_log->value (editQSO->getField(QSLRDATE));
	inpQSLsentdate_log->value (editQSO->getField(QSLSDATE));
	inpNotes_log->value (editQSO->getField(NOTES));
	inpSerNoIn_log->value(editQSO->getField(SRX));
	inpSerNoOut_log->value(editQSO->getField(STX));
	inpXchgIn_log->value(editQSO->getField(XCHG1));
	inpMyXchg_log->value(editQSO->getField(MYXCHG));
	inpIOTA_log->value(editQSO->getField(IOTA));
	inpDXCC_log->value(editQSO->getField(DXCC));
	inpCONT_log->value(editQSO->getField(CONT));
	inpCQZ_log->value(editQSO->getField(CQZ));
	inpITUZ_log->value(editQSO->getField(ITUZ));
	inpTX_pwr_log->value(editQSO->getField(TX_PWR));
	editGroup->show();
}

std::string sDate_on = "";

void AddRecord ()
{
	if (txt_sta->value()[0] == 0) return;
	char *szt = szTime(0);
	char *szdt = szDate(0x86);
	char sznbr[6];
	snprintf(sznbr, sizeof(sznbr), "%d", progStatus.serial_nbr);

	string xin = txt_xchg->value();
	for (size_t n = 0; n < xin.length(); n++) xin[n] = toupper(xin[n]);

	inpCall_log->value(txt_sta->value());
	inpName_log->value (txt_name->value());
	inpTimeOn_log->value (szt);
	inpTimeOff_log->value (szt);
	inpDate_log->value(szdt);
	inpRstR_log->value ("599");
	inpRstS_log->value ("599");
	inpFreq_log->value(xcvr_freq->strval().c_str());
	inpMode_log->value ("CW");
	inpState_log->value ("");
	inpVE_Prov_log->value ("");
	inpCountry_log->value ("");

	inpSerNoIn_log->value("");
	inpSerNoOut_log->value(sznbr);
	inpXchgIn_log->value(xin.c_str());
	inpMyXchg_log->value(progStatus.xout.c_str());

	inpQth_log->value ("");
	inpLoc_log->value ("");
	inpQSLrcvddate_log->value ("");
	inpQSLsentdate_log->value ("");
	inpNotes_log->value ("");

	inpTX_pwr_log->value ("");
	inpIOTA_log->value("");
	inpDXCC_log->value("");
	inpCONT_log->value("");
	inpCQZ_log->value("");
	inpITUZ_log->value("");

	saveRecord();

	txt_sta->value("");
	txt_name->value("");
	txt_xchg->value("");
}

void cb_browser (Fl_Widget *w, void *data )
{
	Table *table = (Table *)w;
	editNbr = atoi(table->valueAt(-1,6));
	EditRecord (editNbr);
}

void loadBrowser(bool keep_pos)
{
	cQsoRec *rec;
	char sNbr[6];
	int row = wBrowser->value(), pos = wBrowser->scrollPos();
	if (row >= qsodb.nbrRecs()) row = qsodb.nbrRecs() - 1;
	wBrowser->clear();
	if (qsodb.nbrRecs() == 0)
		return;
	for( int i = 0; i < qsodb.nbrRecs(); i++ ) {
		rec = qsodb.getRec (i);
		snprintf(sNbr,sizeof(sNbr),"%d",i);
		wBrowser->addRow (7,
			rec->getField(QSO_DATE),
			rec->getField(TIME_ON),
			rec->getField(CALL),
			rec->getField(NAME),
			rec->getField(FREQ),
			rec->getField(MODE),
			sNbr);
	}
	if (keep_pos && row >= 0) {
		wBrowser->value(row);
		wBrowser->scrollTo(pos);
	}
	else {
		if (cQsoDb::reverse == true)
			wBrowser->FirstRow ();
		else
			wBrowser->LastRow ();
	}
	char szRecs[6];
	snprintf(szRecs, sizeof(szRecs), "%5d", qsodb.nbrRecs());
	txtNbrRecs_log->value(szRecs);
}

//=============================================================================
// Cabrillo reporter
//=============================================================================

const char *contests[] =
{	"AP-SPRINT",
	"ARRL-10", "ARRL-160", "ARRL-DX-CW", "ARRL-DX-SSB", "ARRL-SS-CW",
	"ARRL-SS-SSB", "ARRL-UHF-AUG", "ARRL-VHF-JAN", "ARRL-VHF-JUN", "ARRL-VHF-SEP",
	"ARRL-RTTY",
	"BARTG-RTTY", "BARTG-SPRINT",
	"CQ-160-CW", "CQ-160-SSB", "CQ-WPX-CW", "CQ-WPX-RTTY", "CQ-WPX-SSB", "CQ-VHF",
	"CQ-WW-CW", "CQ-WW-RTTY", "CQ-WW-SSB",
	"DARC-WAEDC-CW", "DARC-WAEDC-RTTY", "DARC-WAEDC-SSB",
	"FCG-FQP", "IARU-HF", "JIDX-CW", "JIDX-SSB",
	"NAQP-CW", "NAQP-RTTY", "NAQP-SSB", "NA-SPRINT-CW", "NA-SPRINT-SSB", "NCCC-CQP",
	"NEQP", "OCEANIA-DX-CW", "OCEANIA-DX-SSB", "RDXC", "RSGB-IOTA",
	"SAC-CW", "SAC-SSB", "STEW-PERRY", "TARA-RTTY", 0 };

enum icontest {
	AP_SPRINT,
	ARRL_10, ARRL_160, ARRL_DX_CW, ARRL_DX_SSB, ARRL_SS_CW,
	ARRL_SS_SSB, ARRL_UHF_AUG, ARRL_VHF_JAN, ARRL_VHF_JUN, ARRL_VHF_SEP,
	ARRL_RTTY,
	BARTG_RTTY, BARTG_SPRINT,
	CQ_160_CW, CQ_160_SSB, CQ_WPX_CW, CQ_WPX_RTTY, CQ_WPX_SSB, CQ_VHF,
	CQ_WW_CW, CQ_WW_RTTY, CQ_WW_SSB,
	DARC_WAEDC_CW, DARC_WAEDC_RTTY, DARC_WAEDC_SSB,
	FCG_FQP, IARU_HF, JIDX_CW, JIDX_SSB,
	NAQP_CW, NAQP_RTTY, NAQP_SSB, NA_SPRINT_CW, NA_SPRINT_SSB, NCCC_CQP,
	NEQP, OCEANIA_DX_CW, OCEANIA_DX_SSB, RDXC, RSGB_IOTA,
	SAC_CW, SAC_SSB, STEW_PERRY, TARA_RTTY
};

bool bInitCombo = true;
icontest contestnbr;

void setContestType()
{
	contestnbr = (icontest)cboContest->index();

	btnCabCall->value(true);
	btnCabFreq->value(true);
	btnCabMode->value(true);
	btnCabQSOdateOn->value(false);
	btnCabTimeOn->value(false);
	btnCabQSOdateOff->value(true);
	btnCabTimeOff->value(true);
	btnCabRSTsent->value(true);
	btnCabRSTrcvd->value(true);
	btnCabSerialIN->value(true);
	btnCabSerialOUT->value(true);
	btnCabXchgIn->value(true);
	btnCabMyXchg->value(true);

	switch (contestnbr) {
		case ARRL_SS_CW :
		case ARRL_SS_SSB :
			btnCabRSTrcvd->value(false);
			break;
		case BARTG_RTTY :
		case BARTG_SPRINT :
			break;
		case ARRL_UHF_AUG :
		case ARRL_VHF_JAN :
		case ARRL_VHF_JUN :
		case ARRL_VHF_SEP :
		case CQ_VHF :
			btnCabRSTrcvd->value(false);
			btnCabSerialIN->value(false);
			btnCabSerialOUT->value(false);
			break;
		case AP_SPRINT :
		case ARRL_10 :
		case ARRL_160 :
		case ARRL_DX_CW :
		case ARRL_DX_SSB :
		case CQ_160_CW :
		case CQ_160_SSB :
		case CQ_WPX_CW :
		case CQ_WPX_RTTY :
		case CQ_WPX_SSB :
		case RDXC :
		case OCEANIA_DX_CW :
		case OCEANIA_DX_SSB :
			break;
		case DARC_WAEDC_CW :
		case DARC_WAEDC_RTTY :
		case DARC_WAEDC_SSB :
			break;
		case NAQP_CW :
		case NAQP_RTTY :
		case NAQP_SSB :
		case NA_SPRINT_CW :
		case NA_SPRINT_SSB :
			break;
		case RSGB_IOTA :
			break;
		default :
			break;
	}
}

void cb_Export_Cabrillo() {
	if (qsodb.nbrRecs() == 0) return;
	cQsoRec *rec;
	char line[80];
	int indx = 0;

	if (bInitCombo) {
		bInitCombo = false;
		while (contests[indx]) 	{
			cboContest->add(contests[indx]);
			indx++;
		}
	}
	cboContest->index(0);
	chkCabBrowser->clear();
	chkCabBrowser->textfont(FL_COURIER);
	chkCabBrowser->textsize(12);
	for( int i = 0; i < qsodb.nbrRecs(); i++ ) {
		rec = qsodb.getRec (i);
		memset(line, 0, sizeof(line));
		snprintf(line,sizeof(line),"%8s  %4s  %-32s  %10s  %-s",
			rec->getField(QSO_DATE),
			rec->getField(TIME_OFF),
			rec->getField(CALL),
			rec->getField(FREQ),
			rec->getField(MODE) );
		chkCabBrowser->add(line);
	}
	wCabrillo->show();
}

void cabrillo_append_qso (FILE *fp, cQsoRec *rec)
{
	char freq[16] = "";
	string rst_in, rst_out, exch_in, exch_out, date, time, mode, mycall, call;
	string qsoline = "QSO: ";
	int rst_len = 3;
	int ifreq = 0;
	size_t len = 0;

	exch_out.clear();

	if (btnCabFreq->value()) {
		ifreq = (int)(1000.0 * atof(rec->getField(FREQ)));
		snprintf(freq, sizeof(freq), "%d", ifreq);
		qsoline.append(freq); qsoline.append(" ");
	}

	if (btnCabMode->value()) {
		mode = rec->getField(MODE);
		if (mode.compare("USB") == 0 || mode.compare("LSB") == 0 ||
			mode.compare("PH") == 0 ) mode = "PH";
		else if (mode.compare("FM") == 0 || mode.compare("CW") == 0 ) ;
		else mode = "RY";
		if (mode.compare("PH") == 0 || mode.compare("FM") == 0 ) rst_len = 2;
		qsoline.append(mode); qsoline.append(" ");
	}

	if (btnCabQSOdateOn->value()) {
		date = rec->getField(QSO_DATE);
		date.insert(4,"-");
		date.insert(7,"-");
		qsoline.append(date); qsoline.append(" ");
	}
	if (btnCabTimeOn->value()) {
		time = rec->getField(TIME_ON);
		qsoline.append(time); qsoline.append(" ");
	}

	if (btnCabQSOdateOff->value()) {
		date = rec->getField(QSO_DATE_OFF);
		date.insert(4,"-");
		date.insert(7,"-");
		qsoline.append(date); qsoline.append(" ");
	}
	if (btnCabTimeOff->value()) {
		time = rec->getField(TIME_OFF);
		qsoline.append(time); qsoline.append(" ");
	}

//	mycall = progStatus.tag_cll;
//	mycall = progdefaults.myCall;
	if (mycall.length() > 13) mycall = mycall.substr(0,13);
	if ((len = mycall.length()) < 13) mycall.append(13 - len, ' ');
	qsoline.append(mycall); qsoline.append(" ");

	if (btnCabRSTsent->value()) {
		rst_out = rec->getField(RST_SENT);
		rst_out = rst_out.substr(0,rst_len);
		exch_out.append(rst_out).append(" ");
	}

	if (btnCabSerialOUT->value()) {
		exch_out.append(rec->getField(STX)).append(" ");
	}

	if (btnCabMyXchg->value()) {
		exch_out.append(rec->getField(MYXCHG)).append(" ");
	}

	if (contestnbr == BARTG_RTTY && exch_out.length() < 11) {
		exch_out.append(rec->getField(TIME_OFF)).append(" ");
	}

	if (exch_out.length() > 14) exch_out = exch_out.substr(0,14);
	if ((len = exch_out.length()) < 14) exch_out.append(14 - len, ' ');

	qsoline.append(exch_out).append(" ");

	if (btnCabCall->value()) {
		call = rec->getField(CALL);
		if (call.length() > 13) call = call.substr(0,13);
		if ((len = call.length()) < 13) call.append(13 - len, ' ');
		qsoline.append(call); qsoline.append(" ");
	}

	if (btnCabRSTrcvd->value()) {
		rst_in = rec->getField(RST_RCVD);
		rst_in = rst_in.substr(0,rst_len);
		qsoline.append(rst_in); qsoline.append(" ");
	}

	if (btnCabSerialIN->value()) {
		exch_in = rec->getField(SRX);
		if (exch_in.length())
			exch_in += ' ';
	}
	if (btnCabXchgIn->value())
		exch_in.append(rec->getField(XCHG1));
	if (exch_in.length() > 14) exch_in = exch_in.substr(0,14);
	if ((len = exch_in.length()) < 14) exch_in.append(14 - len, ' ');
	qsoline.append(exch_in);

	fprintf (fp, "%s\n", qsoline.c_str());
	return;
}

void WriteCabrillo()
{
	if (chkCabBrowser->nchecked() == 0) return;

	cQsoRec *rec;

	string filters = "TEXT\t*.txt";
	string strContest = "";

	const char* p = FSEL::saveas(_("Create cabrillo report"), filters.c_str(),
					 "contest.txt");
	if (!p)
		return;

	for (int i = 0; i < chkCabBrowser->nitems(); i++) {
		if (chkCabBrowser->checked(i + 1)) {
			rec = qsodb.getRec(i);
			rec->putField(EXPORT, "E");
			qsodb.qsoUpdRec (i, rec);
		}
	}

	FILE *cabFile = fopen (p, "w");
	if (!cabFile)
		return;

	strContest = cboContest->value();
	contestnbr = (icontest)cboContest->index();

	fprintf (cabFile,
"START-OF-LOG: 3.0\n\
CREATED-BY: %s %s\n\
\n\
# The callsign used during the contest.\n\
CALLSIGN: %s\n\
\n\
# ASSISTED or NON-ASSISTED\n\
CATEGORY-ASSISTED: \n\
\n\
# Band: ALL, 160M, 80M, 40M, 20M, 15M, 10M, 6M, 2M, 222, 432, 902, 1.2G\n\
CATEGORY-BAND: \n\
\n\
# Mode: SSB, CW, RTTY, MIXED \n\
CATEGORY-MODE: \n\
\n\
# Operator: SINGLE-OP, MULTI-OP, CHECKLOG \n\
CATEGORY-OPERATOR: \n\
\n\
# Power: HIGH, LOW, QRP \n\
CATEGORY-POWER: \n\
\n\
# Station: FIXED, MOBILE, PORTABLE, ROVER, EXPEDITION, HQ, SCHOOL \n\
CATEGORY-STATION: \n\
\n\
# Time: 6-HOURS, 12-HOURS, 24-HOURS \n\
CATEGORY-TIME: \n\
\n\
# Transmitter: ONE, TWO, LIMITED, UNLIMITED, SWL \n\
CATEGORY-TRANSMITTER: \n\
\n\
# Overlay: ROOKIE, TB-WIRES, NOVICE-TECH, OVER-50 \n\
CATEGORY-OVERLAY: \n\
\n\
# Integer number\n\
CLAIMED-SCORE: \n\
\n\
# Name of the radio club with which the score should be aggregated.\n\
CLUB: \n\
\n\
# Contest: AP-SPRINT, ARRL-10, ARRL-160, ARRL-DX-CW, ARRL-DX-SSB, ARRL-SS-CW,\n\
# ARRL-SS-SSB, ARRL-UHF-AUG, ARRL-VHF-JAN, ARRL-VHF-JUN, ARRL-VHF-SEP,\n\
# ARRL-RTTY, BARTG-RTTY, CQ-160-CW, CQ-160-SSB, CQ-WPX-CW, CQ-WPX-RTTY,\n\
# CQ-WPX-SSB, CQ-VHF, CQ-WW-CW, CQ-WW-RTTY, CQ-WW-SSB, DARC-WAEDC-CW,\n\
# DARC-WAEDC-RTTY, DARC-WAEDC-SSB, FCG-FQP, IARU-HF, JIDX-CW, JIDX-SSB,\n\
# NAQP-CW, NAQP-RTTY, NAQP-SSB, NA-SPRINT-CW, NA-SPRINT-SSB, NCCC-CQP,\n\
# NEQP, OCEANIA-DX-CW, OCEANIA-DX-SSB, RDXC, RSGB-IOTA, SAC-CW, SAC-SSB,\n\
# STEW-PERRY, TARA-RTTY \n\
CONTEST: %s\n\
\n\
# Optional email address\n\
EMAIL: \n\
\n\
LOCATION: \n\
\n\
# Operator name\n\
NAME: \n\
\n\
# Maximum 4 address lines.\n\
ADDRESS: \n\
ADDRESS: \n\
ADDRESS: \n\
ADDRESS: \n\
\n\
# A space-delimited list of operator callsign(s). \n\
OPERATORS: \n\
\n\
# Offtime yyyy-mm-dd nnnn yyyy-mm-dd nnnn \n\
# OFFTIME: \n\
\n\
# Soapbox comments.\n\
SOAPBOX: \n\
SOAPBOX: \n\
SOAPBOX: \n\n",
		PACKAGE_NAME, PACKAGE_VERSION,
		"",//progStatus.tag_cll.c_str(),
//		progdefaults.myCall.c_str(),
		strContest.c_str() );

	qsodb.SortByDate();
	for (int i = 0; i < qsodb.nbrRecs(); i++) {
		rec = qsodb.getRec(i);
		if (rec->getField(EXPORT)[0] == 'E') {
			cabrillo_append_qso(cabFile, rec);
			rec->putField(EXPORT,"");
			qsodb.qsoUpdRec(i, rec);
		}
	}
	fprintf(cabFile, "END-OF-LOG:\n");
	fclose (cabFile);
	return;
}

/*
#include <tr1/unordered_map>
typedef tr1::unordered_map<string, unsigned> dxcc_entity_cache_t;
static dxcc_entity_cache_t dxcc_entity_cache;
static bool dxcc_entity_cache_enabled = false;

#include "dxcc.h"

static void dxcc_entity_cache_clear(void)
{
	if (dxcc_entity_cache_enabled)
		dxcc_entity_cache.clear();
}

static void dxcc_entity_cache_add(cQsoRec* r)
{
	if (!dxcc_entity_cache_enabled | !r)
		return;

	const dxcc* e = dxcc_lookup(r->getField(CALL));
	if (e)
		dxcc_entity_cache[e->country]++;
}
static void dxcc_entity_cache_add(cQsoDb& db)
{
	if (!dxcc_entity_cache_enabled)
		return;

	int n = db.nbrRecs();
	for (int i = 0; i < n; i++)
		dxcc_entity_cache_add(db.getRec(i));
	if (!dxcc_entity_cache.empty())
		LOG_INFO("Found %" PRIuSZ " countries in %d QSO records",
			 dxcc_entity_cache.size(), n);
}

static void dxcc_entity_cache_rm(cQsoRec* r)
{
	if (!dxcc_entity_cache_enabled || !r)
		return;

	const dxcc* e = dxcc_lookup(r->getField(CALL));
	if (!e)
		return;
	dxcc_entity_cache_t::iterator i = dxcc_entity_cache.find(e->country);
	if (i != dxcc_entity_cache.end()) {
		if (i->second)
			i->second--;
		else
			dxcc_entity_cache.erase(i);
	}
}

void dxcc_entity_cache_enable(bool v)
{
	if (dxcc_entity_cache_enabled == v)
		return;

	dxcc_entity_cache_clear();
	if ((dxcc_entity_cache_enabled = v))
		dxcc_entity_cache_add(qsodb);
}

bool qsodb_dxcc_entity_find(const char* country)
{
	return dxcc_entity_cache.find(country) != dxcc_entity_cache.end();
}
*/
