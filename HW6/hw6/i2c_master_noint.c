#include<xc.h>
#include "i2c_master_noint.h"

void initExpander(){
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    i2c_master_setup();
    //writeExpander(0x00,0b11110000);
    //writeExpander(0x0A,0b00001111);
            
    //I2C2BRG = 50; // I2C2BRG = [1/(2*Fsck)-PGD]*PBlck -2
    //I2C2CONbits.ON = 1; // turn on
}

#define ADDR 0b0100000

void writeExpander(unsigned char reg, unsigned char val){
    i2c_master_start(); // send start bit
    i2c_master_send(ADDR<<1|0); // tell it we are talking to it
    i2c_master_send(reg); // tell which register we are changing
    i2c_master_send(val); // tell it what value we are changing it to
    i2c_master_stop(); // send stop bit
}

unsigned char readExpander(){
    i2c_master_start(); 
    i2c_master_send(ADDR<<1|0);
    i2c_master_send(0x09);
    i2c_master_restart(); // restart
    i2c_master_send(ADDR<<1|1); // 
    unsigned char r = i2c_master_recv();
    i2c_master_ack(1); // done
    i2c_master_stop();
    return r; // return what we read
}

// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C channel you are using
// I2C pins need pull-up resistors, 2k-10k

void i2c_master_setup(void) {
  I2C2BRG = 53;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2
                                    // look up PGD for your PIC32
  I2C2CONbits.ON = 1;               // turn on the I2C1 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {
    I2C2CONbits.RSEN = 1;           // send a restart
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}
