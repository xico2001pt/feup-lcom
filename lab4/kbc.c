#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include <i8042.h>

int kbc_issue_command(uint8_t cmd, int port) {
  uint8_t stat = 0;
  int cycles = 0;
  while(cycles++ < 20) {
    if (util_sys_inb(KBC_ST_REG, &stat) != OK)
      return 1;
    if((stat & KBC_ST_IBF) == 0 ) {
      sys_outb(port, cmd);
      //printf("%s: Wrote 0x%x to 0x%x\n", __func__, cmd, KBC_CMD_REG);
      return 0;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
  printf("%s: Exiting!\n", __func__);
  return 1;
}

uint8_t out_byte;
int kbc_read_outb(){
  out_byte = 0;
  uint8_t stat;
  if(util_sys_inb(KBC_ST_REG, &stat) != OK){
    printf("%s: util_sys_inb() failed\n", __func__);
    return 1;
  } 
  if( stat & KBC_ST_OBF ) {
    uint8_t data;
    if(util_sys_inb(KBC_OUT_BUF, &data) != OK){
      printf("%s: util_sys_inb() failed\n", __func__);
      return 1;
    }
    if ( (stat &(KBC_PAR_ERR | KBC_TO_ERR)) == 0 ){
      out_byte = data;
      return 0;
    }
    else {
      printf("%s: KBC_PAR_ERR or KBC_TO_ERR occured\n", __func__);
      return 1;
    }
  }
  printf("Failed\n");
  return 1;
}

void(kbc_ih)(){
  while( 1 ) {
    if(kbc_read_outb() == 0){
      break;
    }
    tickdelay(micros_to_ticks(DELAY_US));
  }
}

void kbc_poll(){
  kbc_read_outb();
  tickdelay(micros_to_ticks(DELAY_US));
}
