#ifndef _MAIN_APP_USED_GPIOS_H_
#define _MAIN_APP_USED_GPIOS_H_

#define GPIO_BANK_0_BASE 480
#define GPIO_BANK_1_BASE 448
#define GPIO_BANK_2_BASE 416

#define GPIO_CNT_PER_BANK 32

#define GPIO_VALUE_LOW 0
#define GPIO_VALUE_HIGH 1

typedef struct
{
    int number;
    const char* direction;
}gpio_cfg_tbl_s_t;

/*
 * 
 按键/信号  |PIN名称|PIN编号|Name   |GPIO   |有效性
-----------|-------|-------|-------|-------|------
范围指示灯 |SW3    |50     |ND_D5  |GPIO38 |低有效
曝光开关   |SW4    |49     |ND_D6  |GPIO39 |低有效
档位+      |SW1    |4      |JTMS   |GPIO15 |低有效
档位-      |SW2    |6      |JTDI   |GPIO14 |低有效
充电器插入 |Charge |52     |ND_D7  |GPIO40 |高有效
满电指示   |SC_PG  |46     |ND_CS_N|GPIO34 |高有效
 */

#define GPIO_CHARGER_FULL_IND 34


#define GPIO_CTL_PTH "/sys/class/gpio"
#define GPIO_MAX_PTN_STR_LEN 256

#define GPIO_MAX_VALUE_DIGITS 10

int get_effective_gpio_number(int n);
void export_gpios_to_app();
int app_read_gpio_value(int n);
int app_write_gpio_value(int n, int value);

#endif