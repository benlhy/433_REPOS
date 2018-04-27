#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <math.h>
#include <stdio.h>
#include "ST7735.h"
#include "i2c_master_noint.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 8/2*24/2 = 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0xfffc // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define MAX_X 152
#define MAX_Y 120


void drawProgressBar(short x, short y, short h, int len1, short c1, int len2, short c2){
    int i;
    for (i=0;i<len1;i++){
        int j;
        for (j=0;j<h;j++){
            LCD_drawPixel(x+i,y+j,c1);
        }
    }
    for (i=len1;i<len2;i++){
        int j;
        for (j=0;j<h;j++){
            LCD_drawPixel(x+i,y+j,c2);
        }
    }
}

void drawString(short x, short y, char* str, short fg, short bg){
    int i = 0;
    while(str[i]){
        drawChar(x+5*i,y,str[i],fg,bg);
        i++;
    }
}


void drawChar(short x, short y, unsigned char c, short fg, short bg){
    char row = c - 0x20;
    int i;
    if ((MAX_X-x>7)&&(MAX_Y-y>7)){
        for(i=0;i<5;i++){
            char pixels = ASCII[row][i]; // so we have a list of pixies to go through
            int j;
            for(j=0;j<8;j++){
                if ((pixels>>j)&1==1){
                    LCD_drawPixel(x+i,y+j,fg);
                }
                else {
                    LCD_drawPixel(x+i,y+j,bg);
                }
            }
        }
    }
}


int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISA = 0b0<<4; // INIT A4 to output
    

    LATAbits.LATA4 = 0; //HIGH
    
    
    //TRISB = 0b1<<4; // INIT B4 to input
    SPI1_init();
    

    __builtin_enable_interrupts();
    
    char output[30];
    
    
    LCD_init();
    
    LCD_clearScreen(BLUE);
    i2c_init();
    LATAINV=0b1<<4; //toggle pin 4 as heartbeat
    i2c_write(0x10,0b10000010); // turn on, 2g, 100Hz filter
    LATAINV=0b1<<4; //toggle pin 4 as heartbeat
    i2c_write(0x11,0b10001000); // gyro, 1000dps
    LATAINV=0b1<<4; //toggle pin 4 as heartbeat
    i2c_write(0x12,0b00000100);
    LATAINV=0b1<<4; //toggle pin 4 as heartbeat
    
    
    //imu_init();
    
    while (i2c_read_one(0x0F)!=105){ // reads the WHOAMI register
        ; //hang
    }
    _CP0_SET_COUNT(0);
    //drawChar(0,0,'c',WHITE,BLUE);
    //drawString(10,8,output,WHITE,BLUE);
    //drawProgressBar(0,16,8,20,YELLOW,40,RED);
    
    int val=0;
    while(1) {
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
	// remember the core timer runs at half the sysclk
        _CP0_SET_COUNT(0); // reset count
        output[0]='\0';
     
        sprintf(output,"Hello World %d!  ",val);
        drawString(10,24,output,WHITE,BLUE);
        drawProgressBar(10,32,8,val,YELLOW,100,RED);
        val++;
        if (val>100){
            val = 0;
        }

        LATAINV=0b1<<4; //toggle pin 4 as heartbeat

        while(_CP0_GET_COUNT() < (48000000/2)/(10)){ // 10Hz
            // do nothing.
            
            
            
            
        }
        
        //while(!PORTBbits.RB4) {
        //    ;   // Pin B4 is the USER switch, low (FALSE) if pressed.
        //}
    }
}


