#include "cc1120.h"

void strobe(uint8_t strobe)
{
    uint8_t tx = READ_BIT | strobe;
    volatile uint8_t rx = 0;

    k_gpio_write(FAKE_CS, 0);
    k_spi_write_read(SPI_BUS, &tx, &rx, 1);
    status.val = rx;
    k_gpio_write(FAKE_CS, 1);
}

uint8_t transferByte(uint8_t cmd_address, uint8_t value)
{
    uint8_t r;
    volatile uint8_t rx = 0;

    k_gpio_write(FAKE_CS, 0);
    k_spi_write_read(SPI_BUS, &cmd_address, &rx, 1);
    status.val = rx;
    k_spi_write_read(SPI_BUS, &value, &rx, 1);
    r = value;
    k_gpio_write(FAKE_CS, 1);

    return r;
}
uint8_t writeRegister(uint8_t address, uint8_t value)
{
    return transferByte(address, value);
}

uint8_t readRegister(uint8_t address)
{
    return transferByte(READ_BIT | address, 0x00);
}

uint8_t readExtendedRegister(uint8_t extended_address)
{
    uint8_t r;
    uint8_t tx = READ_BIT | EXTENDED_ADDRESS;
    volatile uint8_t rx = 0;

    k_gpio_write(FAKE_CS, 0);
    k_spi_write_read(SPI_BUS, &tx, &rx, 1);
    status.val = rx;
    k_spi_write_read(SPI_BUS, &extended_address, &rx, 1);
    tx = 0xff;
    k_spi_write_read(SPI_BUS, &tx, &rx, 1);
    r = rx;
    k_gpio_write(FAKE_CS, 1);

    //SPI.beginTransaction(s);
    //digitalWrite(cs_cc1120, LOW);
    //status.val = SPI.transfer(READ_BIT | EXTENDED_ADDRESS);
    //SPI.transfer(extended_address);
    //r = SPI.transfer(0xff);
    //digitalWrite(cs_cc1120, HIGH);
    //SPI.endTransaction();
    return r;
}
