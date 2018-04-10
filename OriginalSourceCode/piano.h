/*
*********************************************************************************************************
*
*                                          PIANO HEADER CODE
*
*                                            CYCLONE V SOC
*
* Filename      : piano.h
* Version       : V1.00
* Programmer(s) : Mingjun Zhao (zhao2@ualberta.ca ), Daniel Tran (dtran3@ualberta.ca)
* References    : Changes to this project include referenced and modified code from:
* 				  		Title: "Pianoputer"
* 				  		Original Author: Zulko
* 				  		Date accessed: Jan 31, 2018
* 				  		https://github.com/Zulko/pianoputer
*
*********************************************************************************************************
* Note(s)       : This file is mainly a sample based piano synthesizer which takes
				  in a sample piano sound waveform (Middle C) and transforms it into
				  other different piano sounds by changing its frequency and stretch
				  its length. Also it provides a way to mix the sounds by wave
				  superposition.
*********************************************************************************************************
*/

#include <stdio.h>
#include <math.h>
#include "../KissFFT/kiss_fft.h"
#include "samples.h"

#define PI 3.1415926535897932384626

//#define C1 4
//#define C2 16
//#define C3 28
//#define C4 40
//#define C5 52
//#define C6 64
//#define C7 76
//#define C8 88

//#define C1_LOW 1
//#define C1_HIGH 10
//#define C2_LOW 11
//#define C2_HIGH 22
//#define C3_LOW 23
//#define C3_HIGH 34
//#define C4_LOW 35
//#define C4_HIGH 46
//#define C5_LOW 47
//#define C5_HIGH 58
//#define C6_LOW 59
//#define C6_HIGH 70
//#define C7_LOW 71
//#define C7_HIGH 82
//#define C8_LOW 83
//#define C8_HIGH 88

#define C2 1
#define C3 13
#define C4 25
#define C5 37
#define C6 49
#define C7 61

#define C2_LOW 1
#define C2_HIGH 7
#define C3_LOW 8
#define C3_HIGH 19
#define C4_LOW 20
#define C4_HIGH 31
#define C5_LOW 32
#define C5_HIGH 43
#define C6_LOW 44
#define C6_HIGH 55
#define C7_LOW 56
#define C7_HIGH 67

#define WINDOW_SIZE 1024
#define H 256
#define MAX_STRETCH_ARRAY_SIZE 300000
#define MAX_PITCH_SHIFT_OUTPUT_ARRAY_SIZE 210000
#define MAX_TEMP_FLOAT_ARRAY_SIZE 300000

typedef struct {
    short  *data;
    int     size;
}Sample;

static Sample *SAMPLE_C1 = &(Sample) { .size = 151554, .data = C1_data };
static Sample *SAMPLE_C2 = &(Sample) { .size = 158070, .data = C2_data };
static Sample *SAMPLE_C3 = &(Sample) { .size = 124219, .data = C3_data };
static Sample *SAMPLE_C4 = &(Sample) { .size = 76321,  .data = C4_data };
static Sample *SAMPLE_C5 = &(Sample) { .size = 77777,  .data = C5_data };
static Sample *SAMPLE_C6 = &(Sample) { .size = 70008,  .data = C6_data };
static Sample *SAMPLE_C7 = &(Sample) { .size = 70243,  .data = C7_data };
static Sample *SAMPLE_C8 = &(Sample) { .size = 65422,  .data = C8_data };



/* Method declarations */
void initFFT(void);
void destoryFFT(void);
void pitchshift(Sample **inputSoundSample, Sample **outputSoundSample, int num_semitones);
void speedx(Sample **inputSoundSample, Sample **outputSoundSample, float factor);
void superposition(Sample **inputSoundSample1, Sample **inputSoundSample2, Sample **outputSoundSample, int offset);
void stretch(Sample **inputSoundSample, Sample **outputSoundSample, float factor);
Sample *sizeOfSound(int pianoKeyIndex, int *octaveKeyIndex);
void generateSound(int pianoKeyIndex, Sample **outputSoundSample);
