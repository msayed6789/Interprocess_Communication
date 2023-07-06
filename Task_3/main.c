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

/*Task #3*/
TaskHandle_t Rising_Edge_Handler=NULL;
TaskHandle_t Falling_Edge_Handler=NULL;
TaskHandle_t Consumer_Task_Handler=NULL;
TaskHandle_t Task4_Handler=NULL;


/*Button configuratioon*/
#define BUTTON1_PORT      PORT_0
#define BUTTON1_PIN       PIN4
#define BUTTON2_PORT      PORT_0
#define BUTTON2_PIN       PIN3

#define BUTTON_MESSAGE_NUM        4
#define BUTTON_MESSAGE_SIZE       25
const signed char u8_Button_Str[BUTTON_MESSAGE_NUM][BUTTON_MESSAGE_SIZE]={"Button#1 is Released\n\r","Button#2 is Released\n\r" ,"Button#1 is Pressed\n\r","Button#2 is Pressed\n\r"};


#define TASK_4_MESSAGE_NUM        10
#define TASK_4_MESSAGE_SIZE       10
const signed char u8_Task_4_Str[TASK_4_MESSAGE_SIZE][10]={"Task#1\n\r",
                                                          "Task#2\n\r",
                                                          "Task#3\n\r",
                                                          "Task#4\n\r",
                                                          "Task#5\n\r",
                                                          "Task#6\n\r",
                                                          "Task#7\n\r",
                                                          "Task#8\n\r",
	                                                        "Task#9\n\r",
	                                                        "Task#10\n\r"};

uint8_t u8_falling_check_flag_button1=0;
uint8_t u8_falling_check_flag_button2=0;

void Rising_Edge (void* PvParameters)
{
	for( ; ;)
	{
		/*Task code goes here*/
		if(GPIO_read(BUTTON1_PORT, BUTTON1_PIN)==PIN_IS_HIGH && u8_falling_check_flag_button1==1)
		{
			u32_data_mailbox=1;
			u8_falling_check_flag_button1=0;
			xTaskNotify(Consumer_Task_Handler,u32_data_mailbox,eSetValueWithOverwrite);
		}
		 if (GPIO_read(BUTTON2_PORT, BUTTON2_PIN)==PIN_IS_HIGH && u8_falling_check_flag_button2==1)
		{
			u32_data_mailbox=2;
			u8_falling_check_flag_button2=0;
			xTaskNotify(Consumer_Task_Handler,u32_data_mailbox,eSetValueWithOverwrite);
		}
		vTaskDelay(100);
	}
}


void Falling_Edge (void* PvParameters)
{
	for( ; ;)
	{
		if(GPIO_read(BUTTON1_PORT, BUTTON1_PIN)==PIN_IS_LOW && u8_falling_check_flag_button1==0)
		{
			u32_data_mailbox=3;
			u8_falling_check_flag_button1=1;
			xTaskNotify(Consumer_Task_Handler,u32_data_mailbox,eSetValueWithOverwrite);
		}
		 if (GPIO_read(BUTTON2_PORT, BUTTON2_PIN)==PIN_IS_LOW && u8_falling_check_flag_button2==0)
		{
			u32_data_mailbox=4;
			u8_falling_check_flag_button2=1;
			xTaskNotify(Consumer_Task_Handler,u32_data_mailbox,eSetValueWithOverwrite);
		}
		vTaskDelay(100);
	}
}

void Consumer_Task (void* PvParameters)
{
	uint32_t u32_data=0;
	for( ; ;)
	{
		
		xTaskNotifyWait
		                (0x00,
		                 0xff,
		                 &u32_data,
		                 portMAX_DELAY);
		if (u32_data==1)
		{
			u32_data=0;
			vSerialPutString(&u8_Button_Str[u32_data_mailbox-1][0],21);
			u32_data_mailbox=0;
		}
		else if (u32_data==2)
		{
			u32_data=0;
			vSerialPutString(&u8_Button_Str[u32_data_mailbox-1][0],21);
			u32_data_mailbox=0;
		}
		else if (u32_data==3)
		{
			u32_data=0;
			vSerialPutString(&u8_Button_Str[u32_data_mailbox-1][0],21);
			u32_data_mailbox=0;
		}
		else if (u32_data==4)
		{
			u32_data=0;
			vSerialPutString(&u8_Button_Str[u32_data_mailbox-1][0],21);
			u32_data_mailbox=0;
		}
		else if (u32_data==5)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==6)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==7)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==8)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==9)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==10)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==11)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==12)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==13)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
		else if (u32_data==14)
		{
			vSerialPutString(&u8_Task_4_Str[u32_data-5][0],8);
			u32_data=0;
		}
	}
}

void Task4 (void* PvParameters)
{
	uint8_t u8_count=0;
	for( ; ;)
	{
		  for (u8_count=5;u8_count<15;u8_count++)
		{
			xTaskNotify(Consumer_Task_Handler,u8_count,eSetValueWithOverwrite);
			vTaskDelay(100);
		}
		u8_count=5;
		
	}
}

int main( void )
{
	prvSetupHardware();
	/*create the task*/
	/******Assessment 2****************/

/*Task #3*/
	xTaskCreate 
	(
	Rising_Edge,     	                     /*Function that implements the task*/
	"Rising_Edge",                          /*Text name for the task*/
	configMINIMAL_STACK_SIZE,            /*Stack size in the words not bytes*/
	(void*)NULL,                          /*parameters that passed into the task*/
	1,                                   /*priority at which the task is created*/
	&Rising_Edge_Handler                    /*used to pass out the created task's handle*/
	);
	
	xTaskCreate 
	(
	Falling_Edge,     	                     /*Function that implements the task*/
	"Falling_Edge",                          /*Text name for the task*/
	configMINIMAL_STACK_SIZE,            /*Stack size in the words not bytes*/
	(void*)NULL,                          /*parameters that passed into the task*/
	2,                                   /*priority at which the task is created*/
	&Falling_Edge_Handler                    /*used to pass out the created task's handle*/
	);
	
	xTaskCreate 
	(
	Consumer_Task,     	                     /*Function that implements the task*/
	"Consumer_Task",                          /*Text name for the task*/
	configMINIMAL_STACK_SIZE,            /*Stack size in the words not bytes*/
	(void*)NULL,                          /*parameters that passed into the task*/
	2,                                   /*priority at which the task is created*/
	&Consumer_Task_Handler                    /*used to pass out the created task's handle*/
	);
	
	xTaskCreate 
	(
	Task4,     	                     /*Function that implements the task*/
	"Task4",                          /*Text name for the task*/
	configMINIMAL_STACK_SIZE,            /*Stack size in the words not bytes*/
	(void*)NULL,                          /*parameters that passed into the task*/
	1,                                   /*priority at which the task is created*/
	&Task4_Handler                    /*used to pass out the created task's handle*/
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


