# TeensyMoog
## What is it ?
_Teensymoog_ is a synth, inspired by the legendary minimoog.

It aims at looking and functionning as close as possible of the orinal, but using digital tools, namely Teensy and Arduino boards.

It uses three boards, one Teensy 4.0 that handles the heavy computing (sound generation), and two Arduino Mega (chinese copies in a small form factor), that are used as "port expander", which read keys, switches and potentiometers and send their value on change to the Teensy.

