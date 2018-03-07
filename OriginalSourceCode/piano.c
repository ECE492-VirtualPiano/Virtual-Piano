/*
*********************************************************************************************************
*
*                                          PIANO CODE
*
*                                         CYCLONE V SOC
*
* Filename      : piano.c
* Version       : V1.00
* References    : Changes to this project include reference to:
* 				  		Title: "Pianoputer"
* 				  		Original Author: Zulko
* 				  		Date accessed: Jan 31, 2018
* 				  		https://github.com/Zulko/pianoputer
* Creation 		: January 30, 2018
* Created by 	: Mingjun Zhao (zhao2@ualberta.ca ), Daniel Tran (dtran3@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This file is mainly a sample based piano synthesizer which takes
				  in a sample piano sound waveform (Middle C) and transforms it into
				  other different piano sounds by changing its frequency and stretch
				  its length. Also it provides a way to mix the sounds by wave
				  superposition.
*********************************************************************************************************
*/

#include "piano.h"

void initFFT()
{
	if((cfg = kiss_fft_alloc(WINDOW_SIZE, 0, NULL, NULL)) == NULL ||
	   (cfg_i = kiss_fft_alloc(WINDOW_SIZE, 1, NULL, NULL)) == NULL)
	{
		printf("not enough memory?\n");
		exit(-1);
	}
}

void destroyFFT()
{
	free(cfg);
	free(cfg_i);
}

