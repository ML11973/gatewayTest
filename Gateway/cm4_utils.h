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

int nanosleep(const struct timespec *req, struct timespec *rem);
void delayMs(uint32_t milliseconds);
void printBuffer(const uint8_t * buffer, int size);
void printBufferHex(const uint8_t * buffer, int size);

#endif // CM4_UTILS_H
