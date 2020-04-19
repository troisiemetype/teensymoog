#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dcFilterEnvelope; //xy=108.33332824707031,538
AudioSynthWaveformDc     dcOscTune;      //xy=167.3333282470703,147
AudioSynthWaveformDc     dcKeyTrack;     //xy=170.3333282470703,111
AudioSynthWaveformDc     dcPitchBend;    //xy=173.3333282470703,182
AudioSynthWaveformDc     dcFilter;       //xy=275.3333282470703,593
AudioSynthNoisePink      pinkNoise;      //xy=297.3333282470703,318
AudioSynthWaveformDc     dcLfoFreq;      //xy=299.3333282470703,367
AudioSynthNoiseWhite     whiteNoise;     //xy=300.3333282470703,282
AudioEffectEnvelope      filterEnvelope; //xy=306.3333282470703,538
AudioSynthWaveformDc     dcFilterKeyTrack; //xy=308.3333282470703,627
AudioAmplifier           ampPitchBend;   //xy=346.3333282470703,182
AudioMixer4              noiseMixer;     //xy=483.3333282470703,315
AudioSynthWaveformModulated lfoWaveform;    //xy=488.3333282470703,367
AudioAmplifier           ampOsc3Mod;     //xy=488.3333282470703,435
AudioAmplifier           ampModEg;       //xy=498.3333282470703,473
AudioMixer4              mainTuneMixer;  //xy=570.3333282470703,131
AudioMixer4              modMix2;        //xy=694.3333282470703,468
AudioMixer4              modMix1;        //xy=695.3333282470703,397
AudioSynthWaveformDc     dcOsc3;         //xy=697.3333282470703,191
AudioMixer4              osc3ControlMixer; //xy=872.3333282470703,197
AudioMixer4              modMixer;       //xy=884.3333282470703,446
AudioSynthWaveformDc     dcOsc2Tune;     //xy=1044.3333282470703,153
AudioSynthWaveformDc     dcOsc3Tune;     //xy=1045.3333282470703,219
AudioAmplifier           ampModWheelOsc; //xy=1094.3333282470703,426
AudioAmplifier           ampModWheelFilter; //xy=1101.3333282470703,461
AudioMixer4              osc3TuneMixer;  //xy=1228.3333282470703,215
AudioMixer4              osc2TuneMixer;  //xy=1229.3333282470703,151
AudioSynthWaveformDc     dcPulse;        //xy=1245.3333282470703,63
AudioSynthWaveformModulated osc1Waveform;   //xy=1462.3333282470703,112
AudioSynthWaveformModulated osc2Waveform;   //xy=1463.3333282470703,149
AudioSynthWaveformModulated osc3Waveform;   //xy=1463.3333282470703,186
AudioMixer4              oscMixer;       //xy=1649.3333282470703,155
AudioMixer4              globalMixer;    //xy=1858.3333282470703,202
AudioAmplifier           ampPreFilter;   //xy=2022.3333282470703,201
AudioMixer4              filterMixer;    //xy=2040.3333282470703,444
AudioFilterStateVariable vcf;            //xy=2209.3333282470703,438
AudioAnalyzePeak         peakPreFilter;  //xy=2271.3333282470703,188
AudioAnalyzePrint        printPreFilter; //xy=2271.3333282470703,225
AudioMixer4              bandMixer;      //xy=2380.3333282470703,433
AudioEffectEnvelope      mainEnvelope;   //xy=2559.3333282470703,434
AudioAnalyzePeak         peakPostFilter; //xy=2559.3333282470703,503
AudioAnalyzePrint        printPostFilter; //xy=2562.3333282470703,471
AudioEffectBitcrusher    bitCrushOutput; //xy=2795.3333282470703,431
AudioAmplifier           masterVolume;   //xy=2988.3333282470703,430
AudioOutputI2S           i2s;            //xy=3159.3333282470703,430
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
AudioConnection          patchCord25(modMixer, ampModWheelOsc);
AudioConnection          patchCord26(modMixer, ampModWheelFilter);
AudioConnection          patchCord27(dcOsc2Tune, 0, osc2TuneMixer, 1);
AudioConnection          patchCord28(dcOsc3Tune, 0, osc3TuneMixer, 1);
AudioConnection          patchCord29(ampModWheelOsc, 0, mainTuneMixer, 3);
AudioConnection          patchCord30(ampModWheelFilter, 0, filterMixer, 0);
AudioConnection          patchCord31(osc3TuneMixer, 0, osc3Waveform, 0);
AudioConnection          patchCord32(osc2TuneMixer, 0, osc2Waveform, 0);
AudioConnection          patchCord33(dcPulse, 0, osc1Waveform, 1);
AudioConnection          patchCord34(dcPulse, 0, osc2Waveform, 1);
AudioConnection          patchCord35(dcPulse, 0, osc3Waveform, 1);
AudioConnection          patchCord36(osc1Waveform, 0, oscMixer, 0);
AudioConnection          patchCord37(osc2Waveform, 0, oscMixer, 1);
AudioConnection          patchCord38(osc3Waveform, 0, oscMixer, 2);
AudioConnection          patchCord39(osc3Waveform, ampOsc3Mod);
AudioConnection          patchCord40(oscMixer, 0, globalMixer, 0);
AudioConnection          patchCord41(globalMixer, ampPreFilter);
AudioConnection          patchCord42(ampPreFilter, 0, vcf, 0);
AudioConnection          patchCord43(ampPreFilter, printPreFilter);
AudioConnection          patchCord44(filterMixer, 0, vcf, 1);
AudioConnection          patchCord45(vcf, 0, bandMixer, 0);
AudioConnection          patchCord46(vcf, 1, bandMixer, 1);
AudioConnection          patchCord47(vcf, 2, bandMixer, 2);
AudioConnection          patchCord48(bandMixer, mainEnvelope);
AudioConnection          patchCord49(bandMixer, 0, globalMixer, 1);
AudioConnection          patchCord50(mainEnvelope, bitCrushOutput);
AudioConnection          patchCord51(bitCrushOutput, masterVolume);
AudioConnection          patchCord52(masterVolume, 0, i2s, 0);
AudioConnection          patchCord53(masterVolume, 0, i2s, 1);

// for debug purpose, uncomment to test audio with internal DAC, or USB.

// on board DAC may need a decoupling capacitor (10uF is a safe value)
// AudioOutputAnalog        dac1;           //xy=3166.3333282470703,501.3333282470703
// AudioConnection          patchCord54(masterVolume, dac1);

// USB needs the sketch to be compiled with USB type set to audio, MIDI + audio or MIDI + serial + audio in the IDE
// AudioOutputUSB           usb1;           //xy=3159.3333740234375,363.3333435058594
// AudioConnection          patchCord55(masterVolume, 0, usb1, 0);
// AudioConnection          patchCord56(masterVolume, 0, usb1, 1);


// Sync connection
//AudioConnection          patchCord54(osc1Waveform, 1, osc2Waveform, 2);
//AudioConnection          patchCord55(osc1Waveform, 1, osc3Waveform, 2);
// Sync connection  -end


// GUItool: end automatically generated code
