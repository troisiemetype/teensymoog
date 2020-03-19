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

// includes
#include "MIDI.h"
#include "PushButton.h"
#include "ExpFilter.h"
#include "defs.h"

// Constants
const uint8_t NUM_KEYS = 30;
// const uint8_t MIDI_OFFSET = 48; // todo : move to teensy !

const uint8_t NUM_SWITCHES = 15;
const uint8_t NUM_POTS = 16;
const uint8_t NUM_SELECTORS = 6;

const uint8_t POT_FILTER_COEF = 15;

// Digital pin definition
const uint8_t KEYS[NUM_KEYS] = {
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51
};

PushButton keys[NUM_KEYS];
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

const uint8_t APIN[NUM_POTS] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};

// Vars
uint16_t potState[NUM_POTS];

const uint8_t PIN[NUM_SWITCHES] = {3, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 17, 14, 15, 16};

PushButton switches[NUM_SWITCHES];
ExpFilter pots[NUM_POTS];
uint8_t selectors[NUM_SELECTORS];

// Keyboard
bool keyState[NUM_KEYS];

// Mixer
uint16_t mix[5];
bool mixSw[5];

// Misc
uint8_t defaultVelocity = 96;

bool update = 0;

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

	midi1.setHandleControlChange(handleControlChange);
	midi1.begin(1);
	midi1.turnThruOff();


	// Key initialisation
	for(uint8_t i = 0; i < NUM_KEYS; ++i){
		keys[i].begin(KEYS[i], INPUT_PULLUP);
		keys[i].setDebounceDelay(8);
	}

	// Switches initialisation
	for(uint8_t i = 0; i < NUM_SWITCHES; ++i){
		switches[i].begin(PIN[i], INPUT_PULLUP);
		switches[i].setDebounceDelay(1);
	}

	// potentiometers initialisation
	for (uint8_t i = 0; i < NUM_POTS; ++i){
		pots[i].begin(analogRead(APIN[i]));
		pots[i].setCoef(POT_FILTER_COEF);
	}

}

void loop(){
	midi1.read();
	updateKeys();
	updateSwitches();
	updateControls();
	update = 0;
}

void sendLongControlChange(uint8_t controlChange, uint16_t value, uint8_t channel = 1){
	uint8_t valueHigh = value >> 7;
	uint8_t valueLow = value & 0x7F;
	midi1.sendControlChange(controlChange, valueHigh, channel);
	midi1.sendControlChange(controlChange + 32, valueLow, channel);
}

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
/*
		bool newState = digitalRead(KEYS[i]);
		if(newState != keyState[i]){
			uint8_t key = i + MIDI_OFFSET;
			keyState[i] = newState;
			if(newState){
				// MIDI note on
				midi1.sendNoteOn(key, defaultVelocity, 1);

			} else {
				// MIDI note off
				midi1.sendNoteOff(key, 0, 1);
			}
		}
*/
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
				midi1.sendPitchBend((int16_t)value - 512, 1);
				continue;
			case 4:
				controlChange = CC_MOD_WHEEL;
				break;
			case 5:
				// rotary selector : value must be divided by ~170
				controlChange = CC_OSC1_RANGE;
				value /= 170;
				value = 5 - value;
				// We have to check if the value after dividing is different from the previous one !
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
				// mix is to be sent only if switch is on.
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

		if( controlChange < 32){
			sendLongControlChange(controlChange, value, 1);
		} else {
			midi1.sendControlChange(controlChange, value, 1);
		}
	}
}

void handleControlChange(uint8_t channel, uint8_t command, uint8_t value){
	switch(command){
		case CC_ASK_FOR_DATA:
			update = 1;
			break;
	}
}