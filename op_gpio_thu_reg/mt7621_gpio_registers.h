#ifndef _MT7621_REGISTERS_H_
#define _MT7621_REGISTERS_H_

#define MT7621_REG_SYSCTL_BASE 0x1E000000

#define RALINK_GPIO_DIR_IN		0
#define RALINK_GPIO_DIR_OUT		1

#define RALINK_REG_PIOINT		0x690
#define RALINK_REG_PIOEDGE		0x6A0
#define RALINK_REG_PIORENA		0x650
#define RALINK_REG_PIOFENA		0x660
#define RALINK_REG_PIODATA		0x620
#define RALINK_REG_PIODIR		0x600
#define RALINK_REG_PIOSET		0x630
#define RALINK_REG_PIORESET		0x640

#define RALINK_REG_PIO6332INT		0x694
#define RALINK_REG_PIO6332EDGE		0x6A4
#define RALINK_REG_PIO6332RENA		0x654
#define RALINK_REG_PIO6332FENA		0x664
#define RALINK_REG_PIO6332DATA		0x624
#define RALINK_REG_PIO6332DIR		0x604
#define RALINK_REG_PIO6332SET		0x634
#define RALINK_REG_PIO6332RESET		0x644

#define RALINK_REG_PIO9564INT		0x698
#define RALINK_REG_PIO9564EDGE		0x6A8
#define RALINK_REG_PIO9564RENA		0x658
#define RALINK_REG_PIO9564FENA		0x668
#define RALINK_REG_PIO9564DATA		0x628
#define RALINK_REG_PIO9564DIR		0x608
#define RALINK_REG_PIO9564SET		0x638
#define RALINK_REG_PIO9564RESET		0x648
#endif
