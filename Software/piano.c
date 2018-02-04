/*
	Entity name: 	piano.c

	Author: 		Mingjun Zhao & Daniel Tran

	Date: 			Feb 4, 2018

	Project: 		ECE 492 Capstone Project - Virtual Piano

	Description: 	This file is mainly a sample based piano synthesizer which takes
				 	in a sample piano sound waveform (Middle C) and transforms it into
				 	other different piano sounds by changing its frequency and stretch
				 	its length. Also it provides a way to mix the sounds by wave 
				 	superposition. 

	Derived from: 	Pianoputer by Zulko
				  	https://github.com/Zulko/pianoputer
*/


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

/*
	Name: 			void pitchshift(samples, samples_t, n)
	
	Description: 	Changes the pitch of a sound by "n" semitones.

	Inputs: 		
			Sample** 	samples 		The address of the input sample waveform 
			int 		n 				The number of semitones

	Outputs:
			Sample**	samples_t 		The address of the output sample waveform 
*/
void pitchshift(Sample **samples, Sample **samples_t, int n)
{
	// The window size for FFT
	const int window_size = pow(2, 10);
	const int h = pow(2, 8);

	Sample *stretched = NULL;

	// The factor of frequency change in terms of semitones
	// The frequency doubles with one more octave (12 semitones) 
	float factor = pow(2.0f, (1.0f * n / 12.0f));

	// Stretch the wave by the reciprocal of the factor
	stretch(samples, &stretched, 1.0f / factor, window_size, h);

	// A temporary Sample variable to hold a part of the wave of 'strecthed'
	Sample *tempsamples_t = (Sample*)malloc(sizeof(Sample));
	tempsamples_t->size = stretched->size - window_size;
	tempsamples_t->data = (int16_t*)malloc(tempsamples_t->size* sizeof(int16_t));

	for (int i = 0; i < tempsamples_t->size; ++i)
	{
		tempsamples_t->data[i] = stretched->data[i + window_size];
	}

	// Change the frequency and the length of the wave by the factor
	// The wave after change will have the same length as the template wave
	speedx(&tempsamples_t, samples_t, factor);

	// Free the memory of temporary variables
	free(stretched->data);
	free(stretched);
	free(tempsamples_t->data);
	free(tempsamples_t);
}


/*
	Name: 			void speedx(samples, samples_t, factor)
	
	Description: 	Speeds up / slows down a sound, by given factor

	Inputs: 		
			Sample** 	samples 		The address of the input sample waveform 
			int 		factor  		The factor of speed

	Outputs:
			Sample**	samples_t 		The address of the output sample waveform 
*/
void speedx(Sample **samples, Sample **samples_t, float factor)
{
	// Free the memory of output Sample wave if not NULL
	if (*samples_t) 
	{
		free((*samples_t)->data);
		free(*samples_t);
	}

	// Allocate the memory for the output Sample wave
	*samples_t = (Sample*)malloc(sizeof(Sample));
	(*samples_t)->size = (*samples)->size / factor;
	(*samples_t)->data = (int16_t*)calloc((*samples_t)->size, sizeof(int16_t));

	// The index of output data array
	int res_i = 0;

	// Find the indecies of the input array by incrementing by factor and put the 
	// corresponding data into the output array.
	//
	// Ex. if factor is 2. Put every other element from input array into the output
	// 	   array
	for (float step = 0; step < (*samples)->size; step += factor)
	{
		int i = (int)round(step);

		// Ensures the index never exceeding the limit
		if (res_i < (*samples_t)->size)
		{
			(*samples_t)->data[res_i++] = (*samples)->data[i];
		}
	}
}


/*
	Name: 			void superposition(samples1, samples2, samples_t, offset)
	
	Description: 	Combine two input sample waveforms with a given offset into a 
					new waveform

	Inputs: 		
			Sample** 	samples1 		The address of the first input waveform 
			Sample** 	samples2 		The address of the second input waveform 
			int 		offset  		The start offset of the second waveform

	Outputs:
			Sample**	samples_t 		The address of the output sample waveform 
*/
void superposition(Sample **samples1, Sample **samples2, Sample **samples_t, int offset)
{
	// Free the memory of output Sample wave if not NULL
	if(*samples_t)
	{
		free((*samples_t)->data);
		free(*samples_t);
	}

	// Allocate the memory for the output Sample wave
	*samples_t = (Sample*)malloc(sizeof(Sample));

	// The length of output waveform
	// = start offset of the second wave
	// + Max(the length of the second waveform,
	//       the length of the first waveform - start offset of the second waveform)
	(*samples_t)->size = offset + (((*samples2)->size > (*samples1)->size - offset) ? (*samples2)->size : (*samples1)->size - offset);
	(*samples_t)->data = (int16_t*)calloc((*samples_t)->size, sizeof(int16_t));

	// Combines the two input waveforms by superposition
	for (int i = 0; i < (*samples_t)->size; i++)
	{
		(*samples_t)->data[i] = 0;

		if (i < (*samples1)->size)
		{
			(*samples_t)->data[i] += (*samples1)->data[i];
		}

		if (i >= offset && i < offset + (*samples2)->size)
		{
			(*samples_t)->data[i] += (*samples2)->data[i - offset];
		}
	}
}


