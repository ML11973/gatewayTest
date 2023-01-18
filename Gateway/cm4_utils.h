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
