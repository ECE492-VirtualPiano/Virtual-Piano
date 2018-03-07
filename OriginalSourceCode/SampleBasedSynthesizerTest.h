/*
*********************************************************************************************************
*
*                             SAMPLE-BASED SYNTHESIZER TESTBENCH  HEADER CODE
*
*                                            CYCLONE V SOC
*
* Filename      : SampleBasedSynthesizerTest.h
* Version       : V1.00
* Creation 		: February 26, 2018
* Created by 	: Mingjun Zhao (zhao2@ualberta.ca ), Daniel Tran (dtran3@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This file is used to test the functionality of the sample-based synthesizer code.
				  The functions in this testbench test the speed of each of the core functions in the 
				  synthesizer, as well as provides inside information on the state of the stack before 
				  and after function execution.
*********************************************************************************************************
*/

#include  <time.h>
#include  "../Synthesizer/piano.h"

#include  <os.h>

void pitchshiftSpeedTest(Sample *inputSample, int semitone);
void speedxSpeedTest(Sample *testSample, float factor);
void stretchSpeedTest(Sample *inputSample, float factor);
void superpositionSpeedTest(Sample *inputSample1, Sample *inputSample2, int offset);
void pitchshiftFrequencyTest(int num_waveform_data_points, float step, short amplitude);
void pitchshiftStackUsageTest(INT8U task_prio, Sample **inputSample, Sample **outputSoundSample, int semitone);
void stackTest(INT8U task_prio);
void testSampleBasedSynthesizer(INT8U task_prio);
