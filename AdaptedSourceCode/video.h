/*
*********************************************************************************************************
*
*                                          VIDEO HEADER CODE
*
*                                            CYCLONE V SOC
*
* Filename      : video.h
* Version       : V1.00
* Programmer(s) : Kevin Wong (kwong4@ualberta.ca), Alvin Huang (aehuang@ualberta.ca)
* References    : Changes to this project include referenced and modified code from:
* 				  		Title: "VGA display of video input using a bus_master to copy input image"
* 				  		Original Author: Bruce Land (bruce.land@cornell.edu)
* 				  		Date accessed: Feb 2, 2018
* 				  		Used with Permission from Author
* 				  		http://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Bus_master_slave_index.html
*
*********************************************************************************************************
* Note(s)       : This is a Video and VGA driver for the Altera University IP "VGA Controller" and "Video-In Decoder",
* 				  for use with the DE1-SoC.  This code was referenced from Bruce Land's "VGA display of video input using
* 				  a bus_master to copy input image"
*
* Copyright     : Copyright Cornell University February 28, 2018
*
*********************************************************************************************************
*/


// COLORS
#define COLOR_RED   0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE  0X001F
#define COLOR_WHITE 0XFFFF
#define COLOR_BLACK 0X0000

#define NOISE_MARGIN 1

// KEYBOARD INFORMATION
#define LAST_KEY 60
#define FIRST_KEY 0
#define NUM_KEYS 61
#define NUM_WHITE_KEYS 36
#define NUM_KEYS_IN_OCTAVE 12
#define WHITE_KEY_WIDTH 14
#define WHITE_KEY_HEIGHT 63
#define BLACK_KEY_WIDTH 8
#define BLACK_KEY_HEIGHT 43
#define KEYBOARD_START_X 3
#define KEYBOARD_START_Y 138
#define KEYBOARD_CENTER_X (2 * KEYBOARD_START_X + NUM_WHITE_KEYS * WHITE_KEY_WIDTH) / 2

// NUMBER OF BRIGHT PIXELS THRESHOLD
#define THRESHOLD 2

// KEY STATES
#define KEY_HELD 2
#define KEY_PRESSED 1
#define KEY_INACTIVE 0

// OFFSETS BETWEEN THE START OF THE KEY AND THE START OF THE NEXT WHITE KEY
static short KEY_OFFSET[NUM_KEYS_IN_OCTAVE] = { 0, 6, 0, 3, 0, 0, 6, 0, 4, 0, 3, 0 };


// VIDEO-IN OFFSET
#define CONTROL_OFFSET 0x0c
#define RESOLUTION_OFFSET 0x08
#define EDGE_OFFSET 0x10

// VIDEO CONFIGURATION DATA VALUES
#define VIDEO_ON 0x04
#define VIDEO_DIMENSION 0x00f00140
#define NEW_VIDEO_DIMENSION 0x00f001FE
#define EDGE_ON 0x01
#define EDGE_OFF 0x00

// Video In Dimensions
#define VIDEO_IN_WIDTH 640
#define VIDEO_IN_HEIGHT 480
#define VIDEO_IN_PIXEL_SIZE (VIDEO_IN_WIDTH * VIDEO_IN_HEIGHT)
#define VGA_Y 1024
#define VGA_AREA (((VGA_Y * VIDEO_IN_HEIGHT) + VIDEO_IN_WIDTH) << 1)

// VGA Macro
#define VGA_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)vga_pixel_ptr + ((((y)<<10) + (x))<<1)) ;\
	*(short *)pixel_ptr = (color);\
} while(0)

// Video Macro
#define VIDEO_IN_PIXEL(x,y,color) do{\
	int  *pixel_ptr ;\
	pixel_ptr = (int*)((char *)video_in_ptr + ((((y)<<9) + (x))<<1)) ;\
	*(short *)pixel_ptr = (color);\
} while(0)

// Draw a box
void VGA_box (volatile unsigned int *, int, int, int, int, short);

// Draw a line
void VGA_line(volatile unsigned int *, int, int, int, int, short) ;

// Read from VGA
int VGA_read_pixel(volatile unsigned int *, int, int) ;

// Read from Video-In
int video_in_read_pixel(volatile unsigned int *, int, int);

// Get the luminous value from the pixel
float GetLuminFromPixel(int pixel);
