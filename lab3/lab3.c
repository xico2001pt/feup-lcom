#include <lcom/lcf.h>
#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>
#include "utils.h"
#include "timer.h"
#include <i8042.h>
#include "keyboard.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern uint8_t out_byte;
extern int hook_id;

int(kbd_test_scan)() {
  uint8_t bit_no = 1;
  uint32_t irq_set = BIT(bit_no);
  int r, ipc_status, size = 0;
  uint8_t bytes[2];
  message msg;

  if (kbd_subscribe_int(&bit_no) != OK)
    return 1;

  while(bytes[0] != ESC_BREAKCODE) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */
          if (msg.m_notify.interrupts & irq_set) { /* subscribed interrupt */
            kbc_ih();
            bytes[size] = out_byte;

            if(out_byte == 0)
              return 1;

            if(out_byte != 0xE0) {
              kbd_print_scancode(scancode_is_make(out_byte), size + 1, bytes);
              size = 0;
            }
            else
              size++;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    } else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
    }
  }
  kbd_unsubscribe_int(bit_no);
  return 0;
}

int(kbd_test_poll)() {
  uint8_t bytes[2];
  int n = 0;
  while(out_byte != ESC_BREAKCODE){
    kbc_poll();
    if(out_byte){
      bytes[n] = out_byte;
      if(out_byte == 0xE0){
        n = 1;
      }
      else{
        kbd_print_scancode(scancode_is_make(out_byte), n+1, bytes);
        n = 0;
      }
    }
  }
  
  // Read current command byte to outbyte
  kbc_issue_command(KBC_RD_CMD, KBC_CMD_REG);
  kbc_read_outb();

  // Write command byte with interrupts enabled
  kbc_issue_command(KBC_WR_CMD, KBC_CMD_REG);
  kbc_issue_command(out_byte | BIT(0), KBC_OUT_BUF);
  return 0;
}

uint64_t timer_counter;
int(kbd_test_timed_scan)(uint8_t n) {
  struct device_info timer = create_device(0);
  if(timer_subscribe_int(&timer.bit_no) != OK){
    printf("%s: timer_subscribe_int() failed\n", __func__);
    return 1;
  }
  timer.hook_id = hook_id;

  struct device_info kbd = create_device(1);
  if(kbd_subscribe_int(&kbd.bit_no) != OK){
    printf("%s: kb_subscribe_int() failed\n", __func__);
    return 1;
  }
  kbd.hook_id = hook_id;

  const uint8_t FREQ = 60;
  int r, ipc_status, size = 0;
  uint8_t bytes[2];
  message msg;

  uint8_t seconds_elapsed = 0;

  while(bytes[0] != ESC_BREAKCODE && seconds_elapsed < n) { /* You may want to use a different condition */
    /* Get a request message. */
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE: /* hardware interrupt notification */
          if(msg.m_notify.interrupts & timer.irq_set){
            timer_int_handler();
            if(timer_counter % FREQ == 0){
              seconds_elapsed++;
            }
          }
          if (msg.m_notify.interrupts & kbd.irq_set) { /* subscribed interrupt */
            seconds_elapsed = 0; timer_counter = 0; // Reset the timer
            kbc_ih();
            bytes[size] = out_byte;

            if(out_byte == 0)
              return 1;

            if(out_byte != 0xE0) {
              kbd_print_scancode(scancode_is_make(out_byte), size + 1, bytes);
              size = 0;
            }
            else
              size++;
          }
          break;
        default:
          break; /* no other notifications expected: do nothing */
      }
    } else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
    }
  }

  hook_id = kbd.hook_id;
  kbd_unsubscribe_int();
  hook_id = timer.hook_id;
  timer_unsubscribe_int();
  return 0;
}
