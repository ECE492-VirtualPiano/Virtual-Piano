/*
*********************************************************************************************************
*
*                                          VIDEO HEADER CODE
*
*                                            CYCLONE V SOC
*
* Filename      			: video.h
* Version       			: V1.00
* References    			: Changes to this project include referenced and modified code from:
* 				  					Title: "VGA display of video input using a bus_master to copy input image"
* 				  					Original Author: Bruce Land (bruce.land@cornell.edu)
* 				  					Date accessed: Feb 2, 2018
* 				  					Used with Permission from Author
* 				  					http://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Bus_master_slave_index.html
* Creation/Reference Date 	: Feb 2, 2018
* Modified By 				: Alvin Huang (aehuang@ualberta.ca), Kevin Wong (kwong4@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This is a Video and VGA driver for the Altera University IP "VGA Controller" and "Video-In Decoder",
* 				  for use with the DE1-SoC.  This code was referenced from Bruce Land's "VGA display of video input using
* 				  a bus_master to copy input image"
*
* Copyright     : Copyright Cornell University February 28, 2018
*
*********************************************************************************************************
*/

// VIDEO-IN OFFSET
#define CONTROL_OFFSET 0x0c
#define RESOLUTION_OFFSET 0x08
#define EDGE_OFFSET 0x10

// VIDEO CONFIGURATION DATA VALUES
#define VIDEO_ON 0x04
#define VIDEO_DIMENSION 0x00f00140
#define EDGE_ON 0x01
#define EDGE_OFF 0x00

// Video In Dimensions
#define VIDEO_IN_WIDTH 640
#define VIDEO_IN_HEIGHT 480
#define VIDEO_IN_PIXEL_SIZE (VIDEO_IN_WIDTH * VIDEO_IN_HEIGHT)
#define VGA_Y 1024
#define VGA_AREA (VGA_Y * VIDEO_IN_HEIGHT) + VIDEO_IN_WIDTH

// VGA Macro
#define VGA_PIXEL(x,y,color) do{\
	char  *pixel_ptr ;\
	pixel_ptr = (char *)vga_pixel_ptr + ((y)<<10) + (x) ;\
	*(char *)pixel_ptr = (color);\
} while(0)

// Video Macro
#define VIDEO_IN_PIXEL(x,y,color) do{\
	char  *pixel_ptr ;\
	pixel_ptr = (char *)video_in_ptr + ((y)<<9) + (x) ;\
	*(char *)pixel_ptr = (color);\
} while(0)

// Draw a box
void VGA_box (volatile unsigned int *, int, int, int, int, short);

// Draw a line
void VGA_line(volatile unsigned int *, int, int, int, int, short) ;

// Read from VGA
int VGA_read_pixel(volatile unsigned int *, int, int) ;

// Read from Video-In
int video_in_read_pixel(volatile unsigned int *, int, int);
