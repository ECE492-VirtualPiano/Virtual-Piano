/*
*********************************************************************************************************
*
*                               SAMPLE-BASED SYNTHESIZER TESTBENCH CODE
*
*                                         CYCLONE V SOC
*
* Filename      : SampleBasedSynthesizerTest.c
* Version       : V1.00
* Programmer(s) : Mingjun Zhao (zhao2@ualberta.ca ), Daniel Tran (dtran3@ualberta.ca)
* References    : 
*********************************************************************************************************
* Note(s)       : This file is used to test the functionality of the sample-based synthesizer code.
				  The functions in this testbench test the speed of each of the core functions in the 
				  synthesizer, as well as provides inside information on the state of the stack before 
				  and after function execution.
*********************************************************************************************************
*/

#include "SampleBasedSynthesizerTest.h"

kiss_fft_cfg cfg, cfg_i;


/*
	Name: 			void pitchshiftSpeedTest(Sample *inputSample, int semitone) 
	
	Description: 	Tests the speed of the pitchshift function from piano.c

	Inputs: 		
			Sample* 	inputSample  	The input sample waveform
			int 		semitone		The number of semitones

	Outputs:
			Prints the execution time of the pitchshift function
*/
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

/*
	Name: 			void speedxSpeedTest(Sample *testSample, float factor) 
	
	Description: 	Tests the speed of the speedx function from piano.c

	Inputs: 		
			Sample* 	testSample  	The input sample waveform
			int 		factor			The scaling factor of the wave

	Outputs:
			Prints the execution time of the speedx function
*/
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

/*
	Name: 			void stretchSpeedTest(Sample *inputSample, float factor) 
	
	Description: 	Tests the speed of the stretch function from piano.c

	Inputs: 		
			Sample* 	inputSample  	The input sample waveform
			int 		semitone		The stretching factor for the waveform

	Outputs:
			Prints the execution time of the stretch function
*/
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

/*
	Name: 			void superpositionSpeedTest(Sample *inputSample1, Sample *inputSample2, int offset)  
	
	Description: 	Tests the speed of the superposition function from piano.c

	Inputs: 		
			Sample* 	inputSample1  	The first input sample waveform
			Sample* 	inputSample2  	The second input sample waveform
			int 		offset			The offset that the waves will be superpositioned by

	Outputs:
			Prints the execution time of the superposition function
*/
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

/*
	Name: 			void pitchshiftFrequencyTest()
	
	Description: 	Creates a sine wave and passes the sine wave through the pitchshifting function
					that will double the input waveform. The output waveform will be checked to ensure 
					that it is the double the frequency.

	Inputs: 		
			int		 	num_waveform_data_points  	The number of datapoints that the test waveform will contain
			int 		step						The stepping rate through the waveform

	Outputs:
			Prints the frequency of the original sine wave and the frequency of the shifted sin wave
*/
void pitchshiftFrequencyTest(int num_waveform_data_points, int step) 
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

/*
	Name: 			void pitchshiftStackUsageTest(INT8U task_prio, Sample **inputSample, Sample **outputSoundSample, int semitone)
	
	Description: 	Tests the speed of the pitchshift function from piano.c

	Inputs: 		
			Sample** 	inputSample  	The input sample waveform
			int 		semitone		The number of semitones

	Outputs:
			Execution time of the pitchshift function
*/
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

/*
	Name: 			void pitchshiftSpeedTest(Sample *inputSample, int semitone) 
	
	Description: 	Tests the speed of the pitchshift function from piano.c

	Inputs: 		
			Sample** 	inputSample  	The input sample waveform
			int 		semitone		The number of semitones

	Outputs:
			Execution time of the pitchshift function
*/
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

/*
	Name: 			void pitchshiftSpeedTest(Sample *inputSample, int semitone) 
	
	Description: 	Tests the speed of the pitchshift function from piano.c

	Inputs: 		
			Sample** 	inputSample  	The input sample waveform
			int 		semitone		The number of semitones

	Outputs:
			Execution time of the pitchshift function
*/
void testSampleBasedSynthesizer() 
{
	Sample *samples_t1 = NULL;
	short sampleData1[MAX_PITCH_SHIFT_OUTPUT_ARRAY_SIZE];
	int tempIndex1 = 0;
	int *index1 = &tempIndex1;
	Sample *tempSample11 = sizeOfSound(1, index1);
	samples_t1 = &(Sample) { .size = tempSample11->size, .data = sampleData1 };
	pitchshiftStackUsageTest(GENERATE_SOUND_TASK_PRIO, &tempSample11, &samples_t1, -5);
	pitchshiftSpeedTest(NULL, -10);
	speedxSpeedTest(NULL, 0);
	stretchSpeedTest(NULL, 0);
	superpositionSpeedTest(NULL, NULL, 0);
	pitchshiftFrequencyTest();

	stretchFFT_Timing(GENERATE_SOUND_TASK_PRIO, &tempSample11, &samples_t1, 0);

	os_err = OSTaskStkChk(GENERATE_SOUND_TASK_PRIO, &data);

	if (os_err == OS_ERR_NONE)
	{
		printf("%4ld    %4ld    %4ld\n", data.OSFree + data.OSUsed, data.OSFree, data.OSUsed);
	}

	int i = 0, i_s= 1;
}