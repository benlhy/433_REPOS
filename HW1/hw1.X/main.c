#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro

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


void spiConfig(void){
    TRISB=0b0<<15; // init A4
    LATBbits.LATB15=1; // set LAT4
    
    // Pick A1 -> SDO 1
    // Pick B8 -> SDI 1
    // Pick B15 -> SS 1
    SS1Rbits.SS1R = 0b0011;
    SDI1Rbits.SDI1R = 0b0100;
    RPA1Rbits.RPA1R = 0b011;
    
    SPI1CONbits.ON=0;// Turn off SPI
    SPI1BUF=0; // clear the buffer
    SPI1BRG=1; // What is the peripheral bus clock?
    // Fsck = Fpb/(2*(SPI1BRG+1))
    // Fsck = 48Mhz/(2*(1+1)) = 12Mhz
    SPI1CONbits.SSEN=0; // Pin controlled by port function
    SPI1CONbits.ON=1; // Turn on SPI
}

char SPI1_IO(char write){
    LATBbits.LATB15=0; // down
    SPI1BUF = write;
    while(!SPI1STATbits.SPIRBF);
    LATBbits.LATB15=1; // up
    return SPI1BUF;
}

void setVoltage(char channel, char voltage){
    int i;
    char output[8];
    for (i=0;i<8;i++){
        output[i]=(voltage>>i) & 1;
    }
    uint16_t carrier;
    if (channel == 'A'){
        carrier=0b0<<15;
    }
    else if (channel == 'B'){
        carrier==0b1<<15;
    }
    carrier = carrier | 0b0<<14; // Unbuffered
    carrier = carrier | 0b1<<13; // Output gain 1
    carrier = carrier | 0b1 <<12; // Active mode
    carrier = carrier | channel << 4; // shift 4
    LATBbits.LATB15=0;
    SPI1BUF = carrier;
     while(!SPI1STATbits.SPIRBF);
    LATBbits.LATB15=1; // up
    
}

void delay(void);

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
    LATAbits.LATA4 = 1; //HIGH
    TRISB = 0b1<<4; // INIT B4 to input
    
    

    __builtin_enable_interrupts();
    _CP0_SET_COUNT(0);

    while(1) {
	// use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
	// remember the core timer runs at half the sysclk
        if(_CP0_GET_COUNT()> (48000000/2)/(2*1000)){ // 1kHz
            LATAINV=0b1<<4; //toggle pin 4
            _CP0_SET_COUNT(0); // reset count
        }
        while(!PORTBbits.RB4) {
            ;   // Pin B4 is the USER switch, low (FALSE) if pressed.
        }
    }
}

void delay(void) {
  _CP0_SET_COUNT(0);
  while(_CP0_GET_COUNT()< (48000000/2)){
    while(!PORTBbits.RB4) {
        ;   // Pin D7 is the USER switch, low (FALSE) if pressed.
    }
  }
}
