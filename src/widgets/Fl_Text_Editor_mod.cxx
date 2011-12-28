#include <config.h>

#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR < 3
#  include "Fl_Text_Editor_mod_1_1.cxx"
#elif FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
#  include "Fl_Text_Editor_mod_1_3.cxx"
#endif
