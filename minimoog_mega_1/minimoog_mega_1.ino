// Minimoog - mega 1
/*
 * This program is part of a minimoog-like synthesizer based on teensy 4.0
 * Copyright (C) 2020  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* This program is a synthesizer very similar to the minimoog model D.
 * It is intended to run on a Teensy 4.0, using the PJRC audio library.
 * It also uses two Arduino Mega boards to manage all the user inputs :
 * keyboard, switches, potentiometers, etc.
 * All user inputs are handled and send to the teensy board using midi commands
 */

/*
 * This program is for the Arduino Mega 1, it handles the readings of :
 * Category			input 				type		pin				MIDI
 * 
 * Keyboard
 *					Keys				switch 		22-51			note on / note off
 * Modulation
 *					LFO rate			pot			A0 				CC 30
 *					modulation mix 		pot			A1 				CC  3
 *					glide				pot			A2				CC  5
 *					pitchbend wheel		pot			A3				pitchbend change
 *					modulation wheel	pot			A4 				CC  1
 *					modulation mix 1	switch 		3				CC 117
 *					modulation mix 2 	switch 		2				CC 118
 *					osc modulation 		switch 		4 				CC 115
 *					glide				switch 		5				CC 65
 *					LFO waveform		switch 		6				CC 119
 * Oscillators
 *					osc 1 range			switch 		A5 				CC 102
 *					osc 1 waveform		switch 		A8 				CC 103
 *					osc 2 range			switch 		A6				CC 104
 *					osc 2 waveform		switch 		A9 				CC 105
 *					osc 3 range			switch 		A7 				CC 106
 *					osc 3 waveform		switch 		A10 			CC 107
 *					osc 3 control		switch 		7 				CC 108
 * Mixer
 *					osc 1 mix 			pot			A11				CC 85
 *					osc 2 mix 			pot			A12				CC 86
 *					osc 3 mix 			pot			A13				CC 87
 *					noise mix 			pot			A14				CC 88
 *					feedback mix 		pot			A15				CC 89
 *					osc 1				switch 		8 				Directly added to mix
 *					osc 2				switch 		9				ditto
 *					osc 3				switch 		10				ditto
 *					noise				switch 		11				ditto
 *					feedback			switch 		12				ditto
 *					noise color			switch 		17				CC 114 // led on 13 lowers tension
 * Other
 *					function			switch 		14 				CC 113
 *					transpose +			switch 		15 				CC 112
 *					transpose -			switch 		16				CC 112
 * Communication
 *					TX1 to teensy					18
 *					RX1 from teensy					19
 */

/*
 * Note on switches : every switch is active low, using the internal pull-up resistor.
 * The keys are active low with internal pull-up too.
 */
/*
 * Note on rotary selectors : the one I found are 12-positions selector, with a setting washer limiting their travel.
 * Here six positions are used, but more or less could be used as well to suit one needs for different waveforms.
 * They are wired with a resistor array between their pins, with ground on the first pin, +5V on the sixth pin,
 * and a 510ohm resistor between first and second pin, second and third, etc.
 * (five resistors total for a six-position switch)
 */
/*
 * Note on transpose (octave) switch : I used a three position temporary switch (ON)-OFF-(ON),
 * each of its (ON) pin mapped to a different digital input.
 * They share the same MIDI control command, sending 0 for octave - and 127 for octave +.
 * Two tact switches can be used instead of this three-position switch, without modification of the code.
 */

// includes
#include "MIDI.h"			// https://github.com/FortySevenEffects/arduino_midi_library
#include "PushButton.h"		// https://github.com/troisiemetype/PushButton
#include "ExpFilter.h"		// https://github.com/troisiemetype/expfilter
#include "defs.h"

// Constants
const uint8_t NUM_KEYS = 30;
// const uint8_t MIDI_OFFSET = 48; // moved to teensy !

const uint8_t NUM_SWITCHES = 15;
const uint8_t NUM_POTS = 16;
const uint8_t NUM_SELECTORS = 6;

const uint8_t POT_FILTER_COEF = 20;

// Digital pin definition
const uint8_t KEYS[NUM_KEYS] = {
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51
};

PushButton keys[NUM_KEYS];

/*
 * For memory : pin definitions. Some have moved, I let them here in case it would be needed.
 * They could be used to init switch and pots tables and make them more readable,
 * but once they are set they are unlikely to be modified.
 */
