#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dcFilterEnvelope; //xy=101.75,518
AudioSynthWaveformDc     dcOscTune;      //xy=160.75,127
AudioSynthWaveformDc     dcKeyTrack;     //xy=163.75,91
AudioSynthWaveformDc     dcPitchBend;    //xy=166.75,162
AudioSynthWaveformDc     dcFilter;       //xy=268.75,573
AudioSynthNoisePink      pinkNoise;      //xy=290.75,298
AudioSynthWaveformDc     dcLfoFreq;      //xy=292.75,347
AudioSynthNoiseWhite     whiteNoise;     //xy=293.75,262
AudioEffectEnvelope      filterEnvelope; //xy=299.75,518
AudioSynthWaveformDc     dcFilterKeyTrack; //xy=301.75,607
AudioAmplifier           ampPitchBend;   //xy=339.75,162
AudioMixer4              noiseMixer;     //xy=476.75,295
AudioSynthWaveformModulated lfoWaveform;    //xy=481.75,347
AudioAmplifier           ampOsc3Mod;     //xy=481.75,415
AudioAmplifier           ampModEg;       //xy=491.75,453
AudioMixer4              mainTuneMixer;  //xy=563.75,111
AudioMixer4              modMix2;        //xy=687.75,448
AudioMixer4              modMix1;        //xy=688.75,377
AudioSynthWaveformDc     dcOsc3;         //xy=690.75,171
AudioMixer4              osc3ControlMixer; //xy=865.75,177
AudioMixer4              modMixer;       //xy=877.75,426
AudioSynthWaveformDc     dcOsc2Tune;     //xy=1037.75,133
AudioSynthWaveformDc     dcOsc3Tune;     //xy=1038.75,199
AudioAmplifier           ampModWheel;    //xy=1091.75,426
AudioMixer4              osc3TuneMixer;  //xy=1221.75,195
AudioMixer4              osc2TuneMixer;  //xy=1222.75,131
AudioSynthWaveformDc     dcPulse;        //xy=1238.75,43
AudioSynthWaveformModulated osc1Waveform;   //xy=1455.75,92
AudioSynthWaveformModulated osc2Waveform;   //xy=1456.75,129
AudioSynthWaveformModulated osc3Waveform;   //xy=1456.75,166
AudioMixer4              oscMixer;       //xy=1642.75,135
AudioMixer4              globalMixer;    //xy=1851.75,182
AudioAmplifier           ampPreFilter;   //xy=2015.75,181
AudioMixer4              filterMixer;    //xy=2033.75,424
AudioFilterStateVariable vcf;            //xy=2202.75,418
AudioMixer4              bandMixer;      //xy=2373.75,413
AudioEffectEnvelope      mainEnvelope;   //xy=2552.75,414
AudioEffectBitcrusher    bitCrushOutput; //xy=2788.75,411
AudioAmplifier           masterVolume;           //xy=2981.75,410.75
AudioOutputI2S           i2s;            //xy=3150.75,391
//AudioOutputUSB           usbOut;           //xy=3153.75,429.75
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
AudioConnection          patchCord42(filterMixer, 0, vcf, 1);
AudioConnection          patchCord43(vcf, 0, bandMixer, 0);
AudioConnection          patchCord44(vcf, 1, bandMixer, 1);
AudioConnection          patchCord45(vcf, 2, bandMixer, 2);
AudioConnection          patchCord46(bandMixer, mainEnvelope);
AudioConnection          patchCord47(mainEnvelope, bitCrushOutput);
AudioConnection          patchCord48(mainEnvelope, 0, globalMixer, 1);
AudioConnection          patchCord49(bitCrushOutput, masterVolume);
AudioConnection          patchCord50(masterVolume, 0, i2s, 0);
AudioConnection          patchCord51(masterVolume, 0, i2s, 1);
//AudioConnection          patchCord52(masterVolume, 0, usbOut, 0);
//AudioConnection          patchCord53(masterVolume, 0, usbOut, 1);
// GUItool: end automatically generated code
