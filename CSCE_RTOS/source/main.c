#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#include "cc112x_spi.h"
#include "hal_spi_rf_trxeb.h"
#include "kubos-core/modules/klog.h"
#include "kubos-hal/gpio.h"
#include "kubos-hal/i2c.h"
#include "kubos-hal/spi.h"
#include "kubos-hal/uart.h"

// TMP102 temperature driver
#include "tmp102.h"

union cc1120_status {
    struct
    {
        uint8_t reserved : 4;
        uint8_t STATE : 3;
        uint8_t CHIP_RDY : 1;
        uint8_t PREV_STATE : 3;
    } fields;
    uint8_t data;
};

union cc1120_status status;

uint8_t i;
uint16_t temperature_sum = 0;
uint8_t temperature_count = 0;

#define FAN_IDLE 250
#define FAN_LOW 500
#define FAN_MED 750
#define FAN_HIGH 1000
#define TEMP_TX_THRESHOLD 85

void debugPrintSTATE(int line)
{
    if (status.fields.PREV_STATE == status.fields.STATE)
        return;
    printf("[%d] STATE changed from %d to %d\r\n", line, status.fields.PREV_STATE, status.fields.STATE);
    status.fields.PREV_STATE = status.fields.STATE;
}

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

    uint8_t prev_state = status.fields.STATE;

    while (1)
    {
        tmp102_read_temperature(&temp);
        temperature_sum += temp;
        temperature_count++;
        printf("temp read - %d [%d: %d]\r\n", temp, temperature_count, temperature_sum);

        /* Send single byte over spi and then receive it */
        status.data = trxSpiCmdStrobe(CC112X_SNOP);
        debugPrintSTATE(__LINE__);

        // only TX if temp is greater or equal to temperature threshold
        if (temp >= TEMP_TX_THRESHOLD)
        {
            printf("TX[%d] temp %d!\r\n", i++, temp);
            // strobe to on
            status.data = trxSpiCmdStrobe(CC112X_STX);
            debugPrintSTATE(__LINE__);
            // put the radio into frequency on state
            status.data = trxSpiCmdStrobe(CC112X_SNOP);
            debugPrintSTATE(__LINE__);

            vTaskDelay(10);

            // load up FIFO
            tx = (uint8_t)temp;
            char dataToSend[] = "\05hello";
            cc112xSpiWriteTxFifo((uint8_t *)dataToSend, 6);

            vTaskDelay(10);

            // send an STX
            status.data = trxSpiCmdStrobe(CC112X_STX);
            debugPrintSTATE(__LINE__);

            // flush the transmit strobe if there is an error
            if (status.fields.STATE == 7)
            {
                printf("\r\nERROR: Transmit FIFO error\r\n");
                trxSpiCmdStrobe(CC112X_SFTX);
            }
        }
        else
        {
            k_gpio_write(FAKE_CS, 0);
        }
        vTaskDelay(980 / portTICK_RATE_MS);
    }
}

void task_set_fan_speed(void *p)
{
    uint16_t speed = FAN_IDLE, new_speed = FAN_IDLE;
    uint8_t average_temperature;

    while (1)
    {
        if (temperature_count)
        {
            average_temperature = temperature_sum / temperature_count;
            printf("  Fan speed: %d\tAverage Temp: %d\r\n", speed, average_temperature);
            if (average_temperature > TEMP_TX_THRESHOLD)
                new_speed = FAN_HIGH;
            else if (average_temperature > 80)
                new_speed = FAN_MED;
            else if (average_temperature > 75)
                new_speed = FAN_LOW;
            else
                new_speed = FAN_IDLE;

            if (new_speed != speed)
            {
                speed = new_speed;
                printf("! New speed set: %d\r\n", speed);
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

    // set up cc1120 RF
    configureRadio();

    xTaskCreate(task_read_temp, "Read temp Task", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);
    xTaskCreate(task_set_fan_speed, "Set fan speed Task", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);

    vTaskStartScheduler();

    while (1)
        ;

    return 0;
}
