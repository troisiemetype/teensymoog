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

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include "audio_setup.h"
#include "defs.h"

//#include "MIDI.h"

// constants
const uint8_t KEYTRACK_MAX = 10;

const uint8_t MAX_OCTAVE = 10;
const uint8_t FILTER_MAX_OCTAVE = 5;

const float NOTE_MIDI_0 = 8.1757989156434;
const float NOTE_RATIO = 1.0594630943593;

const float HALFTONE_TO_DC = (float)1 / (MAX_OCTAVE * 12);
const float FILTER_HALFTONE_TO_DC = (float)1 / (FILTER_MAX_OCTAVE * 12);

const float MAX_MIX = 0.8;

// variables

uint8_t midiChannel = 1;

int8_t transposeOffset = 5;

uint16_t glide = 0;
bool glideEn  = 0;

bool noteRetrigger = 1;

bool filterKeyTrack1 = 0;
bool filterKeyTrack2 = 0;

int8_t transpose = 0;
bool function = 0;

bool oscMod = 0;
bool decay = 0;

float filterDecay = 0;
float egDecay = 0;

// Waveforms
uint8_t waveforms[6] = {WAVEFORM_SINE, WAVEFORM_TRIANGLE, WAVEFORM_SAWTOOTH,
						WAVEFORM_SAWTOOTH_REVERSE, WAVEFORM_SQUARE, WAVEFORM_PULSE};

// keyTrack
uint8_t keyTrackIndex = 0;
uint8_t keyTrack[KEYTRACK_MAX] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

enum keyMode_t{
	KEY_FIRST = 0,
	KEY_LAST,
	KEY_LOWER,
	KEY_UPPER,
};

keyMode_t keyMode = KEY_LAST;


//MIDI_CREATE_DEFAULT_INSTANCE();
//MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, usbMIDI);


void setup() {

	// midi settings, start and callback	usbMIDI.begin(1);	usbMIDI.setHandleNoteOn(handleNoteOn)	usbMIDI.setHandleNoteOff(handleNoteOff)	usbMIDI.setHandlePitchBend(handlePitchBend)	usbMIDI.setHandleControlChange(handleControlChange);

	usbMIDI.setHandleNoteOn(handleNoteOn);
	usbMIDI.setHandleNoteOff(handleNoteOff);
	usbMIDI.setHandlePitchChange(handlePitchBend);
	usbMIDI.setHandleControlChange(handleControlChange);

	Serial.begin(115200);
//	Serial.println("started...");

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
	ampPitchBend.gain(3 * HALFTONE_TO_DC * 2);
	ampModWheel.gain(0);
	ampPreFilter.gain(0.7);
	ampModEg.gain(0.01);
	ampOsc3Mod.gain(1);

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

	// filter
	vcf.frequency(640);
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

	usbMIDI.sendControlChange(CC_ASK_FOR_DATA, 127, 1);

}

void loop() {
	usbMIDI.read();
}

void noteOn(uint8_t note, uint8_t velocity, bool trigger = 1){
	float duration = (float)glideEn * (float)glide * 30.0;
	float level = (float)note * HALFTONE_TO_DC;
	float filterLevel = (float)note * FILTER_HALFTONE_TO_DC;

	AudioNoInterrupts();
	dcKeyTrack.amplitude(level, duration);
	dcFilterKeyTrack.amplitude(filterLevel, duration);
	if(trigger){
		filterEnvelope.noteOn();
		mainEnvelope.noteOn();
	}
	AudioInterrupts();
}

void noteOff(){
	AudioNoInterrupts();
	filterEnvelope.noteOff();
	mainEnvelope.noteOff();
	AudioInterrupts();
}

int8_t keyTrackAddNote(uint8_t note){
	// We only keep count of a limited quantity of notes !
	if (keyTrackIndex >= KEYTRACK_MAX) return -1;

	keyTrack[keyTrackIndex] = note;
	return keyTrackIndex++;
}

