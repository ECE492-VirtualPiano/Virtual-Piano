/*
*********************************************************************************************************
*
*                                          ADDRESS_MAP_ARM HEADER FILE
*
*                                            CYCLONE V SOC
*
* Filename      : address_map_arm_brl4.h
* Version       : V1.00
* Reference     : Changes to this project include referenced and modified code from:
* 				  		Title: "VGA display of video input using a bus_master to copy input image"
* 				  		Original Author: Bruce Land (bruce.land@cornell.edu)
* 				  		Date accessed: Feb 2, 2018
* 				  		Used with Permission from Author
* 				  		http://people.ece.cornell.edu/land/courses/ece5760/DE1_SOC/HPS_peripherials/Bus_master_slave_index.html
*
*********************************************************************************************************
* Note(s)       : This files provides address values that exist in the system.
* 				  This code was referenced from Bruce Land's "VGA display of video input using
* 				  a bus_master to copy input image"
*
* Copyright     : Copyright Cornell University February 28, 2018
*
*********************************************************************************************************
*/


#define BOARD                 "DE1-SoC"

/* Memory */
#define DDR_BASE              0x00000000
#define DDR_END               0x3FFFFFFF
#define A9_ONCHIP_BASE        0xFFFF0000
#define A9_ONCHIP_END         0xFFFFFFFF
#define SDRAM_BASE            0xC0000000
#define SDRAM_END             0xC3FFFFFF
#define FPGA_ONCHIP_BASE      0xC8000000
#define FPGA_ONCHIP_END       0xC803FFFF

// FPGA Dimensions and Length
#define FPGA_ONCHIP_SPAN      0x00080000
#define FPGA_CHAR_BASE        0xC9000000 
#define FPGA_CHAR_END         0xC9001FFF
#define FPGA_CHAR_SPAN        0x00002000

/* Cyclone V FPGA DEVICES */
#define HW_REGS_BASE          0xff200000
#define HW_REGS_SPAN          0x00005000 

// Cyclone V FPGA OFFSET
#define LEDR_BASE             0x00000000
#define SW_BASE               0x00000040
#define KEY_BASE              0x00000050
#define JP1_BASE              0x00000060
#define JP2_BASE              0x00000070
#define PS2_BASE              0x00000100
#define PS2_DUAL_BASE         0x00000108
#define JTAG_UART_BASE        0x00001000
#define JTAG_UART_2_BASE      0x00001008
#define IrDA_BASE             0x00001020
#define TIMER_BASE            0x00002000
#define TIMER_2_BASE          0x00002020
#define AV_CONFIG_BASE        0x00003000
#define PIXEL_BUF_CTRL_BASE   0x00003020
#define CHAR_BUF_CTRL_BASE    0x00003030
#define VIDEO_IN_BASE         0x00003060
#define ADC_BASE              0x00004000

/* Cyclone V HPS devices */
#define HPS_GPIO1_BASE        0xFF709000
#define I2C0_BASE             0xFFC04000
#define I2C1_BASE             0xFFC05000
#define I2C2_BASE             0xFFC06000
#define I2C3_BASE             0xFFC07000
#define HPS_TIMER0_BASE       0xFFC08000
#define HPS_TIMER1_BASE       0xFFC09000
#define HPS_TIMER2_BASE       0xFFD00000
#define HPS_TIMER3_BASE       0xFFD01000
#define FPGA_BRIDGE           0xFFD0501C
