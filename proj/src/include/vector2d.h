#ifndef __VECTOR2D_H__
#define __VECTOR2D_H__

#include <stdint.h>

/**
 * @brief Representation of unsigned 16 bit integer 2d vector
 */
typedef struct {
    uint16_t x; /**< x value */
    uint16_t y; /**< y value */
} vec2du_t;

/**
 * @brief Representation of 32bit floating point 2d vector
 */
typedef struct {
    float x; /**< x value */
    float y; /**< y value */
} vec2df_t;

/**
 * @brief Representation of signed 16bit integer 2d vector
 */
typedef struct {
    int16_t x; /**< x value */
    int16_t y; /**< y value */
} vec2di_t;

#endif /* __VECTOR_2D_H__ */