/*
const uint8_t PIN_MOD_MIX_1 = 2;
const uint8_t PIN_MOD_MIX_2 = 3;
const uint8_t PIN_OSC_MOD = 4;
const uint8_t PIN_GLIDE = 5;
const uint8_t PIN_LFO_WAVEFORM = 6;
const uint8_t PIN_OSC3_CTRL = 7;

const uint8_t PIN_MIX_OSC_1 = 8;
const uint8_t PIN_MIX_OSC_2 = 9;
const uint8_t PIN_MIX_OSC_3 = 10;
const uint8_t PIN_MIX_OSC_NOISE = 11;
const uint8_t PIN_MIX_OSC_EXT = 12;
const uint8_t PIN_MIX_OSC_NOISE_COLOR = 13;
const uint8_t PIN_FUNCTION = 14;

const uint8_t PIN_TRANSPOSE_PLUS = 52;
const uint8_t PIN_TRANSPOSE_MINUS = 53;
*/
// Analog pin definition
/*
const uint8_t APIN_LFO_RATE = A0;
const uint8_t APIN_MOD_MIX = A1;
const uint8_t APIN_GLIDE = A2;
const uint8_t APIN_PITCH_WHEEL = A3;
const uint8_t APIN_MOD_WHEEL = A4;

const uint8_t APIN_OSC_1_RANGE = A5;
const uint8_t APIN_OSC_1_WAVEFORM = A6;
const uint8_t APIN_OSC_2_RANGE = A7;
const uint8_t APIN_OSC_2_WAVEFORM = A8;
const uint8_t APIN_OSC_3_RANGE = A9;
const uint8_t APIN_OSC_3_WAVEFORM = A10;

const uint8_t APIN_MIX_OSC_1 = A11;
const uint8_t APIN_MIX_OSC_2 = A12;
const uint8_t APIN_MIX_OSC_3 = A13;
const uint8_t APIN_MIX_NOISE = A14;
const uint8_t APIN_MIX_EXT = A15;
*/

// Every pin is defined through tables grouped by category. Pots, switches, keys (above).
// The main and setup loop can thus iterate the table to update every reanding easily.
const uint8_t APIN[NUM_POTS] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};

// Vars
// storing the last reading
uint16_t potState[NUM_POTS];

// switches pinout. Every switch is active low, and uses the internal pull-up resistor of the Atmega chip.
const uint8_t PIN[NUM_SWITCHES] = {3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 14, 15, 16};

// Deboucing is handled by the push button library, as well as key initialisation and reading.
PushButton switches[NUM_SWITCHES];
// ExpFilter "debounces" ADC readings, filter noise. The readings only change when the users efectively moves a pot.
ExpFilter pots[NUM_POTS];
// 
uint8_t selectors[NUM_SELECTORS];

// Keyboard
bool keyState[NUM_KEYS];

// Mixer
// Mixer stores the values from potentiometers and switches, because switch turn channel on / off.
// Depending of the switch position, the value from the potentiometer is to be sent or not,
// and when turned on the potentiometer value has to be sent again.
uint16_t mix[5];
bool mixSw[5];

// Misc
uint8_t defaultVelocity = 64;

// update flag for data request from Teensy.
bool update = 0;

// Internal communication between the three boards ca be faster then MIDI standard. They handle it well.
// They could probably handle a baudrate of 250000 or 500000.
struct midiSettings : public midi::DefaultSettings{
//	static const bool UseRunningStatus = true;
	static const long BaudRate = 115200;
};

// The one we use on synth
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, midi1, midiSettings);
// For debug purposes
//MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, midi1, midiSettings);

void setup(){
	// initialisation

//	Serial.begin(115200);

	// Key initialisation
	for(uint8_t i = 0; i < NUM_KEYS; ++i){
		keys[i].begin(KEYS[i], INPUT_PULLUP);
		keys[i].setDebounceDelay(8);
	}

	// Switches initialisation
	for(uint8_t i = 0; i < NUM_SWITCHES; ++i){
		switches[i].begin(PIN[i], INPUT_PULLUP);
		switches[i].setDebounceDelay(5);
	}

	// potentiometers initialisation
	for (uint8_t i = 0; i < NUM_POTS; ++i){
		pots[i].begin(analogRead(APIN[i]));
		pots[i].setCoef(POT_FILTER_COEF);
	}
/*
	// Run init sequence to debounce switches and filter pots, so it's running on stable values.
	uint32_t initEnd = millis() + 500;

	while(initEnd > millis()){

		for(uint8_t i = 0; i < NUM_KEYS; ++i){
			keys[i].update();
		}

		for(uint8_t i = 0; i < NUM_SWITCHES; ++i){
			switches[i].update();
		}

// This causes the most inexplicable bug I've ever seen.
// When the switches (above) are depressed in the main loop, the board reboots.
// Probably something like a segmentation error, I suspect the board is partially HS.
// For now it will stay the same...
/*
		uint16_t value = 0;
		for(uint8_t i = 0; i < NUM_POTS; ++i){
			uint16_t temp = 0;
			value = analogRead(APIN[i]);
			temp = pots[i].filter(value);
			potState[i] = temp;
		}

	}
*/
	midi1.setHandleControlChange(handleControlChange);
	midi1.begin(1);
	midi1.turnThruOff();
}

