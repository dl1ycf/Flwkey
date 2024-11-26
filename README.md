# Flwkey
David Freese's (W1HKJ) FLWKEY program, slightly adapted and modified:

Corrections:

- several small but obvious error corrections
- "modernized" obsolete autoconf constructs,
  e.g. AC_HELP_STRING ==> AS_HELP_STRING
- small changes such that flwkey compiles on recent versions of MacOS
- Error with the PTT lead-in and tail times corrected.

Additions:

- read/write parameters from/to WinKeyer EEPROM, these then become
  effective in the WinKeyer "standalone" mode

Note:

- FLTK 1.1 support dropped, works on FLTK 1.3.x and 1.4.x
  but still uses some deprecated FLTK 1.3 features.
