/*
*********************************************************************************************************
*
*                                          VIDEO DRIVER CODE
*
*                                            CYCLONE V SOC
*
* Filename     			 : video.c
* Version      			 : V1.00
* References    		 : Changes to this project include referenced and modified code from:
* 				  				Title: "VGA display of video input using a bus_master to copy input image"
* 				  				Original Author: Bruce Land (bruce.land@cornell.edu)
* 				  				Date accessed: Feb 2, 2018
* 				  				Used with Permission from Author
* 				  				http://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Bus_master_slave_index.html
* Creation/Reference Date: Feb 2, 2018
* Modified By			 : Alvin Huang (aehuang@ualberta.ca), Kevin Wong (kwong4@ualberta.ca)
*********************************************************************************************************
* Note(s)       : This is a Video and VGA driver for the Altera University IP "VGA Controller" and "Video-In Decoder",
* 				  for use with the DE1-SoC.  This code was referenced from Bruce Land's "VGA display of video input using
* 				  a bus_master to copy input image"
*
* Copyright     : Copyright Cornell University February 28, 2018
*
*********************************************************************************************************
*/

#include "../Video/video.h"

/****************************************************************************************
 * Subroutine to read a pixel from the video input
****************************************************************************************/
int video_in_read_pixel(volatile unsigned int * video_in_ptr, int x, int y){
	char  *pixel_ptr;
	pixel_ptr = (char *)video_in_ptr + ((y)<<9) + (x);
	return *pixel_ptr;
}

/****************************************************************************************
 * Subroutine to read a pixel from the VGA monitor
****************************************************************************************/
int  VGA_read_pixel(volatile unsigned int * vga_pixel_ptr, int x, int y){
	char  *pixel_ptr;
	pixel_ptr = (char *)vga_pixel_ptr + ((y)<<10) + (x);
	return *pixel_ptr;
}

/****************************************************************************************
 * Draw a filled rectangle on the VGA monitor
****************************************************************************************/
#define SWAP(X,Y) do{\
	int temp=X;\
	X=Y;\
	Y=temp;\
} while(0)

void VGA_box(volatile unsigned int * vga_pixel_ptr, int x1, int y1, int x2, int y2, short pixel_color)
{
	// Buffer Pixel Pointer
	char *pixel_ptr ;

	// Row and Column indexes
	int row, col;

	/* check and fix box coordinates to be valid */
	if (x1 > (VIDEO_IN_WIDTH - 1)) {
		x1 = (VIDEO_IN_WIDTH - 1);
	}
	if (y1 > (VIDEO_IN_HEIGHT - 1)) {
		y1 = (VIDEO_IN_HEIGHT - 1);
	}
	if (x2 > (VIDEO_IN_WIDTH - 1)) {
		x2 = (VIDEO_IN_WIDTH - 1);
	}
	if (y2 > (VIDEO_IN_HEIGHT - 1)) {
		y2 = (VIDEO_IN_HEIGHT - 1);
	}
	if (x1 < 0) {
		x1 = 0;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (x2 < 0) {
		x2 = 0;
	}
	if (y2 < 0) {
		y2 = 0;
	}
	if (x1 > x2) {
		SWAP(x1, x2);
	}
	if (y1 > y2) {
		SWAP(y1, y2);
	}

	// Cycle through indexes
	for (row = y1; row <= y2; row++)
		for (col = x1; col <= x2; ++col)
		{
			// Set address of current index
			pixel_ptr = (char *)vga_pixel_ptr + (row<<10) + col ;

			// Set Pixel Color
			*(char *)pixel_ptr = pixel_color;
		}
}

// =============================================
// === Draw a line
// =============================================
//	plot a line
//	at x1,y1 to x2,y2 with color
//	Code is from David Rodgers,
//	"Procedural Elements of Computer Graphics",1985
void VGA_line(volatile unsigned int * vga_pixel_ptr, int x1, int y1, int x2, int y2, short c) {

	// Local pointer and index values
	int e;
	signed int dx,dy,j, temp;
	signed int s1,s2, xchange;
    signed int x,y;
	char *pixel_ptr ;

	/* Check and Fix line coordinates to be valid */
	if (x1 > (VIDEO_IN_WIDTH - 1)) {
		x1 = (VIDEO_IN_WIDTH - 1);
	}
	if (y1 > (VIDEO_IN_HEIGHT - 1)) {
		y1 = (VIDEO_IN_HEIGHT - 1);
	}
	if (x2 > (VIDEO_IN_WIDTH - 1)) {
		x2 = (VIDEO_IN_WIDTH - 1);
	}
	if (y2 > (VIDEO_IN_HEIGHT - 1)) {
		y2 = (VIDEO_IN_HEIGHT - 1);
	}
	if (x1 < 0) {
		x1 = 0;
	}
	if (y1 < 0) {
		y1 = 0;
	}
	if (x2 < 0) {
		x2 = 0;
	}
	if (y2 < 0) {
		y2 = 0;
	}

	// Get current x and y
	x = x1;
	y = y1;

	// Take Absolute Value
	if (x2 < x1) {
		dx = x1 - x2;
		s1 = -1;
	}
	else if (x2 == x1) {
		dx = 0;
		s1 = 0;
	}
	else {
		dx = x2 - x1;
		s1 = 1;
	}
	if (y2 < y1) {
		dy = y1 - y2;
		s2 = -1;
	}
	else if (y2 == y1) {
		dy = 0;
		s2 = 0;
	}
	else {
		dy = y2 - y1;
		s2 = 1;
	}

	// Set x change to 0
	xchange = 0;

	// Check if change in y larger than x
	if (dy > dx) {
		temp = dx;
		dx = dy;
		dy = temp;
		xchange = 1;
	}

	// Calculate left shift of change in y minus x
	e = ((int)dy<<1) - dx;

	// Cycle through change in x
	for (j=0; j<=dx; j++) {

		// Set address of current index
		pixel_ptr = (char *)vga_pixel_ptr + (y<<10) + x;

		// Set Pixel Color
		*(char *)pixel_ptr = c;

		// Check if e greater or equal to than 0
		if (e >= 0) {
			// Check if xchange equal to 1
			if (xchange==1) {
				x = x + s1;
			}
			// Else increment y
			else {
				y = y + s2;
			}

			// Decrement e by change in x left shifted by 1
			e = e - ((int)dx<<1);
		}

		// Check if xchange equal to
		if (xchange == 1) {
			y = y + s2;
		}
		else {
			x = x + s1;
		}

		// Increment e by change in y left shifted by 1
		e = e + ((int)dy << 1);
	}
}
