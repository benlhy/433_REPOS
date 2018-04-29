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

#define IMU_ADDR 0b1101011
#define MULTIPLIER 19.62

/*
 * len goes from -50 to 50
 */
void drawYBar(short x, short y, short h, int len, int c1, int c2){
    int i;
    // if len is negative, start drawing from negative to center
    if (len < 0){
        drawProgressBarVert(x, y, h, 50+len, c2, 50, c1);// flip the colors
        drawProgressBarVert(x,y+50, h,0,c1,50,c2);
    }
    else{
        drawProgressBarVert(x,y,h,0,c1,50,c2);
        drawProgressBarVert(x,y+50,h,len,c1,50,c2);
    }
}


/*
 * len goes from -50 to 50
 */
void drawXBar(short x, short y, short h, int len, int c1, int c2){
    
    // if len is negative, start drawing from negative to center
    if (len < 0){
        drawProgressBar(x, y, h, 50+len, c2, 50, c1);// flip the colors
        drawProgressBar(x+50,y, h,0,c1,50,c2);
    }
    else{
        drawProgressBar(x,y,h,0,c1,50,c2);
        drawProgressBar(x+50,y,h,len,c1,50,c2);
    }
}

void drawProgressBarVert(short x, short y, short h, int len1, short c1, int len2, short c2){
    int i;
    for (i=0;i<len1;i++){
        int j;
        for (j=0;j<h;j++){
            LCD_drawPixel(x+j,y+i,c1);
        }
    }
    for (i=len1;i<len2;i++){
        int j;
        for (j=0;j<h;j++){
            LCD_drawPixel(x+j,y+i,c2);
        }
    }
}

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

    
    
    imu_init();
    imu_calibrate();
    while (i2c_read_one(0x0F)!=105){ // reads the WHOAMI register
        ; //hang
    }
    unsigned char imu_data[14];

    _CP0_SET_COUNT(0);
    //drawChar(0,0,'c',WHITE,BLUE);
    //drawString(10,8,output,WHITE,BLUE);
    //drawProgressBar(0,16,8,20,YELLOW,40,RED);
    
    int val=50;
    int i;
    while(1) {
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
	// remember the core timer runs at half the sysclk
        _CP0_SET_COUNT(0); // reset count
        
        
        i2c_read_multiple(IMU_ADDR,0x20,imu_data,14);
        

        
        
        /*
        i2c_master_start(); 
        i2c_master_send(IMU_ADDR<<1|0); // i want to send
        i2c_master_send(0x20); // what to read
        i2c_master_restart(); // restart
        i2c_master_send(IMU_ADDR<<1|1); // i want to read
        for (i=0;i<14;i++){
            imu_data[i]=i2c_master_recv();
            if (i<13){
                i2c_master_ack(0);
            }
            else {
                i2c_master_ack(1);
            }
        }
        i2c_master_stop();  
        */
        output[0]='\0';
        short temp = imu_data[0] | imu_data [1]<<8;
        short gx = imu_data[2] | imu_data [3]<<8;
        short gy = imu_data[4] | imu_data [5]<<8;
        short gz = imu_data[6] | imu_data [7]<<8;
        short ax = imu_data[8] | imu_data [9]<<8;
        short ay = imu_data[10] | imu_data [11]<<8;
        short az = imu_data[12] | imu_data [13]<<8;
        
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

        while(_CP0_GET_COUNT() < (48000000/2)/(10)){ // 10Hz
            // do nothing.
            
            
            
            
        }
        
        //while(!PORTBbits.RB4) {
        //    ;   // Pin B4 is the USER switch, low (FALSE) if pressed.
        //}
    }
}


