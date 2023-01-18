/**
 ******************************************************************************
 * @file    buffered_uart.h
 * @author  nicolas.brunner@heig-vd.ch
 * @date    07-August-2018
 * @brief   Driver for UART using circular buffer
 ******************************************************************************
 * @copyright HEIG-VD
 *
 * License information
 *
 ******************************************************************************
 */

#ifndef BUFFERED_UART_H
#define BUFFERED_UART_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "hardware.h"

/* Exported types ------------------------------------------------------------*/

typedef enum {PARITY_NONE, PARITY_EVEN, PARITY_ODD} parity_t;
typedef enum {STOP_BITS_1, STOP_BITS_2} stopBits_t;
typedef enum {FLOW_CONTROL_NONE, FLOW_CONTROL_RTS_CTS} flowControl_t;

/* Exported functions --------------------------------------------------------*/

void buffered_uart_init(uint32_t baudrate, uint32_t word_length, parity_t parity, flowControl_t flow_control);
void buffered_uart_deinit(void);
void buffered_uart_start(void);
void buffered_uart_set_baudrate(uint32_t baudrate);

uint16_t buffered_uart_read(uint8_t* data, uint16_t size);

uint16_t buffered_uart_write(const uint8_t* data, uint16_t size);

uint16_t buffered_uart_get_read_available(void);

uint16_t buffered_uart_get_write_available(void);

bool buffered_uart_is_busy(void);

#endif