void loop(){

	midi1.read();
	updateKeys();
	updateSwitches();
	updateControls();
	update = 0;
/*
	Serial.println("keys");
	for(uint8_t i = 22; i < (22 + NUM_KEYS); ++i){
		Serial.print(digitalRead(i));
		Serial.print('\t');
	}
	Serial.println();
	Serial.println("pots");
	for(uint8_t i = 0; i < NUM_POTS; ++i){
		Serial.print(analogRead(APIN[i]));
		Serial.print('\t');
	}
	Serial.println();
	Serial.println("switches");
	for(uint8_t i = 0; i < NUM_SWITCHES; ++i){
		Serial.print(digitalRead(PIN[i]));
		Serial.print('\t');
	}
	Serial.println("\n\n");
	delay(50);
*/
}

// This is equivalent to the map() function provided by Arduino.
// For some reason it didn't work well, so I wrote this one, that also bounds the value.
int32_t remap(int32_t value, int32_t lowerIn, int32_t upperIn, int32_t lowerOut, int32_t upperOut){
	int32_t inRange = upperIn - lowerIn;
	int32_t outRange = upperOut - lowerOut;
	float ratio = (float)outRange / (float)inRange;
	value -= lowerIn;
	value *= ratio;
	value += lowerOut;
	if(value < lowerOut) value = lowerOut;
	if(value > upperOut) value = upperOut;
	return value;
}

// Send a 14-bits control change.
// Control change from 0 to 31 are 14-bits long control change,
// each one associated with a LSB CC command ranging from 32 to 63.
void sendLongControlChange(uint8_t controlChange, uint16_t value, uint8_t channel = 1){
	uint8_t valueHigh = value >> 7;
	uint8_t valueLow = value & 0x7F;
	midi1.sendControlChange(controlChange, valueHigh, channel);
	midi1.sendControlChange(controlChange + 32, valueLow, channel);
}

// Handle switch + potentiometer combo for mixer.
/*
 * Update is sent when :
 * 		Switch is turned OFF : volume 0 is sent.
 * 		Switch is turned ON : last potentiometer value is sent.
 * 		Potentiomter is moved AND switch is on : current volume is sent.
 * When potentiometer is moved but switch is OFF, current value is localy stored but not sent.
 */
void updateMix(uint8_t ch, bool fromSw = 0){
	uint16_t value = 0;
	if(mixSw[ch]){
		value = mix[ch];
	}
	if(value || fromSw) sendLongControlChange(CC_OSC1_MIX + ch, value, 1);

}

void updateKeys(){
	// reading keys
	for(uint8_t i = 0; i < NUM_KEYS; ++i){
		keys[i].update();
//		uint8_t key = i + MIDI_OFFSET;
		if(keys[i].justPressed()){
			midi1.sendNoteOn(i, defaultVelocity, 1);
		} else if(keys[i].justReleased()){
			midi1.sendNoteOff(i, 0, 1);
		} else {
			// do nothing
		}

	}

}

void updateSwitches(){
	for(uint8_t i = 0; i < NUM_SWITCHES; ++i){
		uint8_t change = 0;
		switches[i].update();

		if(switches[i].justPressed()){
			change = 127;
		} else if(switches[i].justReleased()){
			change = 0;
		} else if(update){
			change = (uint8_t)switches[i].isPressed();
			change *= 127;
		} else {
			// If no change, skip midi update.
			continue;
		}

		int8_t controlChange = -1;

		switch(i){
			case 0:
			// pin 2
				controlChange = CC_MOD_MIX_1;
				break;
			case 1:
			// pin 3
				controlChange = CC_MOD_MIX_2;
				break;
			case 2:
			// pin 4
				controlChange = CC_OSC_MOD;
				break;
			case 3:
			// pin 5
				controlChange = CC_PORTAMENTO_ON_OFF;
				break;
			case 4:
			// pin 6
				controlChange = CC_LFO_SHAPE;
				break;
			case 5:
			// pin 7
				controlChange = CC_OSC3_CTRL;
				break;
			case 6:
			// pin 8
			// Mixer switches are handled by a dedicate function with pots.
				mixSw[0] = (bool)change;
				updateMix(0, 1);
				continue;
			case 7:
			// pin 9
				mixSw[1] = (bool)change;
				updateMix(1, 1);
				continue;
			case 8:
			// pin 10
				mixSw[2] = (bool)change;
				updateMix(2, 1);
				continue;
			case 9:
			// pin 11
				mixSw[3] = (bool)change;
				updateMix(3, 1);
				continue;
			case 10:
			// pin 12
				mixSw[4] = (bool)change;
				updateMix(4, 1);
				continue;
			case 11:
			// pin 13
				controlChange = CC_NOISE_COLOR;
				break;
			case 12:
			// pin 14
				controlChange = CC_FUNCTION;
				break;
			case 13:
			// pin 15
				controlChange = CC_TRANSPOSE;
				if(change == 0) continue;
				change = 127;
				break;
			case 14:
			// pin 16
				controlChange = CC_TRANSPOSE;
				if(change == 127) continue;
				change = 0;
				break;
			default:
				continue;			
		}

		midi1.sendControlChange(controlChange, change, 1);
	}
}

