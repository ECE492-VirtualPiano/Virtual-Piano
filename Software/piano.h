#include <stdio.h>
#include <math.h>
#include "wav.h"
#include "kiss_fft.h"

#define PI 3.1415926535897932384626

/* Method declarations */
void pitchshift(Sample **samples, Sample **samples_t, int n);
void speedx(Sample **samples, Sample **samples_t, float factor);
void superposition(Sample **samples1, Sample **samples2, Sample **samples_t, int offset);
void stretch(Sample **samples, Sample **samples_t, float factor, int window_size, int h);
void pitchshiftTest();
