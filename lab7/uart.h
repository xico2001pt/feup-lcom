#ifndef __UART__
#define __UART__

#define COM1_IRQ 4
#define COM1_BASE_ADDR 0x3F8
#define COM2_IRQ 3
#define COM2_BASE_ADDR 0x2F8

// Register Adresses
#define RB_REG 0
#define TH_REG 0
#define DIV_LATCH_LSB_REG 0
#define IE_REG 1
#define DIV_LATCH_MSB_REG 1
#define II_REG 2
#define FC_REG 2
#define LC_REG 3
#define LS_REG 5

// LCR (Line Control Register) Control bits
#define LCR_8_BPC (BIT(1) | BIT(0)) // 8 bits per char
#define LCR_1_STOP_BIT 0
#define LCR_2_STOP_BIT BIT(2)
#define LCR_NO_PARITY 0
#define LCR_ODD_PARITY BIT(3)
#define LCR_EVEN_PARITY (BIT(4) | BIT(3))
#define LCR_BREAK_CONTROL BIT(6)
#define LCR_DLAB BIT(7)
#define LCR_RW 0

// LSR (Line Status Register)
#define LSR_RECEIVER_DATA BIT(0)
#define LSR_OVERRUN_ERROR BIT(1)
#define LSR_PARITY_ERROR BIT(2)
#define LSR_FRAMING_ERROR BIT(3)
#define LSR_BREAK_INTERRUPT BIT(4)
#define LSR_THR_EMPTY BIT(5)
#define LSR_TRANSMITTER_EMPTY BIT(6)
#define LSR_FIFO_ERROR BIT(7)

// IER (Interrupt Enable Register)
#define IER_RECEIVED_DATA_AVAIL BIT(0)
#define IER_THR_EMPTY BIT(1)
#define IER_RECEIVER_LINE_STATUS BIT(2)

// IIR (Interrupt Identification Register)
#define IIR_NO_INT BIT(0)
//      Pending interrupts:
#define IIR_INTERRUPT_MASK (BIT(3) | BIT(2) | BIT(1))
#define IIR_RECEIVER_LINE_STATUS (BIT(2) | BIT(1))
#define IIR_RECEIVED_DATA_AVAIL BIT(2)
#define IIR_CHAR_TIMEOUT (BIT(3) | BIT(2))
#define IIR_THR_EMPTY BIT(1)
//      FIFO Stuff:
#define IIR_64BYTE_FIFO_ENABLED BIT(5)
#define IIR_FCR_SET (BIT(7) | BIT(6))

// FCR (Fifo Control Register)
#define FCR_ENABLE_BOTH_FIFO BIT(0)
#define FCR_CLEAR_RCVR_BYTES BIT(1)
#define FCR_CLEAR_XMIT_BYTES BIT(2)
#define FCR_ENABLE_64BYTE BIT(5)
#define FCR_TRIGGER_LEVEL_1 0
#define FCR_TRIGGER_LEVEL_4 BIT(6)
#define FCR_TRIGGER_LEVEL_8 BIT(7)
#define FCR_TRIGGER_LEVEL_14 (BIT(6) | BIT(7))

#endif