#ifndef _OP_GPIO_THU_REG_H_
#define _OP_GPIO_THU_REG_H_

int gpio_mmap_for_read(void);
int gpio_munmap_for_read(void);
int gpio_pin_read(int pin);

#endif
