/**
 * @file cm4_utils.h
 * @author marc.leemann@master.hes-so.ch
 * @brief Utilities for the Pi CM4 platform. Linux compatible.
 */

#ifndef CM4_UTILS_H
#define CM4_UTILS_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Redeclaration of time/nanosleep to make parser and compiler
 * happy
 * @param req requested delay
 * @param rem remaining delay
 */
int nanosleep(const struct timespec *req, struct timespec *rem);
/**
 * @brief Sleep for argument milliseconds
 * @param milliseconds delay in ms
 */
void delayMs(uint32_t milliseconds);
/**
 * @brief Print buffer in console
 * @param buffer pointer to buffer
 * @param size buffer size
 */
void printBuffer(const uint8_t * buffer, int size);
/**
 * @brief Print buffer in console, hexadecimal format
 * @param buffer pointer to buffer
 * @param size buffer size
 */
void printBufferHex(const uint8_t * buffer, int size);

#endif // CM4_UTILS_H
