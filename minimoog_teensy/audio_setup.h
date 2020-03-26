#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dcFilterEnvelope; //xy=91,497
AudioSynthWaveformDc     dcOscTune;      //xy=150,106
AudioSynthWaveformDc     dcKeyTrack;     //xy=153,70
AudioSynthWaveformDc     dcPitchBend;    //xy=156,141
AudioSynthWaveformDc     dcFilter;       //xy=258,552
AudioSynthNoisePink      pinkNoise;      //xy=280,277
AudioSynthWaveformDc     dcLfoFreq;      //xy=282,326
AudioSynthNoiseWhite     whiteNoise;     //xy=283,241
AudioEffectEnvelope      filterEnvelope; //xy=289,497
AudioSynthWaveformDc     dcFilterKeyTrack; //xy=291,586
AudioAmplifier           ampPitchBend;   //xy=329,141
AudioMixer4              noiseMixer;     //xy=466,274
AudioSynthWaveformModulated lfoWaveform;    //xy=471,326
AudioAmplifier           ampOsc3Mod;     //xy=471,394
AudioAmplifier           ampModEg;       //xy=481,432
AudioMixer4              mainTuneMixer;  //xy=553,90
AudioMixer4              modMix2;        //xy=677,427
AudioMixer4              modMix1;        //xy=678,356
AudioSynthWaveformDc     dcOsc3;         //xy=680,150
AudioMixer4              osc3ControlMixer; //xy=855,156
AudioMixer4              modMixer;       //xy=867,405
AudioSynthWaveformDc     dcOsc2Tune;     //xy=1027,112
AudioSynthWaveformDc     dcOsc3Tune;     //xy=1028,178
AudioAmplifier           ampModWheel;    //xy=1081,405
AudioMixer4              osc3TuneMixer;  //xy=1211,174
AudioMixer4              osc2TuneMixer;  //xy=1212,110
AudioSynthWaveformDc     dcPulse;        //xy=1228,22
AudioSynthWaveformModulated osc1Waveform;   //xy=1445,71
AudioSynthWaveformModulated osc2Waveform;   //xy=1446,108
AudioSynthWaveformModulated osc3Waveform;   //xy=1446,145
AudioMixer4              oscMixer;       //xy=1632,114
AudioMixer4              globalMixer;    //xy=1841,161
AudioAmplifier           ampPreFilter;   //xy=2005,160
AudioMixer4              filterMixer;    //xy=2023,403
AudioFilterStateVariable vcf;            //xy=2192,397
AudioAnalyzePeak         peakPreFilter;          //xy=2254.7498779296875,147.75
AudioAnalyzePrint        printPreFilter;         //xy=2254.7498779296875,184.75001525878906
AudioMixer4              bandMixer;      //xy=2363,392
AudioEffectEnvelope      mainEnvelope;   //xy=2542,393
AudioAnalyzePrint        printPostFilter;         //xy=2542.75,427.75
AudioAnalyzePeak         peakPostFilter;          //xy=2542.75,462.75
AudioEffectBitcrusher    bitCrushOutput; //xy=2778,390
AudioAmplifier           masterVolume;   //xy=2971,389
AudioOutputI2S           i2s;            //xy=3140,370
AudioConnection          patchCord1(dcFilterEnvelope, filterEnvelope);
AudioConnection          patchCord2(dcOscTune, 0, mainTuneMixer, 1);
AudioConnection          patchCord3(dcKeyTrack, 0, mainTuneMixer, 0);
AudioConnection          patchCord4(dcPitchBend, ampPitchBend);
AudioConnection          patchCord5(dcFilter, 0, filterMixer, 2);
AudioConnection          patchCord6(pinkNoise, 0, noiseMixer, 1);
AudioConnection          patchCord7(dcLfoFreq, 0, lfoWaveform, 0);
AudioConnection          patchCord8(whiteNoise, 0, noiseMixer, 0);
AudioConnection          patchCord9(filterEnvelope, 0, filterMixer, 1);
AudioConnection          patchCord10(filterEnvelope, ampModEg);
AudioConnection          patchCord11(dcFilterKeyTrack, 0, filterMixer, 3);
AudioConnection          patchCord12(ampPitchBend, 0, mainTuneMixer, 2);
AudioConnection          patchCord13(noiseMixer, 0, modMix1, 0);
AudioConnection          patchCord14(noiseMixer, 0, oscMixer, 3);
AudioConnection          patchCord15(lfoWaveform, 0, modMix1, 1);
AudioConnection          patchCord16(ampOsc3Mod, 0, modMix2, 0);
AudioConnection          patchCord17(ampModEg, 0, modMix2, 1);
AudioConnection          patchCord18(mainTuneMixer, 0, osc3ControlMixer, 0);
AudioConnection          patchCord19(mainTuneMixer, 0, osc1Waveform, 0);
AudioConnection          patchCord20(mainTuneMixer, 0, osc2TuneMixer, 0);
AudioConnection          patchCord21(modMix2, 0, modMixer, 1);
AudioConnection          patchCord22(modMix1, 0, modMixer, 0);
AudioConnection          patchCord23(dcOsc3, 0, osc3ControlMixer, 1);
AudioConnection          patchCord24(osc3ControlMixer, 0, osc3TuneMixer, 0);
AudioConnection          patchCord25(modMixer, ampModWheel);
AudioConnection          patchCord26(dcOsc2Tune, 0, osc2TuneMixer, 1);
AudioConnection          patchCord27(dcOsc3Tune, 0, osc3TuneMixer, 1);
AudioConnection          patchCord28(ampModWheel, 0, filterMixer, 0);
AudioConnection          patchCord29(ampModWheel, 0, mainTuneMixer, 3);
AudioConnection          patchCord30(osc3TuneMixer, 0, osc3Waveform, 0);
AudioConnection          patchCord31(osc2TuneMixer, 0, osc2Waveform, 0);
AudioConnection          patchCord32(dcPulse, 0, osc1Waveform, 1);
AudioConnection          patchCord33(dcPulse, 0, osc2Waveform, 1);
AudioConnection          patchCord34(dcPulse, 0, osc3Waveform, 1);
AudioConnection          patchCord35(osc1Waveform, 0, oscMixer, 0);
AudioConnection          patchCord36(osc2Waveform, 0, oscMixer, 1);
AudioConnection          patchCord37(osc3Waveform, 0, oscMixer, 2);
AudioConnection          patchCord38(osc3Waveform, ampOsc3Mod);
AudioConnection          patchCord39(oscMixer, 0, globalMixer, 0);
AudioConnection          patchCord40(globalMixer, ampPreFilter);
AudioConnection          patchCord41(ampPreFilter, 0, vcf, 0);
AudioConnection          patchCord42(ampPreFilter, printPreFilter);
AudioConnection          patchCord43(filterMixer, 0, vcf, 1);
AudioConnection          patchCord44(vcf, 0, bandMixer, 0);
AudioConnection          patchCord45(vcf, 1, bandMixer, 1);
AudioConnection          patchCord46(vcf, 2, bandMixer, 2);
AudioConnection          patchCord47(bandMixer, mainEnvelope);
AudioConnection          patchCord48(bandMixer, 0, globalMixer, 1);
AudioConnection          patchCord49(mainEnvelope, bitCrushOutput);
AudioConnection          patchCord50(bitCrushOutput, masterVolume);
AudioConnection          patchCord51(masterVolume, 0, i2s, 0);
AudioConnection          patchCord52(masterVolume, 0, i2s, 1);
// GUItool: end automatically generated code
