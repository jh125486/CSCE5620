#include "tmp102.h"

void tmp102_setup()
{
    KI2CConf conf = {
        .addressing_mode = K_ADDRESSINGMODE_7BIT,
        .role = K_MASTER,
        .clock_speed = 10000};
    // Initialize first i2c bus with configuration
    k_i2c_init(I2C_DEV, &conf);
}

void tmp102_read_temperature(int *temp)
{
    uint8_t cmd = 0x00;
    uint16_t value;
    uint8_t buffer[2];

    k_i2c_write(I2C_DEV, I2C_SLAVE_ADDR, &cmd, 1);
    // Processing delay
    vTaskDelay(50);
    // Read back 3 byte response from slave
    k_i2c_read(I2C_DEV, I2C_SLAVE_ADDR, &buffer, 2);

    value = (buffer[0] << 4) | (buffer[1] >> 4);
    value /= 16;

    value *= 9 / 5.0;
    value += 32;

    *temp = value;
}