// Minimoog - Teensy
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
  * Pinout
  *
  * RX from mega 1 (through tension divider) 		0
  * TX to mega 1 (serial 1)							1
  * mega 1 reset 									2
  * 	
  * RX from mega 2 (through tension divider) 		16
  * TX to mega 2 (serial 4)							17
  * mega 2 reset 									18
  * 	
  * I2S OUT1A 										7
  * I2S LRCLK1										20
  * I2S BCLK1 										21
  *
  * There are provision on the rear panel for sustain and expression control, not implemented yet.
  */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <EEPROM.h>

#include "audio_setup.h"
#include "defs.h"

#include "MIDI.h"					// https://github.com/troisiemetype/PushButton
// #include "Timer.h"

// constants

// my pots never go full clockwise... :/ So this can be used to adapt their range.
// These two commented out values for testing with external midi triggering (like puredata).
//const uint16_t RESO = 127;
//const uint16_t RESO = 16383;
const uint16_t RESO = 1005;
const uint16_t HALF_RESO = RESO / 2;

// There is a function that handles and tracks the key presses. Here is the max.
// It can me more, but whith ten fingers on a monophonic synth, I think this is enough !
const uint8_t KEYTRACK_MAX = 10;

// Mega1 sends midi note 0 for the lower note ; we offset it by for octave to get into the usefull range
const uint8_t MIDI_OFFSET = 48;
// To be modified according to keybed used. It's actualy not used, and any note can be handled from MIDI in.
const uint8_t NUM_KEYS = 30;

// Octave range for oscillators and filter.
const uint8_t MAX_OCTAVE = 10;
const uint8_t FILTER_MAX_OCTAVE = 5;

// The base for computing DC levels for keys, modulation, pitchbend, etc.
const float NOTE_MIDI_0 = 8.1757989156434;
const float NOTE_RATIO = 1.0594630943593;
const float HALFTONE_TO_DC = (float)1 / (MAX_OCTAVE * 12);
const float FILTER_HALFTONE_TO_DC = (float)1 / (FILTER_MAX_OCTAVE * 12);

// Filter base frequency : the filter cutoff frequency varies around this value.
const float FILTER_BASE_FREQUENCY = 440.0;
const float FILTER_BASE_NOTE = (log(FILTER_BASE_FREQUENCY / NOTE_MIDI_0)) / (log(NOTE_RATIO));
// This corresponds to the min and max values for the variable filter available in the Teensy Audio library,
// which is used by the synth.
/* Note about the state variable (Chamberlin) filter:
 * 	This filter can self oscillate, if given a resonnance value of around 50000.
 *	(and the library modified to accept such value ; the setter limits it to 5).
 *	It gives a nice sounding, pure sinewave.
 * 	Problem is, usefull range for varying resonnace is about what the bare library allows (from 0.7 to 5).
 *	It would be nice to test some exponential course to enable both continuous resonance modification AND self-oscillation.
 *	I've tried some modifications, but couldn't come to something usable.
 */
const float FILTER_MIN_Q = 0.7;
const float FILTER_MAX_Q = 5;
const float FILTER_DIV_Q = RESO / (FILTER_MAX_Q - FILTER_MIN_Q);

// Max mixer value for eahc channel if we want to avoid clipping.
// Value of 1 can be used, but induces distortion when more than one oscillator is used.
// Feedback should have its own value. If MAX_MIX is one feedback is powerfull,
// but with this value the difference with and without feedback is barrely noticeable.
const float MAX_MIX = 0.32;

// To be put in Mega1 sketch, so it sends a value on 14 bits.
// Note : the pitchbend wheel poses problems on the other sketch. For now it will stay like that,
// but there is room for improvement.
//343 - 492 - 500 - 651
const int16_t PITCH_BEND_MIN = 343;
const int16_t PITCH_BEND_MAX = 651;
const int16_t PITCH_BEND_NEUTRAL = PITCH_BEND_MIN + (PITCH_BEND_MAX - PITCH_BEND_MIN) / 2;
const int16_t PITCH_BEND_COURSE = PITCH_BEND_MAX - PITCH_BEND_MIN;
const float PITCH_BEND_INTERNAL_TO_MIDI = 8192.0 / PITCH_BEND_COURSE;

// Maximum attack, decay, release and glide time (in milliseconds)
// These use an exponential course to have both fine-tune of low values and a great range.
const float MAX_ATTACK_TIME = 10000;
const float MAX_DECAY_TIME = 10000;
const float MAX_RELEASE_TIME = 10000;
const float MAX_GLIDE_TIME = 10000;
/*
// Moved to Mega 1
const uint16_t MOD_WHEEL_MIN = 360;
const uint16_t MOD_WHEEL_MAX = 666;
const uint16_t MOD_WHEEL_NEUTRAL = MOD_WHEEL_MIN + (MOD_WHEEL_MAX - MOD_WHEEL_MIN) / 2;
const uint16_t MOD_WHEEL_COURSE = MOD_WHEEL_MAX - MOD_WHEEL_MIN;
*/

// Teensy can reset both Mega, if needed.
const uint8_t MEGA1_RST = 2;
const uint8_t MEGA2_RST = 18;

// Addresses for settings storing in memory. It uses Arduino's EEPROM functions but is saved to RAM on Teensy 4.0.
const uint16_t EE_BITCRUSH_ADD = 0;
const uint16_t EE_KEYBOARD_MODE_ADD = 1;
const uint16_t EE_MIDI_IN_CH_ADD = 2;
const uint16_t EE_MIDI_OUT_CH_ADD = 3;
const uint16_t EE_TRIGGER_ADD = 4;
const uint16_t EE_DETUNE_ADD = 5;
const uint16_t EE_FILTER_MODE = 6;
const uint16_t EE_PITCH_BEND_RANGE = 7;
const uint16_t EE_MOD_WHEEL_OSC_RANGE = 8;
const uint16_t EE_MOD_WHEEL_FILTER_RANGE = 9;
const uint16_t EE_DETUNE_TABLE_ADD = 20;

