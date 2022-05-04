/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xiicps.h"
#include "sleep.h"

#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID
#define CMPS_ADDRESS	0x1E
#define CMPS_READ		0x3D
#define CMPS_WRITE		0x3C
#define IIC_SCLK_RATE	100000

int initIicDriver(XIicPs *Iic, u16 DeviceId);

int main()
{
	u8 rx_buffer[6];
	u8 tx_buffer[2];
	s16 x,y,z;
	int Status;
	XIicPs Iic;

    init_platform();

    print("Start \n");

    Status = initIicDriver(&Iic, IIC_DEVICE_ID);
    if(Status != XST_SUCCESS){
    	print("Init FAILURE \n");
    	return XST_FAILURE;
    }

    tx_buffer[0] = 0x02;
    tx_buffer[1] = 0x00;
    Status = XIicPs_MasterSendPolled(&Iic, tx_buffer, 2, CMPS_ADDRESS);
    if(Status != XST_SUCCESS){
    	print("Read reg fail \n");
    	printf("Status 1 = %d \n", Status);
    	return 1;
    }

    while (1) {
		tx_buffer[0] = 0x03;
		Status = XIicPs_MasterSendPolled(&Iic, tx_buffer, 1, CMPS_ADDRESS);
		if(Status != XST_SUCCESS){
			print("Read reg fail \n");
			printf("Status 2 = %d \n", Status);
			return 1;
		}

		Status = XIicPs_MasterRecvPolled(&Iic, rx_buffer, 6, CMPS_ADDRESS);
		if(Status != XST_SUCCESS){
			print("Recv fail \n");
			printf("Status 3 = %d \n", Status);
			return 1;
		}
		sleep(2);
		x = rx_buffer[1] << 8 | rx_buffer[0];
		y = rx_buffer[3] << 8 | rx_buffer[2];
		z = rx_buffer[5] << 8 | rx_buffer[4];
		printf("x = %d \n", x);
		printf("y = %d \n", y);
		printf("z = %d \n", z);
		printf("\n");
    }

    cleanup_platform();

    return 0;
}

int initIicDriver(XIicPs *Iic, u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;

	Config = XIicPs_LookupConfig(DeviceId);
	if(Config == NULL){
		printf("Error: XIicPs_LookupConfig()\r\n");
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(Iic, Config, Config->BaseAddress);
	if(Status != XST_SUCCESS){
		printf("Error: XIicPs_CfgInitialize()\r\n");
		return XST_FAILURE;
	}

	Iic->IsRepeatedStart = 0;

	Status = XIicPs_SelfTest(Iic);
	if(Status != XST_SUCCESS){
		printf("Error: XIicPs_SelfTest()\r\n");
		return XST_FAILURE;
	}

	XIicPs_SetSClk(Iic, IIC_SCLK_RATE);

	return XST_SUCCESS;
}


