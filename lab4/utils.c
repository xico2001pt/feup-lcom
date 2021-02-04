#include <lcom/lcf.h>

#include <stdint.h>
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

struct device_info create_device(uint8_t bit_no){
  struct device_info dev;
  dev.bit_no = bit_no;
  dev.irq_set = (uint32_t) BIT(dev.bit_no);
  return dev;
}
