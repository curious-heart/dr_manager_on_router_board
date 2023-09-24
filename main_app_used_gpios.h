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
曝光开关按键    |SW3    |38     |ND_CLE   |GPIO43 |低有效
范围指示灯按键  |SW4    |40     |ND_ALE   |GPIO44 |低有效
档位+按键       |SW1    |4      |JTMS     |GPIO15 |低有效
档位-按键       |SW2    |6      |JTDI     |GPIO14 |低有效
充电器插入gpio  |Charge |39     |ND_WP    |GPIO41 |低有效
满电指示gpio    |SC_PG  |3      |JTDO     |GPIO13 |高有效
------------------------------------------------------------
说明：
1) “范围指示灯”按键仅仅是一个“开启/关闭范围指示灯”的输入信号，dr_manager收到该信号后，
仅向下位机发送RangeIndicatorStart命令；实际的指示灯控制是在下位机上完成的。
2) SW1~4 and Charge is configured in dts and theire press/release event is processed in application
gpio_key_monitor. Only SC_PG is read by dr_manager, in period.
 */
/*
 * 2023-09-24
 * GPIO13 (J1 PIN 3) 为之前我们讨论准备使用的管脚，与原理图“TR7621ASP_DB-lzb改2023-8-21.pdf”保持一致。
 * 但这个GPIO配置后，会导致曝光开关的GPIO43始终处于低电平而无法使用。
 *
 * GPIO 34 (J1 PIN 46) 与东哥在20230921通过微信发来的原理图TR7621ASP_DB-V1.1.pdf保持一致（原理图中的管脚名称为“C/S”）。
 * 但这个GPIO配置后，插着充电器无法开机，PIN 46典雅为0.15V。
 * （如果不插充电器，可以开机，开机后PIN 46电压为3.27V；此时再连接充电器后，电压变为1.89V）。
 *
 * 创凌提供的原始软件版本中，GPIO 13层被用作LED的GPIO，所以可能它是可以用的，但和现在的硬件设计有冲突。
 * 所以软件这里还是保持记录为13,但实际去掉对这个GPIO的配置和读取功能（也即软件不再判断满电与否——软件也没有参与充电管理）。
 * */
#define GPIO_CHARGER_FULL_IND 13 //0 //42//34


#define GPIO_CTL_PTH "/sys/class/gpio"
#define GPIO_MAX_PTN_STR_LEN 256

#define GPIO_MAX_VALUE_DIGITS 10

int get_effective_gpio_number(int n);
void export_gpios_to_app();
int app_read_gpio_value(int n);
int app_write_gpio_value(int n, int value);

#endif
