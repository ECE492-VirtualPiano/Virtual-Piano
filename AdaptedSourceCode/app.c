/*
*********************************************************************************************************
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2009-2014; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          APPLICATION CODE
*
*                                            CYCLONE V SOC
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JBL
* Modifications	: Nancy Minderman nancy.minderman@ualberta.ca, Brendan Bruner bbruner@ualberta.ca
* 				  Changes to this project include scatter file changes and BSP changes for port from
* 				  Cyclone V dev kit board to DE1-SoC
*
* 				  Group 7: Kevin Wong, Mingjun Zhao, Daniel Tran, and Alvin Huang.
* 				  Changes to this project include referenced and modified code from:
* 				  		Title: "VGA display of video input using a bus_master to copy input image"
* 				  		Original Author: Bruce Land (bruce.land@cornell.edu)
* 				  		Date accessed: Feb 2, 2018
* 				  		Used with Permission from Author
* 				  		http://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Bus_master_slave_index.html
*
*********************************************************************************************************
* Note(s)       : Main program
*
* Copyright     : Copyright Cornell University February 28, 2018
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

// Memory and App Config Headers
#include <address_map_arm_brl4.h>
#include  <app_cfg.h>
#include  <lib_mem.h>

// Board Support Libraries
#include  <bsp.h>
#include  <bsp_int.h>
#include  <bsp_os.h>

// CPU Libraries
#include  <cpu.h>
#include  <cpu_core.h>
#include  <cpu_cache.h>

// OS, HPS, and Hardware Libraries
#include  <os.h>
#include  <hps.h>
#include  <socal.h>
#include  <hwlib.h>

// Timing Library
#include  "timer.h"

// Audio Synthesizer Libraries
#include  "../Audio/audio.h"
#include  "../Audio/audio_cfg.h"
#include  "../Synthesizer/piano.h"

// Video Processing Libraries
#include  <ucos_ii.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <alt_16550_uart.h>
#include  <string.h>
#include  "../Video/video.h"


// Compute absolute address of any slave component attached to lightweight bridge
// base is address of component in QSYS window
// This computation only works for slave components attached to the lightweight bridge
// base should be ranged checked from 0x0 - 0x1fffff
#define FPGA_TO_HPS_LW_ADDR(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

#define SWITCH_ADDR 0X40
#define SWITCH_BASE	FPGA_TO_HPS_LW_ADDR(SWITCH_ADDR)

// App Priority
#define APP_TASK_PRIO 5
#define VIDEO_PROCESS_TASK_PRIO 6
#define PLAY_SOUND_TASK_PRIO 7

// Task Size
#define TASK_STACK_SIZE 4096
#define AUDIO_TASK_STACK_SIZE 16384
#define VIDEO_TASK_STACK_SIZE 8192

// Switch mask
#define SW9 0x200

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

// Initialize Sample arrays
short SAMPLE_DATA[NUM_KEYS+10][200000];
extern short CIRCULAR_BUFFER[160000];
extern int CB_INDEX;

// Task Stacks
CPU_STK AppTaskStartStk[TASK_STACK_SIZE];
CPU_STK VideoProcessTaskStk[VIDEO_TASK_STACK_SIZE];
CPU_STK PlaySoundTaskStk[AUDIO_TASK_STACK_SIZE];

// Screen Buffer
short SCREEN_BUFFER[VGA_AREA];

// The Light Weight Video-In Controller
volatile unsigned int *h2p_lw_video_in_control_addr = NULL;
volatile unsigned int *h2p_lw_video_in_resolution_addr = NULL;
volatile unsigned int *h2p_lw_video_edge_control_addr = NULL;

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

// Task Processes
static void WatchDogFeederTask (void *p_arg);
static void VideoProcessTask (void *p_arg);

// Helper functions
void GenerateSamples(void);

// The following functions using the global variable SCREEN_BUFFER must be
// declared in app.c.
// An extern variable from other source files will cause memory issues and
// the program crashes.
void CheckKeys(int *current_playing_keys);
int CheckArea(int start_x, int end_x, int start_y, int end_y);
void PaintArea(int start_x, int end_x, int start_y, int end_y, int color);
void DrawVerticalLine(int start_x, int start_y, int length, int color);
void DrawHorizontalLine(int start_x, int start_y, int length, int color);

/*
*********************************************************************************************************
*                                               main()
*
* Description : Entry point for C code. Set Control Registers and configure system.
*
* Arguments   : none.
*
* Returns     : none.
*
* Note(s)     : (1) It is assumed that your code will call main() once you have performed all necessary
*                   initialization.
*********************************************************************************************************
*/
int main ()
{

	// Error Check
    INT8U os_err;

    // Reset the watchdog as soon as possible.
    // Scatter loading is complete. Now the caches can be activated.
    BSP_WatchDog_Reset();

    // Enable branch prediction.
    BSP_BranchPredictorEn();

    // Configure the L2 cache controller.
    BSP_L2C310Config();

    // Enable L1 I&D caches + L2 unified cache
    BSP_CachesEn();


    // Initalize CPU, Memory, Board Support, and OS
    CPU_Init();
    Mem_Init();
    BSP_Init();
    OSInit();

    // Initialize FFT
    initFFT();

    // Audio Configuration
    write_audio_cfg_register(LEFT_LINE_IN, LINE_IN_HIGH_VOLUME); // Set Left Line in LINVOL (Volume)
	write_audio_cfg_register(RIGHT_LINE_IN, LINE_IN_HIGH_VOLUME); // Set Right Line in LINVOL (Volume)
	write_audio_cfg_register(LEFT_HEADPHONE_OUT, HEADPHONE_HIGH_VOLUME); // Set Left Headphone Out LHPVOL (Volume)
	write_audio_cfg_register(RIGHT_HEADPHONE_OUT, HEADPHONE_HIGH_VOLUME); // Set Right Headphone Out RHPVOL (Volume)
	write_audio_cfg_register(ANALOG_AUDIO_PATH, DIGITAL_AUDIO_CONVERTER); // Bits 2, 3, and 4 Corresponding to selecting MIC/LINEIn (0/1), LINE-IN BYPASS, DAC output, MIC BYPASS respectively
	write_audio_cfg_register(DIGITAL_AUDIO_PATH, HIGH_PASS); // Set Digital Audio Path Control
	write_audio_cfg_register(POWER_DOWN, POWER_ON); // Set Power Down Control (All 0)
	write_audio_cfg_register(DIGITAL_AUDIO_INTERFACE, LEFT_JUSTIFIED); // Set Digital Audio Interface Format
	write_audio_cfg_register(SAMPLING_CONTROL, NORMAL_MODE); // Set Sampling Control (Sampling Rate. Refer to Table 18 Normal Mode Sample Rate Look-up)
	write_audio_cfg_register(ACTIVE_CONTROL, ACTIVE); // Set Active Control

	// The Light Weight Video-In Controller Base Addresses + Offset
	h2p_lw_video_in_control_addr = (volatile unsigned int *) (FPGA_TO_HPS_LW_ADDR(VIDEO_IN_BASE) + CONTROL_OFFSET);
	h2p_lw_video_in_resolution_addr = (volatile unsigned int *) (FPGA_TO_HPS_LW_ADDR(VIDEO_IN_BASE) + RESOLUTION_OFFSET);
	h2p_lw_video_edge_control_addr= (volatile unsigned int *) (FPGA_TO_HPS_LW_ADDR(VIDEO_IN_BASE) + EDGE_OFFSET);

	// Video In Configuration
	*(h2p_lw_video_in_resolution_addr) = NEW_VIDEO_DIMENSION;
	*(h2p_lw_video_in_control_addr) = VIDEO_ON; // Turn On Video Capture
	*h2p_lw_video_edge_control_addr = EDGE_ON; // 1 means edges
	*h2p_lw_video_edge_control_addr = EDGE_OFF; // 1 means edges

	// Create the start task.
	// Watchdog Timer Loop
	os_err = OSTaskCreateExt((void (*)(void *)) WatchDogFeederTask,
								 (void          * ) 0,
								 (OS_STK        * )&AppTaskStartStk[TASK_STACK_SIZE - 1],
								 (INT8U           ) APP_TASK_PRIO,
								 (INT16U          ) APP_TASK_PRIO,
								 (OS_STK        * )&AppTaskStartStk[0],
								 (INT32U          ) TASK_STACK_SIZE,
								 (void          * )0,
								 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	if (os_err != OS_ERR_NONE) {
		exit(0); /* Handle error. */
	}

	// Create the Video task.
	// Processes Video frames
	os_err = OSTaskCreateExt((void (*)(void *)) VideoProcessTask,
	                                 (void          * ) 0,
	                                 (OS_STK        * )&VideoProcessTaskStk[VIDEO_TASK_STACK_SIZE - 1],
	                                 (INT8U           ) VIDEO_PROCESS_TASK_PRIO,
	                                 (INT16U          ) VIDEO_PROCESS_TASK_PRIO,
	                                 (OS_STK        * )&VideoProcessTaskStk[0],
	                                 (INT32U          ) VIDEO_TASK_STACK_SIZE,
	                                 (void          * )0,
	                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

	if (os_err != OS_ERR_NONE) {
		exit(0); /* Handle error. */
	}

	// CPU Initalize/Config
	CPU_IntEn();

	// Start Multitasking
	OSStart();

}

/*
*********************************************************************************************************
*                                           WatchDogFeederTask()
*
* Description : Watchdog Initialization and Reset Loop
*
* Arguments   : p_arg       Argument passed by 'OSTaskCreate()'.
*
* Returns     : none.
*
* Created by  : main().
*
* Notes       : (1) The ticker MUST be initialised AFTER multitasking has started.
*********************************************************************************************************
*/

static  void  WatchDogFeederTask (void *p_arg)
{
	// Configure and enable OS tick interrupt.
    BSP_OS_TmrTickInit(OS_TICKS_PER_SEC);

    // Loop Forever
    for(;;) {

    	// Reset the watchdog.
    	BSP_WatchDog_Reset();

    	// Delay
		OSTimeDlyHMSM(0, 0, 0, 500);

		// Light on
		BSP_LED_On();

		// Delay
		OSTimeDlyHMSM(0, 0, 0, 500);

		// Light off
		BSP_LED_Off();

    }

}

/*
*********************************************************************************************************
*                                           VideoProcessTask()
*
* Description : Video Processing Task.
*
* Arguments   : p_arg       Argument passed by 'OSTaskCreate()'.
*
* Returns     : none.
*
* Created by  : main().
*
* Notes       : (1) The ticker MUST be initialised AFTER multitasking has started.
*********************************************************************************************************
*/
static  void  VideoProcessTask (void *p_arg)
{
	// Video Input Index
	GenerateSamples();

	// Initialize the timer interrupt
	InitFPGATimerInterrupt();

	// VGA Pixel Buffer
	volatile unsigned int * vga_pixel_ptr = SDRAM_BASE;

	// Video Input Buffer
	volatile unsigned int * video_in_ptr = FPGA_ONCHIP_BASE;

	// Clear the screen
	VGA_box (vga_pixel_ptr, 0, 0, 639, 479, COLOR_BLUE);

	// Copy current screen
	memcpy(SCREEN_BUFFER, vga_pixel_ptr, VGA_AREA * sizeof(short *));

	// The array indicating the current playing keys
	int current_playing_keys[NUM_KEYS];
	// Clear the array
	for (int i; i < NUM_KEYS; i++)
	{
		current_playing_keys[i] = 0;
	}

	volatile int * audio_ptr = (int *) AUDIO_BASE;

	// Loop Forever
    for(;;) {

    	// Check the key status
		CheckKeys(current_playing_keys);

		// Loop through each playing keys
		for (int i = 0; i < NUM_KEYS; i++)
		{
			if (current_playing_keys[i] == KEY_PRESSED)
			{
				// Get the sample data of the index
				int octaveKeyIndex;
				Sample *sample = sizeOfSound(i+1, &octaveKeyIndex); // samples are from 1 to 61

				// Put the sample data into the circular buffer
				for (int j = 0; j < sample->size; j++)
				{
					int index = (CB_INDEX + j) % 160000;

					// Superposition with existing data in circular buffer
					CIRCULAR_BUFFER[index] += SAMPLE_DATA[NUM_KEYS + 5 - i][j];
				}
			}
		}

		// Read the state of the switch
		int switch_state = alt_read_word(SWITCH_BASE) & SW9;
		if (switch_state)
		{
			// Memory Copy Screen Buffer to VGA
			memcpy(vga_pixel_ptr, SCREEN_BUFFER, VGA_AREA * sizeof(short *));

			// Delay
			OSTimeDlyHMSM(0, 0, 0, 500);
		}
    }

}

/*
	Name: 			void GenerateSamples(void)

	Description: 	Generate all piano notes using the prerecorded samples
*/
void GenerateSamples()
{
	int j = 0;

	// NOTICE:
	//
	// 1. For some unknown reason this loop only works decrementing
	//
	// 2. The loop has a weird bug that the last a few samples in SAMPLE_DATA are corrupted.
	// 	  Therefore, we have a "+5" here to guarantee that the data in range (1 to NUM_KEYS)
	// 	  is valid and the corrupted samples sits in range we are not going to use
	// 	  (NUM_KEYS+1 to NUM_KEYS+5).
	for (int i = NUM_KEYS + 5; i >= 0; i-=1)
	{
		// Find the template sample to generate the sound of given index
		int tempIndex = 0;
		int *index = &tempIndex;
		Sample *templateSample = sizeOfSound(i+1, index);

		// Set up the sample size and the data pointer
		int sample_size = templateSample->size;
		Sample *targetSample = &(Sample) { .size = sample_size, .data = SAMPLE_DATA[j] };

		// Generate the sound
		pitchshift(&templateSample, &targetSample, *index);
		j++;
	}
}

/*
	Name: 			void check_keys(current_playing_keys, pixels_in_keys)

	Description: 	Check key status(on/off) from a frame of the video feed by checking
					the number of bright pixels in the key area.

	Outputs:
			int *	current_playing_keys 		The array of index of current playing keys
*/
void CheckKeys(int *current_playing_keys)
{
	// The current number of black keys at the current key_index
	int black_key_count = 0;

	// Iterate through all keys
	for (int key_index = 0; key_index < NUM_KEYS; key_index++)
	{
		// Number of bright pixels in current key area
		int bright_pixel_cnt = 0;

		// The start position of the white key or the nearby white key (if it's a black key)
		int key_intersection = (key_index - black_key_count) * WHITE_KEY_WIDTH;

		// If the key offset in the octave is not zero, it's a black key
		if (KEY_OFFSET[key_index % NUM_KEYS_IN_OCTAVE])
		{
			// The start location of the white key
			int start = key_intersection - KEY_OFFSET[key_index % NUM_KEYS_IN_OCTAVE];

			// Get number of bright pixels in the black key area
			bright_pixel_cnt += CheckArea(start, start + BLACK_KEY_WIDTH, 0, BLACK_KEY_HEIGHT);

			// Draw the outline of the black key
			DrawVerticalLine(start, 0, BLACK_KEY_HEIGHT - 1, COLOR_BLACK);
			DrawVerticalLine(start + BLACK_KEY_WIDTH - 1, 0, BLACK_KEY_HEIGHT - 1, COLOR_BLACK);
			DrawHorizontalLine(start, BLACK_KEY_HEIGHT - 1, BLACK_KEY_WIDTH, COLOR_BLACK);

			// A noise margin is applied with the threshold to change the status of the key
			//
			// From KEY_INACTIVE to KEY_PRESSED, value needs to be above THRESHOLD + NOISE_MARGIN
			// From KEY_PRESSED/KEY_HELD to KEY_PRESSED, value needs to be below THRESHOLD - NOISE_MARGIN
			// From KEY_PRESSED to KEY_HELD, value must not be below THRESHOLD - NOISE_MARGIN
			// Otherwise, the status stay unchanged
			if (bright_pixel_cnt <= THRESHOLD - NOISE_MARGIN)
			{
				current_playing_keys[key_index] = KEY_INACTIVE;
			}
			else
			{
				if (current_playing_keys[key_index] == KEY_PRESSED)
				{
					current_playing_keys[key_index] = KEY_HELD;
				}
				else if (current_playing_keys[key_index] == KEY_INACTIVE && bright_pixel_cnt > THRESHOLD + NOISE_MARGIN)
				{
					current_playing_keys[key_index] = KEY_PRESSED;
				}

				// Paint the black key area red when active
				PaintArea(start, start + BLACK_KEY_WIDTH, 0, BLACK_KEY_HEIGHT, COLOR_RED);
			}

			// Increment the black key count
			black_key_count++;
		}
		// It's a white key
		else
		{
			// If it's the first key
			if (key_index == FIRST_KEY)
			{
				// The second key should be a black key
				if (KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE])
				{
					// The shape of the white key
					//
					//	   ___
					//    |   |
					//    |   |_
					//	  |     |
					//    |		|
					//    |_____|

					// The start position of the second black key
					int next_start = (key_index - black_key_count + 1) * WHITE_KEY_WIDTH - KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE];

					// Get the number of bright pixels in both areas of the white key (two rectangles)
					bright_pixel_cnt += CheckArea((key_index - black_key_count) * WHITE_KEY_WIDTH, next_start, 0, WHITE_KEY_HEIGHT);
					bright_pixel_cnt += CheckArea(next_start, next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
															 BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT);

					// Draws the outline of the white key
					DrawVerticalLine((key_index - black_key_count) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT, COLOR_WHITE);
					DrawVerticalLine(next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE], BLACK_KEY_HEIGHT,
									 WHITE_KEY_HEIGHT - BLACK_KEY_HEIGHT, COLOR_WHITE);
					DrawHorizontalLine((key_index - black_key_count) * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, WHITE_KEY_WIDTH,
									   COLOR_WHITE);

					if (bright_pixel_cnt <= THRESHOLD - NOISE_MARGIN)
					{
						current_playing_keys[key_index] = 0;
					}
					else
					{
						if (current_playing_keys[key_index] == KEY_PRESSED)
						{
							current_playing_keys[key_index] = KEY_HELD;
						}
						else if (current_playing_keys[key_index] == KEY_INACTIVE && bright_pixel_cnt > THRESHOLD + NOISE_MARGIN)
						{
							current_playing_keys[key_index] = KEY_PRESSED;
						}

						// Paint both the white key areas green when active
						PaintArea((key_index - black_key_count) * WHITE_KEY_WIDTH, next_start, 0, WHITE_KEY_HEIGHT, COLOR_GREEN);
						PaintArea(next_start, next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
								   BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT, COLOR_GREEN);
					}
				}
			}
			// If it's the last key, its left key should be a white key
			else if (key_index == LAST_KEY)
			{
				// The shape of the white key
				//
				//	   _____
				//	  |     |
				//    |     |
				//	  |     |
				//    |		|
				//    |_____|

				bright_pixel_cnt += CheckArea((key_index - black_key_count) * WHITE_KEY_WIDTH,
														 (key_index - black_key_count + 1) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT);

				DrawVerticalLine((key_index - black_key_count) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT, COLOR_WHITE);
				DrawVerticalLine((key_index - black_key_count + 1) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT, COLOR_WHITE);
				DrawHorizontalLine((key_index - black_key_count) * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, WHITE_KEY_WIDTH,
						           COLOR_WHITE);

				if (bright_pixel_cnt <= THRESHOLD - NOISE_MARGIN)
				{
					current_playing_keys[key_index] = 0;
				}
				else
				{
					if (current_playing_keys[key_index] == KEY_PRESSED)
					{
						current_playing_keys[key_index] = KEY_HELD;
					}
					else if (current_playing_keys[key_index] == KEY_INACTIVE && bright_pixel_cnt > THRESHOLD + NOISE_MARGIN)
					{
						current_playing_keys[key_index] = KEY_PRESSED;
					}
					PaintArea((key_index - black_key_count) * WHITE_KEY_WIDTH,
							   (key_index - black_key_count + 1) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT, COLOR_GREEN);
				}
			}
			else
			{
				// If the previous key and the next key are both black keys
				if (KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE] && KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE])
				{
					// The shape of the white key
					//
					//		 _
					//		| |
					//     _| |_
					//	  |     |
					//    |		|
					//    |_____|

					// The end position of the previous black key
					int prev_end = (key_index - black_key_count) * WHITE_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE] + BLACK_KEY_WIDTH;
					// The start position of the next black key
					int next_start = (key_index - black_key_count + 1) * WHITE_KEY_WIDTH - KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE];

					// Get the number of bright pixels from the three areas of the white key
					bright_pixel_cnt += CheckArea(prev_end, next_start, 0, WHITE_KEY_HEIGHT);
					bright_pixel_cnt += CheckArea(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]), prev_end,
													   	     BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT);
					bright_pixel_cnt += CheckArea(next_start, next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
															 BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT);

					// Draws the outline of the white key
					DrawVerticalLine(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]),
							         BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT - BLACK_KEY_HEIGHT, COLOR_WHITE);
					DrawVerticalLine(next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
							         BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT - BLACK_KEY_HEIGHT, COLOR_WHITE);
					DrawHorizontalLine(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]),
									   WHITE_KEY_HEIGHT, WHITE_KEY_WIDTH, COLOR_WHITE);

					if (bright_pixel_cnt <= THRESHOLD - NOISE_MARGIN)
					{
						current_playing_keys[key_index] = 0;
					}
					else
					{
						if (current_playing_keys[key_index] == KEY_PRESSED)
						{
							current_playing_keys[key_index] = KEY_HELD;
						}
						else if (current_playing_keys[key_index] == KEY_INACTIVE && bright_pixel_cnt > THRESHOLD + NOISE_MARGIN)
						{
							current_playing_keys[key_index] = KEY_PRESSED;
						}

						PaintArea(prev_end, next_start, 0, WHITE_KEY_HEIGHT, COLOR_GREEN);
						PaintArea(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]), prev_end,
								   BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT, COLOR_GREEN);
						PaintArea(next_start, next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
								   BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT, COLOR_GREEN);
					}
				}
				// if the next key is black and the previous key is white
				else if (KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE])
				{
					// The shape of the white key
					//
					//	   ___
					//    |   |
					//    |   |_
					//	  |     |
					//    |		|
					//    |_____|

					int next_start = (key_index - black_key_count + 1) * WHITE_KEY_WIDTH - KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE];

					bright_pixel_cnt += CheckArea((key_index - black_key_count) * WHITE_KEY_WIDTH, next_start, 0, WHITE_KEY_HEIGHT);
					bright_pixel_cnt += CheckArea(next_start, next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
															 BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT);

					DrawVerticalLine((key_index - black_key_count) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT, COLOR_WHITE);
					DrawVerticalLine((key_index - black_key_count + 1) * WHITE_KEY_WIDTH, BLACK_KEY_HEIGHT,
							         WHITE_KEY_HEIGHT - BLACK_KEY_HEIGHT, COLOR_WHITE);
					DrawHorizontalLine((key_index - black_key_count) * WHITE_KEY_WIDTH, WHITE_KEY_HEIGHT, WHITE_KEY_WIDTH,
									   COLOR_WHITE);

					if (bright_pixel_cnt <= THRESHOLD - NOISE_MARGIN)
					{
						current_playing_keys[key_index] = 0;
					}
					else
					{
						if (current_playing_keys[key_index] == KEY_PRESSED)
						{
							current_playing_keys[key_index] = KEY_HELD;
						}
						else if (current_playing_keys[key_index] == KEY_INACTIVE && bright_pixel_cnt > THRESHOLD + NOISE_MARGIN)
						{
							current_playing_keys[key_index] = KEY_PRESSED;
						}
						PaintArea((key_index - black_key_count) * WHITE_KEY_WIDTH, next_start, 0, WHITE_KEY_HEIGHT, COLOR_GREEN);

						PaintArea(next_start, next_start + KEY_OFFSET[(key_index + 1) % NUM_KEYS_IN_OCTAVE],
								   BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT, COLOR_GREEN);
					}
				}
				else if (KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE])
				{
					// The shape of the white key
					//
					//	     ___
					//      |   |
					//     _|   |
					//	  |     |
					//    |		|
					//    |_____|

					int prev_end = (key_index - black_key_count) * WHITE_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE] + BLACK_KEY_WIDTH;

					bright_pixel_cnt += CheckArea(prev_end, (key_index - black_key_count + 1) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT);
					bright_pixel_cnt += CheckArea(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]), prev_end,
									BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT);

					DrawVerticalLine(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]),
							         BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT - BLACK_KEY_HEIGHT, COLOR_WHITE);
					DrawVerticalLine((key_index - black_key_count + 1) * WHITE_KEY_WIDTH, 0,
							         WHITE_KEY_HEIGHT, COLOR_WHITE);
					DrawHorizontalLine(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]),
							           WHITE_KEY_HEIGHT, WHITE_KEY_WIDTH, COLOR_WHITE);

					if (bright_pixel_cnt <= THRESHOLD - NOISE_MARGIN)
					{
						current_playing_keys[key_index] = 0;
					}
					else
					{
						if (current_playing_keys[key_index] == KEY_PRESSED)
						{
							current_playing_keys[key_index] = KEY_HELD;
						}
						else if (current_playing_keys[key_index] == KEY_INACTIVE && bright_pixel_cnt > THRESHOLD + NOISE_MARGIN)
						{
							current_playing_keys[key_index] = KEY_PRESSED;
						}
						PaintArea(prev_end, (key_index - black_key_count + 1) * WHITE_KEY_WIDTH, 0, WHITE_KEY_HEIGHT, COLOR_GREEN);

						PaintArea(prev_end - (BLACK_KEY_WIDTH - KEY_OFFSET[(key_index - 1) % NUM_KEYS_IN_OCTAVE]), prev_end,
								   BLACK_KEY_HEIGHT, WHITE_KEY_HEIGHT, COLOR_GREEN);
					}
				}
			}
		}
	}
}