// variables
// Note : 
uint8_t internalMidiChannel = 1;
uint8_t midiInChannel = 1;
uint8_t midiOutChannel = 1;

float glide = 0;
bool glideEn  = 0;

bool noteRetrigger = 1;

bool filterKeyTrack1 = 0;
bool filterKeyTrack2 = 0;

int8_t transpose = 0;
bool function = 0;

bool oscMod = 0;
bool decay = 0;

// note : not used. It was intended to copy decay to release as on the orignal minimoog,
// but a release pot is present here.
// Anyway, the code is there, commented out, and there is a CC for it.
float filterDecay = 0;
float egDecay = 0;

// Stores the current band value, for recall when the band mode is changed.
int16_t filterBandValue = 0;

uint8_t pitchBendRange = 3;
uint8_t modWheelOscRange = 3;
uint8_t modWheelFilterRange = 12;

// Waveforms
uint8_t waveforms[6] = {WAVEFORM_SINE, WAVEFORM_TRIANGLE, WAVEFORM_SAWTOOTH,
						WAVEFORM_SAWTOOTH_REVERSE, WAVEFORM_SQUARE, WAVEFORM_PULSE};
// detune table. For emulating resistor-ladder keybed and induce key detuning.
float detuneTable[128];
// keyTrack
/*
 * Note on keytrack : there are two "keytracks" on the synth.
 *	One is the filter keytrack, that reflects the note being played on the filter's cutoff frequency.
 *	The other (this one) is a system tracks key being pressed to send not according to note priority setting.
 * A better name for one or the other should be found...
 */
uint8_t keyTrackIndex = 0;
struct {
	uint8_t key;
	uint8_t velocity;
} keyTrack[KEYTRACK_MAX];

int8_t nowPlaying = -1;

// double CC track
uint8_t ccTempValue[32];

enum function_t{
	FUNCTION_KEYBOARD_MODE = 0,
	FUNCTION_RETRIGGER,
	FUNCTION_DETUNE,
	FUNCTION_BITCRUSH,
	FUNCTION_FILTER_MODE,
	FUNCTION_MIDI_IN_CHANNEL,
	FUNCTION_MIDI_OUT_CHANNEL,
	FUNCTION_PITCH_BEND_RANGE,
	FUNCTION_MOD_WHEEL_OSC_RANGE,
	FUNCTION_MOD_WHEEL_FILTER_RANGE,
};

function_t currentFunction = FUNCTION_KEYBOARD_MODE;

enum keyMode_t{
	KEY_LOWER = 0,
	KEY_FIRST,
	KEY_LAST,
	KEY_UPPER,
};

keyMode_t keyMode = KEY_LAST;

enum detune_t{
	DETUNE_OFF = 0,
	DETUNE_SOFT,
	DETUNE_MEDIUM,
	DETUNE_HARD,
	DETUNE_RESET,
};

detune_t detune = DETUNE_OFF;
float detuneCoeff[4] = {0, 0.1, 0.3, 0.5};

enum filterMode_t{
	FILTER_BAND_PASS = 0,
	FILTER_BAND_STOP,
};

filterMode_t filterMode = FILTER_BAND_PASS;

uint8_t bitCrushLevel = 16;

struct midiSettings : public midi::DefaultSettings{
//	static const bool UseRunningStatus = true;
	static const long BaudRate = 115200;
};

// USB midi for sending and receiving to and from other device or computer.
MIDI_CREATE_DEFAULT_INSTANCE();
// The ones we use on synth for internal communication between Mega and Teensy
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, midi1, midiSettings);
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial4, midi2, midiSettings);

// for debug purpose, to send to serial the CPU used by audio library.
// Timer timerCPU;
// Timer timerGraph;