/*
	Name: 			void pitchshift(samples, samples_t, n)
	
	Description: 	Changes the pitch of a sound by "n" semitones.

	Inputs: 		
			Sample** 	inputSoundSample  The address of the input sample waveform
			int 		num_semitones	  The number of semitones

	Outputs:
			Sample**	outputSoundSample 		The address of the output sample waveform
*/
void pitchshift(Sample **inputSoundSample, Sample **outputSoundSample, int num_semitones)
{

	// The factor of frequency change in terms of semitones
	// The frequency doubles with one more octave (12 semitones) 
	float factor = pow(2.0f, (1.0f * num_semitones / 12.0f));

	int sizeOfSample = (*inputSoundSample)->size * factor + WINDOW_SIZE;

	short stretchedData[MAX_STRETCH_ARRAY_SIZE];

	Sample *stretchedInputSample = &(Sample) { .size = sizeOfSample, .data = stretchedData };

	// Stretch the wave by the reciprocal of the factor
	stretch(inputSoundSample, &stretchedInputSample, 1.0f / factor);

	// A temporary Sample variable to hold a part of the wave of 'stretched'

	// Change the frequency and the length of the wave by the factor
	// The wave after change will have the same length as the template wave
	speedx(&stretchedInputSample, outputSoundSample, factor);
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
void speedx(Sample **inputSoundSample, Sample **outputSoundSample, float factor)
{
	// The index of output data array
	int res_i = 0;

	// Find the indecies of the input array by incrementing by factor and put the 
	// corresponding data into the output array.
	//
	// Ex. if factor is 2. Put every other element from input array into the output
	// 	   array

	for (float step = 0; step < (*inputSoundSample)->size - WINDOW_SIZE; step += factor)
	{
		int i = (int)round(step);

		// Ensures the index never exceeding the limit
		if (res_i < (*outputSoundSample)->size)
		{
			if (i + WINDOW_SIZE < (*inputSoundSample)->size)
			{
				(*outputSoundSample)->data[res_i++] = (*inputSoundSample)->data[i + WINDOW_SIZE];
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
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
void superposition(Sample **inputSoundSample1, Sample **inputSoundSample2, Sample **outputSoundSample, int offset)
{
	// Free the memory of output Sample wave if not NULL
	if(*outputSoundSample)
	{
		free((*outputSoundSample)->data);
		free(*outputSoundSample);
	}

	// Allocate the memory for the output Sample wave
	*outputSoundSample = (Sample*)malloc(sizeof(Sample));

	// The length of output waveform
	// = start offset of the second wave
	// + Max(the length of the second waveform,
	//       the length of the first waveform - start offset of the second waveform)
	(*outputSoundSample)->size = offset + (((*inputSoundSample2)->size > (*inputSoundSample1)->size - offset) ? (*inputSoundSample2)->size : (*inputSoundSample1)->size - offset);
	(*outputSoundSample)->data = (short*)calloc((*outputSoundSample)->size, sizeof(short));

	// Combines the two input waveforms by superposition
	for (int i = 0; i < (*outputSoundSample)->size; i++)
	{
		(*outputSoundSample)->data[i] = 0;

		if (i < (*inputSoundSample1)->size)
		{
			(*outputSoundSample)->data[i] += (*inputSoundSample1)->data[i];
		}

		if (i >= offset && i < offset + (*inputSoundSample2)->size)
		{
			(*outputSoundSample)->data[i] += (*inputSoundSample2)->data[i - offset];
		}
	}
}


/*
	Name: 			void stretch(samples, samples_t, factor, WINDOW_SIZE, h)
	
	Description: 	Stretches / shortens a sound wave, by given factor

	Inputs: 		
			Sample** 	samples 		The address of the input waveform 
			float 		factor  		The stretch factor
			int 		WINDOW_SIZE		The window size used in FFT
			int 		h 				The h factor

	Outputs:
			Sample**	samples_t 		The address of the output sample waveform 
*/
void stretch(Sample **inputSoundSample, Sample **outputSoundSample, float factor)
{
	// Initialize the phase vector and the hanning window vector
	float phase[WINDOW_SIZE], hanning_window[WINDOW_SIZE];
	for (int i = 0; i < WINDOW_SIZE; ++i)
	{
		phase[i] = 0;

		// The formular for hanning window 
		hanning_window[i] = 0.5 * (1 - cos(2 * PI * i / (WINDOW_SIZE - 1)));
	}

	// The result array
	float result[MAX_TEMP_FLOAT_ARRAY_SIZE];

    // The classical phase vocoder process
    //
    // Break the sound into overlapping bits and rearrange these bits so that
    // they will overlap more (shortening) or less (stretching)
	for (float step = 0; step < (*inputSoundSample)->size - (WINDOW_SIZE + H); step += H * factor)
	{
		// Two potentially overlapping subarrays from the input array
		short a1[WINDOW_SIZE], a2[WINDOW_SIZE];

		int index = 0;
		for (int j = (int)step; j < (int)step + WINDOW_SIZE; ++j)
		{
			if (index < WINDOW_SIZE && j < (*inputSoundSample)->size && (j + H) < (*inputSoundSample)->size)
			{
				a1[index] = (*inputSoundSample)->data[j];
				a2[index] = (*inputSoundSample)->data[j + H];
				index++;
			}
			else {
				break;
			}
		}

		// The input and output complex arrays for FFT
	    kiss_fft_cpx s1_in[WINDOW_SIZE], s1_out[WINDOW_SIZE],
	    			 s2_in[WINDOW_SIZE], s2_out[WINDOW_SIZE];

	    for (int i = 0; i < WINDOW_SIZE; ++i)
	    {
	    	s1_in[i].r = a1[i] * hanning_window[i];
	    	s2_in[i].r = a2[i] * hanning_window[i];
	    }

	    // Resynchronize the second array on the first by taking the FFT of the 
	    // arrays and make adjustments so that they are in phase
	    kiss_fft(cfg, s1_in, s1_out);
	 	kiss_fft(cfg, s2_in, s2_out);

	    for (int i = 0; i < WINDOW_SIZE; ++i)
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

		kiss_fft_cpx a2_rephased[WINDOW_SIZE], s2_rephased[WINDOW_SIZE];

		// Changes the phase of s2 to be in phase with s1
		for (int i = 0; i < WINDOW_SIZE; ++i)
		{
			float abs_v = sqrt(pow(s2_out[i].r, 2) + pow(s2_out[i].i, 2));
			s2_rephased[i].r = abs_v * cos(phase[i]);
			s2_rephased[i].i = abs_v * sin(phase[i]); 
		}

		// Perform inverse FFT to get rephased a2 in time domain
		kiss_fft(cfg_i, s2_rephased, a2_rephased);

		// Add to result
		int i2 = (int)(step / factor);
		for (int i = 0; i < WINDOW_SIZE; ++i)
		{
			if ((i + i2) < MAX_TEMP_FLOAT_ARRAY_SIZE) 
			{
				result[i + i2] += hanning_window[i] * a2_rephased[i].r;
			}
			else 
			{
				break;
			}
		}
	}

	// Find the max absolute value of the waveform
	float max = 0;
	for (int i = 0; i < (*outputSoundSample)->size; ++i)
	{
		if (i < MAX_TEMP_FLOAT_ARRAY_SIZE) 
		{
			max = fabs(result[i]) > max ? fabs(result[i]) : max;
		}
		else 
		{
			break;
		}
	}

	// Normalize the waveform (16 bit)
	for (int i = 0; i < (*outputSoundSample)->size; ++i)
	{
		if (i < MAX_TEMP_FLOAT_ARRAY_SIZE)
		{
			float value = (pow(2, 12) * result[i] / max);
			(*outputSoundSample)->data[i] = (short)value;
		}
		else 
		{
			break;
		}
	}
}

/*
	Name: 			void generateSound(index, samples_t)

	Description: 	Generate a sound sample of the given index

	Inputs:
			int 		pianoKeyIndex 			The index of the piano key (1 to 88)

	Outputs:
			Sample* 	A sample waveform
			int * 		octaveKeyIndex			The key number within the octave
*/
Sample *sizeOfSound(int pianoKeyIndex, int *octaveKeyIndex)
{
	// Generate the sound from the closest prerecorded sample

	// For a sample closest to Sample C1
	if (pianoKeyIndex >= C1_LOW && pianoKeyIndex <= C1_HIGH)
	{
		// Get the key number in the first octave
		*octaveKeyIndex = pianoKeyIndex - C1;
		return SAMPLE_C1;
	}
	// For a sample closest to Sample C2
	else if (pianoKeyIndex >= C2_LOW && pianoKeyIndex <= C2_HIGH)
	{
		// Get the key number in the second octave
		*octaveKeyIndex = pianoKeyIndex - C2;
		return SAMPLE_C2;
	}
	// For a sample closest to Sample C3
	else if (pianoKeyIndex >= C3_LOW && pianoKeyIndex <= C3_HIGH)
	{
		// Get the key number in the third octave
		*octaveKeyIndex = pianoKeyIndex - C3;
		return SAMPLE_C3;
	}
	// For a sample closest to Sample C4
	else if (pianoKeyIndex >= C4_LOW && pianoKeyIndex <= C4_HIGH)
	{
		// Get the key number in the fourth octave
		*octaveKeyIndex = pianoKeyIndex - C4;
		return SAMPLE_C4;
	}
	// For a sample closest to Sample C5
	else if (pianoKeyIndex >= C5_LOW && pianoKeyIndex <= C5_HIGH)
	{
		// Get the key number in the fifth octave
		*octaveKeyIndex = pianoKeyIndex - C5;
		return SAMPLE_C5;
	}
	// For a sample closest to Sample C6
	else if (pianoKeyIndex >= C6_LOW && pianoKeyIndex <= C6_HIGH)
	{
		// Get the key number in the sixth octave
		*octaveKeyIndex = pianoKeyIndex - C6;
		return SAMPLE_C6;
	}
	// For a sample closest to Sample C7
	else if (pianoKeyIndex >= C7_LOW && pianoKeyIndex <= C7_HIGH)
	{
		// Get the key number in the seventh octave
		*octaveKeyIndex = pianoKeyIndex - C7;
		return SAMPLE_C7;
	}
	// For a sample closest to Sample C8
	else if (pianoKeyIndex >= C8_LOW && pianoKeyIndex <= C8_HIGH)
	{
		// Get the key number in the eighth octave
		*octaveKeyIndex = pianoKeyIndex - C8;
		return SAMPLE_C8;
	}
	return NULL;
}

/*
	Name: 			void generateSound(int pianoKeyIndex, Sample **outputSoundSample)

	Description: 	Generate a sound sample of the given index

	Inputs:
			int 		pianoKeyIndex		The index of the piano key (1 to 88)

	Outputs:
			Sample**	outputSoundSample 	The address of the output sample waveform
*/
void generateSound(int pianoKeyIndex, Sample **outputSoundSample)
{
	// Perform pitch shift on the Sample C1
	if (pianoKeyIndex >= C1_LOW && pianoKeyIndex <= C1_HIGH)
	{
		pitchshift(&SAMPLE_C1, outputSoundSample, pianoKeyIndex - C1);
	}
	// Perform pitch shift on the Sample C2
	else if (pianoKeyIndex >= C2_LOW && pianoKeyIndex <= C2_HIGH)
	{
		pitchshift(&SAMPLE_C2, outputSoundSample, pianoKeyIndex - C2);
	}
	// Perform pitch shift on the Sample C3
	else if (pianoKeyIndex >= C3_LOW && pianoKeyIndex <= C3_HIGH)
	{
		pitchshift(&SAMPLE_C3, outputSoundSample, pianoKeyIndex - C3);
	}
	// Perform pitch shift on the Sample C4
	else if (pianoKeyIndex >= C4_LOW && pianoKeyIndex <= C4_HIGH)
	{
		pitchshift(&SAMPLE_C4, outputSoundSample, pianoKeyIndex - C4);
	}
	// Perform pitch shift on the Sample C5
	else if (pianoKeyIndex >= C5_LOW && pianoKeyIndex <= C5_HIGH)
	{
		pitchshift(&SAMPLE_C5, outputSoundSample, pianoKeyIndex - C5);
	}
	// Perform pitch shift on the Sample C6
	else if (pianoKeyIndex >= C6_LOW && pianoKeyIndex <= C6_HIGH)
	{
		pitchshift(&SAMPLE_C6, outputSoundSample, pianoKeyIndex - C6);
	}
	// Perform pitch shift on the Sample C7
	else if (pianoKeyIndex >= C7_LOW && pianoKeyIndex <= C7_HIGH)
	{
		pitchshift(&SAMPLE_C7, outputSoundSample, pianoKeyIndex - C7);
	}
	// Perform pitch shift on the Sample C8
	else if (pianoKeyIndex >= C8_LOW && pianoKeyIndex <= C8_HIGH)
	{
		pitchshift(&SAMPLE_C8, outputSoundSample, pianoKeyIndex - C8);
	}
}
