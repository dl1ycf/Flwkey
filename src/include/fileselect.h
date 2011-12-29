#ifndef FILESELECT_H
#define FILESELECT_H

#include <config.h>

//#ifdef __WIN32__
//#  define FSEL_THREAD 1
//#endif
#define FSEL_THREAD 0

#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR < 3
class Fl_Native_File_Chooser;
#else
#include <FL/Fl_Native_File_Chooser.H>
#endif

class FSEL
{
public:
	static void create(void);
	static void destroy(void);
	static const char* select(const char* title, const char* filter, const char* def = 0, int* fsel = 0);
	static const char* saveas(const char* title, const char* filter, const char* def = 0, int* fsel = 0);
	static const char* dir_select(const char* title, const char* filter, const char* def = 0);
	~FSEL();
private:
	FSEL();
	FSEL(const FSEL&);
	FSEL& operator=(const FSEL&);

	const char* get_file(void);
#if FSEL_THREAD
	static void* thread_func(void* arg);
#endif
private:
	static FSEL* inst;
	Fl_Native_File_Chooser* chooser;
	int result;
};

#endif // FILESELECT_H