void setup() {
	pinMode(13, OUTPUT);
	digitalWrite(13, 1);

	// Mega resets
	pinMode(MEGA1_RST, OUTPUT);
	pinMode(MEGA2_RST, OUTPUT);
	digitalWrite(MEGA1_RST, 1);
	digitalWrite(MEGA2_RST, 1);

	// midi settings, start and callback
	midi1.begin(1);
	midi1.turnThruOff();
	midi1.setHandleNoteOn(handleInternalNoteOn);
	midi1.setHandleNoteOff(handleInternalNoteOff);
	midi1.setHandlePitchBend(handleInternalPitchBend);
	midi1.setHandleControlChange(handleControlChange);

	midi2.begin(1);
	midi2.turnThruOff();
	midi2.setHandleControlChange(handleControlChange);
/*
	Serial.begin(115200);
	Serial.println("started...");
*/
	// Getting the settings from "eeprom"
	EEPROM.get(EE_BITCRUSH_ADD, bitCrushLevel);
	EEPROM.get(EE_KEYBOARD_MODE_ADD, keyMode);
	EEPROM.get(EE_MIDI_IN_CH_ADD, midiInChannel);
	EEPROM.get(EE_MIDI_OUT_CH_ADD, midiOutChannel);
	EEPROM.get(EE_TRIGGER_ADD, noteRetrigger);
	EEPROM.get(EE_DETUNE_ADD, detune);
	EEPROM.get(EE_FILTER_MODE, filterMode);
	EEPROM.get(EE_MOD_WHEEL_OSC_RANGE, modWheelOscRange);
	EEPROM.get(EE_MOD_WHEEL_FILTER_RANGE, modWheelFilterRange);

	uint16_t address = EE_DETUNE_TABLE_ADD;
	for(uint16_t i = 0; i < 128; ++i){
		EEPROM.get(address, detuneTable[i]);
		address += 4;
	}
	// TODO : check how to receive and transmit on different channels.
	usbMIDI.setHandleNoteOn(handleNoteOn);
	usbMIDI.setHandleNoteOff(handleNoteOff);
	usbMIDI.setHandlePitchChange(handlePitchBend);
//	usbMIDI.setHandleNoteOn(handleInternalNoteOn);
//	usbMIDI.setHandleNoteOff(handleInternalNoteOff);
//	usbMIDI.setHandlePitchBend(handleInternalPitchBend);
//	usbMIDI.setHandleControlChange(handleControlChange);
	usbMIDI.begin();

	AudioMemory(200);

	// audio settings
	// dc
	dcKeyTrack.amplitude(0.0);
	dcPitchBend.amplitude(0.0);
	dcFilterEnvelope.amplitude(1.0);
	dcFilter.amplitude(0.0);
	dcFilterKeyTrack.amplitude(0.0);
	dcOsc3.amplitude(0.2);
	dcLfoFreq.amplitude(0.0);
	dcOscTune.amplitude(0.0);
	dcOsc2Tune.amplitude(0.0);
	dcOsc3Tune.amplitude(0.0);
	dcPulse.amplitude(-0.95);

	// amp
	ampPitchBend.gain(pitchBendRange * HALFTONE_TO_DC * 2);
	ampModWheelOsc.gain(0.0);
	ampModWheelFilter.gain(0.0);
	ampPreFilter.gain(1.0);
	ampModEg.gain(0.1);
	ampOsc3Mod.gain(1);
	masterVolume.gain(1.0);

	osc1Waveform.frequencyModulation(MAX_OCTAVE);
	osc2Waveform.frequencyModulation(MAX_OCTAVE);
	osc3Waveform.frequencyModulation(MAX_OCTAVE);
	osc1Waveform.begin(1, NOTE_MIDI_0, WAVEFORM_TRIANGLE);
	osc2Waveform.begin(1, NOTE_MIDI_0, WAVEFORM_SAWTOOTH);
	osc3Waveform.begin(1, NOTE_MIDI_0, WAVEFORM_SQUARE);

	// noise
	whiteNoise.amplitude(1);
	pinkNoise.amplitude(1);

	// LFO
	lfoWaveform.begin(1, 0.1, WAVEFORM_TRIANGLE);
	lfoWaveform.frequencyModulation(11);

	// mixers
	mainTuneMixer.gain(0, 1);
	mainTuneMixer.gain(1, 1);
	mainTuneMixer.gain(2, 1);
	mainTuneMixer.gain(3, 1);
	osc2TuneMixer.gain(0, 1);
	osc2TuneMixer.gain(1, 1);
	osc3TuneMixer.gain(0, 1);
	osc3TuneMixer.gain(1, 1);

	oscMixer.gain(0, 1);
	oscMixer.gain(1, 0);
	oscMixer.gain(2, 0);
	oscMixer.gain(3, 0);

	globalMixer.gain(0, 1);
	globalMixer.gain(1, 0);
	globalMixer.gain(2, 1);

	noiseMixer.gain(0, 1);
	noiseMixer.gain(1, 0);

	osc3ControlMixer.gain(0, 1);
	osc3ControlMixer.gain(1, 0);

	modMix1.gain(0, 0);
	modMix1.gain(1, 1);
	modMix2.gain(0, 1);
	modMix2.gain(1, 0);
	modMixer.gain(0, 1);
	modMixer.gain(1, 0);

	filterMixer.gain(0, 0);
	filterMixer.gain(1, 0);
	filterMixer.gain(2, 1);
	filterMixer.gain(3, 0);

	bandMixer.gain(0, 1);
	bandMixer.gain(1, 0);
	bandMixer.gain(2, 0);

	// filter
	vcf.frequency(FILTER_BASE_FREQUENCY);
	vcf.resonance(0.7);
	vcf.octaveControl(FILTER_MAX_OCTAVE);

	// envelopes
	mainEnvelope.delay(0);
	mainEnvelope.attack(10);
	mainEnvelope.hold(0);
	mainEnvelope.decay(25);
	mainEnvelope.sustain(0.9);
	mainEnvelope.release(100);

	filterEnvelope.delay(0);
	filterEnvelope.attack(200);
	filterEnvelope.hold(0);
	filterEnvelope.decay(100);
	filterEnvelope.sustain(0.8);
	filterEnvelope.release(50);

	bitCrushOutput.bits(16);
	bitCrushOutput.sampleRate(44100.0);

	delay(500);

	digitalWrite(13, 0);
	delay(200);

// Blink. For debug. And letting a bit more time to Mega 1 to start.
	for(uint8_t i = 0; i < 5; ++i){
		digitalWrite(13, 1);
		delay(100);
		digitalWrite(13, 0);
		delay(50);
	}

//	Serial.println("asking for all controls");
	midi1.sendControlChange(CC_ASK_FOR_DATA, 127, 1);
	midi2.sendControlChange(CC_ASK_FOR_DATA, 127, 1);


/*
	timerCPU.init();
	timerCPU.setDelay(500);
	timerCPU.start(Timer::LOOP);
	Serial.print("max CPU usage");
	Serial.println(AudioProcessorUsageMax());
*/
/*
	timerGraph.init();
	timerGraph.setDelay(2);
	timerGraph.start(Timer::LOOP);
	printPostFilter.length(1);
*/
}

void loop() {
	midi1.read();
	midi2.read();
	usbMIDI.read(midiInChannel);
/*
	if(timerCPU.update()){
		Serial.print("cpu usage :");
		Serial.println(AudioProcessorUsage());
	}
*/
/*
	if(peakPreFilter.available()){
		Serial.print("pre filter  :\t");
		Serial.println(peakPreFilter.read());
	}
*/
/*
	if(peakPostFilter.available()){
		Serial.print("post filter :\t");
		Serial.println(peakPostFilter.read());
	}
*/

//	if(timerGraph.update()) printPostFilter.trigger();
}

// handle note on. compute dc to waveforms, glide enveloppe triggering, etc.
void noteOn(uint8_t note, uint8_t velocity, bool trigger = 1){
/*
	Serial.print("playing :");
	Serial.println(note);
*/
	// Note tracking.
	nowPlaying = note;
	// Applying detune per key.
	float fineTune = detuneTable[note] * detuneCoeff[detune];
//	float duration = 1.0 + (float)glideEn * (float)glide * 3.75;
	float duration = 1.0 + (float)glideEn * glide * MAX_GLIDE_TIME;
	float level = ((float)note + 12 * transpose) * HALFTONE_TO_DC;
	level += fineTune;
	// filter level is for cutoff freqeuncy keytrack. It's computed anyway, but enable through the corresponding mixer.
	float filterLevel = (((float)note - FILTER_BASE_NOTE) + (12 * transpose)) * FILTER_HALFTONE_TO_DC;
	filterLevel += fineTune;

	AudioNoInterrupts();
	dcKeyTrack.amplitude(level, duration);
	dcFilterKeyTrack.amplitude(filterLevel, duration);
	if(trigger){
		filterEnvelope.noteOn();
		mainEnvelope.noteOn();
	}
	AudioInterrupts();
}