/*
	Name: 			void stretch(samples, samples_t, factor, window_size, h)
	
	Description: 	Stretches / shortens a sound wave, by given factor

	Inputs: 		
			Sample** 	samples 		The address of the input waveform 
			float 		factor  		The stretch factor
			int 		window_size		The window size used in FFT
			int 		h 				The h factor

	Outputs:
			Sample**	samples_t 		The address of the output sample waveform 
*/
void stretch(Sample **samples, Sample **samples_t, float factor, int window_size, int h)
{
	// Free the memory of output Sample wave if not NULL
	if(*samples_t)
	{
		free((*samples_t)->data);
		free(*samples_t);
	} 

	// Allocate the memory for the output Sample wave
	*samples_t = (Sample*)malloc(sizeof(Sample));
	(*samples_t)->size = (*samples)->size / factor + window_size;
	(*samples_t)->data = (int16_t*)calloc((*samples_t)->size, sizeof(int16_t));

	// Initialize the phase vector and the hanning window vector
	float phase[window_size], hanning_window[window_size];
	for (int i = 0; i < window_size; ++i)
	{
		phase[i] = 0;

		// The formular for hanning window 
		hanning_window[i] = 0.5 * (1 - cos(2 * PI * i / (window_size - 1)));
	}

	// The result array
	// (Only works using dynamic memory allocation for some unknown reason)
	float *result = (float*)calloc((*samples_t)->size, sizeof(float));

	// Initialize the configuration of FFT and inverse FFT
    kiss_fft_cfg cfg, cfg_i;
    if((cfg = kiss_fft_alloc(window_size, 0, NULL, NULL)) == NULL ||
       (cfg_i = kiss_fft_alloc(window_size, 1, NULL, NULL)) == NULL)
    {
    	printf("not enough memory?\n");
    	exit(-1);
    }

    // The classical phase vocoder process
    //
    // Break the sound into overllaping bits and rearrange these bits so that
    // they will overlap more (shortening) or less (stretching)
	for (float step = 0; step < (*samples)->size - (window_size + h); step += h * factor)
	{
		// Two potentially overllaping subarrays from the input array
		int16_t a1[window_size], a2[window_size];

		int index = 0;
		for (int j = (int)step; j < (int)step + window_size; ++j)
		{
			a1[index] = (*samples)->data[j];
			a2[index] = (*samples)->data[j + h];
			index++;
		}

		// The input and output complex arrays for FFT
	    kiss_fft_cpx s1_in[window_size], s1_out[window_size], 
	    			 s2_in[window_size], s2_out[window_size];

	    for (int i = 0; i < window_size; ++i)
	    {
	    	s1_in[i].r = a1[i] * hanning_window[i];
	    	s2_in[i].r = a2[i] * hanning_window[i];
	    }

	    // Resynchronize the second array on the first by taking the FFT of the 
	    // arrays and make adjustments so that they are in phase
	    kiss_fft(cfg, s1_in, s1_out);
	 	kiss_fft(cfg, s2_in, s2_out);

	    for (int i = 0; i < window_size; ++i)
	    {
	    	/*
	    	  Complex division s2 / s1

	    	  (s2_r + s2_i * i)   (s2_r + s2_i * i) * (s1_r - s1_i * i)    
	    	  ----------------- = -------------------------------------
	    	  (s1_r + s1_i * i)   (s1_r + s1_i * i) * (s1_r - s1_i * i)      
	    	
			    (s1_r * s2_r + s1_i * s2_i) + (s1_r * s2_i - s1_i * s2_r)i
			  = ----------------------------------------------------------
			                         s1_r^2 + s1_i^2
			*/

	    	float res_i = s2_out[i].i * s1_out[i].r - s2_out[i].r * s1_out[i].i;
	    	float res_r = s2_out[i].r * s1_out[i].r + s2_out[i].i * s1_out[i].i;

			/*
			  Perform arctan2(s2, s1) to get the angle in four quadrants between
			  the two complex numbers. 
			*/	    	
	    	float atan_v = atan2(res_i, res_r);

	    	phase[i] = fmod(phase[i] + atan_v, 2.0f * PI);
	    }

		kiss_fft_cpx a2_rephased[window_size], s2_rephased[window_size];

		// Changes the phase of s2 to be in phase with s1
		for (int i = 0; i < window_size; ++i) 
		{
			float abs_v = sqrt(pow(s2_out[i].r, 2) + pow(s2_out[i].i, 2));
			s2_rephased[i].r = abs_v * cos(phase[i]);
			s2_rephased[i].i = abs_v * sin(phase[i]); 
		}

		// Perform inverse FFT to get rephased a2 in time domain
		kiss_fft(cfg_i, s2_rephased, a2_rephased);

		// Add to result
		int i2 = (int)(step / factor);
		for (int i = 0; i < window_size; ++i)
		{
			result[i + i2] += hanning_window[i] * a2_rephased[i].r;
		}
	}

	// Find the max absolute value of the waveform
	float max = 0;
	for (int i = 0; i < (*samples_t)->size; ++i)
	{
		max = fabs(result[i]) > max ? fabs(result[i]) : max;
	}

	// Normalize the waveform (16 bit)
	for (int i = 0; i < (*samples_t)->size; ++i)
	{
		float value = (pow(2, 14) * result[i] / max);
		(*samples_t)->data[i] = (int16_t)value;
	}

	// Free the memory allocated
	free(result);
	free(cfg);
	free(cfg_i);
}