/*
	Name: 			void check_area(start_x, end_x, start_y, end_y)

	Description: 	Return the number of bright pixels in the rectangle area

	Inputs:
			int 	start_x					The horizontal start x
			int 	end_x 					The horizontal end x
			int 	start_y 				The vertical start y
			int 	end_y					The vertical end y

	Outputs:
			int 	num_of_bright_pixels 	The number of bright pixels in the area
*/
int CheckArea(int start_x, int end_x, int start_y, int end_y)
{
	volatile unsigned int * video_in_ptr = FPGA_ONCHIP_BASE;
	volatile unsigned int * vga_pixel_ptr = SDRAM_BASE;

	int bright_pixel_count = 0;
	float threshold = 0.79f;

	// Check every pixel in the rectangle
	for (int x = start_x + KEYBOARD_START_X; x < end_x + KEYBOARD_START_X; x+=1)
	{
		for (int y = start_y + KEYBOARD_START_Y; y < end_y + KEYBOARD_START_Y; y+=1)
		{
			int pixel = video_in_read_pixel(video_in_ptr, x, y);

			SCREEN_BUFFER[((y+240)<<10) + (x+130)] = pixel;

			// If the brightness of the pixel is greater than the threshold
			if (GetLuminFromPixel(pixel) > threshold)
			{
				// Increment the counter
				bright_pixel_count++;
			}
		}
	}

	return bright_pixel_count;
}

