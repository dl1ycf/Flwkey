#include "config.h"

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/x.H>
#include <FL/Fl_Help_Dialog.H>
#include <FL/Fl_Menu_Item.H>

#ifdef WIN32
#  include "flwkeyrc.h"
#  include "compat.h"
#  define dirent fl_dirent_no_thanks
#endif

#include <FL/filename.H>
#ifdef __MINGW32__
#  undef dirent
#endif

//#include "fileselect.h"
#include <dirent.h>

#include <FL/x.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Image.H>

//#include "images/flwkey.xpm"
#include "support.h"
#include "dialogs.h"
#include "status.h"
#include "debug.h"
#include "util.h"
#include "gettext.h"
#include "flwkey_icon.cxx"
#include "wkey_dialogs.h"
#include "fileselect.h"
#include "logbook.h"

int parse_args(int argc, char **argv, int& idx);

Fl_Double_Window *mainwindow;
string WKeyHomeDir;
string TempDir;
string defFileName;
string title;

pthread_t *serial_thread = 0;
pthread_t *digi_thread = 0;

pthread_mutex_t mutex_serial = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_xmlrpc = PTHREAD_MUTEX_INITIALIZER;

bool WKEY_DEBUG = 0;

//----------------------------------------------------------------------
void about()
{
	string msg = "\
%s\n\
Version %s\n\
copyright W1HKJ, 2009\n\
w1hkj@@w1hkj.com";
	fl_message(msg.c_str(), PACKAGE_TARNAME, PACKAGE_VERSION);
}

void visit_URL(void* arg)
{
	const char* url = reinterpret_cast<const char *>(arg);
#ifndef __WOE32__
	const char* browsers[] = {
#  ifdef __APPLE__
		getenv("FLDIGI_BROWSER"), // valid for any OS - set by user
		"open"                    // OS X
#  else
		"fl-xdg-open",            // Puppy Linux
		"xdg-open",               // other Unix-Linux distros
		getenv("FLDIGI_BROWSER"), // force use of spec'd browser
		getenv("BROWSER"),        // most Linux distributions
		"sensible-browser",
		"firefox",
		"mozilla"                 // must be something out there!
#  endif
	};
	switch (fork()) {
	case 0:
#  ifndef NDEBUG
		unsetenv("MALLOC_CHECK_");
		unsetenv("MALLOC_PERTURB_");
#  endif
		for (size_t i = 0; i < sizeof(browsers)/sizeof(browsers[0]); i++)
			if (browsers[i])
				execlp(browsers[i], browsers[i], url, (char*)0);
		exit(EXIT_FAILURE);
	case -1:
		fl_alert(_("Could not run a web browser:\n%s\n\n"
			 "Open this URL manually:\n%s"),
			 strerror(errno), url);
	}
#else
	if ((int)ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) <= 32)
		fl_alert(_("Could not open url:\n%s\n"), url);
#endif
}

//----------------------------------------------------------------------

extern void saveFreqList();

void * flwkey_terminate(void) {
	std::cerr << "terminating" << std::endl;
	fl_message("Closing flwkey");
	cbExit();
	return 0;
}

void showEvents(void *)
{
	debug::show();
}

#if defined(__WIN32__) && defined(PTW32_STATIC_LIB)
static void ptw32_cleanup(void)
{
	(void)pthread_win32_process_detach_np();
}

void ptw32_init(void)
{
	(void)pthread_win32_process_attach_np();
	atexit(ptw32_cleanup);
}
#endif // __WIN32__

#define KNAME "Flwkey"
#if !defined(__WIN32__) && !defined(__APPLE__)
Pixmap  WKey_icon_pixmap;

void make_pixmap(Pixmap *xpm, const char **data)
{
	Fl_Window w(0,0, KNAME);
	w.xclass(KNAME);
	w.show();
	w.make_current();
	Fl_Pixmap icon(data);
	int maxd = (icon.w() > icon.h()) ? icon.w() : icon.h();
	*xpm = fl_create_offscreen(maxd, maxd);
	fl_begin_offscreen(*xpm);
	fl_color(FL_BACKGROUND_COLOR);
	fl_rectf(0, 0, maxd, maxd);
	icon.draw(maxd - icon.w(), maxd - icon.h());
	fl_end_offscreen();
}

