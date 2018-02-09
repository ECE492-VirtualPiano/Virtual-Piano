#include <stdio.h>
#include <math.h>
#include "wav.h"
#include "kiss_fft.h"

#define PI 3.1415926535897932384626

#define C1 4
#define C2 16
#define C3 28
#define C4 40
#define C5 52
#define C6 64
#define C7 76
#define C8 88

#define C1_LOW 1
#define C1_HIGH 10
#define C2_LOW 11
#define C2_HIGH 22
#define C3_LOW 23
#define C3_HIGH 34
#define C4_LOW 35
#define C4_HIGH 46
#define C5_LOW 47
#define C5_HIGH 58
#define C6_LOW 59
#define C6_HIGH 70
#define C7_LOW 71
#define C7_HIGH 82
#define C8_LOW 83
#define C8_HIGH 88

/* Method declarations */
void pitchshift(Sample **samples, Sample **samples_t, int n);
void speedx(Sample **samples, Sample **samples_t, float factor);
void superposition(Sample **samples1, Sample **samples2, Sample **samples_t, int offset);
void stretch(Sample **samples, Sample **samples_t, float factor, int window_size, int h);
void pitchshiftTest();
void generateSound(int index, Sample **samples_t);
