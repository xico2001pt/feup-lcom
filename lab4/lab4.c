// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

// Any header files included below this line should have been created by you
#include "i8254.h"
#include "i8042.h"
#include "kbc.h"
#include "mouse.h"
#include "timer.h"
#include "utils.h"

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

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

int (mouse_test_packet)(uint32_t cnt) {
    struct device_info mouse = create_device(MOUSE_IRQ);

    mouse_enable_reporting();
    if(mouse_subscribe_int(&mouse.bit_no) != OK){
      printf("%s: mouse_subscribe_int() failed\n", __func__);
      return 1;
    }
    mouse.hook_id = hook_id;

    int r, ipc_status;
    message msg;

    uint8_t bytes[3];
    uint8_t byte_count = 0;
    uint32_t packets_received = 0;

    while(packets_received < cnt) { /* You may want to use a different condition */
      /* Get a request message. */
      if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & mouse.irq_set){
              mouse_ih();
              if(out_byte & BIT(3) || byte_count != 0){
                bytes[byte_count] = out_byte;
                byte_count++;
              }
              if(byte_count == 3){
                byte_count = 0;
                packets_received++;

                struct packet pp = process_packets(bytes);

                mouse_print_packet(&pp);
              }
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */
        }
      } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
      }
    }

    hook_id = mouse.hook_id;
    if(mouse_unsubscribe_int() != 0){
      printf("%s: mouse_unsubscribe_int() failed\n", __func__);
    }
    if(mouse_disable_reporting() != 0){
      printf("%s: mouse_disable_reporting() failed\n", __func__);
    }
    return 0;
}

uint64_t timer_counter;
int (mouse_test_async)(uint8_t idle_time) {
    struct device_info timer = create_device(TIMER0_IRQ);
    struct device_info mouse = create_device(MOUSE_IRQ);

    mouse_enable_reporting();

    if(timer_subscribe_int(&timer.bit_no) != OK){
      printf("%s: timer_subscribe_int() failed\n", __func__);
      return 1;
    }
    timer.hook_id = hook_id;

    if(mouse_subscribe_int(&mouse.bit_no) != OK){
      printf("%s: mouse_subscribe_int() failed\n", __func__);
      return 1;
    }
    mouse.hook_id = hook_id;

    int r, ipc_status;
    message msg;

    const uint8_t FREQ = sys_hz();
    uint8_t bytes[3];
    uint8_t byte_count = 0;
    uint8_t seconds_elapsed = 0;

    while(seconds_elapsed < idle_time) { /* You may want to use a different condition */
      /* Get a request message. */
      if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & timer.irq_set) {
              timer_int_handler();
              if (timer_counter % FREQ == 0) {
                seconds_elapsed++;
              }
            }
            if (msg.m_notify.interrupts & mouse.irq_set){
              seconds_elapsed = 0; timer_counter = 0;
              mouse_ih();
              if(byte_count == 0){
                if(out_byte & BIT(3)){
                  bytes[byte_count] = out_byte;
                  byte_count++;
                }
              }
              else{
                bytes[byte_count] = out_byte;
                byte_count++;
              }
              if(byte_count == 3){
                byte_count = 0;

                struct packet mousep = process_packets(bytes);
                mouse_print_packet(&mousep);
              }
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */
      }
    } else { /* received a standard message, not a notification */
      /* no standard messages expected: do nothing */
    }
  }

  hook_id = mouse.hook_id;
  if(mouse_unsubscribe_int() != 0){
    printf("%s: mouse_unsubscribe_int() failed\n", __func__);
  }

  hook_id = timer.hook_id;
  if(timer_unsubscribe_int() != 0){
    printf("%s: timer_unsubscribe_int() failed\n", __func__);
  }

  if(mouse_disable_reporting() != 0) {
    printf("%s: mouse_disable_reporting() failed\n", __func__);
  }
  return 0;
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
    struct device_info mouse = create_device(MOUSE_IRQ);

    mouse_enable_reporting();
    if(mouse_subscribe_int(&mouse.bit_no) != OK){
      printf("%s: mouse_subscribe_int() failed\n", __func__);
      return 1;
    }
    mouse.hook_id = hook_id;

    int r, ipc_status;
    message msg;

    uint8_t bytes[3];
    uint8_t byte_count = 0;
    uint32_t packets_received = 0;

    bool LB = 0, RB = 0;
    bool over = false;

    while(!over) { /* You may want to use a different condition */
      /* Get a request message. */
      if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
        printf("driver_receive failed with: %d", r);
        continue;
      }
      if (is_ipc_notify(ipc_status)) { /* received notification */
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE: /* hardware interrupt notification */
            if (msg.m_notify.interrupts & mouse.irq_set){
              mouse_ih();
              if(out_byte & BIT(3) || byte_count != 0){
                bytes[byte_count] = out_byte;
                byte_count++;
              }
              if(byte_count == 3){
                byte_count = 0;
                packets_received++;

                event_type event = MOVE;
                struct packet pp = process_packets(bytes);
                if(pp.lb != LB){
                  event = pp.lb ? LBdown : LBup;
                  LB = pp.lb;
                }
                if(pp.rb != RB){
                  event = pp.rb ? RBdown : RBup;
                  RB = pp.rb;
                }
                struct event_t ev = {event, pp.delta_x, pp.delta_y};
                over = check_inv_v(&ev, x_len, tolerance);
                mouse_print_packet(&pp);
              }
            }
            break;
          default:
            break; /* no other notifications expected: do nothing */
        }
      } else { /* received a standard message, not a notification */
        /* no standard messages expected: do nothing */
      }
    }

    hook_id = mouse.hook_id;
    if(mouse_unsubscribe_int() != 0){
      printf("%s: mouse_unsubscribe_int() failed\n", __func__);
    }
    if(mouse_disable_reporting() != 0){
      printf("%s: mouse_disable_reporting() failed\n", __func__);
    }
    return 0;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    uint8_t packet[3];
    int counter = 0;

    while (cnt > 0) {
      read_data_remote();

      packet[counter++] = out_byte;
      if (counter == 1 && !(packet[0] & BIT(3))) {
        printf("unsynced");
        counter = 0;
      }
      if (counter == 3) {

        counter = 0;
        cnt--;
        struct packet pp = process_packets(packet);
        mouse_print_packet(&pp);
      }
      tickdelay(micros_to_ticks(period * 1000));
    }

    // set streaming mode, data report dis and reset cmd byte
    if (mouse_set_streaming_mode() != 0) {
      printf("%s:mouse_set_streaming_mode() failed\n", __func__);
      return 1;
    }
    if (mouse_disable_reporting() != 0) {
      printf("%s:mouse_disable_reporting() failed\n", __func__);
      return 1;
    }

    uint8_t cmd_byte = minix_get_dflt_kbc_cmd_byte();
    if(kbc_issue_command(KBC_WR_CMD, KBC_CMD_REG) != 0){
      printf("%s:kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if(kbc_issue_command(cmd_byte, KBC_OUT_BUF) != 0){
      printf("%s:kbc_issue_command() failed\n", __func__);
      return 1;
    }
    return 0;
}
