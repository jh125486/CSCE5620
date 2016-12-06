#ifndef CC1120_H_INCLUDED
#define CC1120_H_INCLUDED

#include "common.h"
#include "kubos-hal/gpio.h"
#include "kubos-hal/spi.h"

// misc bits
#define READ_BIT 0x80

// registers
#define FREQ0 0x0e
#define EXTENDED_ADDRESS 0x2f

// strobes
#define SNOP 0x3d
#define SFSTXON 0x31
#define SXOFF 0x32

// extended registers
#define PARTNUMBER 0x8f
#define PARTVERSION 0x90

void strobe(uint8_t);
uint8_t transferByte(uint8_t, uint8_t);
uint8_t writeRegister(uint8_t, uint8_t);
uint8_t readRegister(uint8_t);
uint8_t readExtendedRegister(uint8_t);

struct cc1120_status
{
    uint8_t reserved : 4;
    uint8_t STATE : 3;
    uint8_t CHIP_RDY : 1;
};
union cc_st {
    struct cc1120_status st;
    uint8_t val;
};
union cc_st status;

#endif