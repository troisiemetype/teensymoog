// Minimoog - mega 2
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
 * This program is for the Arduino Mega 2, it handles the readings of :
 * Category			input 				type		pin				MIDI
 * 
 * Oscillators
 *					global tune			pot 		A0				CC   9
 *					osc 2 tune			pot 		A1 				CC  14
 *					osc 3 tune			pot 		A2 				CC  15
 * Filter
 *					filter pass			pot 		A3 				CC  19
 *					cutoff frequency	pot 		A4 				CC  20
 *					emphasis 			pot 		A5 				CC  21
 *					contour				pot 		A6 				CC  22
 *					attack				pot 		A7 				CC  23
 *					decay 				pot 		A8 				CC  24
 *					sustain 			pot 		A9 				CC  25
 *					release 			pot 		A10				CC  26
 *					filter modulation 	switch 		2				CC 109
 *					keyboard control 1 	switch 		3				CC 110
 *					keyboard control 2  switch 		4				CC 111
 * Enveloppe
 *					attack				pot 		A11 			CC  27
 *					decay 				pot 		A12				CC  28
 *					sustain 			pot 		A13 			CC  29
 *					release				pot 		A14				CC  30
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
const uint8_t NUM_SWITCHES = 3;
const uint8_t NUM_POTS = 15;
const uint8_t POT_FILTER_COEF = 10;

// Note : pins are defined via tables, to improve code efficiency.
// Digital pin definition

const uint8_t PIN_FILTER_MOD = 2;
/*
const uint8_t PIN_KEYBOARD_CTRL_1 = 3;
const uint8_t PIN_KEYBOARD_CTRL_2 = 4;
*/
// Analog pin definition
/*
const uint8_t APIN_GLOBAL_TUNE = A0;
const uint8_t APIN_OSC2_TUNE = A1;
const uint8_t APIN_OSC3_TUNE = A2;
const uint8_t APIN_FILTER_BAND = A3;
const uint8_t APIN_FILTER_CUTOFF = A4;
const uint8_t APIN_FILTER_EMPHASIS = A5;
const uint8_t APIN_FILTER_CONTOUR = A6;
const uint8_t APIN_FILTER_ATTACK = A7;
const uint8_t APIN_FILTER_DECAY = A8;
const uint8_t APIN_FILTER_SUSTAIN = A9;
const uint8_t APIN_FILTER_RELEASE = A10;
const uint8_t APIN_ATTACK = A11;
const uint8_t APIN_DECAY = A12;
const uint8_t APIN_SUSTAIN = A13;
const uint8_t APIN_RELEASE = A14;
*/

const uint8_t APIN[NUM_POTS] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14};

// Variables
uint16_t potState[NUM_POTS];

PushButton switches[NUM_SWITCHES];
ExpFilter pots[NUM_POTS];

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
	for(uint8_t i = 0; i < NUM_SWITCHES; ++i){
		switches[i].begin(i + PIN_FILTER_MOD, INPUT_PULLUP);
		switches[i].setDebounceDelay(1);
	}

	for (uint8_t i = 0; i < NUM_POTS; ++i){
		pots[i].begin(analogRead(APIN[i]));
		pots[i].setCoef(POT_FILTER_COEF);
	}

	midi1.setHandleControlChange(handleControlChange);
	midi1.begin(1);
	midi1.turnThruOff();

}

void loop(){
	midi1.read();
	updateControls();
	updateSwitches();
	update = 0;
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
				controlChange = CC_OSC_TUNE;
				break;
			case 1:
				controlChange = CC_OSC2_TUNE;
				break;
			case 2:
			// This one has a problem : sends CC10 (instead of CC13) then CC45 as should be.
			// or maybe pure data has a bug that shifts bits.
			// It seems it's a bug of pure data : 13 are replaced by 10 also for values...
				controlChange = CC_OSC3_TUNE;
				break;
			case 3:
				controlChange = CC_FILTER_BAND;
				break;
			case 4:
				controlChange = CC_FILTER_CUTOFF_FREQ;
				break;
			case 5:
				controlChange = CC_FILTER_EMPHASIS;
				break;
			case 6:
				controlChange = CC_FILTER_CONTOUR;
				break;
			case 7:
				controlChange = CC_FILTER_ATTACK;
				break;
			case 8:
				controlChange = CC_FILTER_DECAY;
				break;
			case 9:
				controlChange = CC_FILTER_SUSTAIN;
				break;
			case 10:
				controlChange = CC_FILTER_RELEASE;
				break;
			case 11:
				controlChange = CC_EG_ATTACK;
				break;
			case 12:
				controlChange = CC_EG_DECAY;
				break;
			case 13:
				controlChange = CC_EG_SUSTAIN;
				break;
			case 14:
				controlChange = CC_EG_RELEASE;
				break;
			default:
				continue;

		}

		uint8_t valueHigh = value >> 7;
		uint8_t valueLow = value & 0x7F;
		midi1.sendControlChange(controlChange, valueHigh, 1);
		midi1.sendControlChange(controlChange + 32, valueLow, 1);
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
			// If no change, skip the midi command sending.
			continue;
		}

		int8_t controlChange = -1;

		switch(i){
			case 0:
				controlChange = CC_FILTER_MOD;
				break;
			case 1:
				controlChange = CC_FILTER_KEYTRACK_1;
				break;
			case 2:
				controlChange = CC_FILTER_KEYTRACK_2;
				break;
			default:
				continue;			
		}

		midi1.sendControlChange(controlChange, change, 1);
	}
}


void handleControlChange(uint8_t channel, uint8_t command, uint8_t value){
	switch(command){
		case CC_ASK_FOR_DATA:
			update = 1;
			break;
	}
}