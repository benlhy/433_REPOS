# I2C
- 8 bit
- 7 bits address
- 8th bit read and write
- Baud: 100kHz/400kHz/1Mhz
- Overhead on communication is high - address 
- low data rate (sensor/memory vs LCD screen)

## PIC Specific
- Turn off Analog, earlier wins, so we need to turn off Analog
- I2C2 written because of silicon Errata

## Baud Rate generator
 - PGD, in datasheet for PIC (pulse gobbler): 104ns
 - Fsck = 48Mhz
 - I2CBRG = [1/(2*Fsck)]