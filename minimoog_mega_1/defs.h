// Minimoog
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

#define CC0								0
#define CC1								1
#define CC2								2
#define CC3								3
#define CC4								4
#define CC5								5
#define CC6								6
#define CC7								7
#define CC8								8
#define CC9								9
#define CC10							10
#define CC11							11
#define CC12							12
#define CC13							13
#define CC14							14
#define CC15							15
#define CC16							16
#define CC17							17
#define CC18							18
#define CC19							19
#define CC20							20
#define CC21							21
#define CC22							22
#define CC23							23
#define CC24							24
#define CC25							25
#define CC26							26
#define CC27							27
#define CC28							28
#define CC29							29
#define CC30							30
#define CC31							31
#define CC32							32
#define CC33							33
#define CC34							34
#define CC35							35
#define CC36							36
#define CC37							37
#define CC38							38
#define CC39							39
#define CC40							40
#define CC41							41
#define CC42							42
#define CC43							43
#define CC44							44
#define CC45							45
#define CC46							46
#define CC47							47
#define CC48							48
#define CC49							49
#define CC50							50
#define CC51							51
#define CC52							52
#define CC53							53
#define CC54							54
#define CC55							55
#define CC56							56
#define CC57							57
#define CC58							58
#define CC59							59
#define CC60							60
#define CC61							61
#define CC62							62
#define CC63							63
#define CC64							64
#define CC65							65
#define CC66							66
#define CC67							67
#define CC68							68
#define CC69							69
#define CC70							70
#define CC71							71
#define CC72							72
#define CC73							73
#define CC74							74
#define CC75							75
#define CC76							76
#define CC77							77
#define CC78							78
#define CC79							79
#define CC80							80
#define CC81							81
#define CC82							82
#define CC83							83
#define CC84							84
#define CC85							85
#define CC86							86
#define CC87							87
#define CC88							88
#define CC89							89
#define CC90							90
#define CC91							91
#define CC92							92
#define CC93							93
#define CC94							94
#define CC95							95
#define CC96							96
#define CC97							97
#define CC98							98
#define CC99							99
#define CC100							100
#define CC101							101
#define CC102							102
#define CC103							103
#define CC104							104
#define CC105							105
#define CC106							106
#define CC107							107
#define CC108							108
#define CC109							109
#define CC110							110
#define CC111							111
#define CC112							112
#define CC113							113
#define CC114							114
#define CC115							115
#define CC116							116
#define CC117							117
#define CC118							118
#define CC119							119
#define CC120							120
#define CC121							121
#define CC122							122
#define CC123							123
#define CC124							124
#define CC125							125
#define CC126							126
#define CC127							127