int8_t keyTrackRemoveNote(uint8_t note){
	int8_t update = -1;
	for(uint8_t i = 0; i < keyTrackIndex; ++i){
		if(keyTrack[i] == note){
			update = i;
			keyTrackIndex--;
			break;
		}
	}

	if(update >= 0){
		for(uint8_t i = update; i < keyTrackIndex; ++i){
			keyTrack[i] = keyTrack[i + 1];
		}
	}

	return update;
}

void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity){
	switch(keyMode){
		// When KEY_FIRST, we play the note only if there is not one already playing
		case KEY_FIRST:
			if(keyTrackIndex != 0) return;
			keyTrack[0] = note;
			keyTrackIndex = 1;
			noteOn(note, velocity);
//			if(keyTrackAddNote(note) == 0) noteOn(note, velocity);
			break;
		// When KEY_LAST, we play the new note anyway.
		case KEY_LAST:
			keyTrack[0] = note;
			if(keyTrackIndex != 0){
				noteOn(note, velocity, noteRetrigger);
			} else {
				noteOn(note, velocity);
			}
			keyTrackIndex = 1;
//			if(keyTrackAddNote(note) == (keyTrackIndex - 1)) noteOn(note, velocity);
			break;
		case KEY_LOWER:
		case KEY_UPPER:
		default:
			break;
	}

}

void handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity){
	switch(keyMode){
		case KEY_FIRST:
/*			if(keyTrackRemoveNote(note) == 0){
				noteOff();
			}
			break;
*/		case KEY_LAST:
			if(keyTrack[0] == note){
				keyTrackIndex = 0;
				noteOff();
			}
/*			if(keyTrackRemoveNote(note) == keyTrackIndex){
				noteOff();
			}
*/			break;			
		case KEY_LOWER:
		case KEY_UPPER:
		default:
			break;
	}

}

void handlePitchBend(uint8_t channel, int16_t bend){
	dcPitchBend.amplitude(((float)bend - 64) / 128);
}

