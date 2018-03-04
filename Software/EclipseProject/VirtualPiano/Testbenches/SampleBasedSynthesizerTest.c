#include  <time.h>
#include  "../Synthesizer/piano.h"

#include  <os.h>

kiss_fft_cfg cfg, cfg_i;

void pitchshiftSpeedTest(Sample *inputSample, int semitone) 
{
	if (inputSample == NULL) 
	{
		inputSample = SAMPLE_C1;
	}

	if (semitone == -10) 
	{
		semitone = -5;
	}

	clock_t start_time, end_time;
	int i = 0, i_s= 1;

	Sample *samples_t = NULL;
	short sampleData[MAX_PITCH_SHIFT_OUTPUT_ARRAY_SIZE];

	samples_t = &(Sample) { .size = inputSample->size, .data = sampleData };

	start_time = clock();
	pitchshift(&inputSample, &samples_t, semitone);
	end_time = clock();

	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;
	printf("-----PITCH SHIFT TIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

void speedxSpeedTest(Sample *testSample, float factor)
{
	if (testSample == NULL) 
	{
		testSample = SAMPLE_C1;
	}

	// If factor is NULL, generate the longest speedx time possible, which 
	// is the smallest factor
	if (factor == 0) 
	{
		factor = pow(2.0f, (1.0f * -5 / 12.0f));
	}

	clock_t start_time, end_time;
	Sample *outputSample = NULL;
	start_time = clock();
	speedx(&SAMPLE_C1, &outputSample, 1);
	end_time = clock();

	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;
	printf("-----SPEEDX SHIFT TIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

void stretchSpeedTest(Sample *inputSample, float factor)
{
	if (inputSample == NULL)
	{
		inputSample = SAMPLE_C1;
	}

	if (factor == 0) 
	{
		factor = 1.0f / (pow(2.0f, (1.0f * -5 / 12.0f)));
	}

	clock_t start_time, end_time;
	Sample *outputSample = NULL;

	start_time = clock();
	stretch(&inputSample, &outputSample, factor);
	end_time = clock();

	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;
	printf("-----STRETCH SHIFT TIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

void superpositionSpeedTest(Sample *inputSample1, Sample *inputSample2, int offset) 
{
	if (inputSample1 == NULL) 
	{
		inputSample1 = SAMPLE_C1;
	}

	if (inputSample2 == NULL) {
		inputSample2 = SAMPLE_C1;
	}

	clock_t start_time, end_time;
	Sample *outputSample = NULL;

	start_time = clock();
	superposition(&inputSample1, &inputSample2, &outputSample, offset);
	end_time = clock();

	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;
	printf("-----SUPERPOSITION SHIFT TIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

void pitchshiftFrequencyTest() 
{
    Sample *inputSample = NULL;
    Sample *outputSample = NULL;

    int sampleSize = 10000;

    short outputData[sampleSize];
    outputSample->data = outputData;
    outputSample->size = sampleSize;

    short sinData[sampleSize];
    float x = 0.0f;

    short amplitude = 20;

    for (int i = 0; i < sampleSize; i++) 
    {
        sinData[i] = amplitude * sin(x);
        x += 0.1;
    }
    inputSample->data = sinData;
    inputSample->size = sampleSize;
    
    // 12 semitones should have 2x the frequency
    pitchshift(&inputSample, &outputSample, 12);

    int frequency = 0;

    for (int i = 0; i < sampleSize - 1; i++) 
    {
        if (outputSample->data[i] >= 0 && outputSample->data[i+1] < 0)
        {
            frequency++;
        }
    }
    printf("Original Sample Frequency: 159\n");
    printf("pitchShift Sample Frequency: %i\n", frequency);
}

void stretchFFT_Timing(INT8U task_prio, Sample **inputSoundSample, Sample **outputSoundSample, float factor)
{

	if ((0.00001 - factor) <= 0.00001 && (0.00001 - factor) >= 0.00001)
	{
		factor = 1.0f / (pow(2.0f, (1.0f * -5 / 12.0f)));
	}

	stackTest(task_prio);
    // Initialize the phase vector and the hanning window vector
	float phase[WINDOW_SIZE], hanning_window[WINDOW_SIZE];
	for (int i = 0; i < WINDOW_SIZE; ++i)
	{
		phase[i] = 0;

		// The formular for hanning window
		hanning_window[i] = 0.5 * (1 - cos(2 * PI * i / (WINDOW_SIZE - 1)));
	}

	// The result array
	// (Only works using dynamic memory allocation for some unknown reason)
	stackTest(task_prio);
	float result[MAX_TEMP_FLOAT_ARRAY_SIZE];
	stackTest(task_prio);
    // The classical phase vocoder process
    //
    // Break the sound into overlapping bits and rearrange these bits so that
    // they will overlap more (shortening) or less (stretching)
	for (float step = 0; step < (*inputSoundSample)->size - (WINDOW_SIZE + H); step += H * factor)
	{
		// Two potentially overllaping subarrays from the input array
		short a1[WINDOW_SIZE], a2[WINDOW_SIZE];

		int index = 0;
		for (int j = (int)step; j < (int)step + WINDOW_SIZE; ++j)
		{
			a1[index] = (*inputSoundSample)->data[j];
			a2[index] = (*inputSoundSample)->data[j + H];
			index++;
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
			result[i + i2] += hanning_window[i] * a2_rephased[i].r;
		}
		stackTest(task_prio);
	}

	// Find the max absolute value of the waveform
	float max = 0;
	for (int i = 0; i < (*outputSoundSample)->size; ++i)
	{
		max = fabs(result[i]) > max ? fabs(result[i]) : max;
	}

	// Normalize the waveform (16 bit)
	for (int i = 0; i < (*outputSoundSample)->size; ++i)
	{
		float value = (pow(2, 12) * result[i] / max);
		(*outputSoundSample)->data[i] = (short)value;
	}
	stackTest(task_prio);
}

void pitchshiftStackUsageTest(INT8U task_prio, Sample **inputSample, Sample **outputSoundSample, int semitone)
{

	stackTest(task_prio);
	if (inputSample == NULL)
	{
		inputSample = SAMPLE_C1;
	}

	if (semitone == -10)
	{
		semitone = -5;
	}
// The factor of frequency change in terms of semitones
	// The frequency doubles with one more octave (12 semitones)
	float factor = pow(2.0f, (1.0f * semitone / 12.0f));

	int sizeOfSample = (*inputSample)->size * factor + WINDOW_SIZE;

	short stretchedData[MAX_STRETCH_ARRAY_SIZE];

	Sample *stretchedInputSample = &(Sample) { .size = sizeOfSample, .data = stretchedData };

	// Stretch the wave by the reciprocal of the factor
	stretch(inputSample, &stretchedInputSample, 1.0f / factor);

	// Change the frequency and the length of the wave by the factor
	// The wave after change will have the same length as the template wave
	speedx(&stretchedInputSample, outputSoundSample, factor);

	stackTest(task_prio);
}

void stackTest(INT8U task_prio)
{
	OS_STK_DATA data;
	INT8U os_err;

	os_err = OSTaskStkChk(task_prio, &data);

	if (os_err == OS_ERR_NONE)
	{
		printf("Stack For Task %i: %4ld    %4ld    %4ld\n", task_prio, data.OSFree + data.OSUsed, data.OSFree, data.OSUsed);
	}
}
