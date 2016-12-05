#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#include "kubos-core/modules/klog.h"
#include "kubos-hal/gpio.h"
#include "kubos-hal/i2c.h"
#include "kubos-hal/spi.h"
#include "kubos-hal/uart.h"

// TMP102 temperature driver
#include "tmp102.h"

// radio driver
#include "cc1120.h"

uint8_t i;
uint16_t temperature_sum = 0;
uint8_t temperature_count = 0;

void task_read_temp(void *p)
{
    static int x = 0;
    int ret;
    int temp;

    tmp102_setup();

#define SPI_BUS K_SPI1
    uint8_t val;
    /* data to send */
    uint8_t tx;
    /* data to receive */
    volatile uint8_t rx = 0;
    /* create own config */
    KSPIConf conf = {
        .role = K_SPI_MASTER,
        .direction = K_SPI_DIRECTION_2LINES,
        .data_size = K_SPI_DATASIZE_8BIT,
        .speed = 4000000};

    /* Initialize spi bus with configuration */
    k_spi_init(SPI_BUS, &conf);

    while (1)
    {
        tmp102_read_temperature(&temp);
        printf("temp read - %d\r\n", temp);
        tx = (uint8_t)temp;
        temperature_sum += temp;
        temperature_count++;

        /* Send single byte over spi and then receive it */
        strobe(SNOP);
        printf("cc1120_status.CHIP_RDY: %x", status.st.CHIP_RDY);
        printf(" cc1120_status..STATE: %x\r\n", status.st.STATE);

        val = readExtendedRegister(PARTNUMBER);
        printf("Partnumber: %x\r\n", val);

        val = readExtendedRegister(PARTVERSION);
        printf("Partversion: %x\r\n", val);

        writeRegister(FREQ0, i);
        printf("write Freq0: %x\r\n", i);

        val = readRegister(FREQ0);
        printf("Freq0: %x\r\n", val);

        if (i & 0x1)
        {
            strobe(SFSTXON);
        }
        else
        {
            strobe(SXOFF);
        }

        // k_gpio_write(FAKE_CS, 0);
        // k_spi_write_read(SPI_BUS, &tx, &rx, 1);
        // k_gpio_write(FAKE_CS, 1);

        /* print out over console */
        printf("tx: %d\trx: %d\r\n", tx, rx);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

void task_set_fan_speed(void *p)
{
    uint16_t speed = 0, new_speed = 0;
    uint8_t average_temperature;

    while (1)
    {
        printf("\t\t\tFan speed: %d\t%d[%d]\r\n", speed, temperature_sum, temperature_count);
        if (temperature_count)
        {
            average_temperature = temperature_sum / temperature_count;

            if (average_temperature > 80)
            {
                new_speed = 1000;
            }
            else if (average_temperature > 75)
            {
                new_speed = 750;
            }
            else if (average_temperature > 70)
            {
                new_speed = 500;
            }
            else
            {
                new_speed = 250;
            }

            if (new_speed != speed)
            {
                speed = new_speed;
                printf("New fan speed: %d\r\n", speed);
            }

            temperature_sum = 0;
            temperature_count = 0;
        }
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}

int main(void)
{
    k_uart_console_init();

#ifdef TARGET_LIKE_STM32
    k_gpio_init(K_LED_GREEN, K_GPIO_OUTPUT, K_GPIO_PULL_NONE);
    k_gpio_init(K_LED_ORANGE, K_GPIO_OUTPUT, K_GPIO_PULL_NONE);
    k_gpio_init(K_LED_RED, K_GPIO_OUTPUT, K_GPIO_PULL_NONE);
    k_gpio_init(K_LED_BLUE, K_GPIO_OUTPUT, K_GPIO_PULL_NONE);
    k_gpio_init(K_BUTTON_0, K_GPIO_INPUT, K_GPIO_PULL_NONE);
#endif

#ifdef TARGET_LIKE_MSP430
    k_gpio_init(K_LED_GREEN, K_GPIO_OUTPUT, K_GPIO_PULL_NONE);
    k_gpio_init(K_LED_RED, K_GPIO_OUTPUT, K_GPIO_PULL_NONE);
    k_gpio_init(K_BUTTON_0, K_GPIO_INPUT, K_GPIO_PULL_UP);

    /* Stop the watchdog. */
    WDTCTL = WDTPW + WDTHOLD;

    __enable_interrupt();

    P2OUT = BIT1;
#endif

    xTaskCreate(task_read_temp, "Read temp Task", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);
    xTaskCreate(task_set_fan_speed, "Set fan speed Task", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1)
        ;

    return 0;
}