// #define CC_BANK_SELECT 					CC0
#define CC_MOD_WHEEL 					CC1
// #define CC_BREATH_CTRL					CC2
#define CC_MODULATION_MIX				CC3
// #define CC_FOOT_CTRL					CC4
#define CC_PORTAMENTO_TIME				CC5
// #define CC_DATA_ENTRY_MSB				CC6
// #define CC_CHANNEL_VOL					CC7
// #define CC_BALANCE						CC8
#define CC_OSC_TUNE						CC9
// #define CC_PAN							CC10
// #define CC_EXPRESSION_CTRL				CC11
#define CC_OSC2_TUNE					CC12
#define CC_OSC3_TUNE					CC13
#define CC_OSC1_MIX 					CC14
#define CC_OSC2_MIX 					CC15
#define CC_OSC3_MIX 					CC16
#define CC_NOISE_MIX 					CC17
#define CC_FEEDBACK_MIX 				CC18
#define CC_FILTER_BAND					CC19
#define CC_FILTER_CUTOFF_FREQ			CC20
#define CC_FILTER_EMPHASIS				CC21
#define CC_FILTER_CONTOUR 				CC22
#define CC_FILTER_ATTACK 				CC23
#define CC_FILTER_DECAY					CC24
#define CC_FILTER_SUSTAIN				CC25
#define CC_FILTER_RELEASE				CC26
#define CC_EG_ATTACK 					CC27
#define CC_EG_DECAY 					CC28
#define CC_EG_SUSTAIN 					CC29
#define CC_EG_RELEASE					CC30
#define CC_LFO_RATE						CC31
// #define CC_BANK_SELECT_LSB				CC32
#define CC_MOD_WHEEL_LSB				CC33
// #define CC_BREATH_CTRL_LSB				CC34
#define CC_MODULATION_MIX_LSB			CC35
// #define CC_FOOT_CTRL_LSB				CC36
#define CC_PORTAMENTO_TIME_LSB			CC37
// #define CC_DATA_ENTRY_MSB_LSB			CC38
// #define CC_CHANNEL_VOL_LSB				CC39
// #define CC_BALANCE_LSB					CC40
#define CC_OSC_TUNE_LSB					CC41
// #define CC_PAN_LSB						CC42
// #define CC_EXPRESSION_CTRL_LSB			CC43
#define CC_OSC2_TUNE_LSB				CC44
#define CC_OSC3_TUNE_LSB				CC45
#define CC_OSC1_MIX_LSB					CC46
#define CC_OSC2_MIX_LSB					CC47
#define CC_OSC3_MIX_LSB					CC48
#define CC_NOISE_MIX_LSB				CC49
#define CC_FEEDBACK_MIX_LSB				CC50
#define CC_FILTER_BAND_LSB				CC51
#define CC_FILTER_CUTOFF_FREQ_LSB		CC52
#define CC_FILTER_EMPHASIS_LSB			CC53
#define CC_FILTER_CONTOUR_LSB			CC54
#define CC_FILTER_ATTACK_LSB			CC55
#define CC_FILTER_DECAY_LSB				CC56
#define CC_FILTER_SUSTAIN_LSB			CC57
#define CC_FILTER_RELEASE_LSB			CC58
#define CC_EG_ATTACK_LSB				CC59
#define CC_EG_DECAY_LSB					CC60
#define CC_EG_SUSTAIN_LSB				CC61
#define CC_EG_RELEASE_LSB				CC62
#define CC_LFO_RATE_LSB					CC63
// #define CC_DAMPER_PEDAL_ON_OFF 			CC64
#define CC_PORTAMENTO_ON_OFF 			CC65
// #define CC_SOSTENUTO_ON_OFF 			CC66
// #define CC_SOFT_PEDAL_ON_OFF 			CC67
// #define CC_LEGATO_PEDAL 				CC68
// #define CC_HOLD_2 						CC69
// #define CC_SOUND_CTRL_1 				CC70
// #define CC_SOUND_CTRL_2 				CC71
// #define CC_SOUND_CTRL_3 				CC72
// #define CC_SOUND_CTRL_4 				CC73
// #define CC_SOUND_CTRL_5 				CC74
// #define CC_SOUND_CTRL_6 				CC75
// #define CC_SOUND_CTRL_7 				CC76
// #define CC_SOUND_CTRL_8 				CC77
// #define CC_SOUND_CTRL_9 				CC78
// #define CC_SOUND_CTRL_10 				CC79
// #define CC_GEN_PURPOSE_CTRL_5 			CC80
// #define CC_GEN_PURPOSE_CTRL_6 			CC81
// #define CC_GEN_PURPOSE_CTRL_7 			CC82
// #define CC_GEN_PURPOSE_CTRL_8 			CC83
// #define CC_PORTAMENTO_CTRL 				CC84
// #define CC_ 								CC85
// #define CC_ 								CC86
// #define CC_ 								CC87
// #define CC_ 								CC88
// #define CC_ 								CC89
#define CC_ASK_FOR_DATA						CC90
#define CC_BITCRUSH_OUT 					CC91
// #define CC_EFFECT_2_DEPTH 				CC92
// #define CC_EFFECT_3_DEPTH 				CC93
// #define CC_EFFECT_4_DEPTH 				CC94
// #define CC_EFFECT_5_DEPTH 				CC95
// #define CC_DATA_INC 					CC96
// #define CC_DATA_DEC 					CC97
// #define CC_NRPN_LSB 					CC98
// #define CC_NRPN_MSB 					CC99
// #define CC_RPN_LSB 						CC100
// #define CC_RPN_MSB 						CC101
#define CC_OSC1_RANGE 					CC102
#define CC_OSC1_WAVEFORM 				CC103
#define CC_OSC2_RANGE 					CC104
#define CC_OSC2_WAVEFORM 				CC105
#define CC_OSC3_RANGE					CC106
#define CC_OSC3_WAVEFORM				CC107
#define CC_OSC3_CTRL 					CC108
#define CC_FILTER_MOD 					CC109
#define CC_FILTER_KEYTRACK_1 			CC110
#define CC_FILTER_KEYTRACK_2 			CC111
#define CC_TRANSPOSE 					CC112
#define CC_FUNCTION 					CC113
#define CC_NOISE_COLOR					CC114
#define CC_OSC_MOD						CC115
#define CC_DECAY_SW						CC116
#define CC_MOD_MIX_1					CC117
#define CC_MOD_MIX_2					CC118
#define CC_LFO_SHAPE					CC119
// #define CC_ALL_SOUND_OFF 				CC120
// #define CC_RST_ALL_CTRL 				CC121
// #define CC_LOCAL_CTRL_ON_OFF 			CC122
// #define CC_ALL_NOTE_OFF 				CC123
// #define CC_OMNI_MODE_OFF 				CC124
// #define CC_OMNI_MODE_ON 				CC125
// #define CC_MONO_MODE_ON 				CC126
// #define CC_POLY_MODE_ON 				CC127
