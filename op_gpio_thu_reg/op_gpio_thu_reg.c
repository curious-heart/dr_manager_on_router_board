/* forgotfun.org forgotfun(佐须之男) */
/*
 * reference:
 * https://www.cnblogs.com/hztd/p/17110956.html
 * https://github.com/ForgotFun/mt76x8
 * */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "mt7621_gpio_registers.h"
#include "logger.h"

#define MMAP_PATH	"/dev/mem"
#define MMAP_LENGTH 2048 //1024 

static uint8_t* gs_gpio_mmap_reg = MAP_FAILED;
static int gs_gpio_mmap_fd = 0;

int gpio_mmap_for_read(void)
{
	if ((gs_gpio_mmap_fd = open(MMAP_PATH, O_RDONLY)) < 0) 
    {
        DIY_LOG(LOG_ERROR, "open %s error:%d\n", MMAP_PATH, errno);
		return -1;
	}

	gs_gpio_mmap_reg 
        = (uint8_t*)mmap(NULL, MMAP_LENGTH, PROT_READ, MAP_FILE | MAP_SHARED, gs_gpio_mmap_fd, MT7621_REG_SYSCTL_BASE);
	if(gs_gpio_mmap_reg == MAP_FAILED) 
    {
        DIY_LOG(LOG_ERROR, "mmap error: %d\n", errno);
		close(gs_gpio_mmap_fd);
		return -1;
	}
    /*According to man mmap, fd can be closed after mmap return.*/
    close(gs_gpio_mmap_fd);

	return 0;
}

int gpio_munmap_for_read(void)
{
    if(gs_gpio_mmap_reg != MAP_FAILED)
    {
        int ret = munmap(gs_gpio_mmap_reg, MMAP_LENGTH);
        gs_gpio_mmap_reg = MAP_FAILED;
        gs_gpio_mmap_fd = 0;
        return ret;
    }
    else
    {
        DIY_LOG(LOG_WARN, "mmap reg is MAP_FAILED, no need to unmap.\n");
        return 0;
    }
}

#define CHECK_AND_SET_MMAP_REG \
    if(MAP_FAILED == gs_gpio_mmap_reg)\
    {\
        if(gpio_mmap_for_read() < 0)\
        {\
            gs_gpio_mmap_reg = MAP_FAILED;\
            return -1;\
        }\
    }

int gpio_pin_read(int pin)
{
	uint32_t tmp = -1;

    CHECK_AND_SET_MMAP_REG;

	/* MT7621, MT7628 */
    if(pin < 0 || pin > 95)
    {
        DIY_LOG(LOG_ERROR, "Invalid gpio pin number: %d.\n", pin);
        return tmp;
    }

	if (pin <= 31) {
		tmp = *(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIODATA);
		tmp = (tmp >> pin) & 1u;
	} else if (pin <= 63) {
		tmp = *(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO6332DATA);
		tmp = (tmp >> (pin-32)) & 1u;
	} else if (pin <= 95) {
		tmp = *(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO9564DATA);
		tmp = (tmp >> (pin-64)) & 1u;
		tmp = (tmp >> (pin-24)) & 1u;
	}
	return tmp;

}

#if 0
void mt7621_gpio_set_pin_direction(int pin, int is_output)
{
	uint32_t tmp;

	/* MT7621, MT7628 */
	if (pin <= 31) {
		tmp = *(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIODIR);
		if (is_output)
			tmp |=  (1u << pin);
		else
			tmp &= ~(1u << pin);
		*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIODIR) = tmp;
	} else if (pin <= 63) {
		tmp = *(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO6332DIR);
		if (is_output)
			tmp |=  (1u << (pin-32));
		else
			tmp &= ~(1u << (pin-32));
		*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO6332DIR) = tmp;
	} else if (pin <= 95) {
		tmp = *(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO9564DIR);
		if (is_output)
			tmp |=  (1u << (pin-64));
		else
			tmp &= ~(1u << (pin-64));
		*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO9564DIR) = tmp;
	}
}

void mt7621_gpio_set_pin_value(int pin, int value)
{
	uint32_t tmp;

	/* MT7621, MT7628 */
	if (pin <= 31) {
		tmp = (1u << pin);
		if (value)
			*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIOSET) = tmp;
		else
			*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIORESET) = tmp;
	} else if (pin <= 63) {
		tmp = (1u << (pin-32));
		if (value)
			*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO6332SET) = tmp;
		else
			*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO6332RESET) = tmp;
	} else if (pin <= 95) {
		tmp = (1u << (pin-64));
		if (value)
			*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO9564SET) = tmp;
		else
			*(volatile uint32_t *)(gs_gpio_mmap_reg + RALINK_REG_PIO9564RESET) = tmp;
	}
}

int main(int argc, char **argv)
{
	int ret = -1;

	if (gpio_mmap())
		return -1;

	printf("get pin 39 input %d\n", mt7621_gpio_get_pin(39));
	printf("get pin 40 input %d\n", mt7621_gpio_get_pin(40));
	printf("get pin 41 input %d\n", mt7621_gpio_get_pin(41));
	printf("get pin 42 input %d\n", mt7621_gpio_get_pin(42));

	
	printf("set pin 39 output 1\n");
	mt7621_gpio_set_pin_direction(39, 1);
	mt7621_gpio_set_pin_value(39, 1);
	printf("set pin 40 output 0\n");
	mt7621_gpio_set_pin_direction(40, 1);
	mt7621_gpio_set_pin_value(40, 0);
	printf("set pin 41 output 1\n");
	mt7621_gpio_set_pin_direction(41, 1);
	mt7621_gpio_set_pin_value(41, 1);
	printf("set pin 42 output 0\n");
	mt7621_gpio_set_pin_direction(42, 1);
	mt7621_gpio_set_pin_value(42, 0);

	while (1)
	{
		mt7621_gpio_set_pin_value(42, 0);
		mt7621_gpio_set_pin_value(42, 1);
	}
	close(gs_gpio_mmap_fd);

	return ret;
}
#endif