// Stop note.
void noteOff(){
	AudioNoInterrupts();
	filterEnvelope.noteOff();
	mainEnvelope.noteOff();
	AudioInterrupts();
}

// Keytrack functions
// This one check if the key is the lower one, or not, and returns the index of the lower one.
int8_t keyTrackGetLower(uint8_t note){
	uint8_t lower = 127;
	int8_t lowerIndex = keyTrackIndex - 1;

	for(uint8_t i = 0; i < keyTrackIndex; ++i){
		if(keyTrack[i].key < lower){
			lower = keyTrack[i].key;
			lowerIndex = i;
		}
	}
/*
	Serial.print("lower note   : ");
	Serial.print(lower);
	Serial.print("\t index : ");
	Serial.println(lowerIndex);
*/
	return lowerIndex;
}

// Ditto upper key.
int8_t keyTrackGetUpper(uint8_t note){
	uint8_t upper = 0;
	int8_t upperIndex = keyTrackIndex - 1;

	for(uint8_t i = 0; i < keyTrackIndex; ++i){
		if(keyTrack[i].key > upper){
			upper = keyTrack[i].key;
			upperIndex = i;
		}
	}
/*
	Serial.print("upper note   : ");
	Serial.print(upper);
	Serial.print("\t index : ");
	Serial.println(upperIndex);
*/
	return upperIndex;
}

// Add a key to the keytrack table.
int8_t keyTrackAdd(uint8_t note, uint8_t velocity){
	// We only keep count of a limited quantity of notes !
	if (keyTrackIndex >= KEYTRACK_MAX) return -1;
/*
	Serial.print("note added   : ");
	Serial.print(note);
	Serial.print("\t index : ");
	Serial.println(keyTrackIndex);
*/
	keyTrack[keyTrackIndex].key = note;
	keyTrack[keyTrackIndex].velocity = velocity;

	return keyTrackIndex++;
}

// remove a key that has been released on the keyboard, and reorder all that were pressed after.
int8_t keyTrackRemove(uint8_t note){
	int8_t update = -1;
	for(uint8_t i = 0; i < keyTrackIndex; ++i){
		if(keyTrack[i].key == note){
			update = i;
			keyTrackIndex--;
			break;
		}
	}

	if(update >= 0){
/*
		Serial.print("note removed : ");
		Serial.print(note);
		Serial.print("\t index : ");
		Serial.println(update);
*/
		for(uint8_t i = update; i < keyTrackIndex; ++i){
			keyTrack[i] = keyTrack[i + 1];
		}
	}

	return update;
}

// Handle note on. Internal MIDI from Mega 1 lands here.
// Dispatch to settings if the function switch is enable.
// Apply the keyboard offset (Mega 1 has its first key corresponding to MIDI note 0)
// Echo the offset note to usbMIDI out.
void handleInternalNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
	if(function){
		handleKeyboardFunction(note, 1);
		return;
	}
	usbMIDI.sendNoteOn(note + MIDI_OFFSET + 12 * transpose, velocity, midiOutChannel);
	handleNoteOn(channel, note + MIDI_OFFSET, velocity);
}

// Handle note ON. usbMIDI in lands here.
// Define if the new note has to be played, according to notes already played and key priority.
void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
/*
	Serial.print("note ");
	Serial.print(note);
	Serial.println(" on");
*/

	int8_t newIndex = -1;
	int8_t lowerIndex = -1;
	int8_t upperIndex = -1;
	switch(keyMode){
		// When KEY_FIRST, we play the note only if there is not one already playing
		// But we keep track of all notes depressed !
		case KEY_FIRST:
			if(keyTrackAdd(note, velocity) == 0)
			noteOn(note, velocity);
			break;
		// When KEY_LAST, we play the new note anyway.
		// And keep track. Of course.
		case KEY_LAST:
//			if(keyTrackAdd(note, velocity) >= 0) noteOn(note, velocity);
			newIndex = keyTrackAdd(note, velocity);
			if(newIndex == 0){
				noteOn(note, velocity, 1);
			} else if(newIndex > 0){
				noteOn(note, velocity, noteRetrigger);
			}
			break;
		case KEY_LOWER:
			// add note to the keytrack table.
			// check if there is a lower one.
			// if no, play the note.
			// if yes, do nothing.
//			Serial.println("handle note on");

			newIndex = keyTrackAdd(note, velocity);
			lowerIndex = keyTrackGetLower(note);
/*
			Serial.print("new   : ");
			Serial.print(newIndex);
			Serial.print("\tlower : ");
			Serial.println(lowerIndex);
*/
			if(lowerIndex == (keyTrackIndex - 1)){
				if(newIndex == 0){
					noteOn(note, velocity);
				} else if(newIndex > 0){
					noteOn(note, velocity, noteRetrigger);
				}
			}
			break;
		case KEY_UPPER:
			// add note to the keytrack table.
			// check if there is an upper one.
			// If no, play the note.
			// If yes, do nothing.
			newIndex = keyTrackAdd(note, velocity);
			upperIndex = keyTrackGetUpper(note);

			if(upperIndex == (keyTrackIndex - 1)){
				if(newIndex == 0){
					noteOn(note, velocity);
				} else if(newIndex > 0){
					noteOn(note, velocity, noteRetrigger);
				}
			}
			break;
		default:
			break;
	}

}

// handle internal note off.
// Apply offset from keyboard, echo to usb MIDI out.
void handleInternalNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
	if(function){
//		handleKeyboardFunction(note, 0);
		return;
	}
	usbMIDI.sendNoteOff(note + MIDI_OFFSET + 12 * transpose, 0, midiOutChannel);
	handleNoteOff(channel, note + MIDI_OFFSET, velocity);
}

