#include <config.h>

#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR < 3
#  include "Fl_Text_Display_mod_1_1.H"
#elif FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
#  include "Fl_Text_Display_mod_1_3.H"
#endif
