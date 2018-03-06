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
* Creation/Reference Date: Feb 2, 2018
* Modified By: Alvin Huang (aehuang@ualberta.ca), Kevin Wong (kwong4@ualberta.ca), 
			   Daniel Tran (dtran3@ualberta.ca), Mingjun Zhao (zhao2@ualberta.ca)
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

// Audio Synthesizer Libraries
#include  "../Audio/audio.h"
#include  "../Synthesizer/piano.h"
#include  "../Testbenches/SampleBasedSynthesizerTest.h"

// Video Processing Libraries
#include  <ucos_ii.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include "../Video/video.h"

// Compute absolute address of any slave component attached to lightweight bridge
// base is address of component in QSYS window
// This computation only works for slave components attached to the lightweight bridge
// base should be ranged checked from 0x0 - 0x1fffff
#define FPGA_TO_HPS_LW_ADDR(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

// App Priority
#define APP_TASK_PRIO 5
#define VIDEO_PROCESS_TASK_PRIO 6
#define GENERATE_SOUND_TASK_PRIO 7

// Task Size
#define TASK_STACK_SIZE 4096
#define AUDIO_TASK_STACK_SIZE 16384
#define VIDEO_TASK_STACK_SIZE 8192

/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

// Task Stacks
CPU_STK AppTaskStartStk[TASK_STACK_SIZE];
CPU_STK VideoProcessTaskStk[VIDEO_TASK_STACK_SIZE];
CPU_STK GenerateSoundTaskStk[AUDIO_TASK_STACK_SIZE];

// Screen Buffer
char screen_buffer[VIDEO_IN_PIXEL_SIZE];

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
static void GenerateSoundTask (void *p_arg);

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
	*(h2p_lw_video_in_control_addr) = VIDEO_ON; // Turn On Video Capture
	*(h2p_lw_video_in_resolution_addr) = VIDEO_DIMENSION;  // High 240 Low 320
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

	// Create the Audio task.
	// Generates the Audio
//    os_err = OSTaskCreateExt((void (*)(void *)) GenerateSoundTask,
//                                 (void          * ) 0,
//                                 (OS_STK        * )&GenerateSoundTaskStk[AUDIO_TASK_STACK_SIZE - 1],
//                                 (INT8U           ) GENERATE_SOUND_TASK_PRIO,
//                                 (INT16U          ) GENERATE_SOUND_TASK_PRIO,
//                                 (OS_STK        * )&GenerateSoundTaskStk[0],
//                                 (INT32U          ) AUDIO_TASK_STACK_SIZE,
//                                 (void          * )0,
//                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));
//
//	if (os_err != OS_ERR_NONE) {
//		exit(0); /* Handle error. */
//	}

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

	// VGA Pixel Buffer
	volatile unsigned int * vga_pixel_ptr = SDRAM_BASE;

	// Video Input Buffer
	volatile unsigned int * video_in_ptr = FPGA_ONCHIP_BASE;

	// Video Input Index
	int i,j;

	// Clear the screen
	VGA_box (vga_pixel_ptr, 0, 0, 639, 479, 0x03);

	// Copy current screen
	memcpy(screen_buffer, vga_pixel_ptr, VGA_AREA * sizeof(char *));

	// Loop Forever
    for(;;) {

    	// Delay
		OSTimeDlyHMSM(0, 0, 0, 500);

		// Read Video-in and Image Process frame to buffer
		// Loop through Video dimension
		for (i=0; i<320; i++) {
			for (j=10; j<240; j++) {

				// Read in Pixel
				int pixel = video_in_read_pixel(video_in_ptr, i, j);

				// Find RGB values
				int r = (pixel & 0b11100000) >> 5;
				int g = (pixel & 0b00011100) >> 2;
				int b = pixel & 0b00000011;

				// Check if over white threshold and set to RED
				if(r + g + b > 16) pixel = 0b11100000;

				// Copy pixel to Screen
				screen_buffer[((j+240)<<10) + (i+320)] = pixel;
			}
		}

		// Memory Copy Screen Buffer to VGA
		memcpy(vga_pixel_ptr, screen_buffer, VGA_AREA * sizeof(char));

		// Delay
		OSTimeDlyHMSM(0, 0, 0, 500);
    }

}

/*
*********************************************************************************************************
*                                           GenerateSoundTask()
*
* Description : Generate Sound Task.
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

static  void  GenerateSoundTask (void *p_arg)
{
	// Audio Base
	volatile int * audio_ptr = (int *) AUDIO_BASE;

	// Looping variables
    int i = 0, i_s= 1;

    // Struct Sample initalization
	Sample *samples_t = NULL;	

	// Loop Forever
    for(;;) {

        int fifospace = *(audio_ptr + 1);
    	if ( (fifospace & 0xFF000000) && (fifospace & 0x00FF0000) )
    	{
    		if (samples_t != NULL && i < samples_t->size)
    		{
        		int r = samples_t->data[i++];
        		*(audio_ptr + 2) = r;
        		*(audio_ptr + 3) = r;
    		}
    		else
    		{
    			i = 0;
    			i_s += 1;

    			int tempIndex = 0;
    			int *index = &tempIndex;
    			Sample *tempSample1 = sizeOfSound(i_s, index);
    			short sampleData[MAX_PITCH_SHIFT_OUTPUT_ARRAY_SIZE];

    			samples_t = &(Sample) { .size = tempSample1->size, .data = sampleData };
    			pitchshift(&tempSample1, &samples_t, *index);
    		}

    		if (i_s > 88)
    		{
    			break;
    		}
    	}
    }
}
