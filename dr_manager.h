#ifndef _DR_MANAGER_H
#define _DR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

typedef enum
{
    HOTSPOT_DOWN = -1,
    HOTSPOT_NORMAL_0 = 0, //index for img res.1,2,... means clientes associated.
}hotspot_st_t;

typedef enum
{
    CHARGER_INIT = -1,
    NO_CHARGER_CONNECTED = 0,
    CHARGER_CONNECTED,
}battery_chg_st_t;

typedef enum
{
    /*one bit means one type.*/
    WWAN_BEAR_NONE = 0,
    WWAN_BEAR_CELLULAR = 0x1,
    WWAN_BEAR_WIFI = 0x2,

    WWAN_BEAR_MAX = 0xFF,
}wwan_bear_type_t;

typedef enum
{
    CELLULAR_MODE_INIT = -1,
    CELLULAR_MODE_NOSRV = 0, //index for img res
    CELLULAR_MODE_3G,
    CELLULAR_MODE_4G,
    CELLULAR_MODE_5G,
}cellular_mode_t;

typedef enum
{
    SIM_INIT = -1,
    SIM_NO_CARD = 0,
    SIM_CARD_LOCKED,
    SIM_CARD_NORM,
}sim_card_st_t;

typedef enum
{
    WIFI_WAN_INIT = -1,
    WIFI_WAN_DISCONNECTED = 0, //index for img res. signal 0
    WIFI_WAN_CONNECTED, //signal 1
}wifi_wan_st_t;

typedef enum
{
    EXPOSURE_ST_INIT = -1,
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
    HV_DSP_INIT = -1,
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
    ST_PARAM_DEF(int, cellular_signal_bars)\
    ST_PARAM_DEF(cellular_mode_t, cellular_mode)\
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
    if((var).ele != (value))\
    {\
        (var).ele = (value); (var).ele##_upd = true;\
        updated = true;\
    }

#define ST_PARAM_CLEAR_UPD(var, ele) (var).ele##_upd = false
#define ST_PARAM_CHECK_UPD(var, ele) ((var).ele##_upd)
/*------------------------------------------------------------*/

extern dr_device_st_pool_t g_device_st_pool;
extern const char* g_main_thread_desc;

int init_dev_st_pool_mutex();
int destroy_dev_st_pool_mutex();
/*Return true if any item is updated in st pool, otherwise return false.*/
typedef bool (*access_device_status_pool_func_t)(void*);
bool access_device_st_pool(pthread_t pth_id, const char* desc, access_device_status_pool_func_t func, void* arg);

int init_lcd_upd_sync_mech();
typedef void (*update_lcd_func_t)(void*);
void update_lcd_display(pthread_t pth_id, const char* desc);

typedef struct
{
    float sch_period;
    bool sh_script_debug;
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

int init_tof_th_measure_syncs();
int destroy_tof_th_measure_mutex();
typedef struct
{
    float measure_period;
    const char* dev_name;
    uint8_t dev_addr;
    int tof_mech_cali; //calibratoin in mm, due to mechanical design.
    int tof_internal_cali; //calibration in mm, due to TOF-module itself.
}tof_thread_parm_t;
void* tof_thread_func(void* arg);
extern const char* g_tof_th_desc;
typedef enum
{
    /*each requester use 1 bit.*/
    TOF_REQUESTER_NONE = 0,
    TOF_REQUESTER_RANGE_LED = 0x1,
    TOF_REQUESTER_EXPOSURE = 0x2,
}tof_requester_e_t;
void set_tof_th_measure_flag(tof_requester_e_t requester);
void unset_tof_th_measure_flag(tof_requester_e_t requester);
bool inform_tof_th_to_measure();

/*Return distance in mm.*/
uint16_t request_tof_distance(tof_requester_e_t requester, float seconds);
/*According to GB 9706.254-2020 203.9.102, the minimum allowed focus-to-skin distance should not exceed 20cm.*/
#define MIN_ALLOWED_FSD_IN_CM (20)

/*Call this function after mb server started.*/
uint16_t get_dsp_sw_ver();
bool mb_server_is_ready();

const char* get_wifi_mac_tail6();
#endif
