#include <config.h>

#include <cstring>

#include <FL/Fl.H>
#include <FL/filename.H>

#include "flwkey.h"
#include "logbook.h"
#include "debug.h"
#include "status.h"

using namespace std;

std::string log_checksum;

pthread_t logbook_thread;
pthread_mutex_t logbook_mutex = PTHREAD_MUTEX_INITIALIZER;
bool logbook_exit = false;

void do_load_browser(void *)
{
	loadBrowser();
}

void start_logbook ()
{
	create_logbook_dialogs();
	dlgLogbook->size(580,384);

	if (progStatus.logbookfilename.empty()) {
		logbook_filename = WKeyHomeDir;
		logbook_filename.append("logbook." ADIF_SUFFIX);
		progStatus.logbookfilename = logbook_filename;
	} else
		logbook_filename = progStatus.logbookfilename;

	adifFile.readFile (logbook_filename.c_str(), &qsodb);
	if (qsodb.nbrRecs() == 0)
		adifFile.writeFile(logbook_filename.c_str(), &qsodb);

	string label = "Logbook - ";
	label.append(fl_filename_name(logbook_filename.c_str()));
	dlgLogbook->copy_label(label.c_str());

	loadBrowser();
	qsodb.isdirty(0);
	activateButtons();

}

void close_logbook()
{
	saveLogbook();
}
