/*
 * timer.c
 *
 *  Created on: Nov 1, 2017
 *      Author: nemtech
 */

#include "socal/hps.h"
#include "socal/socal.h"
#include "lib_def.h"
#include "os_cpu.h"
#include "timer.h"
#include  <ucos_ii.h>
#define FOREVER_WAIT 0

#define FPGA_TO_HPS_LW_ADDR(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))
#define COUNTER_ADDR 0x00000500
#define COUNTER_BASE FPGA_TO_HPS_LW_ADDR(COUNTER_ADDR)

OS_EVENT *MsgQueue;

void InitHPSTimerInterrupt(void) {
	// This version will use the oscl_clk based timers on the HPS core
	// Process
	// Choose one of the four timers (oscl timer 0, oscl timer 1, sptimer 0, sptimer 1)
	// Set to use user-defined count mode
	// Choose timer value should be equal to osc1_clk (50000000 HZ) == 2FAF080 actuall is 100000000
	// PSEUDOCODE:
	//  - Disable the timer
	//  - Program the Timer Mode: User Defined
	//  - Initialize the vector table: source 201 for oscl_0_timer
	//  - Set the interrupt mask to unmasked
	//  - Load the counter value
	//  - Enable the timer

	ARM_OSCL_TIMER_0_REG_CONTROL &= ARM_OSCL_TIMER_0_DISABLE;

	ARM_OSCL_TIMER_0_REG_CONTROL |= ARM_OSCL_TIMER_0_USER_MODE;

	ARM_OSCL_TIMER_0_REG_CONTROL &= ARM_OSCL_TIMER_0_INT_UNMASKED;

	ARM_OSCL_TIMER_0_REG_LOADCOUNT = 25000000; // 250000000 of the oscl_clk should be one second

	BSP_IntVectSet   (201u,   // 201 is source for oscl_timer 0
	                         1,	    // prio
							 DEF_BIT_00,	    // cpu target list
							 HPS_TimerISR_Handler  // ISR
							 );

	ARM_OSCL_TIMER_0_REG_CONTROL |= ARM_OSCL_TIMER_0_ENABLE;

	BSP_IntSrcEn(201u);

}


void HPS_TimerISR_Handler(CPU_INT32U cpu_id) {

	// Initial Read Values
	uint32_t read_count;

	// Read New Count
	read_count = alt_read_word(COUNTER_BASE);

	OSQPost(MsgQueue, (void *) read_count);

	// READ EOI Reg to clear interrupt (PAGE 23-10/23-11 of Cyclone V Hard Processor System
	// Technical Reference Manual
	ARM_OSCL_TIMER_0_REG_EOI;
}


void InitFPGATimerInterrupt(void){
	// This version will use the Timer component implemented on the Qsys Design
	// Configure timer component as before
	// see embedded IP guide for the timer IP

	// Turn Timer off
	QSYS_TIMER_REG_CONTROL |= QSYS_TIMER_STOP;

	// Turn on CONT mode
	QSYS_TIMER_REG_CONTROL |= QSYS_TIMER_CONT_ENABLE;

	//Install handler and set prio
	BSP_IntVectSet   (72u,   // 72 is source for irq 0 via lwhpsfpga bus
		                         2,	    // prio
								 DEF_BIT_00,	    // cpu target list
								 FPGA_TimerISR_Handler  // ISR
								 );


	// Turn on Timer IRQs at component level
	QSYS_TIMER_REG_CONTROL |= QSYS_TIMER_IRQ_ENABLE;

	// Start Timer
	QSYS_TIMER_REG_CONTROL |= QSYS_TIMER_START;

	// Enable INT at GIC level
	BSP_IntSrcEn(72u);

}

void FPGA_TimerISR_Handler(CPU_INT32U cpu_id) {

	static INT8U led = false;

		if (led == false) {
			FPGA_LEDS_On();
			led = true;
		} else {
			FPGA_LEDS_Off();
			led = false;
		}

		// Clear interrupt
		QSYS_TIMER_REG_STATUS = 1;
}

