#include <lcom/lcf.h>

#include <stdint.h>
#include <math.h>
#include "utils.h"

int(util_get_LSB)(uint16_t val, uint8_t *lsb) {
  *lsb = (uint8_t) val;
  return 0;
}

int(util_get_MSB)(uint16_t val, uint8_t *msb) {
  *msb = (uint8_t) (val >> 8);
  return 0;
}

int (util_sys_inb)(int port, uint8_t *value) {
  uint32_t value32;
  int status = sys_inb(port, &value32);
  *value = (uint8_t) value32;
  return status;
}

bool (scancode_is_make)(uint8_t scancode){
  return 1 - (scancode >> 7);
}

device_info create_device(uint8_t bit_no){
  device_info dev;
  dev.bit_no = bit_no;
  dev.irq_set = (uint32_t) BIT(dev.bit_no);
  return dev;
}

bool check_rectangle_collision(uint32_t x1, uint32_t y1, uint32_t w1, uint32_t h1, uint32_t x2, uint32_t y2, uint32_t w2, uint32_t h2){
    return x1 < (x2+w2) && (x1+w1) > x2 && y1 < (y2+h2) && (y1+h1) > y2;
}

bool check_circle_collision(uint32_t circle_x, uint32_t circle_y, uint16_t radius, uint32_t x, uint32_t y) {
    return (pow((int) x - (int) circle_x, 2) + pow((int) y - (int) circle_y, 2)) < pow(radius, 2);
}