// Testing 
int main()
{
	// Print the information of the wav file read
    int16_t *samples = NULL;
    wavread("c_chopped.wav", &samples);
    printf("No. of channels: %d\n",     header->num_channels);
    printf("Sample rate:     %d\n",     header->sample_rate);
    printf("Bit rate:        %dkbps\n", header->byte_rate*8 / 1000);
    printf("Bits per sample: %d\n\n",     header->bps);
    printf("Sample 0:        %d\n", samples[0]);
    printf("Sample 1:        %d\n", samples[1]);
    printf("Sample 2:        %d\n", samples[2]);

    // Create the Sample object to hold the waveform
    Sample *samples_c = (Sample*)malloc(sizeof(Sample));
    samples_c->data = samples;
    samples_c->size = header->datachunk_size / sizeof(int16_t);

    // Alternating output Sample waveform
    Sample *samples_t0 = NULL, *samples_t1 = NULL;   

    // Waveforms with different tones
	Sample *samples0 = NULL, *samples1 = NULL, *samples2 = NULL, *samples3 = NULL,
		   *samples4 = NULL, *samples5 = NULL, *samples6 = NULL, *samples7 = NULL, 
		   *samples8 = NULL;

    // Shift pitches to get different sounds
    pitchshift(&samples_c, &samples_t0, 0);
    pitchshift(&samples_c, &samples0, -1);
    pitchshift(&samples_c, &samples1, -2);  
    pitchshift(&samples_c, &samples2, -3);  
    pitchshift(&samples_c, &samples3, -4);  
    pitchshift(&samples_c, &samples4, -5);   
    pitchshift(&samples_c, &samples5, -6);   
    pitchshift(&samples_c, &samples6, -7);   
    pitchshift(&samples_c, &samples7, -8);   

    printf("Pitchshift done!\n");

    // Combine all the waveforms into one waveform
    superposition(&samples_t0, &samples0, &samples_t1, samples_t0->size - samples_c->size / 2);
    superposition(&samples_t1, &samples1, &samples_t0, samples_t1->size - samples_c->size / 2);
    superposition(&samples_t0, &samples2, &samples_t1, samples_t0->size - samples_c->size / 2);
    superposition(&samples_t1, &samples3, &samples_t0, samples_t1->size - samples_c->size / 2);
	superposition(&samples_t0, &samples4, &samples_t1, samples_t0->size - samples_c->size / 2);
	superposition(&samples_t1, &samples5, &samples_t0, samples_t1->size - samples_c->size / 2);
	superposition(&samples_t0, &samples6, &samples_t1, samples_t0->size - samples_c->size / 2);
	superposition(&samples_t1, &samples7, &samples_t0, samples_t1->size - samples_c->size / 2);

    // Write the result waveform into a wav file
    header->datachunk_size = samples_t0->size * sizeof(int16_t);
    wavwrite("track2.wav", samples_t0->data, samples_t0->size);

    // Free the memories
    free(header);
    free(samples_c->data);
    free(samples_c);
    free(samples_t0->data);
    free(samples_t0);
    free(samples_t1->data);
    free(samples_t1);
    free(samples0->data);
    free(samples0);
    free(samples1->data);
    free(samples1);    

    return 0;
}
