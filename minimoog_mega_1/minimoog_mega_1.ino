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
 *					modulation mix 1	switch 		2				CC 117
 *					modulation mix 2 	switch 		3				CC 118
 *					osc modulation 		switch 		4 				CC 115
 *					glide				switch 		5				CC 65
 *					LFO waveform		switch 		6				CC 119
 * Oscillators
 *					osc 1 range			switch 		A5 				CC 102
 *					osc 1 waveform		switch 		A6 				CC 103
 *					osc 2 range			switch 		A7				CC 104
 *					osc 2 waveform		switch 		A8 				CC 105
 *					osc 3 range			switch 		A9 				CC 106
 *					osc 3 waveform		switch 		A10 			CC 107
 *					osc 3 control		switch 		7 				CC 108
 * Mixer
 *					osc 1 mix 			pot			A11				CC 85
 *					osc 2 mix 			pot			A12				CC 86
 *					osc 3 mix 			pot			A13				CC 87
 *					noise mix 			pot			A14				CC 88
 *					external mix 		pot			A15				CC 89
 *					osc 1				switch 		8 				Directly added to mix
 *					osc 2				switch 		9				ditto
 *					osc 3				switch 		10				ditto
 *					noise				switch 		11				ditto
 *					external			switch 		12				ditto
 *					noise color			switch 		13				CC 114
 * Other
 *					function			switch 		14 				CC 113
 *					transpose +			switch 		52 				CC 112
 *					transpose -			switch 		53				CC 112
 * Communication
 *					TX1 to mega 2					18
 *					RX1 from mega 2					19
 */

// includes
#include "MIDI.h"
#include "PushButton.h"

// Constants
const uint8_t NUM_KEYS = 30;
const uint8_t MIDI_OFFSET = 23; // to be modified
// Digital pin definition

const uint8_t KEYS[NUM_KEYS] = {
	22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
	34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51
};

PushButton keys[NUM_KEYS];

const uint8_t PIN_MOD_MIX_1 = 2;
const uint8_t PIN_MOD_MIX_2 = 3;
const uint8_t PIN_OSC_MOD = 4;
const uint8_t PIN_GLIDE = 5;
const uint8_t PIN_DECAY = 6;
const uint8_t PIN_LFO_WAVEFORM = 7;

const uint8_t PIN_MIX_OSC_1 = 9;
const uint8_t PIN_MIX_OSC_2 = 10;
const uint8_t PIN_MIX_OSC_3 = 11;
const uint8_t PIN_MIX_OSC_NOISE = 12;
const uint8_t PIN_MIX_OSC_EXT = 13;
const uint8_t PIN_MIX_OSC_NOISE_COLOR = 14;

const uint8_t PIN_TRANSPOSE_PLUS = 52;
const uint8_t PIN_TRANSPOSE_MINUS = 53;

// Analog pin definition
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


// Vars
// Keyboard
bool keyState[NUM_KEYS] = {};

// Modulation
uint16_t lfoRate = 0;
uint16_t modMix = 0;
uint16_t glide = 0;
uint16_t pitchWheel = 0;
uint16_t modWheel = 0;
bool modMix1 = 0;
bool modMix2 = 0;
bool oscMod = 0;
bool glideEn = 0;
bool decayEn = 0;
bool lfoWaveform = 0;

// Oscillators
uint8_t osc1Range = 0;
uint8_t osc1Waveform = 0;
uint8_t osc2Range = 0;
uint8_t osc2Waveform = 0;
uint8_t osc3Range = 0;
uint8_t osc3Waveform = 0;
bool osc3Control = 0;

// Mixer
uint16_t osc1Mix = 0;
uint16_t osc2Mix = 0;
uint16_t osc3Mix = 0;
uint16_t feedbackMix = 0;
uint16_t noiseMix = 0;
bool osc1MixSw = 0;
bool osc2MixSw = 0;
bool osc3MixSw = 0;
bool feedbackMixSw = 0;
bool noiseMixSw = 0;

// Miscellaneous
bool function = 0;

uint8_t defaultVelocity = 96;

bool update = 0;

struct midiSettings : public midi::DefaultSettings{
//	static const bool UseRunningStatus = true;
	static const long BaudRate = 115200;
};

// The one we use on synth
//MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, midi1, midiSettings);
// For debug purposes
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, midi1, midiSettings);

void setup(){
	// initialisation
	// Serial is for debug
	//Serial.begin(115200);

	midi1.setHandleControlChange(handleControlChange);
	midi1.begin(1);
	// Serial 1 is for sending and receiving messages to and from Teensy
//	Serial1.begin(115200);
	// Serial 2 is for sending and receiving messages to and from Mega 2
//	Serial2.begin(115200);
	// keys
	/*
	for(uint8_t i = 0; i < NUM_KEYS; ++i){
		keyState[i] = 0;
		pinMode(KEYS[i], INPUT_PULLUP);
	}
	*/

	for(uint8_t i = 0; i < NUM_KEYS; ++i){
		keys[i].begin(KEYS[i], INPUT_PULLUP);
		keys[i].setDebounceDelay(1);
	}



}

void loop(){
	midi1.read();
	updateKeys();
	updateControls();
	updateSwitches();
	update = 0;
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
		uint8_t key = i + MIDI_OFFSET;
		if(keys[i].justPressed()){
			midi1.sendNoteOn(key, defaultVelocity, 1);
		} else if(keys[i].justReleased()){
			midi1.sendNoteOff(key, 0, 1);
		} else {
			// do nothing
		}

	}

}

void updateControls(){


}

void updateSwitches(){
	
}

void handleControlChange(uint8_t channel, uint8_t command, uint8_t value){
	switch(command){
		case CC_ASK_FOR_DATA:
			update = 1;
			break;
	}
}