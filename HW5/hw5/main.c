#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include <math.h>

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
    TRISB = 0b0 << 15;
    //TRISB = 0b0<<14; // RB14 - SCK
    //TRISB = 0b0<<8; // RB8 MOSI
    //TRISB = 0b0<<15; // RB15 - CS
    LATAbits.LATA4 = 0; //HIGH
    LATBbits.LATB15 = 0; // LOW
    
    TRISB = 0b1<<4; // INIT B4 to input
    //ANSELA = 0;
    //ANSELB = 0;
    SS1Rbits.SS1R = 0b0011;
    SDI1Rbits.SDI1R = 0b0100;
    RPA1Rbits.RPA1R = 0b0011;


    
    
    

    __builtin_enable_interrupts();
    _CP0_SET_COUNT(0);
    //
    //SPI1_IO('A');
    //setVoltage('A',128);
    SPI1CON=0;
    SPI1BUF;
    SPI1BRG=0x1;
    SPI1STATbits.SPIROV = 0;
    SPI1CONbits.MODE32=0;
    SPI1CONbits.MODE16=0;
    SPI1CONbits.CKE = 1;
    SPI1CONbits.MSTEN=1;
    SPI1CONbits.MSSEN=0; // Pin controlled by port function
    SPI1CONbits.ON = 1;
    
    
    //setVoltage('B',0);
    //setVoltage('A',1022);
    
    /*
    LATBbits.LATB15 = 0; // LOW
    SPI1BUF = 0| 0b1111<<13 | 512<<3 ;
    while(!SPI1STATbits.SPIRBF);
    LATBbits.LATB15 = 1; // HIGH
    
    SPI1BUF;
    
    LATBbits.LATB15 = 0; // LOW
    SPI1BUF = 0| 0b0111<<13 | 1023<<3 ; // A (by datasheet) -  but channel 2?? so probably B?
    while(!SPI1STATbits.SPIRBF);
    LATBbits.LATB15 = 1; // HIGH
    
    SPI1BUF;
    
    int index = 0;
    int increment = 1;
    */

    
    int i = 0;
    
    int coeff = 0;
    while(1) {
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
	// remember the core timer runs at half the sysclk
        _CP0_SET_COUNT(0); // reset count
        
        
        float f = 512.0 + 511.0*sin(i*2.0*3.14/100.0);
        setVoltage(0,(int)f); // supposedly voutA
        float tri =  coeff/100.0*1023.0;
        
        i++;
        coeff = i;
        if(i>200){
            i = 0;
            coeff = i;
        }
        else if (i>99){
            coeff = 200-i;
        }
        setVoltage(1,(int)tri); // supposedly voutb
        LATAINV=0b1<<4; //toggle pin 4
        
        while(_CP0_GET_COUNT() < (48000000/2)/(1000)){ // 1kHz
            // do nothing.
            
            
        }
        
        //while(!PORTBbits.RB4) {
        //    ;   // Pin B4 is the USER switch, low (FALSE) if pressed.
        //}
    }
}
