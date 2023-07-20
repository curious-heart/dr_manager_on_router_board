#ifndef _DR_MANAGER_H
#define _DR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef enum
{
    HOTSPOT_DOWN = -1,
    HOTSPOT_NORMAL_0 = 0,
}hotspot_st_t;

typedef enum
{
    NO_CHARGER_CONNECTED = 0,
    CHARGER_CONNECTED,
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

typedef enum
{
    HV_DSP_DISCONNECTED = 0,
    HV_DSP_CONNECTED,
}hv_dsp_conntion_state_t;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*Define device status related macros, for state record and display.*/
#undef COLLECTION_END_FLAG
#define COLLECTION_END_FLAG
#undef ST_PARAM_DEF
#define ST_PARAM_DEF(var_t, var_n) var_t var_n; bool var_n##_upd;
#define ST_PARAMS_COLLECTION \
{\
    ST_PARAM_DEF(hotspot_st_t, hot_spot_st)\
    ST_PARAM_DEF(battery_chg_st_t, bat_chg_st)\
    ST_PARAM_DEF(uint16_t, bat_lvl)\
    ST_PARAM_DEF(bool, bat_chg_full)\
    ST_PARAM_DEF(wwan_bear_type_t, wan_bear)\
    ST_PARAM_DEF(cellular_srv_st_t, cellular_st)\
    ST_PARAM_DEF(wifi_wan_st_t, wifi_wan_st)\
    ST_PARAM_DEF(sim_card_st_t, sim_card_st)\
    ST_PARAM_DEF(hv_dsp_conntion_state_t, hv_dsp_conn_st)\
    ST_PARAM_DEF(uint16_t, expo_volt_kv) /*uint: kV*/\
    ST_PARAM_DEF(uint16_t, expo_dura_ms) /*uint: ms*/\
    ST_PARAM_DEF(uint32_t, expo_am_ua) /*unit: uA*/\
    ST_PARAM_DEF(exposure_state_t, expo_st)\
    ST_PARAM_DEF(uint16_t, tof_distance)\
\
    COLLECTION_END_FLAG /*this is for enum type end flag.*/\
}
typedef struct ST_PARAMS_COLLECTION dr_device_st_pool_t;

#undef ST_PARAM_DEF
#define ST_PARAM_DEF(var_t, var_n) var_t var_n;
typedef struct ST_PARAMS_COLLECTION dr_device_st_local_buf_t;

/*functions using this macro MUST define a var as the following:
 *      bool updated = false;
 * */
#define ST_PARAM_SET_UPD(var, ele, value) \
    if(var.ele != value)\
    {\
        var.ele = value; var.ele##_upd = true;\
        updated = true;\
    }

#define ST_PARAM_CLEAR_UPD(var, ele) var.ele##_upd = false
/*------------------------------------------------------------*/

extern dr_device_st_pool_t g_device_st_pool;
extern const char* g_main_thread_desc;


int init_dev_st_pool_mutex();
int destroy_dev_st_pool_mutex();
/*Return true if any item is updated in st pool, otherwise return false.*/
typedef bool (*access_device_status_pool_func_t)(void*);
bool access_device_st_pool(pthread_t pth_id, const char* desc, access_device_status_pool_func_t func, void* arg);

int init_lcd_upd_mutex();
int destroy_lcd_upd_mutex();
typedef void (*update_lcd_func_t)(void*);
void update_lcd_display(pthread_t pth_id, const char* desc);

typedef struct
{
    float sch_period;
}dev_monitor_th_parm_t;
void* dev_monitor_thread_func(void* arg);
extern const char* g_dev_monitor_th_desc;

typedef struct
{
    const char* dev_name;
    uint8_t dev_addr;
}lcd_refresh_th_parm_t;
void* lcd_refresh_thread_func(void* arg);
extern const char* g_lcd_refresh_th_desc;

typedef struct
{
    float measure_period;
    const char* dev_name;
    uint8_t dev_addr;
}tof_thread_parm_t;
extern const char* gs_tof_th_desc;
int init_tof_th_check_mutex();
bool get_tof_th_running_flag();
void* tof_thread_func(void* arg);

#endif
