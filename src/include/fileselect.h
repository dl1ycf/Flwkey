#ifndef FILESELECT_H
#define FILESELECT_H

#include <config.h>

//#ifdef __WIN32__
//#  define FSEL_THREAD 1
//#endif
//#define FSEL_THREAD 0

	#ifdef __WIN_32
		class Fl_Native_File_Chooser;
	#else
		#include <FL/Fl_Native_File_Chooser.H>
	#endif

namespace FSEL {

	void create(void);
	void destroy(void);
	const char* select(const char* title, const char* filter, const char* def = 0);
	const char* saveas(const char* title, const char* filter, const char* def = 0);
	const char* dir_select(const char* title, const char* filter, const char* def = 0);

}

#endif // FILESELECT_H
