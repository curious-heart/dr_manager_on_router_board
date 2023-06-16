#ifndef _DR_MANAGER_H
#define _DR_MANAGER_H

#include <stdint.h>
#include <pthread.h>

typedef enum
{
    NO_CHARGER_CONNECTED = 0,
    CHARGING_NOT_FULL,
    CHARGING_FULL,
}battery_chg_st_t;

typedef enum
{
    /*one bit means one type.*/
    WWAN_BEAR_NONE = 0,
    WWAN_BEAR_CELLULAR_4G_5G = 0x1,
    WWAN_BEAR_WIFI = 0x2,

    WWAN_BEAR_MAX = 0xFF,
}wwan_bear_type_t;

typedef enum
{
    CELLULAR_NO_SERVICE = 0,
    CELLULAR_LIMITED_SERVICE,
    CELLULAR_FULL_SERVICE,
}cellular_srv_st_t;

typedef enum
{
    SIM_NO_CARD = 0,
    SIM_CARD_LOCKED,
    SIM_CARD_NORM,
}sim_card_st_t;

typedef enum
{
    WIFI_WAN_DISCONNECTED = 0,
    WIFI_WAN_CONNECTED,
}wifi_wan_st_t;

typedef enum
{
    /*空闲0、预警1（PROBEEP）、延迟2（STABY）预热3（WARMUP）、曝光4、 延时关闭5*/
    EXPOSURE_ST_IDLE = 0,
    EXPOSURE_ST_ALARM,
    EXPOSURE_ST_STAND_BY,
    EXPOSURE_ST_WARM_UP,
    EXPOSURE_ST_X_RAY_NOW,
    EXPOSURE_ST_CLOSING,
}exposure_state_t;

typedef struct
{
    battery_chg_st_t bat_chg_st;
    uint16_t bat_lvl;
    wwan_bear_type_t wan_bear;
    cellular_srv_st_t cellular_st;
    wifi_wan_st_t wifi_wan_st;
    sim_card_st_t sim_card_st;

    uint16_t expo_volt /*uint: kV*/, expo_dura /*uint: ms*/;
    uint32_t expo_am; /*unit: uA*/
    exposure_state_t expo_st;
}dr_device_st_pool_t;

void init_dev_st_pool_mutex();
void destroy_dev_st_pool_mutex();
void init_lcd_upd_mutex();
void destroy_lcd_upd_mutex();

typedef void (*update_device_status_pool_func_t)();
typedef void (*update_lcd_func_t)();
void update_device_st_pool(pthread_t pth_id, update_device_status_pool_func_t func);

extern dr_device_st_pool_t g_device_st_pool;
#endif
