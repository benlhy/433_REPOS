// functions to operate the ST7735 on the PIC32
// adapted from https://github.com/sumotoy/TFT_ST7735
// and https://github.com/adafruit/Adafruit-ST7735-Library

// pin connections:
// VCC - 3.3V
// GND - GND
// CS - B7
// RESET - 3.3V
// A0 - B15
// SDA - A1
// SCK - B14
// LED - 3.3V

// B8 is turned into SDI1 but is not used or connected to anything

#include <xc.h>
#include "pwm.h"

#define PERIOD 1024


void PWM_init(){
    RPA0Rbits.RPA0R=0b0101; // set to A0 - OC1
    RPB13Rbits.RPB13R=0b0101; // set to B13 - OC4
    
    // Share timer 2 between OC1 and OC4
    
    T2CONbits.TCKPS = 2; // prescaler N = 4
    T2CONbits.ON = 1;
    PR2 = 1999; // period = (PR2+1)*N*20.8ns = 6kHz okay...
    TMR2 = 0; // start initial count at 0
    
    
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 0b110;
    OC1CONbits.ON = 1;
    OC1RS = 500; // OC1RS/(PR2+1) = 25%
    OC1R = 500; // The value of OC1RS is transferred to OC1RS at the start of every period
    

    OC4CONbits.OCTSEL = 0;
    OC4CONbits.OCM = 0b110;
    OC4CONbits.ON = 1;
    OC4RS = 500; // OC1RS/(PR2+1) = 25%
    OC4R = 500; // 
}

void setPWM(int motor,int pwm){
    if (motor == 1){
        OC1RS = pwm;
    }
    else if (motor == 2){
        OC4RS = pwm;
    }
}