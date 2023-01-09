/**
 ******************************************************************************
 * @file    framed_uart.c
 * @author  nicolas.brunner@heig-vd.ch
 * @date    06-August-2018
 * @brief   Driver for UART using frame
 ******************************************************************************
 * @copyright HEIG-VD
 *
 * License information
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include <assert.h>
#include <time.h>
#include <stdio.h>

#include "buffered_uart.h"
#include "framed_uart.h"

/* Private define ------------------------------------------------------------*/

#define RX_MAX_FRAME_SIZE 256

/* Private variables ---------------------------------------------------------*/

static uint8_t rx_frame[RX_MAX_FRAME_SIZE];
static uint16_t rx_frame_index = 0;
static uint32_t tick_rx_start;
static uint32_t frame_timeout = 100;
static framed_uart_function_t rx_function;

/* Public functions ----------------------------------------------------------*/

void framed_uart_init(framed_uart_function_t rx_function_, uint32_t frame_timeout_)
{
    rx_function = rx_function_;
    frame_timeout = frame_timeout_;
    rx_frame_index = 0;

    buffered_uart_init(9600, 8, PARITY_NONE, FLOW_CONTROL_NONE);
}

void framed_uart_deinit(void)
{
    buffered_uart_deinit();
}

void framed_uart_start(void)
{
    buffered_uart_start();
}

bool framed_uart_tx(const uint8_t* data, uint16_t size)
{
    if (buffered_uart_get_write_available() >= size) {
        buffered_uart_write(data, size);
        return true;
    }
    return false;
}

void framed_uart_set_baudrate(uint32_t baudrate)
{
    buffered_uart_set_baudrate(baudrate);
}

void framed_uart_flush(void)
{
    rx_frame_index = 0;
}

void framed_uart_tick(void)
{
    // Read the first byte (size of frame)
    if (rx_frame_index == 0) {
        if (buffered_uart_get_read_available() > 0) {
            //printf("buffered_uart_read\n");
            buffered_uart_read(rx_frame, 1);
            if (rx_frame[0] == 0) {
                // one byte frame
                rx_function(rx_frame, 1);
                return;
            }
            rx_frame_index = 1;
            tick_rx_start = clock();
        }
    }

    if (rx_frame_index != 0) {
        // Read the next bytes up to frame size
        if (buffered_uart_get_read_available() > 0) {
            uint16_t read_size = buffered_uart_read(&rx_frame[rx_frame_index], rx_frame[0] - (rx_frame_index - 1));
            rx_frame_index += read_size;
            if ((rx_frame_index - 1) == rx_frame[0]) {
                rx_function(rx_frame, rx_frame_index);
                rx_frame_index = 0;
            }
        }
    }

    if (rx_frame_index != 0) {
        // Check timeout
        uint32_t tick = clock();
        if ((tick - tick_rx_start) > frame_timeout) {
            rx_function(rx_frame, rx_frame_index);
            rx_frame_index = 0;
        }
    }
}

bool framed_uart_is_busy(void)
{
    return (rx_frame_index != 0) || buffered_uart_is_busy();
}
