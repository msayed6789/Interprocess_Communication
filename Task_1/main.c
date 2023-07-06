/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"

#include "semphr.h"
//#include "queue. h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

//TaskHandle_t LedTask_Handler=NULL;
//TaskHandle_t LedTask2_Handler=NULL;
//TaskHandle_t LedTask3_Handler=NULL;
/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */

/******Assessment 2****************/
/*Task #1*/

/*Tasks Handler*/
TaskHandle_t LedTask_Handler=NULL;
TaskHandle_t Button_Task_Handler=NULL;


/*Button configuratioon*/
#define BUTTON_PORT      PORT_0
#define BUTTON_PIN       PIN4

uint32_t u32_data_mailbox=0;

void Button_Task (void* PvParameters)
{
	for( ; ;)
	{
		/*Task code goes here*/
		if(GPIO_read(BUTTON_PORT, BUTTON_PIN)==PIN_IS_LOW)
		{
			vTaskDelay(10);
		if(GPIO_read(BUTTON_PORT, BUTTON_PIN)==PIN_IS_LOW) 
		{
       continue;
		}
		u32_data_mailbox=1;
		xTaskNotify(LedTask_Handler,u32_data_mailbox,eSetValueWithOverwrite);
		  u32_data_mailbox=0;
	  }
		  
		  vTaskDelay(100);
	}
}

/*LED Configuration*/
#define LED_PORT               PORT_0
#define LED_PIN                PIN5
#define DELAY_LED_OFF_STATE    10

void Led_Task (void* PvParameters)
{
	for( ; ;)
	{
		uint32_t u32_data=0;
		/*Task code goes here*/
		xTaskNotifyWait
		                (0x00,
		                 0xff,
		                 &u32_data,
		                 portMAX_DELAY);
		if (u32_data==1)
		{
			u32_data=0;
			if (GPIO_read(LED_PORT, LED_PIN)==PIN_IS_LOW)
			{
				GPIO_write(LED_PORT,LED_PIN,PIN_IS_HIGH);
			}
			else
			{
				GPIO_write(LED_PORT,LED_PIN,PIN_IS_LOW);
			}
		}
		vTaskDelay(DELAY_LED_OFF_STATE);
	}
}

int main( void )
{
	prvSetupHardware();
	/*create the task*/
	/******Assessment 2****************/
/*Task #1*/
	xTaskCreate 
	(
	Button_Task,     	                     /*Function that implements the task*/
	"Button_Task",                          /*Text name for the task*/
	configMINIMAL_STACK_SIZE,            /*Stack size in the words not bytes*/
	(void*)NULL,                          /*parameters that passed into the task*/
	1,                                   /*priority at which the task is created*/
	&Button_Task_Handler                    /*used to pass out the created task's handle*/
	);
	
	xTaskCreate 
	(
	Led_Task,     	                     /*Function that implements the task*/
	"Led_Task",                          /*Text name for the task*/
	configMINIMAL_STACK_SIZE,            /*Stack size in the words not bytes*/
	(void*)NULL,                          /*parameters that passed into the task*/
	2,                                   /*priority at which the task is created*/
	&LedTask_Handler                    /*used to pass out the created task's handle*/
	);

	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