void updateControls(){
	for(uint8_t i = 0; i < NUM_POTS; ++i){
		uint16_t value = 0;

		value = pots[i].filter(analogRead(APIN[i]));
		if((value != potState[i]) || update){
			potState[i] = value;
		} else {
			// If not change, skip midi update
			continue;				
		}

		int8_t controlChange = -1;

		switch(i){
			case 0:
				controlChange = CC_LFO_RATE;
				break;
			case 1:
				controlChange = CC_MODULATION_MIX;
				break;
			case 2:
				controlChange = CC_PORTAMENTO_TIME;
				break;
			case 3:
//				int16_t val = remap(value, 360, 660, -8190, 8190);
				midi1.sendPitchBend((int16_t)value, 1);
				continue;
			case 4:
				// Mod wheel uses a standard 270° potentiometer, but its course is around 90°.
				// The input value is then remaped to the standard MIDI 14-bits range.
				controlChange = CC_MOD_WHEEL;
				value = remap(value, 360, 660, 0, 16384);
//				value = map(value, 360, 660, 0, 16384);
//				value = constrain(value, 0, 16384);				
				break;
			case 5:
				// rotary selector : value must be divided by ~170
				// Selectors use resistor array, as explained at the begining of this file. 
				// the divisions "re-maps" the 10-bits range of the ADC to the 0-6 range we need.
				controlChange = CC_OSC1_RANGE;
				value /= 170;
				value = 5 - value;
				// We have to check if the value after dividing is different from the previous one !
				// Otherwise each selector change will send dozens of useless update !
				if(value == selectors[0]){
					continue;
				} else {
					selectors[0] = value;
				}
				break;
			case 6:
				controlChange = CC_OSC2_RANGE;
				value /= 170;
				value = 5 - value;
				if(value == selectors[1]){
					continue;
				} else {
					selectors[1] = value;
				}
				break;
			case 7:
				controlChange = CC_OSC3_RANGE;
				value /= 170;
				value = 5 - value;
				if(value == selectors[2]){
					continue;
				} else {
					selectors[2] = value;
				}
				break;
			case 8:
				controlChange = CC_OSC1_WAVEFORM;
				value /= 170;
				if(value == selectors[3]){
					continue;
				} else {
					selectors[3] = value;
				}
				break;
			case 9:
				controlChange = CC_OSC2_WAVEFORM;
				value /= 170;
				if(value == selectors[4]){
					continue;
				} else {
					selectors[4] = value;
				}
				break;
			case 10:
				controlChange = CC_OSC3_WAVEFORM;
				value /= 170;
				if(value == selectors[5]){
					continue;
				} else {
					selectors[5] = value;
				}
				break;
			case 11:
				// mix is to be sent only if switch is on, and is handled by a dedicated function (see above).
//				controlChange = CC_OSC1_MIX;
				mix[0] = value;
				updateMix(0);
				continue;
			case 12:
//				controlChange = CC_OSC2_MIX;
				mix[1] = value;
				updateMix(1);
				continue;
			case 13:
//				controlChange = CC_OSC3_MIX;
				mix[2] = value;
				updateMix(2);
				continue;
			case 14:
//				controlChange = CC_NOISE_MIX;
				mix[3] = value;
				updateMix(3);
				continue;
			case 15:
//				controlChange = CC_FEEDBACK_MIX;
				mix[4] = value;
				updateMix(4);
				continue;
			default:
				continue;
		}

		// See comment about 14-bits control change at the sendLongControlChange() function.
		if( controlChange < 32){
			sendLongControlChange(controlChange, value, 1);
		} else {
			midi1.sendControlChange(controlChange, value, 1);
		}
	}
}

// Handle requests from Teensy. For now, the only one needed is a global update.
// Maybe it could be usefull to use the data byte to specify which kind of controls are to be updated.
void handleControlChange(uint8_t channel, uint8_t command, uint8_t value){
	switch(command){
		case CC_ASK_FOR_DATA:
			update = 1;
			break;
	}
}