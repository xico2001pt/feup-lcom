// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <video_gr.h>
#include <vbe.h>
#include <stdint.h>
#include <stdio.h>
#include <i8042.h>
#include <keyboard.h>
#include <sprite.h>

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

uint64_t timer_counter = 0;

int(video_test_init)(uint16_t mode, uint8_t delay) {
  vg_init(mode);

  // RTC
  struct reg86 r;
  memset(&r, 0, sizeof(r));
  r.intno = 0x1a;
  r.ah = 0x04;
  if( (sys_int86)(&r) != OK ) {
    printf("\tsys_int86() failed \n");
    return 1;
  }
  printf("century:%x | %x/%x/%x  ", r.ch, r.dl, r.dh, r.cl);
  memset(&r, 0, sizeof(r));
  r.intno = 0x1a;
  r.ah = 0x02;
  if( (sys_int86)(&r) != OK ) {
    printf("\tsys_int86() failed \n");
    return 1;
  }
  printf("%x:%x:%x\n", r.ch, r.cl, r.dh);

  vg_draw_rectangle(0,0,1024,768, 0x14);
  vg_draw_char();
  vg_show_buffer();
  tickdelay(micros_to_ticks(delay*1000000));
  vg_exit();
  return 1;
}

extern uint8_t out_byte;
extern int hook_id;

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  uint8_t bit_no = 1;
  uint32_t irq_set = BIT(bit_no);
  int r, ipc_status, size = 0;
  uint8_t bytes[2];
  message msg;

  vg_init(mode);
  vg_draw_rectangle(x,y,width,height,color);
  vg_show_buffer();

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
  vg_exit();

  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  uint8_t bit_no = 1;
  uint32_t irq_set = BIT(bit_no);
  int r, ipc_status, size = 0;
  uint8_t bytes[2];
  message msg;

  vg_init(mode);
  vg_draw_pattern(mode, no_rectangles, first, step);
  vg_show_buffer();

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
  vg_exit();
  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  uint8_t bit_no = 1;
  uint32_t irq_set = BIT(bit_no);
  int r, ipc_status, size = 0;
  uint8_t bytes[2];
  message msg;

  vg_init(MODE_1024x768_INDEX);
  vg_draw_xpm(xpm, x,y);
  vg_show_buffer();

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
  vg_exit();

  return 0;
}

int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {
  uint8_t bytes[2];
  int size = 0;
  fr_rate = sys_hz() / fr_rate;

  struct device_info timer = create_device(0);
  timer_subscribe_int(&timer.bit_no);
  timer.hook_id = hook_id;
  printf("timer subscribed\n");

  struct device_info kb = create_device(1);
  kbd_subscribe_int(&kb.bit_no);
  kb.hook_id = hook_id;
  printf("keyboard subscribed\n");


  vg_init(MODE_1024x768_INDEX);

  Sprite * sprite = create_sprite(xpm, xi, yi, 0, 0);

  int frames_between_update = 1;
  if(speed < 0){
    frames_between_update = -speed;
    if(xi == xf){
      sprite->yspeed = 1;
    }
    else{
      sprite->xspeed = 1;
    }
  }
  else{
    if(xi == xf){
      sprite->yspeed = speed;
    }
    else{
      sprite->xspeed = speed;
    }
  }

  int ipc_status;
  message msg;
  timer_counter = 1;
  int frames_elapsed = 0;

  while(bytes[0] != ESC_BREAKCODE) { /* You may want to use a different condition */
    /* Get a request message. */
    int r;
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { /* received notification */
      switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE: /* hardware interrupt notification */
              if (msg.m_notify.interrupts & timer.irq_set) { /* subscribed interrupt */
                  timer_int_handler();  /* process it */
                  if((timer_counter) % fr_rate == 0){
                    frames_elapsed++;
                    if(frames_elapsed == frames_between_update){
                      // Prevent sprite from overshooting the target position
                      if( (sprite->xspeed > 0 && sprite->x > xf) || (sprite->xspeed < 0 && sprite->x < xf) ||
                      (sprite->yspeed > 0 && sprite->y > yf) ||
                      (sprite->yspeed < 0 && sprite->y < yf)) {
                        sprite->xspeed = 0;
                        sprite->yspeed = 0;
                        sprite->x = xf;
                        sprite->y = yf;
                      }
                      if (sprite->x != xf || sprite->y != yf) {
                        animate_sprite(sprite);
                        vg_show_buffer();
                      }
                      frames_elapsed = 0;
                    }
                  }   
              }
              if (msg.m_notify.interrupts & kb.irq_set) { /* subscribed interrupt */
                kbc_ih();
                bytes[size] = out_byte;

                if(out_byte == 0)
                  return 1;

                if(out_byte != 0xE0) {
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
  destroy_sprite(sprite);
  hook_id = timer.hook_id;
  timer_unsubscribe_int();
  hook_id = kb.hook_id;
  kbd_unsubscribe_int();
  vg_exit();

  return 0;
}

int(video_test_controller)() {
  vg_vbe_contr_info_t info_p;
  if (vbe_controller_get_info(&info_p)) {
    return 1;
  }
  if (vg_display_vbe_contr_info(&info_p)) {
    printf("Error while trying to display controller info\n");
    return 1;
  }
  return 0;
}
