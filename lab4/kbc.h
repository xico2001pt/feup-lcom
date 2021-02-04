#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Send cmd to the specified port
 * @param cmd Command byte
 * @param port Port
 */
int kbc_issue_command(uint8_t cmd, int port);

/**
 * Read KBC's OUTB and return it through extern uint8_t out_byte
 */
int kbc_read_outb();

/**
 * Read a single byte using kbc_read_outb and delay the specified time
 */ 
void kbc_poll();

#endif /* _KEYBOARD_H_ */
