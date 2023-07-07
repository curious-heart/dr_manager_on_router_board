#ifndef _GPIO_KEY_PROCESSOR_H_
#define _GPIO_KEY_PROCESSOR_H_

#include <stdbool.h>
#include <stdint.h>

#undef KEY_AND_ACT_NAME
#define KEY_AND_ACT_NAME(k) key_##k,
#undef GPIO_AND_ACT_NAME
#define GPIO_AND_ACT_NAME(g) gpio_##g,
#undef KEY_GPIO_END_FLAG 
#define KEY_GPIO_END_FLAG(k) kg_##k,
#define KEY_GPIO_LIST \
{\
    KEY_AND_ACT_NAME(exp_range_led)\
    KEY_AND_ACT_NAME(exp_start)\
    KEY_AND_ACT_NAME(dose_add)\
    KEY_AND_ACT_NAME(dose_sub)\
    KEY_AND_ACT_NAME(reset)\
    GPIO_AND_ACT_NAME(charger)\
\
    KEY_GPIO_END_FLAG(end_flag)\
}
#define KEY_GPIO_ACT_LIST \
{\
    KEY_AND_ACT_NAME(released)\
    KEY_AND_ACT_NAME(pressed)\
\
    KEY_GPIO_END_FLAG(action_end_flag)\
}
typedef enum KEY_GPIO_LIST key_gpio_e_t;
typedef enum KEY_GPIO_ACT_LIST key_gpio_act_e_t;

typedef struct
{
    bool valid;
    key_gpio_e_t key_gpio;
    key_gpio_act_e_t action;
    uint32_t seen;
    uint32_t seqnum;
}converted_gbh_uevt_s_t;

typedef struct
{
    uint32_t exp_start_key_hold_time; //in secnods
}key_gpio_cfg_params_s_t;

extern const char* g_key_gpio_name_list[kg_end_flag];
extern const char* g_key_gpio_act_name_list[kg_action_end_flag];

bool begin_key_event_handle();
void end_key_event_handle();

typedef void (*key_gpio_handler_t)(converted_gbh_uevt_s_t * evt);
void exp_range_led_key_handler(converted_gbh_uevt_s_t * evt);
void exp_start_key_handler(converted_gbh_uevt_s_t* evt);
void dose_adjust_key_handler(converted_gbh_uevt_s_t* evt);
void reset_key_handler(converted_gbh_uevt_s_t* evt);
void charger_gpio_handler(converted_gbh_uevt_s_t* evt);

#endif
