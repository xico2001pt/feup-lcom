#ifndef __UTILS_H__
#define __UTILS_H__

/** @defgroup utils utils
 * @{
 *
 * LCOM's utility functions
 */

#include <stdint.h>
#include <stdbool.h> 

#ifndef BIT
#  define BIT(n) (1 << (n))
#endif

/**
 * @brief Returns the LSB of a 2 byte integer
 *
 * @param val input 2 byte integer
 * @param lsb address of memory location to be updated with val's LSB
 * @return Return 0 upon success and non-zero otherwise
 */
int (util_get_LSB)(uint16_t val, uint8_t *lsb);

/**
 * @brief Returns the MSB of a 2 byte integer
 *
 * @param val input 2 byte integer
 * @param msb address of memory location to be updated with val's LSB
 * @return Return 0 upon success and non-zero otherwise
 */
int (util_get_MSB)(uint16_t val, uint8_t *msb);

/**
 * @brief Invokes sys_inb() system call but reads the value into a uint8_t variable.
 *
 * @param port the input port that is to be read
 * @param value address of 8-bit variable to be update with the value read
 * @return Return 0 upon success and non-zero otherwise
 */
int (util_sys_inb)(int port, uint8_t *value);

/**
 * Returns whether or not a given scandcode if make or break
 * @param scancode 
 * @return true if make, false if break
 */
bool scancode_is_make(uint8_t scancode);

typedef struct{
  uint8_t bit_no;
  uint32_t irq_set;
  int hook_id;
} device_info;

device_info create_device(uint8_t bit_no);

bool check_rectangle_collision(uint32_t x1, uint32_t y1, uint32_t w1, uint32_t h1, uint32_t x2, uint32_t y2, uint32_t w2, uint32_t h2);

bool check_circle_collision(uint32_t circle_x, uint32_t circle_y, uint16_t radius, uint32_t x, uint32_t y);

#endif
