# Flwkey
David Freese's (W1HKJ) FLWKEY program, slightly adapted and modified:

Corrections:

- some small but obvious error corrections
- "modernized" obsolete autoconf constructs,
  e.g. AC_HELP_STRING ==> AS_HELP_STRING
- small changes such that flwkey compiles on recent versions of MacOS
- Error with the PTT lead-in and tail times corrected.

Additions:

- read parameters from WinKeyer EEPROM, and write current 
  parameters to EEPROM.
