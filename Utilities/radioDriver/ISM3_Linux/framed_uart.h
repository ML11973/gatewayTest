/**
 ******************************************************************************
 * @file    framed_uart.h
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

#ifndef FRAMED_UART_H
#define FRAMED_UART_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

typedef void (*framed_uart_function_t)(const uint8_t* data, uint16_t size);

/* Exported functions --------------------------------------------------------*/

void framed_uart_init(framed_uart_function_t rx_function, uint32_t frame_timeout);

void framed_uart_deinit(void);

void framed_uart_start(void);

void framed_uart_set_baudrate(uint32_t baudrate);

void framed_uart_flush(void);

void framed_uart_tick(void);

bool framed_uart_tx(const uint8_t* data, uint16_t size);

bool framed_uart_is_busy(void);

#endif
