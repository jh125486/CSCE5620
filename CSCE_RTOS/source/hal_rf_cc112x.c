#include <stdint.h>

#include "cc112x_spi.h"
#include "hal_rf.h"

uint8_t halRfWriteReg(uint16_t addr, uint8_t data)
{
  return cc112xSpiWriteReg(addr, &data, 1);
}