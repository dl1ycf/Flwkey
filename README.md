# Flwkey
David Freese's (W1HKJ) FLWKEY program, slightly adapted and modified:

- some small but obvious error corrections
- small changes such that flwkey compiles on recent versions of MacOS
- If configured with "configure --without-xml"
  then all XML stuff (rig control, logbook) is left out such that
  the flwkey window occupies much less space.
- Error with the PTT lead-in and tail times corrected.

Additions:

read parameters from WinKeyer EEPROM, and write current 
parameters to EEPROM. This allows to modify the
"standalone settings".