// Handle note off.
// usbMIDI in lands here.
// Manage note priority, i.e. stopping the note released and re-triggering the previous one if needed.
void handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
/*
	Serial.print("note ");
	Serial.print(note);
	Serial.println(" off");
*/

	int8_t lowerIndex = -1;
	int8_t upperIndex = -1;
	int8_t newIndex = -1;
	switch(keyMode){
		case KEY_FIRST:
			if(keyTrackRemove(note) == 0){
				if(keyTrackIndex > 0){
					noteOn(keyTrack[0].key, keyTrack[0].velocity, noteRetrigger);
				} else {
					noteOff();					
				}
			}
			break;
		case KEY_LAST:
			if(keyTrackRemove(note) == keyTrackIndex){
				if(keyTrackIndex > 0){
					noteOn(keyTrack[keyTrackIndex - 1].key,
						keyTrack[keyTrackIndex - 1].velocity, noteRetrigger);
				} else {
					noteOff();					
				}
			}
			break;			
		case KEY_LOWER:
			// check the keytrack table and remove the note of it.
			// compare it to other notes.
			// if there is no, send note off.
			// if there is a lower, do nothing.
			// if there is an upper, play the new lower note.
//			Serial.println("handle note off");

			lowerIndex = keyTrackGetLower(note);
			newIndex = keyTrackRemove(note);
/*
			Serial.print("new   : ");
			Serial.print(newIndex);
			Serial.print("\tlower : ");
			Serial.println(lowerIndex);
*/

			if(newIndex == lowerIndex){
				if(keyTrackIndex == 0){
					noteOff();
				} else {
					lowerIndex = keyTrackGetLower(note);
					noteOn(keyTrack[lowerIndex].key,
						keyTrack[lowerIndex].velocity, noteRetrigger);
				}
			}
			break;
		case KEY_UPPER:
			upperIndex = keyTrackGetUpper(note);
			newIndex = keyTrackRemove(note);

			if(newIndex == upperIndex){
				if(keyTrackIndex == 0){
					noteOff();
				} else {
					upperIndex = keyTrackGetUpper(note);
					noteOn(keyTrack[upperIndex].key,
						keyTrack[upperIndex].velocity, noteRetrigger);
				}
			}
			break;
		default:
			break;
	}

}

// internal pitch bend from Mega 1.
// Echo to usb MIDI out.
// Still work to do here, as there is still this bug on Mega 1 side.
void handleInternalPitchBend(uint8_t channel, int16_t bend){
	if(function) handlePitchBendFunction();
	int16_t bendAmount = (bend - PITCH_BEND_NEUTRAL) * PITCH_BEND_INTERNAL_TO_MIDI;
	handlePitchBend(channel, bendAmount);
	usbMIDI.sendPitchBend(bendAmount, midiOutChannel);
/*
	Serial.print("pitch bend :");
	Serial.println(bend);
*/
}

// Pitch bend from usb MIDI in lands here.
void handlePitchBend(uint8_t channel, int16_t bend){
	dcPitchBend.amplitude(((float)bend) / 8190);
	// neutral at -11 from u(bend - PITCH_BEND_NEUTRAL) * PITCH_BEND_INTERNAL_TO_MIDIp, -24 from down. :/
}

