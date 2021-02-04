#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
#include "utils.h"

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
    uint16_t initial_value = TIMER_FREQ / freq;
    uint8_t status, control_word, mask, ivlsb, ivmsb, timer_port;
    switch (timer) {
        case 0:
            control_word = TIMER_SEL0;
            timer_port = TIMER_0;
            break;
        case 1:
            control_word = TIMER_SEL1;
            timer_port = TIMER_1;
            break;
        case 2:
            control_word = TIMER_SEL2;
            timer_port = TIMER_2;
            break;
        default:
            return 1;
    }
    if (timer_get_conf(timer, &status))
        return 1;

    mask = BIT(3) | BIT(2) | BIT(1) | BIT(0);
    control_word |= TIMER_LSB_MSB | (status & mask);

    if (sys_outb(TIMER_CTRL, control_word) != OK){
        printf("%s: sys_outb() failed\n", __func__);
        return 1;
    }
        

    if (util_get_LSB(initial_value, &ivlsb) || util_get_MSB(initial_value, &ivmsb))
        return 1;

    if (sys_outb(timer_port, ivlsb) != OK || sys_outb(timer_port, ivmsb) != OK){
        printf("%s: sys_outb() failed\n", __func__);
        return 1;
    }

    return 0;
}

int hook_id;
int (timer_subscribe_int)(uint8_t *bit_no) {
  hook_id = TIMER0_IRQ;
  *bit_no = (uint8_t) hook_id;
  if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != OK){
      printf("%s: sys_irqsetpolicy() failed\n", __func__);
      return 1;
  }
  printf("%s: hook_id=0x%x\n", __func__,  hook_id);
  return 0;
}

int (timer_unsubscribe_int)() {
  if(sys_irqrmpolicy(&hook_id) != OK){
      printf("%s: sys_irqrmpolicy() failed\n", __func__);
      return 1;
  }
  return 0;
}

uint64_t timer_counter;
void(timer_int_handler)() {
  timer_counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  uint8_t timer_port;
  switch(timer){
      case 0: timer_port = TIMER_0; break;
      case 1: timer_port = TIMER_1; break;
      case 2: timer_port = TIMER_2; break;
  }
  uint8_t cmd = TIMER_RB_CMD | !TIMER_RB_STATUS_ | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
  if(sys_outb(TIMER_CTRL, cmd) != OK){
    printf("%s: sys_outb() failed\n", __func__);
    return 1;
  }

  if(util_sys_inb(timer_port, st) != OK){
    printf("%s: util_sys_inb() failed\n", __func__);
    return 1;
  }
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st,
                        enum timer_status_field field) {
    union timer_status_field_val conf;
    switch (field) {
        case tsf_all:
            conf.byte = st;
            break;
        case tsf_initial:
            conf.in_mode = (st & (BIT(5) | BIT(4))) >> 4;
            break;
        case tsf_mode:
            conf.count_mode = (st & (BIT(3) | BIT(2) | BIT(1))) >> 1;
            break;
        case tsf_base:
            conf.bcd = st & BIT(0);
            break;
    }
    if (timer_print_config(timer, field, conf))
        return 1;
    return 0;
}
