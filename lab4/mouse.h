#ifndef __MOUSE_H_
#define __MOUSE_H_

typedef enum {RBup, RBdown, LBup, LBdown, MOVE} event_type;
typedef enum {INITIAL, FORWARD_SLASH, VERTEX, BACK_SLASH} state_t;


struct event_t{
  event_type type;
  int16_t mouse_x;
  int16_t mouse_y;
};

bool check_inv_v(struct event_t *ev, uint8_t x_len, uint8_t tolerance);

int mouse_subscribe_int(uint8_t *bit_no);

int mouse_unsubscribe_int();

int mouse_enable_reporting();

int mouse_disable_reporting();

int read_data_remote();

int mouse_set_streaming_mode();

struct packet process_packets(uint8_t *bytes);

uint16_t cpl2_delta(uint8_t byte, bool MSB);

#define MOUSE_LB BIT(0)
#define MOUSE_RB BIT(1)
#define MOUSE_MB BIT(2)
#define MOUSE_MSB_X BIT(4)
#define MOUSE_MSB_Y BIT(5)
#define MOUSE_X_OVFL BIT(6)
#define MOUSE_Y_OVFL BIT(7)

#endif
