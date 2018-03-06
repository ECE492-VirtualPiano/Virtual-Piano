/*
*********************************************************************************************************
*
*                               SAMPLE-BASED SYNTHESIZER TESTBENCH CODE
*
*                                         CYCLONE V SOC
*
* Filename      : SampleBasedSynthesizerTest.c
* Version       : V1.00
* References    : 
* Creation: February 26, 2018
* Created by: Mingjun Zhao (zhao2@ualberta.ca ), Daniel Tran (dtran3@ualberta.ca)
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
	// If inputSample is NULL, assign it SAMPLE_C1
	if (inputSample == NULL) 
	{
		inputSample = SAMPLE_C1;
	}

	// Initialize clock variables for timing
	clock_t start_time, end_time;
	int i = 0, i_s= 1;

	// Create the output Samples element to contain the output data from 
	// the pitchshift function
	Sample *samples_t = NULL;
	short sampleData[MAX_PITCH_SHIFT_OUTPUT_ARRAY_SIZE];

	samples_t = &(Sample) { .size = inputSample->size, .data = sampleData };

	// Get the time of the clock before running the pitchshift function
	start_time = clock();
	// Run the pitchshift function for timing
	pitchshift(&inputSample, &samples_t, semitone);
	// Get the time after the pitchshift function is ran
	end_time = clock();

	// Get the time difference between the start and end of the pitchshift function 
	// call and convert it to milliseconds instead of clock cylces.
	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;

	// Print the time it takes to run the pitchshift function
	printf("-----PITCH SHIFT RUNTIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

/*
	Name: 			void speedxSpeedTest(Sample *testSample, float factor) 
	
	Description: 	Tests the speed of the speedx function from piano.c

	Inputs: 		
			Sample* 	testSample  	The input sample waveform
			int 		factor			The scaling factor of the wave
										Based on factor = pow(2.0f, (1.0f * num_semitones / 12.0f))
										(If factor == 0: factor = pow(2.0f, (1.0f * -5 / 12.0f)))

	Outputs:
			Prints the execution time of the speedx function
*/
void speedxSpeedTest(Sample *testSample, float factor)
{
	// If inputSample is NULL, assign it SAMPLE_C1
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

	// Initialize clock variables for timing
	clock_t start_time, end_time;

	// Initialize the output Sample variable to contain the Sample after performing speedx
	Sample *outputSample = NULL;

	// Get the time of the clock before running the speedx function
	start_time = clock();
	// Run the speedx function for timing
	speedx(&SAMPLE_C1, &outputSample, 1);
	// Get the time after the speedx function is ran
	end_time = clock();

	// Get the time difference between the start and end of the speedx function 
	// call and convert it to milliseconds instead of clock cylces.
	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;

	// Print the time it takes to run the speedx function	
	printf("-----SPEEDX RUNTIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

/*
	Name: 			void stretchSpeedTest(Sample *inputSample, float factor) 
	
	Description: 	Tests the speed of the stretch function from piano.c

	Inputs: 		
			Sample* 	inputSample  	The input sample waveform
			int 		semitone		The stretching factor for the waveform
										Based on pow(2.0f, (1.0f * num_semitones / 12.0f))
										(If semitone == 0: factor = pow(2.0f, (1.0f * -5 / 12.0f)))

	Outputs:
			Prints the execution time of the stretch function
*/
void stretchSpeedTest(Sample *inputSample, float factor)
{
	// If inputSample is NULL, assign it SAMPLE_C1
	if (inputSample == NULL)
	{
		inputSample = SAMPLE_C1;
	}

	// If factor is 0, then generate the factor based on -5 semitones
	if (factor == 0) 
	{
		factor = 1.0f / (pow(2.0f, (1.0f * -5 / 12.0f)));
	}

	// Initialize clock variables for timing
	clock_t start_time, end_time;

	// Initialize the output Sample variable to contain the Sample after performing stretch
	Sample *outputSample = NULL;

	// Get the time of the clock before running the stretch function
	start_time = clock();
	// Run the stretch function for timing
	stretch(&inputSample, &outputSample, factor);
	// Get the time after the stretch function is ran
	end_time = clock();

	// Get the time difference between the start and end of the stretch function 
	// call and convert it to milliseconds instead of clock cylces.
	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;

	// Print the time it takes to run the stretch function		
	printf("-----STRETCH RUNTIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

/*
	Name: 			void superpositionSpeedTest(Sample *inputSample1, Sample *inputSample2, int offset)  
	
	Description: 	Tests the speed of the superposition function from piano.c

	Inputs: 		
			Sample* 	inputSample1  	The first input sample waveform
										(If inputSample1 == NULL: inputSample1 = SAMPLE_C1)
			Sample* 	inputSample2  	The second input sample waveform
										(If inputSample2 == NULL: inputSample1 = SAMPLE_C1)
			int 		offset			The offset that the waves will be superpositioned by

	Outputs:
			Prints the execution time of the superposition function
*/
void superpositionSpeedTest(Sample *inputSample1, Sample *inputSample2, int offset) 
{
	// If inputSample1 is NULL, assign it SAMPLE_C1
	if (inputSample1 == NULL) 
	{
		inputSample1 = SAMPLE_C1;
	}

	// If inputSample2 is NULL, assign it SAMPLE_C1
	if (inputSample2 == NULL) {
		inputSample2 = SAMPLE_C1;
	}

	// Initialize clock variables for timing
	clock_t start_time, end_time;

	// Initialize the output Sample variable to contain the Sample after performing superposition
	Sample *outputSample = NULL;

	// Get the time of the clock before running the superposition function
	start_time = clock();
	// Run the superposition function for timing
	superposition(&inputSample1, &inputSample2, &outputSample, offset);
	// Get the time after the superposition function is ran
	end_time = clock();

	// Get the time difference between the start and end of the superposition function 
	// call and convert it to milliseconds instead of clock cylces.
	float time_diff = ((float)(end_time - start_time) / CLOCKS_PER_SEC ) * 1000;

	// Print the time it takes to run the superposition function			
	printf("-----SUPERPOSITION RUNTIME-----\n");
	printf("Time: %f milliseconds\n", time_diff);
}

/*
	Name: 			void pitchshiftFrequencyTest(int num_waveform_data_points, int step)
	
	Description: 	Creates a sine wave and passes the sine wave through the pitchshifting function
					that will double the input waveform. The output waveform will be checked to ensure 
					that it is the double the frequency.

	Inputs: 		
			int		 	num_waveform_data_points  	The number of datapoints that the test waveform will contain
			int 		step						The stepping rate through the waveform
			short 		amplitude					The amplitude of the input sine wave

	Outputs:
			Prints the frequency of the original sine wave and the frequency of the shifted sin wave
*/
void pitchshiftFrequencyTest(int num_waveform_data_points, int step, short amplitude) 
{
	// Initialize the input and output sample variables
    Sample *inputSample = NULL;
    Sample *outputSample = NULL;

	// short array to contain the data of the generated sine wave
	short sinData[num_waveform_data_points];
	// Point the inputSample data to the inputData array
	inputSample->data = sinData;
	// Modify the size element of the sample
    inputSample->size = num_waveform_data_points;

	// short array to contain the data of the shifted sine wave
    short outputData[num_waveform_data_points];
	// Point the outputSample data to the outputData array
    outputSample->data = outputData;
	// Modify the size element of the sample
    outputSample->size = num_waveform_data_points;

	// The x position of the sine wave
    float x = 0.0f;

	// The frequency of the generated sine wave
	int generated_frequency = 0;

	// The previous sine point (amplitude * sin(0) = 0) and store the same value into the input data array
	short prev_point = 0;
	sinData[0] = 0;
    
	// Loop through to traverse the sine wave and store the points
	for (int i = 1; i < num_waveform_data_points; i++) 
    {
		// Generate and store the point on the sine wave
        sinData[i] = amplitude * sin(x);

		// Check if there is a transition from positive to negative on the curve, if so increase
		// the frequency
		if (sinData->data[i-1] >= 0 && sinData->data[i] < 0)
        {
            generated_frequency++;
        }

		// Increase the x value
        x += step;
    }

    // 12 semitones should have 2x the frequency
    pitchshift(&inputSample, &outputSample, 12);

	// The frequency of the pitchshifted sine wave
    int frequency = 0;

	// Loop through the modified sine wave to get the frequency
    for (int i = 0; i < num_waveform_data_points - 1; i++) 
    {
        if (outputSample->data[i] >= 0 && outputSample->data[i+1] < 0)
        {
            frequency++;
        }
    }
	if (generated_frequency == frequency) 
	{
		printf("PASS: The pitchShift frequency is double the original sample frequency");
	}
	else
	{
		printf("FAIL: The pitchShift frequency is not double the original sample frequency");
	}

    printf("Original Sample Frequency: %i\n", generated_frequency);
    printf("pitchShift Sample Frequency: %i\n", frequency);
}

/*
	Name: 			void pitchshiftStackUsageTest(INT8U task_prio, Sample **inputSample, Sample **outputSoundSample, int semitone)
	
	Description: 	Tests the speed of the pitchshift function from piano.c

	Inputs: 		
			INT8U 		tesk_prio  		The priority of the task to test the function in
										Implementation is in a task due to its different interaction with the piano functions
										in comparison to the main thread.
			Sample**	inputSample		An input sample. If NULL, SAMPLE_C1 is used
			Sample** 	outputSample	The output sample from the speedx method
			int			semitone		The number of semitones to shift the sample by

	Outputs:
			Prints the status of the stack at the beginning and end of the pitchshift function
*/
void pitchshiftStackUsageTest(INT8U task_prio, Sample **inputSample, Sample **outputSample, int semitone)
{
	// Output the status for the stack of the task with priority task_prio
	stackTest(task_prio);

	// If inputSample is NULL, assign it SAMPLE_C1
	if (inputSample == NULL)
	{
		inputSample = SAMPLE_C1;
	}

	// The factor of frequency change in terms of semitones
	// The frequency doubles with one more octave (12 semitones)
	float factor = pow(2.0f, (1.0f * semitone / 12.0f));

	// Get the size of the sample
	int sizeOfSample = (*inputSample)->size * factor + WINDOW_SIZE;

	// Create a data array to gold the data from the stretching function
	short stretchedData[MAX_STRETCH_ARRAY_SIZE];

	// Create a sample and point its data to the stretchedData array
	Sample *stretchedInputSample = &(Sample) { .size = sizeOfSample, .data = stretchedData };

	// Stretch the wave by the reciprocal of the factor
	stretch(inputSample, &stretchedInputSample, 1.0f / factor);

	// Change the frequency and the length of the wave by the factor
	// The wave after change will have the same length as the template wave
	speedx(&stretchedInputSample, outputSample, factor);

	// Output the status for the stack of the task with priority task_prio
	stackTest(task_prio);
}

/*
	Name: 			void stackTest(INT8U task_prio) 
	
	Description: 	Prints the status of the stack for a given task

	Inputs: 		
			INT8U 		task_prio		The priority of the task in which the stack will be checked

	Outputs:
			Prints the status of the stack for a given task
*/
void stackTest(INT8U task_prio)
{
	// Contains the data for the stack
	OS_STK_DATA data;

	// Holds the error code from the OSTaskSTkChk function
	INT8U os_err;

	// Get the status of the task for the task with priority task_prio
	os_err = OSTaskStkChk(task_prio, &data);

	// If there is no error, print out the status of the stack
	if (os_err == OS_ERR_NONE)
	{
		printf("Stack For Task %i: %4ld    %4ld    %4ld\n", task_prio, data.OSFree + data.OSUsed, data.OSFree, data.OSUsed);
	}
}

/*
	Name: 			void testSampleBasedSynthesizer() 
	
	Description: 	Contains all of the tests above in one function
*/
void testSampleBasedSynthesizer() 
{
	// Initialize a sample to contain the transformed sample
	Sample *samples_t = NULL;
	short sampleData1[MAX_PITCH_SHIFT_OUTPUT_ARRAY_SIZE];
	// Get a sample sound wave and its index
	int tempIndex = 0;
	int *index = &tempIndex;
	Sample *tempSample = sizeOfSound(1, index1);
	samples_t = &(Sample) { .size = tempSample->size, .data = sampleData };

	// Test the stack usage for the pitchshift function
	pitchshiftStackUsageTest(GENERATE_SOUND_TASK_PRIO, &tempSample, &samples_t, -5);
	
	// Test the runtime of the pitchshift function
	pitchshiftSpeedTest(NULL, -5);
	
	// Test the runtime of the speedx function
	speedxSpeedTest(NULL, 0);

	// Test the runtime of the stretch function
	stretchSpeedTest(NULL, 0);

	// Test the runtime of the superposition function
	superpositionSpeedTest(NULL, NULL, 0);

	// Test the correctness of the frequency from the pitchshift function
	pitchshiftFrequencyTest(10000, 0.1, 20);
}