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
------------------------------------------------------------
按键/信号       |PIN名称|PIN编号|Name     |GPIO   |有效性
----------------|-------|-------|---------|-------|------
范围指示灯按键  |SW4    |38     |ND_CLE   |GPIO43 |低有效
曝光开关按键    |SW3    |40     |ND_ALE   |GPIO44 |低有效
档位+按键       |SW1    |4      |JTMS     |GPIO15 |低有效
档位-按键       |SW2    |6      |JTDI     |GPIO14 |低有效
充电器插入gpio  |Charge |39     |ND_WP    |GPIO41 |低有效
满电指示gpio    |SC_PG  |28     |GPIO00   |GPIO0  |高有效
------------------------------------------------------------
说明：
1) “范围指示灯”按键仅仅是一个“开启/关闭范围指示灯”的输入信号，dr_manager收到该信号后，
仅向下位机发送RangeIndicatorStart命令；实际的指示灯控制是在下位机上完成的。
2) SW1~4 and Charge is configured in dts and theire press/release event is processed in application
gpio_key_monitor. Only SC_PG is read by dr_manager, in period.
 */

#define GPIO_CHARGER_FULL_IND 0 //42//34


#define GPIO_CTL_PTH "/sys/class/gpio"
#define GPIO_MAX_PTN_STR_LEN 256

#define GPIO_MAX_VALUE_DIGITS 10

int get_effective_gpio_number(int n);
void export_gpios_to_app();
int app_read_gpio_value(int n);
int app_write_gpio_value(int n, int value);

#endif