// Handle internal control changes, and probably some from outside
// all notes off is the only one implemented for now from usb MIDI in.
// Dispatch to settings when the function switch is on.
void handleControlChange(uint8_t channel, uint8_t command, uint8_t value){
	if(function){
		handleCCFunction(command, value);
		return;
	}
/*
	Serial.print("control change ");
	Serial.println(command);
*/

	// Long value reconstruct the 14-bits value send with CC 0-31, associated to CC LSB 32-63.
	// Ramp value is used for glide, attack, decay and release, which have exponential settings.
	// Short times can be precisely set, but longer are available as well.
	uint16_t longValue = 0;
	float rampValue = 0;
	if(command < 32){
		ccTempValue[command] = value;
/*
		Serial.print("value : ");
		Serial.print(value << 7);
		Serial.print(" (sent : ");
		Serial.print(value);
		Serial.println(')');
*/
	} else if(command < 64){
		longValue = (uint16_t)ccTempValue[command - 32];
		longValue <<= 7;
		longValue += value;
		rampValue = pow((float)longValue / RESO, 2);
/*
		Serial.print("value :    ");
		Serial.println(longValue);
		Serial.print("ramp value : ");
		Serial.println(rampValue * 1000);
		Serial.println();
*/
	} else {
/*
		Serial.print("value : ");
		Serial.println(value);		
*/
	}

	// The first 32 CC are empty, has we wait for the associated LSB CC to apply the whole value at once.
	switch(command){
		case CC_MOD_WHEEL:
		// CC_1
			break;
		case CC_MODULATION_MIX:
		// CC_3
			break;
		case CC_PORTAMENTO_TIME:
		// CC_5
			break;
		case CC_CHANNEL_VOL:
		// CC_7
			break;
		case CC_OSC_TUNE:
		// CC_9
			break;
		case CC_OSC2_TUNE:
		// CC_12
			break;
		case CC_OSC3_TUNE:
		// CC_13
			break;
		case CC_OSC1_MIX:
		// CC_14
			break;
		case CC_OSC2_MIX:
		// CC_15
			break;
		case CC_OSC3_MIX:
		// CC_16
			break;
		case CC_NOISE_MIX:
		// CC_17
			break;
		case CC_FEEDBACK_MIX:
		// CC_18
			break;
		case CC_FILTER_BAND:
		// CC_19
			break;
		case CC_FILTER_CUTOFF_FREQ:
		// CC_20
//			vcf.frequency((float)value * 32);
			break;
		case CC_FILTER_EMPHASIS:
		// CC_21
			break;
		case CC_FILTER_CONTOUR:
		// CC_22
			break;
		case CC_FILTER_ATTACK:
		// CC_23
			break;
		case CC_FILTER_DECAY:
		// CC_24
			break;
		case CC_FILTER_SUSTAIN:
		// CC_25
			break;
		case CC_FILTER_RELEASE:
		// CC_26
			break;
		case CC_EG_ATTACK:
		// CC_27
			break;
		case CC_EG_DECAY:
		// CC_28
			break;
		case CC_EG_SUSTAIN:
		// CC_29
			break;
		case CC_LFO_RATE:
		// CC_31
			break;
		case CC_MOD_WHEEL_LSB:
		// CC_33
//			ampModWheelOsc.gain(((float)longValue - 1 - MOD_WHEEL_MIN) / 12 / MOD_WHEEL_COURSE);
			ampModWheelOsc.gain(((float)(longValue * modWheelOscRange)) / MAX_OCTAVE / 12 / 16384);
			ampModWheelFilter.gain(((float)(longValue * modWheelFilterRange)) / FILTER_MAX_OCTAVE / 12 / 16384);
			// Mod wheel goes from 360 to 666.
/*
			Serial.print("mod wheel : ");
			Serial.println(longValue);
*/
			break;
		case CC_MODULATION_MIX_LSB:
		// CC_35
			AudioNoInterrupts();
			modMixer.gain(0, (float)longValue / RESO);
			modMixer.gain(1, (RESO - (float)longValue) / RESO);
			AudioInterrupts();
			break;
		case CC_PORTAMENTO_TIME_LSB:
		// CC_37
//			glide = longValue;
			glide = rampValue;
			break;
		case CC_CHANNEL_VOL_LSB:
		// CC_39
			masterVolume.gain((float)longValue / RESO);
			break;
		case CC_OSC_TUNE_LSB:
		// CC_41
			dcOscTune.amplitude(HALFTONE_TO_DC * 2 * ((float)longValue - HALF_RESO) / RESO);
			break;
		case CC_OSC2_TUNE_LSB:
		// CC_44
			dcOsc2Tune.amplitude(HALFTONE_TO_DC * 12 * 2 * ((float)longValue - HALF_RESO) / RESO);
			break;
		case CC_OSC3_TUNE_LSB:
		// CC_45
			dcOsc3Tune.amplitude(HALFTONE_TO_DC * 12 * 2 * ((float)longValue - HALF_RESO) / RESO);
			break;
		case CC_OSC1_MIX_LSB:
		// CC_46
			oscMixer.gain(0, MAX_MIX * (float)longValue / RESO);
			break;
		case CC_OSC2_MIX_LSB:
		// CC_47
			oscMixer.gain(1, MAX_MIX * (float)longValue / RESO);
			break;
		case CC_OSC3_MIX_LSB:
		// CC_48
			oscMixer.gain(2, MAX_MIX * (float)longValue / RESO);
			break;
		case CC_NOISE_MIX_LSB:
		// CC_49
			oscMixer.gain(3, MAX_MIX * (float)longValue / RESO);
			break;
		case CC_FEEDBACK_MIX_LSB:
		// CC_50
			globalMixer.gain(1, MAX_MIX * (float)longValue / RESO);
			break;
		case CC_FILTER_BAND_LSB:
		// CC_51
			filterBandValue = longValue;
			AudioNoInterrupts();
			if(filterMode == FILTER_BAND_PASS){
				if(longValue < HALF_RESO){
					bandMixer.gain(0, ((float)HALF_RESO - (float)longValue) / HALF_RESO);
					bandMixer.gain(1, (float)longValue / HALF_RESO);
					bandMixer.gain(2, 0.0);
				} else {
					bandMixer.gain(0, 0.0);
					bandMixer.gain(1, ((float)RESO - (float)longValue) / HALF_RESO);
					bandMixer.gain(2, ((float)longValue - HALF_RESO) / HALF_RESO);
				}
			} else if(filterMode == FILTER_BAND_STOP){
				bandMixer.gain(0, (float)(RESO - longValue) / RESO);
				bandMixer.gain(1, 0.0);
				bandMixer.gain(2, (float)longValue / RESO);
			}
			AudioInterrupts();
			break;
		case CC_FILTER_CUTOFF_FREQ_LSB:
		// CC_52
			dcFilter.amplitude(((float)longValue - HALF_RESO) / HALF_RESO);
			break;
		case CC_FILTER_EMPHASIS_LSB:
		// CC_53
			vcf.resonance(FILTER_MIN_Q + (float)longValue / FILTER_DIV_Q);
			break;
		case CC_FILTER_CONTOUR_LSB:
		// CC_54
//			filterMixer.gain(1, (float)longValue / RESO);
			filterMixer.gain(1, (float)(longValue - HALF_RESO) / RESO);
			break;
		case CC_FILTER_ATTACK_LSB:
		// CC_55
			// original : linear attack
//			filterEnvelope.attack(1 + (float)longValue * 5.0);
			filterEnvelope.attack(rampValue * MAX_ATTACK_TIME);

			break;
		case CC_FILTER_DECAY_LSB:
		// CC_56
//			filterEnvelope.decay((float)longValue * 5.0);
			filterEnvelope.decay(rampValue * MAX_ATTACK_TIME);
			break;
		case CC_FILTER_SUSTAIN_LSB:
		// CC_57
			filterEnvelope.sustain((float)longValue / RESO);
			break;
		case CC_FILTER_RELEASE_LSB:
		// CC_58
//			filterEnvelope.release(1 + (float)longValue * 5.0);
			filterEnvelope.release(rampValue * MAX_ATTACK_TIME);
			break;
		case CC_EG_ATTACK_LSB:
		// CC_59
//			mainEnvelope.attack(1 + (float)longValue * 5.0);
			mainEnvelope.attack(rampValue * MAX_ATTACK_TIME);
			break;
		case CC_EG_DECAY_LSB:
		// CC_60
//			mainEnvelope.decay((float)longValue * 5.0);
			mainEnvelope.decay(rampValue * MAX_ATTACK_TIME);
			break;
		case CC_EG_SUSTAIN_LSB:
		// CC_61
			mainEnvelope.sustain((float)longValue / RESO);
			break;
		case CC_EG_RELEASE_LSB:
		// CC_62
//			mainEnvelope.release(1 + (float)longValue * 5.0);
			mainEnvelope.release(rampValue * MAX_ATTACK_TIME);
			break;
		case CC_LFO_RATE_LSB:
		// CC_63
			dcLfoFreq.amplitude((float)longValue / RESO);
			break;
		case CC_PORTAMENTO_ON_OFF:
		// CC_65
/*
			Serial.print("portamento on off : ");
			Serial.println(value);
*/
			if(value < 64){
				glideEn = 1;
			} else {
				glideEn = 0;
			}
			break;
		case CC_BITCRUSH_OUT:
		// CC_91
			bitCrushOutput.bits(value);
			break;
		case CC_OSC1_RANGE:
		// CC_102
			osc1Waveform.frequency(NOTE_MIDI_0 / pow(2, value));
			break;
		case CC_OSC1_WAVEFORM:
		// CC_103
			osc1Waveform.begin(waveforms[value]);
			break;
		case CC_OSC2_RANGE:
		// CC_104
			osc2Waveform.frequency(NOTE_MIDI_0 / pow(2, value));
			break;
		case CC_OSC2_WAVEFORM:
		// CC_105
			osc2Waveform.begin(waveforms[value]);
			break;
		case CC_OSC3_RANGE:
		// CC_106
			osc3Waveform.frequency(NOTE_MIDI_0 / pow(2, value));
			break;
		case CC_OSC3_WAVEFORM:
		// CC_107
			osc3Waveform.begin(waveforms[value]);
			break;
		case CC_OSC3_CTRL:
		// CC_108
			AudioNoInterrupts();
			if(value > 63){
				osc3ControlMixer.gain(0, 1);
				osc3ControlMixer.gain(1, 0);
			} else {
				osc3ControlMixer.gain(0, 0);
				osc3ControlMixer.gain(1, 1);				
			}
			AudioInterrupts();
			break;
		case CC_FILTER_MOD:
		// CC_109
			if(value > 63){
				filterMixer.gain(0, 2);
			} else {
				filterMixer.gain(0, 0);
			}
			break;
		case CC_FILTER_KEYTRACK_1:
		// CC_110
			if(value > 63){
				filterKeyTrack1 = 1;
			} else {
				filterKeyTrack1 = 0;
			}
			filterMixer.gain(3, ((float)filterKeyTrack1 * 0.333333 + (float)filterKeyTrack2 * 0.666667));
			break;
		case CC_FILTER_KEYTRACK_2:
		// CC_111
			if(value > 63){
				filterKeyTrack2 = 1;
			} else {
				filterKeyTrack2 = 0;
			}
			filterMixer.gain(3, ((float)filterKeyTrack1 * 0.333333 + (float)filterKeyTrack2 * 0.666667));
			break;
		case CC_TRANSPOSE:
		// CC_112
			if(value > 63){
				transpose++;
				if (transpose > 2) transpose = 2;
			} else {
				transpose--;
				if(transpose < -2) transpose = -2;
			}
			break;
		case CC_FUNCTION:
		// CC_113
			if(value < 64){
				noteOff();
				keyTrackIndex = 0;
				usbMIDI.sendControlChange(CC_ALL_NOTE_OFF, 0, midiOutChannel);
				function = 1;
//				Serial.println("enterring function mode");
			} else {
				function = 0;
			}
			break;
		case CC_NOISE_COLOR:
		// CC_114
			AudioNoInterrupts();
			if(value > 0){
				noiseMixer.gain(0, 1);
				noiseMixer.gain(1, 0);
			} else {
				noiseMixer.gain(0, 0);
				noiseMixer.gain(1, 1);
			}
			AudioInterrupts();
			break;
		case CC_OSC_MOD:
		// CC_115
			if(value > 63){
				oscMod = 1;
				mainTuneMixer.gain(3, 1);
			} else {
				oscMod = 0;
				mainTuneMixer.gain(3, 0);
			}
			break;
/*
		case CC_DECAY_SW:
		// CC_116
			AudioNoInterrupts();
			if(value > 63){
				decay = 1;
				filterEnvelope.release(filterDecay);
				mainEnvelope.release(egDecay);
			} else {
				decay = 0;
				filterEnvelope.release(0.0);
				mainEnvelope.release(0.0);
			}
			AudioInterrupts();
			break;
*/
		case CC_MOD_MIX_1:
		// CC_117
			AudioNoInterrupts();
			if(value > 63){
				modMix1.gain(0, 0);
				modMix1.gain(1, 1);
			} else {
				modMix1.gain(0, 1);
				modMix1.gain(1, 0);
			}
			AudioInterrupts();
			break;
		case CC_MOD_MIX_2:
		// CC_118
			AudioNoInterrupts();
			if(value > 63){
				modMix2.gain(0, 0);
				modMix2.gain(1, 1);
			} else {
				modMix2.gain(0, 1);
				modMix2.gain(1, 0);
			}
			AudioInterrupts();
			break;
		case CC_LFO_SHAPE:
		// CC_119
			AudioNoInterrupts();
			// LFO shape is centered around 0 (range from -1 to 1) when triangle (like a finger vibrato on a violin)
			// Square is offset, ranging from 0 to 1, like a duo-tone siren.
			if(value > 63){
				lfoWaveform.begin(WAVEFORM_TRIANGLE);
				lfoWaveform.offset(0.0);
				lfoWaveform.amplitude(1.0);
			} else {
				lfoWaveform.begin(WAVEFORM_SQUARE);
				lfoWaveform.offset(0.5);
				lfoWaveform.amplitude(0.5);
			}
			AudioInterrupts();
			break;
		case CC_ALL_NOTE_OFF:
		// CC_123
			noteOff();
			keyTrackIndex = 0;
			break;
		default:
			break;
	}
}

