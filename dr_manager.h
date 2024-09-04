#ifndef _DR_MANAGER_H
#define _DR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "hv_registers.h"

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
    WWAN_BEAR_WIFI = 0x1,
    WWAN_BEAR_CELLULAR = 0x2,

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
    /*空闲0、预警1（PROBEEP）、延迟2（STABY）预热3（WARMUP）、曝光4、 延时关闭5、cooloing 6*/
    EXPOSURE_ST_IDLE = 0,
    EXPOSURE_ST_ALARM,
    EXPOSURE_ST_STAND_BY,
    EXPOSURE_ST_WARM_UP,
    EXPOSURE_ST_X_RAY_NOW,
    EXPOSURE_ST_CLOSING,
    EXPOSURE_ST_COOLING,
}exposure_state_t;

typedef enum
{
    HV_DSP_INIT = -1,
    HV_DSP_DISCONNECTED = 0,
    HV_DSP_CONNECTED,
}hv_dsp_conntion_state_t;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*Define device status related macros, for state record and display.*/
/* Note:**************************
 * when modify ST_PARAMS_COLLECTION, remeber to modify write_info_to_lcd_funcs_t, gs_lcd_areas definitin in lcd_refresh.c
 ********************************
 * */
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
    ST_PARAM_DEF(uint32_t, expo_am_ua) /*unit: uA*/\
    ST_PARAM_DEF(uint16_t, expo_dura_ms) /*uint: ms*/\
    ST_PARAM_DEF(exposure_state_t, expo_st)\
    ST_PARAM_DEF(uint16_t, tof_distance)\
    ST_PARAM_DEF(bool, range_light_on)\
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
#define ST_PARAM_GET(var, ele) ((var).ele)
/*------------------------------------------------------------*/

extern dr_device_st_pool_t g_device_st_pool;

typedef struct
{
    hv_mb_reg_e_t reg;
    uint16_t val;
}mb_reg_val_pair_s_t;
int get_reg_key_val_pair_to_send_external(mb_reg_val_pair_s_t * pairs[]);

#define REGS_TO_SEND_EXTERNAL \
    {State/* = 4*/, 0},                          /*状态*/\
    {VoltSet/* = 5*/, 0},                        /*5管电压设置值*/\
    {FilamentSet/* = 6*/, 0},                    /*6 管设置值电流 （决定灯丝电流决定管电流）*/\
    {ExposureTime/* = 7*/, 0},                   /*曝光时间*/\
    {RangeIndicationStatus/* = 10*/, 0},         /*范围指示状态*/\
    {ExposureStatus/* = 11*/, 0},                /*曝光状态*/\
    {RangeIndicationStart/* = 12*/, 0},          /*范围指示启动*/\
    {BatteryLevel/* = 14*/, 0},                  /*电池电量*/\
    {BatteryVoltmeter/* = 15*/, 0},\
\
    {EXT_MB_REG_DISTANCE/* = 105*/, 0},                       /* uint16，测距结果。单位mm*/\
    {EXT_MB_REG_HOTSPOT_ST/* = 106*/, 0},           /*uint16，本机Wi-Fi热点状态。*/\
    {EXT_MB_REG_CELLUAR_ST/* = 107*/, 0},              /*uint16，高字节表示蜂窝网的信号格数，有效值0~5；*/\
                                                         /*低字节表示蜂窝网状态：0-无服务；1-3G；2-4G；3-5G*/\
    {EXT_MB_REG_WIFI_WAN_SIG_AND_BAT_LVL/* = 108*/, 0}, /*uint16，高字节指示电池电量格数，有效值0~4；*/\
                                                           /*低字节指示WAN侧Wi-Fi信号格数，有效值0~4*/\
    {EXT_MB_REG_DEV_INFO_BITS/* = 109*/, 0}, /*uint16的每个bit指示一个设备的二值状态信息：*/


/* refer to comments to MB_REG_ENUM in hv_registers.h for the macro definition below.
 * NOTE: This MACRO SHOULD be used in function like update_dev_st_pool_from_main_loop_th that is used in 
 * thread safe case. 
 * */
#define SET_DEV_INFO_BITS(info, set, mask) \
{\
    if((set) == (info))\
    {                     \
        info_word |= (mask);\
    }                     \
    else                  \
    {                     \
        info_word &= ~(mask);\
    }\
}

#define EVAL_EXT_MB_REG_FRO_ST(var) \
case EXT_MB_REG_HOTSPOT_ST:\
    (var) = ST_PARAM_GET(g_device_st_pool, hot_spot_st); \
    break;\
\
case EXT_MB_REG_CELLUAR_ST:\
    (var) = (((uint16_t)(ST_PARAM_GET(g_device_st_pool,cellular_signal_bars))) << 8) \
            | ((uint16_t)(ST_PARAM_GET(g_device_st_pool,cellular_mode)) & 0xFF); \
    break;\
\
case EXT_MB_REG_WIFI_WAN_SIG_AND_BAT_LVL:\
    (var) = (((uint16_t)(ST_PARAM_GET(g_device_st_pool, bat_lvl))) << 8) \
            | ((uint16_t)(ST_PARAM_GET(g_device_st_pool, wifi_wan_st)) & 0xFF); \
    break;\
\
case EXT_MB_REG_DEV_INFO_BITS:\
    SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, bat_chg_st),\
                          CHARGER_CONNECTED, MB_REG_DEV_INFO_BITS_CHG_CONN);\
    SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, bat_chg_full), \
                          true, MB_REG_DEV_INFO_BITS_BAT_FULL);\
    SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, wan_bear) & WWAN_BEAR_WIFI, \
                          WWAN_BEAR_WIFI, MB_REG_DEV_INFO_BITS_WIFI_WAN_CONN);\
    SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, wan_bear) & WWAN_BEAR_CELLULAR, \
                          WWAN_BEAR_CELLULAR, MB_REG_DEV_INFO_BITS_CELL_WAN_CONN);\
    SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool,sim_card_st), \
                          SIM_CARD_NORM, MB_REG_DEV_INFO_BITS_SIM_READY);\
    (var) = info_word;\
    break;
/*------------------------------------------------------------*/

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
    int16_t send_dev_info_period_int_s;
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
uint16_t request_tof_distance(tof_requester_e_t requester, float seconds, bool wait_measure);
/*According to GB 9706.254-2020 203.9.102, the minimum allowed focus-to-skin distance should not exceed 20cm.*/
#define MIN_ALLOWED_FSD_IN_CM (20)

/*Call this function after mb server started.*/
uint16_t get_dsp_sw_ver();
bool mb_server_is_ready();

const char* get_wifi_mac_tail6();
#define MAX_FW_V_LINE_LEN 256
const char* get_whole_fw_version_string(int * s_len);
#endif
