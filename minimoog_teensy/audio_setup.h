#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dcFilterEnvelope; //xy=169.76666259765625,540.0000305175781
AudioSynthWaveformDc     dcOscTune;      //xy=228.76666259765625,149
AudioSynthWaveformDc     dcKeyTrack;     //xy=231.76666259765625,113
AudioSynthWaveformDc     dcPitchBend;    //xy=234.7666778564453,184.00001525878906
AudioSynthWaveformDc     dcFilter;       //xy=336.76666259765625,595.0000305175781
AudioSynthNoisePink      pinkNoise;      //xy=358.76666259765625,320
AudioSynthNoiseWhite     whiteNoise;     //xy=361.76666259765625,284
AudioSynthWaveformDc     dcLfoFreq;      //xy=365.76666259765625,376
AudioEffectEnvelope      filterEnvelope; //xy=367.76666259765625,540.0000305175781
AudioSynthWaveformDc     dcFilterKeyTrack; //xy=369.76666259765625,629.0000305175781
AudioAmplifier           ampPitchBend;   //xy=407.7666778564453,184.00001525878906
AudioMixer4              noiseMixer;     //xy=544.7666625976562,317
AudioSynthWaveformModulated lfoWaveform;    //xy=548.7666625976562,374
AudioAmplifier           ampOsc3Mod;     //xy=549.7666625976562,437
AudioAmplifier           ampModEg;       //xy=559.7666625976562,475
AudioMixer4              mainTuneMixer; //xy=631.7666625976562,133
AudioMixer4              modMix2;        //xy=755.7666015625,470
AudioMixer4              modMix1;        //xy=756.7666015625,399
AudioSynthWaveformDc     dcOsc3;         //xy=758.7667541503906,193.00001525878906
AudioMixer4              osc3ControlMixer; //xy=933.7667236328125,199
AudioMixer4              modMixer;       //xy=945.7666015625,448
AudioSynthWaveformDc     dcOsc2Tune;            //xy=1105.566650390625,155.56666564941406
AudioSynthWaveformDc     dcOsc3Tune; //xy=1106.36669921875,221.56666564941406
AudioAmplifier           ampModWheel;    //xy=1159.7666015625,448
AudioMixer4              osc3TuneMixer; //xy=1289.36669921875,217.56666564941406
AudioMixer4              osc2TuneMixer;         //xy=1290.566650390625,153.56666564941406
AudioSynthWaveformDc     dcPulse;            //xy=1306.566665649414,65.56666564941406
AudioSynthWaveformModulated osc1Waveform;   //xy=1523.7666015625,114
AudioSynthWaveformModulated osc2Waveform;   //xy=1524.7666015625,151
AudioSynthWaveformModulated osc3Waveform;   //xy=1524.7666015625,188
AudioMixer4              oscMixer;       //xy=1710.7666015625,157
AudioMixer4              globalMixer;    //xy=1919.7666015625,204
AudioAmplifier           ampPreFilter;   //xy=2083.7666015625,203
AudioMixer4              filterMixer;    //xy=2101.7666625976562,446
AudioFilterStateVariable vcf;            //xy=2270.7666625976562,440
AudioMixer4              bandMixer;         //xy=2441.566665649414,435.56666564941406
AudioEffectEnvelope      mainEnvelope;   //xy=2620.7664794921875,436
AudioEffectBitcrusher    bitCrushOutput;    //xy=2856.5662841796875,433.566650390625
AudioOutputI2S           i2s;            //xy=3046.7659912109375,435
AudioConnection          patchCord1(dcFilterEnvelope, filterEnvelope);
AudioConnection          patchCord2(dcOscTune, 0, mainTuneMixer, 1);
AudioConnection          patchCord3(dcKeyTrack, 0, mainTuneMixer, 0);
AudioConnection          patchCord4(dcPitchBend, ampPitchBend);
AudioConnection          patchCord5(dcFilter, 0, filterMixer, 2);
AudioConnection          patchCord6(pinkNoise, 0, noiseMixer, 1);
AudioConnection          patchCord7(whiteNoise, 0, noiseMixer, 0);
AudioConnection          patchCord8(dcLfoFreq, 0, lfoWaveform, 0);
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
AudioConnection          patchCord44(vcf, 2, bandMixer, 1);
AudioConnection          patchCord45(bandMixer, mainEnvelope);
AudioConnection          patchCord46(mainEnvelope, 0, globalMixer, 1);
AudioConnection          patchCord47(mainEnvelope, bitCrushOutput);
AudioConnection          patchCord48(bitCrushOutput, 0, i2s, 0);
AudioConnection          patchCord49(bitCrushOutput, 0, i2s, 1);
// GUItool: end automatically generated code