// Handle key press when in function mode.
// Select the function to be set, then apply and save to memory the new setting.
void handleKeyboardFunction(uint8_t key, bool active){

	//
/*
	Serial.print("key pressed : ");
	Serial.println(key);
*/
	// Change function
	switch(key){
		case 0:
		// lower DO
			currentFunction = FUNCTION_KEYBOARD_MODE;
//			Serial.println("keyboard mode");
			break;
		case 2:
		// lower RE
			currentFunction = FUNCTION_RETRIGGER;
//			Serial.println("retrigger");
			break;
		case 4:
		// lower MI
			currentFunction = FUNCTION_DETUNE;
//			Serial.println("detune");
			break;
		case 5:
		// lower FA
			currentFunction = FUNCTION_BITCRUSH;
//			Serial.println("bitcrush");
			break;
		case 7:
		// lower SOL
			currentFunction = FUNCTION_FILTER_MODE;
			break;
		case 9:
		// lower LA
			currentFunction = FUNCTION_MIDI_IN_CHANNEL;
//			Serial.println("midi in channel");
			break;
		case 11:
		// lower SI
			currentFunction = FUNCTION_MIDI_OUT_CHANNEL;
//			Serial.println("midi out channel");
			break;
		default:
			if(key < 12) return;
			key -= 12;
			break;
	}

// note : the EEPROM.put() function could probably be called just once, but not all settings share the same type.
	switch(currentFunction){
		case FUNCTION_KEYBOARD_MODE:
			if(key > KEY_UPPER) return;
			keyMode = (keyMode_t)key;
			EEPROM.put(EE_KEYBOARD_MODE_ADD, keyMode);
			break;
		case FUNCTION_RETRIGGER:
			if(key > 1) return;
			noteRetrigger = key;
			EEPROM.put(EE_TRIGGER_ADD, noteRetrigger);
			break;
		case FUNCTION_DETUNE:
			if(key > DETUNE_RESET) return;
			if(key == DETUNE_RESET){
				// run a new detuning table
				resetDetuneTable();
			} else {
				detune = (detune_t)key;
				EEPROM.put(EE_DETUNE_ADD, detune);
			}
			break;
		case FUNCTION_BITCRUSH:
			if(key > 12) return;
			key += 4;
			bitCrushOutput.bits(key);
			EEPROM.put(EE_BITCRUSH_ADD, key);
			break;
		case FUNCTION_FILTER_MODE:
			if(key > 1) return;
			filterMode = (filterMode_t)key;
			EEPROM.put(EE_FILTER_MODE, filterMode);
			AudioNoInterrupts();
			// This is the same as in the CC handle function.
			// Could probably be a dedicate function called from both place.
			if(filterMode == FILTER_BAND_PASS){
				if(filterBandValue < HALF_RESO){
					bandMixer.gain(0, ((float)HALF_RESO - (float)filterBandValue) / HALF_RESO);
					bandMixer.gain(1, (float)filterBandValue / HALF_RESO);
					bandMixer.gain(2, 0.0);
				} else {
					bandMixer.gain(0, 0.0);
					bandMixer.gain(1, ((float)RESO - (float)filterBandValue) / HALF_RESO);
					bandMixer.gain(2, ((float)filterBandValue - HALF_RESO) / HALF_RESO);
				}
			} else if(filterMode == FILTER_BAND_STOP){
				bandMixer.gain(0, (float)(RESO - filterBandValue) / RESO);
				bandMixer.gain(1, 0.0);
				bandMixer.gain(2, (float)filterBandValue / RESO);
			}
			AudioInterrupts();
			break;			
		case FUNCTION_MIDI_IN_CHANNEL:
			// change (usb) midi in channel
			// for memory : MIDI channels go from 1 to 16, hence the +1 offset.
			if(key > 15)return;
			midiInChannel = key + 1;
			//MIDI.begin(midiInChannel);
			EEPROM.put(EE_MIDI_IN_CH_ADD, midiInChannel);
			break;
		case FUNCTION_MIDI_OUT_CHANNEL:
			// change (usb) midi out channel
			if(key > 15)return;
			midiOutChannel = key + 1;
			//MIDI.begin(midiInChannel);
			EEPROM.put(EE_MIDI_OUT_CH_ADD, midiOutChannel);
			break;
		case FUNCTION_PITCH_BEND_RANGE:
			// Change pitch bend range
			if((key < 1) || (key > 16)) return;
			pitchBendRange = key;
			ampPitchBend.gain(pitchBendRange * HALFTONE_TO_DC * 2);
			EEPROM.put(EE_PITCH_BEND_RANGE, pitchBendRange);
			break;
		case FUNCTION_MOD_WHEEL_OSC_RANGE:
			// Change mod wheel range for osc
			if(key == 0){
				currentFunction = FUNCTION_MOD_WHEEL_FILTER_RANGE;
				break;
			}
			modWheelOscRange = key;
			EEPROM.put(EE_MOD_WHEEL_OSC_RANGE, modWheelOscRange);
			break;
		case FUNCTION_MOD_WHEEL_FILTER_RANGE:
			// Change mod wheel range for osc
			if(key == 0) break;
			modWheelFilterRange = key;
			EEPROM.put(EE_MOD_WHEEL_FILTER_RANGE, modWheelFilterRange);
			break;
		default:
			break;		
	}

}

void handlePitchBendFunction(){
	currentFunction = FUNCTION_PITCH_BEND_RANGE;
}

// handle CC when in function mode
// todo : use filter band mode control to set function in this mode, instead of keyboard.
void handleCCFunction(uint8_t command, uint8_t value){
	switch(command){
		case CC_MOD_WHEEL:
			currentFunction = FUNCTION_MOD_WHEEL_OSC_RANGE;
			break;
		case CC_FUNCTION:
			// CC_113
			if(value < 64){
				function = 1;
			} else {
				function = 0;
//				Serial.println("exiting function mode");
			}
			break;
		default:
			break;
	}
}

// Compute a new detune table.
void resetDetuneTable(){
	uint16_t address = EE_DETUNE_TABLE_ADD;
	randomSeed(millis());
	for(uint8_t i = 0; i < 128; ++i){
		float value = (random() - 0x3FFFFFFF) / (float)0x3FFFFFFF;
		value *= HALFTONE_TO_DC;
		EEPROM.put(address, value);
		address += 4;
		Serial.println(value, 5);
	}
}