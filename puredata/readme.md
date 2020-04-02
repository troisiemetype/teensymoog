### Pure Data patches
These patch has been used when writing the main code running on Teensy, to setup the audio layout and test things.
The panel sends the CC on midi channel 1.
The (very basic !) keyboard sends note on and note off on channel 1 too.

You can use it by uncommenting the 4 usbMIDI commented lines in setup, that maps the  usbMIDI to internal midi functions. The majority of the audio functions of the synth can be accessed, without having a physical interface, like keyboard, knobs, switches, etc. The only thing you need here is the Teensy and an audio DAC.

It has been used before the CC LSB were implemented, so maybe 