void handleControlChange(uint8_t channel, uint8_t command, uint8_t value){
	//	Serial.println("control change");
	switch(command){
		case CC_MOD_WHEEL:						// CC_1
			ampModWheel.gain((float)value / 12 / 127);
			break;
		case CC_MODULATION_MIX:					// CC_3
			AudioNoInterrupts();
			modMixer.gain(0, (float)value / 127);
			modMixer.gain(1, (127 - (float)value) / 127);
			AudioInterrupts();
			break;
		case CC_PORTAMENTO_TIME:
		// CC_5
			glide = value;
			break;
		case CC_OSC_TUNE:
		// CC_9
			dcOscTune.amplitude(HALFTONE_TO_DC * 2 * ((float)value - 64) / 127);
			break;
		case CC_OSC2_TUNE:
		// CC_12
			dcOsc2Tune.amplitude(HALFTONE_TO_DC * 12 * 2 * ((float)value - 64) / 127);
			break;
		case CC_OSC3_TUNE:
		// CC_13
			dcOsc3Tune.amplitude(HALFTONE_TO_DC * 12 * 2 * ((float)value - 64) / 127);
			break;
		case CC_OSC1_MIX:
		// CC_14
			oscMixer.gain(0, MAX_MIX * (float)value / 127);
			break;
		case CC_OSC2_MIX:
		// CC_15
			oscMixer.gain(1, MAX_MIX * (float)value / 127);
			break;
		case CC_OSC3_MIX:
		// CC_16
			oscMixer.gain(2, MAX_MIX * (float)value / 127);
			break;
		case CC_NOISE_MIX:
		// CC_17
			oscMixer.gain(3, MAX_MIX * (float)value / 127);
			break;
		case CC_FEEDBACK_MIX:
		// CC_18
			globalMixer.gain(1, MAX_MIX * (float)value / 127);
			break;
		case CC_FILTER_BAND:
		// CC_19
			AudioNoInterrupts();
			bandMixer.gain(0, ((float)value - 127) / 127);
			bandMixer.gain(1, (float)value / 127);
			AudioInterrupts();
			break;
		case CC_FILTER_CUTOFF_FREQ:
		// CC_20
//			vcf.frequency((float)value * 32);
			dcFilter.amplitude(((float)value - 64) / 127);
			break;
		case CC_FILTER_EMPHASIS:
		// CC_21
			vcf.resonance(0.7 + (float)value / 29.53);
			break;
		case CC_FILTER_CONTOUR:
		// CC_22
			filterMixer.gain(1, (float)value / 127);
		case CC_FILTER_ATTACK:
		// CC_23
			filterEnvelope.attack((float)value * 40);
			break;
		case CC_FILTER_DECAY:
		// CC_24
			filterDecay = value * 40;
			AudioNoInterrupts();
			filterEnvelope.decay(filterDecay);
			if(decay) filterEnvelope.release(filterDecay);
			AudioInterrupts();
			break;
		case CC_FILTER_SUSTAIN:
		// CC_25
			filterEnvelope.sustain((float)value / 127);
			break;
		case CC_FILTER_RELEASE:
		// CC_26
			filterEnvelope.release((float)value / 127);
			break;
		case CC_EG_ATTACK:
		// CC_27
			mainEnvelope.attack((float)value * 40);
			break;
		case CC_EG_DECAY:
		// CC_28
			egDecay = value * 40;
			AudioNoInterrupts();
			mainEnvelope.decay(egDecay);
			if(decay) mainEnvelope.release(egDecay);
			AudioInterrupts();
			break;
		case CC_EG_SUSTAIN:
		// CC_29
			mainEnvelope.sustain((float)value / 127);
			break;
		case CC_LFO_RATE:
		// CC_31
			dcLfoFreq.amplitude((float)value / 127);
			break;
		case CC_MOD_WHEEL_LSB:
		// CC_33
			break;
		case CC_MODULATION_MIX_LSB:
		// CC_35
			break;
		case CC_PORTAMENTO_TIME_LSB:
		// CC_37
			break;
		case CC_OSC_TUNE_LSB:
		// CC_41
			break;
		case CC_OSC2_TUNE_LSB:
		// CC_44
			break;
		case CC_OSC3_TUNE_LSB:
		// CC_45
			break;
		case CC_OSC1_MIX_LSB:
		// CC_46
			break;
		case CC_OSC2_MIX_LSB:
		// CC_47
			break;
		case CC_OSC3_MIX_LSB:
		// CC_48
			break;
		case CC_NOISE_MIX_LSB:
		// CC_49
			break;
		case CC_FEEDBACK_MIX_LSB:
		// CC_50
			break;
		case CC_FILTER_BAND_LSB:
		// CC_51
			break;
		case CC_FILTER_CUTOFF_FREQ_LSB:
		// CC_52
			break;
		case CC_FILTER_EMPHASIS_LSB:
		// CC_53
			break;
		case CC_FILTER_CONTOUR_LSB:
		// CC_54
			break;
		case CC_FILTER_ATTACK_LSB:
		// CC_55
			break;
		case CC_FILTER_DECAY_LSB:
		// CC_56
			break;
		case CC_FILTER_SUSTAIN_LSB:
		// CC_57
			break;
		case CC_EG_ATTACK_LSB:
		// CC_58
			break;
		case CC_EG_DECAY_LSB:
		// CC_59
			break;
		case CC_EG_SUSTAIN_LSB:
		// CC_60
			break;
		case CC_LFO_RATE_LSB:
		// CC_62
			break;
		case CC_PORTAMENTO_ON_OFF:
		// CC_65
			if(value > 63){
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
			filterMixer.gain(3, (filterKeyTrack1 * 0.333 + filterKeyTrack2 * 0.667));
			break;
		case CC_FILTER_KEYTRACK_2:
		// CC_111
			if(value > 63){
				filterKeyTrack2 = 1;
			} else {
				filterKeyTrack2 = 0;
			}
			filterMixer.gain(3, (filterKeyTrack1 * 0.333 + filterKeyTrack2 * 0.667));
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
			if(value > 63){
				function = 1;
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
/*		case CC_LFO_SHAPE:						// CC_119
			if(value > 63){
			} else {
			}
			break;
*/		default:
			break;
	}
}
