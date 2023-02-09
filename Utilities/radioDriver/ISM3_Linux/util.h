/**
 ******************************************************************************
 * @file    util.h
 * @author  nicolas.brunner@heig-vd.ch & laurent.folladore@heig-vd.ch
 * @date    06-September-2012
 * @brief   Utility library, use big endian
 ******************************************************************************
 * @copyright HEIG-VD
 *
 * License information
 *
 ******************************************************************************
 */

#ifndef UTIL_H
#define UTIL_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

/* Exported macro ------------------------------------------------------------*/

#define UTIL_MIN(x, y)     ((x) < (y) ? (x) : (y))
#define UTIL_MAX(x, y)     ((x) > (y) ? (x) : (y))
#define UTIL_CEILING(x, y) (((x) + (y) - 1) / (y))

static inline uint16_t util_byte_array_to_uint16(const uint8_t* data) {
    return ((uint16_t)data[0] << 8) | data[1];
}

static inline uint32_t util_byte_array_to_uint24(const uint8_t* data) {
    return ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
}

static inline uint32_t util_byte_array_to_uint32(const uint8_t* data) {
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) | ((uint32_t)data[2] << 8) | data[3];
}

static inline uint64_t util_byte_array_to_uint64(const uint8_t* data) {
    return  ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) |
            ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32) |
            ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16) |
            ((uint64_t)data[6] << 8) | data[7];
}

static inline void util_uint16_to_byte_array(uint16_t uint, uint8_t* data) {
    data[0] = (uint8_t)((uint >> 8) & 0xFF);
    data[1] = (uint8_t)(uint & 0xFF);
}

static inline void util_uint24_to_byte_array(uint32_t uint, uint8_t* data) {
    data[0] = (uint8_t)((uint >> 16) & 0xFF);
    data[1] = (uint8_t)((uint >> 8) & 0xFF);
    data[2] = (uint8_t)(uint & 0xFF);
}

static inline void util_uint32_to_byte_array(uint32_t uint, uint8_t* data) {
    data[0] = (uint8_t)((uint >> 24) & 0xFF);
    data[1] = (uint8_t)((uint >> 16) & 0xFF);
    data[2] = (uint8_t)((uint >> 8) & 0xFF);
    data[3] = (uint8_t)(uint & 0xFF);
}

static inline void util_float_to_byte_array(float f, uint8_t* data) {
    util_uint32_to_byte_array(*((uint32_t*)(&f)), data);
}

static inline void util_uint64_to_byte_array(uint64_t uint, uint8_t* data) {
    data[0] = (uint8_t)((uint >> 56) & 0xFF);
    data[1] = (uint8_t)((uint >> 48) & 0xFF);
    data[2] = (uint8_t)((uint >> 40) & 0xFF);
    data[3] = (uint8_t)((uint >> 32) & 0xFF);
    data[4] = (uint8_t)((uint >> 24) & 0xFF);
    data[5] = (uint8_t)((uint >> 16) & 0xFF);
    data[6] = (uint8_t)((uint >> 8) & 0xFF);
    data[7] = (uint8_t)(uint & 0xFF);
}

static inline uint32_t util_division_nearest(uint32_t dividend, uint32_t divisor) {
    return (dividend + divisor/2) / divisor;
}

/* Exported functions --------------------------------------------------------*/

uint8_t util_get_number_of_bit_set(uint8_t value);

#endif