#endif

static void checkdirectories(void)
{
	struct {
		string& dir;
		const char* suffix;
		void (*new_dir_func)(void);
	} dirs[] = {
		{ WKeyHomeDir, 0, 0 }
	};

	int r;
	for (size_t i = 0; i < sizeof(dirs)/sizeof(*dirs); i++) {
		if (dirs[i].suffix)
			dirs[i].dir.assign(WKeyHomeDir).append(dirs[i].suffix).append("/");

		if ((r = mkdir(dirs[i].dir.c_str(), 0777)) == -1 && errno != EEXIST) {
			cerr << _("Could not make directory") << ' ' << dirs[i].dir
				 << ": " << strerror(errno) << '\n';
			exit(EXIT_FAILURE);
		}
		else if (r == 0 && dirs[i].new_dir_func)
			dirs[i].new_dir_func();
	}
}

void exit_main(Fl_Widget *w)
{
	if (Fl::event_key() == FL_Escape)
		return;
	cbExit();
}

void startup(void*)
{
	if (!start_wkey_serial()) {
		if (progStatus.serial_port_name.compare("NONE") == 0) {
			LOG_WARN("No comm port ... test mode");
		} else {
			progStatus.serial_port_name = "NONE";
			selectCommPort->value(progStatus.serial_port_name.c_str());
		}
	} else {
		bypass_serial_thread_loop = false;
		open_wkeyer();
	}
}

int main (int argc, char *argv[])
{
	std::terminate_handler(flwkey_terminate);

	int arg_idx;
	Fl::args(argc, argv, arg_idx, parse_args);

	mainwindow = WKey_window();
	mainwindow->callback(exit_main);
	create_comm_dialog();

	char dirbuf[FL_PATH_MAX + 1];
#ifdef __WIN32__
	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$USERPROFILE/flwkey.files/");
#else
	fl_filename_expand(dirbuf, sizeof(dirbuf) - 1, "$HOME/.flwkey/");
#endif
	WKeyHomeDir = dirbuf;
	checkdirectories();

	try {
		debug::start(string(WKeyHomeDir).append("status_log.txt").c_str());
		time_t t = time(NULL);
		LOG(debug::WARN_LEVEL, debug::LOG_OTHER, _("%s log started on %s"), PACKAGE_STRING, ctime(&t));
	}
	catch (const char* error) {
		cerr << error << '\n';
		debug::stop();
		exit(1);
	}

	Fl::lock();

#if defined(__WIN32__) && defined(PTW32_STATIC_LIB)
	ptw32_init();
#endif

	bypass_serial_thread_loop = true;
	serial_thread = new pthread_t;
	if (pthread_create(serial_thread, NULL, serial_thread_loop, NULL)) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}

	progStatus.loadLastState();

	mainwindow->resize( progStatus.mainX, progStatus.mainY, mainwindow->w(), mainwindow->h());

	mainwindow->xclass(KNAME);
	Fl::add_handler(main_handler);

	start_logbook();

#if defined(__WOE32__)
#  ifndef IDI_ICON
#    define IDI_ICON 101
#  endif
	mainwindow->icon((char*)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON)));
	mainwindow->show (argc, argv);
#elif !defined(__APPLE__)
	make_pixmap(&WKey_icon_pixmap, flwkey_icon);
	mainwindow->icon((char *)WKey_icon_pixmap);
	mainwindow->show(argc, argv);
#else
	mainwindow->show(argc, argv);
#endif

	FSEL::create();

	Fl::add_timeout(0.20, startup);

	return Fl::run();

}

int parse_args(int argc, char **argv, int& idx)
{
	if (strcasecmp("--help", argv[1]) == 0) {
		printf("Usage: \n\
  --help this help text\n\
  --version\n\
  --debug\n");
		exit(0);
	} 
	if (strcasecmp("--version", argv[1]) == 0) {
		printf("Version: "VERSION"\n");
		exit (0);
	}
	if (strcasecmp("--debug", argv[1]) == 0) {
		WKEY_DEBUG = 1;
		idx++;
		return 1;
	}
	return 0;
}
