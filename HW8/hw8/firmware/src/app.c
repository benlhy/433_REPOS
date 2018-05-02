/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include <math.h>
#include <stdio.h>
#include "LCD_ST.h"
#include "imu.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define IMU_ADDR 0b1101011
#define MULTIPLIER 19.62

char output[30];
unsigned char imu_data[14];
int val=50;
int i;
short temp;
short gx;
short gy;
short gz;
short ax;
short ay;
short az;

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    
    // do your TRIS and LAT commands here
    TRISA = 0b0<<4; // INIT A4 to output
    TRISB = 0b0 << 15;
    //TRISB = 0b0<<14; // RB14 - SCK
    //TRISB = 0b0<<8; // RB8 MOSI
    //TRISB = 0b0<<15; // RB15 - CS
    LATAbits.LATA4 = 1; //HIGH
    LATBbits.LATB15 = 0; // LOW
    
    TRISB = 0b1<<4; // INIT B4 to input
    SPI1_init();
    
    LCD_init();
    imu_init();
    imu_calibrate();
    while (i2c_read_one(0x0F)!=105){ // reads the WHOAMI register
        ; //hang
    }
    

    _CP0_SET_COUNT(0);
    //drawChar(0,0,'c',WHITE,BLUE);
    //drawString(10,8,output,WHITE,BLUE);
    //drawProgressBar(0,16,8,20,YELLOW,40,RED);
    


    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( appData.state )
    {
        /* Application's initial state. */
        case APP_STATE_INIT:
        {
            bool appInitialized = true;
       
        
            if (appInitialized)
            {
            
                appData.state = APP_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_STATE_SERVICE_TASKS:
        {
            _CP0_SET_COUNT(0); // reset count
        
        
        i2c_read_multiple(IMU_ADDR,0x20,imu_data,14);
        

        
        
        
        output[0]='\0';
        temp = imu_data[0] | imu_data [1]<<8;
        gx = imu_data[2] | imu_data [3]<<8;
        gy = imu_data[4] | imu_data [5]<<8;
        gz = imu_data[6] | imu_data [7]<<8;
        ax = imu_data[8] | imu_data [9]<<8;
        ay = imu_data[10] | imu_data [11]<<8;
        az = imu_data[12] | imu_data [13]<<8;
        
        sprintf(output,"ax: %0.2f, ay: %0.2f    ",(float)ax/32768.0*MULTIPLIER,(float)ay/32768.0*MULTIPLIER);
        drawString(10,8,output,WHITE,BLUE);
        sprintf(output,"az: %0.2f   ",(float)az/32768.0*MULTIPLIER);
        drawString(10,16,output,WHITE,BLUE);
        sprintf(output,"gx: %0.2f, gy: %0.2f   ",(float)gx/32768.0*1000,(float)gy/32768.0*1000);
        drawString(10,24,output,WHITE,BLUE);
        sprintf(output,"gz: %0.2f   ",(float)gz/32768.0*1000);
        drawString(10,32,output,WHITE,BLUE);
        output[0]='\0';
     
        //sprintf(output,"Hello World %d!  ",val);
        //drawString(10,40,output,WHITE,BLUE);
        //drawProgressBar(10,48,8,val,YELLOW,100,RED);
        drawXBar(14,100,8,(int)((-(float)ax/32768.0)*50),YELLOW,RED);
        drawYBar(60,54,8,(int)((-(float)ay/32768.0)*50),YELLOW,RED);
        //drawProgressBarVert(60,56,8,40,YELLOW,100,RED);
        val++;
        if (val>100){
            val = 0;
        }

        LATAINV=0b1<<4; //toggle pin 4 as heartbeat

        while(_CP0_GET_COUNT() < (48000000/2)/(20)){ // 10Hz
            // do nothing.
            
            
        }
            break;
        }

        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */
