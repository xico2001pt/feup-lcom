#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include "kbc.h"
#include <mouse.h>
#include <kbc.h>
#include <i8042.h>

int hook_id;
int mouse_subscribe_int(uint8_t *bit_no) {
  hook_id = MOUSE_IRQ;
  *bit_no = (uint8_t) hook_id;
  if(sys_irqsetpolicy(hook_id, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id) != OK){
    printf("%s: sys_irqsetpolicy() failed\n", __func__);
    return 1;
  }
  return 0;
}

int mouse_unsubscribe_int() {
  if(sys_irqrmpolicy(&hook_id) != OK){
      printf("%s: sys_irqrmpolicy() failed\n", __func__);
      return 1;
  }
  return 0;
}


extern uint8_t out_byte;
int mouse_enable_reporting() {
  for(int i = 0; i < 5; ++i){
    if(kbc_issue_command(WRITE_BYTE_TO_MOUSE, KBC_CMD_REG) != 0){
      printf("%s:kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if(kbc_issue_command(ENABLE_DATA_REPORTING, KBC_OUT_BUF) != 0){
      printf("%s: kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if(util_sys_inb(KBC_OUT_BUF, &out_byte) != 0){
      printf("%s: util_sys_inb() failed\n", __func__);
      return 1;
    }
    if(out_byte == ACK){
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  printf("%s: enable mouse data reporting failed\n", __func__);
  return 1;
}

void(mouse_ih)(){
  while( 1 ) {
    if(kbc_read_outb() == 0){
      break;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}

int mouse_disable_reporting(){
  for(int i = 0; i < 5; ++i){
    if(kbc_issue_command(WRITE_BYTE_TO_MOUSE, KBC_CMD_REG) != 0){
      printf("%s:kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if(kbc_issue_command(DISABLE_DATA_REPORTING, KBC_OUT_BUF) != 0){
      printf("%s: kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if(util_sys_inb(KBC_OUT_BUF, &out_byte) != 0){
      printf("%s: util_sys_inb() failed\n", __func__);
      return 1;
    }
    if(out_byte == ACK){
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  printf("%s: disable mouse data reporting failed\n", __func__);
  return 1;
}

int read_data_remote() {
  for(int i = 0; i < 5; ++i) {
    if (kbc_issue_command(WRITE_BYTE_TO_MOUSE, KBC_CMD_REG) != 0) {
      printf("%s:kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if (kbc_issue_command(READ_DATA, KBC_OUT_BUF) != 0) {
      printf("%s: kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if (kbc_read_outb() != 0) {
      printf("%s: kbc_read_outb() failed\n", __func__);
      return 1;
    }
    if (out_byte != ACK) {
      continue;
    }
    if (kbc_read_outb() == 0) {
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  return 1;
}

int mouse_set_streaming_mode() {
  for(int i = 0; i < 5; ++i) {
    if (kbc_issue_command(WRITE_BYTE_TO_MOUSE, KBC_CMD_REG) != 0) {
      printf("%s:kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if (kbc_issue_command(SET_STREAM_MODE, KBC_OUT_BUF) != 0) {
      printf("%s: kbc_issue_command() failed\n", __func__);
      return 1;
    }
    if (kbc_read_outb() != 0) {
      printf("%s: kbc_read_outb() failed\n", __func__);
      return 1;
    }
    if (out_byte == ACK)
      return 0;
  }
  return 1;
}

struct packet process_packets(uint8_t *bytes){
  struct packet pp;
  for(int i = 0; i < 3; ++i){
    pp.bytes[i] = bytes[i];
  }
  pp.rb = bytes[0] & MOUSE_RB;
  pp.lb = bytes[0] & MOUSE_LB;
  pp.mb = bytes[0] & MOUSE_MB;
  pp.x_ov = bytes[0] & MOUSE_X_OVFL; 
  pp.y_ov = bytes[0] & MOUSE_Y_OVFL;

  pp.delta_x = bytes[1];
  bool msb_x = bytes[0] & MOUSE_MSB_X;
  for(int i = 8; i < 16; ++i){
    pp.delta_x += (msb_x << i);
  }

  pp.delta_y = bytes[2];
  bool msb_y = bytes[0] & MOUSE_MSB_Y;
  for(int i = 8; i < 16; ++i){
    pp.delta_y += (msb_y << i);
  }
  return pp;
}

uint16_t cpl2_delta(uint8_t byte, bool MSB) {
  uint16_t num = byte;
  if(MSB)
    num -= 256;
  return num;
}

bool check_inv_v(struct event_t *ev, uint8_t x_len, uint8_t tolerance){
  static state_t st = INITIAL;
  static int x_displace;
  switch(st){
    case INITIAL:
      printf("INITIAL\n");
      if(ev->type == LBdown){
        st = FORWARD_SLASH;
      }
      break;


    case FORWARD_SLASH:
      printf("FORWARD_SLASH\n");
      if(ev->type == MOVE){
        if(ev->mouse_x < 0 || ev->mouse_y < 0){
          if(ev->mouse_x < 0 && abs(ev->mouse_x) > tolerance){
            st = INITIAL;
          }
          else if(ev->mouse_y < 0 && abs(ev->mouse_y) > tolerance){
            st = INITIAL;
          }
        }        
        else if(ev->mouse_x == 0 || abs(ev->mouse_y / ev->mouse_x) < 1){
          st = INITIAL;
        }
        x_displace += ev->mouse_x;
      }
      else{
        if(ev->type == LBup && x_displace >= x_len){
          st = VERTEX;
          x_displace = 0;
        }
        else{
          st = INITIAL;
        }
      }
      break;


      case VERTEX:
        printf("VERTEX\n");
        if(ev->type == RBdown){
          st = BACK_SLASH;
        }
        else if(ev->type == MOVE){
          if(abs(ev->mouse_x) > tolerance || abs(ev->mouse_y) > tolerance){
            st = INITIAL;
          }
        }
        else{
          st = INITIAL;
        }
        break;


    case BACK_SLASH:
      printf("BACK_SLASH\n");
      if(ev->type == MOVE){
        if(ev->mouse_x < 0 || ev->mouse_y > 0){
          if(ev->mouse_x < 0 && abs(ev->mouse_x) > tolerance){
            st = INITIAL;
          }
          else if(ev->mouse_y > 0 && abs(ev->mouse_y) > tolerance){
            st = INITIAL;
          }
        }        
        else if(ev->mouse_x == 0 || abs(ev->mouse_y / ev->mouse_x) < 1){
          st = INITIAL;
        }
        x_displace += ev->mouse_x;
      }
      else{
        if(ev->type == RBup && x_displace >= x_len){
          printf("FINAL\n");
          return true;
        }
        else{
          st = INITIAL;
        }
      }
  }
  return false;
}