/*
	Name: 			void PaintArea(start_x, end_x, start_y, end_y, color)

	Description: 	Paint the given area with specified color

	Inputs:
			int 	start_x					The horizontal start x
			int 	end_x 					The horizontal end x
			int 	start_y 				The vertical start y
			int 	end_y					The vertical end y
			int 	color 					The 16-bit color
*/
void PaintArea(int start_x, int end_x, int start_y, int end_y, int color)
{
	// Iterate through the pixels in the area and paint them with the color
	for (int x = KEYBOARD_START_X + start_x; x < KEYBOARD_START_X + end_x; x++)
	{
		for (int y = KEYBOARD_START_Y + start_y; y < KEYBOARD_START_Y + end_y; y++)
		{
			SCREEN_BUFFER[((y+240)<<10) + (x+130)] = color;
		}
	}
}

/*
	Name: 			void DrawVerticalLine(start_x, start_y, length, color)

	Description:	Draws a vertical line

	Inputs:
			int 	start_x					The horizontal start x
			int 	start_y					The vertical start y
			int 	length 					The length of the line
			int 	color 					The 16-bit color
*/
void DrawVerticalLine(int start_x, int start_y, int length, int color)
{
	for (int y = KEYBOARD_START_Y + start_y; y < KEYBOARD_START_Y + start_y + length; y++)
	{
		SCREEN_BUFFER[((y+240)<<10) + (KEYBOARD_START_X+start_x+130)] = color;
	}
}

/*
	Name: 			void DrawHorizontalLine(start_x, start_y, length, color)

	Description:	Draws a horizontal line

	Inputs:
			int 	start_x					The horizontal start x
			int 	start_y					The vertical start y
			int 	length 					The length of the line
			int 	color 					The 16-bit color
*/
void DrawHorizontalLine(int start_x, int start_y, int length, int color)
{
	for (int x = KEYBOARD_START_X + start_x; x < KEYBOARD_START_X + start_x + length; x++)
	{
		SCREEN_BUFFER[((KEYBOARD_START_Y+start_y+240)<<10) + (x+130)] = color;
	}
}
