/*
*********************************************************************************************************
*
*                                          AUDIO DRIVER CODE
*
*                                            CYCLONE V SOC
*
* Filename      : audio.h
* Version       : V1.00
* Programmer(s) : Michael Wong (mcwong2@ualberta.ca)
*
*********************************************************************************************************
* Note(s)       : This is a sparse driver for the Altera University IP "Audio" core,
* 				  for use with the DE1-SoC.  This driver was written for ECE 492, and assumes that the
* 				  core is acting as a slave to the Cyclone V's HPS, connected via the lightweight bridge.
*
*********************************************************************************************************
*/

#include <hps.h>
#include <os_cpu.h>
#include <socal.h>

#define FPGA_TO_HPS_LW_ADDR(base)  ((void *) (((char *)  (ALT_LWFPGASLVS_ADDR))+ (base)))

// NB: Set base address of Audio core here, according to QSYS system configuration
#define AUDIO_ADDR 0x00003040
#define AUDIO_BASE (ALT_LWFPGASLVS_ADDR + AUDIO_ADDR)

#define LEFT_CHANNEL 0
#define RIGHT_CHANNEL 1

#define LEFT_LINE_IN 0x0
#define RIGHT_LINE_IN 0x1
#define LEFT_HEADPHONE_OUT 0x2
#define RIGHT_HEADPHONE_OUT 0x3
#define ANALOG_AUDIO_PATH 0x4
#define DIGITAL_AUDIO_PATH 0x5
#define POWER_DOWN 0x6
#define DIGITAL_AUDIO_INTERFACE 0x7
#define SAMPLING_CONTROL 0x8
#define ACTIVE_CONTROL 0x9

#define LINE_IN_HIGH_VOLUME 0x1F
#define HEADPHONE_HIGH_VOLUME 0x7F
#define DIGITAL_AUDIO_CONVERTER 0x14
#define HIGH_PASS 0x06
#define POWER_ON 0x00
#define LEFT_JUSTIFIED 0x41
#define NORMAL_MODE 0x20
#define ACTIVE 0x01

#define AUDIO_CONTROL_OFFSET 0
#define AUDIO_CONTROL_RE_MASK 0x00000001
#define AUDIO_CONTROL_WE_MASK 0x00000002
#define AUDIO_CONTROL_CR_MASK 0x00000004
#define AUDIO_CONTROL_CW_MASK 0x00000008
#define AUDIO_CONTROL_RI_MASK 0x00000100
#define AUDIO_CONTROL_WI_MASK 0x00000200

#define AUDIO_FIFOSPACE_OFFSET 4
#define AUDIO_FIFOSPACE_WSLC_MASK 0xFF000000
#define AUDIO_FIFOSPACE_WSRC_MASK 0x00FF0000
#define AUDIO_FIFOSPACE_RALC_MASK 0x0000FF00
#define AUDIO_FIFOSPACE_RARC_MASK 0x000000FF
#define AUDIO_FIFOSPACE_WSLC_BIT_OFFSET 24
#define AUDIO_FIFOSPACE_WSRC_BIT_OFFSET 16
#define AUDIO_FIFOSPACE_RALC_BIT_OFFSET 8
#define AUDIO_FIFOSPACE_RARC_BIT_OFFSET 0

#define AUDIO_LEFTDATA_OFFSET 8
#define AUDIO_RIGHTDATA_OFFSET 12

void reset_audio_core();
void enable_audio_read_interrupt();
void disable_audio_read_interrupt();
void enable_audio_write_interrupt();
void disable_audio_write_interrupt();
INT32U is_audio_read_interrupt_pending();
INT32U is_audio_write_interrupt_pending();
INT32U read_audio_data(INT32U * buffer, INT32U len, INT32U channel);
INT32U write_audio_data(INT32U * buffer, INT32U len, INT32U channel);

