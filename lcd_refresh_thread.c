#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "common_tools.h"
#include "logger.h"
#include "dr_manager.h"
#include "lcd_display.h"


/* WHEN you want to add/modify LCD display elements, check the following things:
 * 1) ST_PARAMS_COLLECTION macro definition in dr_manager.h.
 * 2) COLLECTION_END_FLAG macro definition for dr_device_st_enum_t type definition in this file.
 * 3) gs_lcd_areas array in this file.
 * 4) gs_write_info_to_lcd_func_list struct in this file.
 *
 */

static pthread_mutex_t gs_lcd_upd_mutex;
static pthread_cond_t gs_lcd_refresh_cond = PTHREAD_COND_INITIALIZER;
static dr_device_st_pool_t gs_device_st_pool_of_lcd;
static bool gs_lcd_opened = false;
const char* g_lcd_refresh_th_desc = "LCD-Refresh";

static void lcd_refresh_thread_cleanup_h(void* arg)
{
    DIY_LOG(LOG_INFO, "%s thread exit cleanup!\n", g_lcd_refresh_th_desc);
    if(gs_lcd_opened)
    {
        close_lcd_dev();
        gs_lcd_opened = false;
    }
}

#include "lcd_resource.h"

/*++++++++++++++++++++++++++++++*/
/*These macros and type definition must be after #include "dr_manager.h"*/
#undef ST_PARAM_DEF
#define ST_PARAM_DEF(var_t, var_n) enum_##var_n,
#undef COLLECTION_END_FLAG
#define COLLECTION_END_FLAG \
    enum_st_end_flag,/*this is an end flag for device status parameters.*/\
    \
    enum_static_device_st_str,\
    enum_static_volt_str,/*cube voltage*/\
    enum_static_am_s_str,/*mAs*/\
    enum_static_amt_str,/*cube mA*/\
    enum_static_dura_str, /*exposure time*/\
    enum_static_distance_str,\
    enum_static_logo,

typedef enum ST_PARAMS_COLLECTION dr_device_st_enum_t;
/*------------------------------*/

typedef struct
{
    int pos_x, pos_y, pos_w, pos_h;
    const lcd_display_resource_t* res;
}lcd_area_info_t;

/* The order of elements in this array must be consitent with the element order of the 
 * ST_PARAMS_COLLECTION definition in dr_manager.h
 * */
static const lcd_area_info_t gs_lcd_areas[] =
{
    {LCD_HOTSPOT_POS_X, LCD_HOTSPOT_POS_Y, LCD_HOTSPOT_POS_W, LCD_HOTSPOT_POS_H, NULL},
    {LCD_CHARGER_POS_X, LCD_CHARGER_POS_Y, LCD_CHARGER_POS_W, LCD_CHARGER_POS_H, NULL},
    {LCD_BAT_POS_X, LCD_BAT_POS_Y, LCD_BAT_POS_W, LCD_BAT_POS_H, NULL},
    {LCD_BAT_POS_X, LCD_BAT_POS_Y, LCD_BAT_POS_W, LCD_BAT_POS_H, NULL},
    {LCD_WAN_CONN_REL_POS_X, LCD_WAN_CONN_REL_POS_X, LCD_WAN_CONN_POS_W, LCD_WAN_CONN_POS_W, NULL},
    {LCD_CELL_SRV_ST_POS_X, LCD_CELL_SRV_ST_POS_Y, LCD_CELL_SRV_ST_POS_W, LCD_CELL_SRV_ST_POS_H, NULL},
    {LCD_CELL_MODE_POS_X, LCD_CELL_MODE_POS_Y, LCD_CELL_MODE_POS_W, LCD_CELL_MODE_POS_H, NULL},
    {LCD_WIFI_WAN_ST_POS_X, LCD_WIFI_WAN_ST_POS_Y, LCD_WIFI_WAN_ST_POS_W, LCD_WIFI_WAN_ST_POS_H, NULL},
    {LCD_SIM_CARD_ST_POS_X, LCD_SIM_CARD_ST_POS_Y, LCD_SIM_CARD_ST_POS_W, LCD_SIM_CARD_ST_POS_H, NULL},
    {LCD_EXPO_ST_POS_X, LCD_EXPO_ST_POS_Y, LCD_EXPO_ST_POS_W, LCD_EXPO_ST_POS_H, NULL},
    {LCD_CUBE_VOLT_POS_X, LCD_CUBE_VOLT_POS_Y, LCD_CUBE_VOLT_POS_W, LCD_CUBE_VOLT_POS_H, NULL},
    {LCD_CUBE_AMTS_POS_X, LCD_CUBE_AMTS_POS_Y, LCD_CUBE_AMTS_POS_W, LCD_CUBE_AMTS_POS_H, NULL},
    {LCD_CUBE_AMTS_POS_X, LCD_CUBE_AMTS_POS_Y, LCD_CUBE_AMTS_POS_W, LCD_CUBE_AMTS_POS_H, NULL},
    {LCD_EXPO_ST_POS_X, LCD_EXPO_ST_POS_Y, LCD_EXPO_ST_POS_W, LCD_EXPO_ST_POS_H, NULL},
    {LCD_DISTANCE_POS_X, LCD_DISTANCE_POS_Y, LCD_DISTANCE_POS_W, LCD_DISTANCE_POS_H, NULL},

    {0, 0, 0, 0, NULL}, //enum_st_end_flag
                              //
    /*The following are static image info.*/
    {LCD_STATIC_DEV_ST_POS_X, LCD_STATIC_DEV_ST_POS_Y, LCD_STATIC_DEV_ST_POS_W, LCD_STATIC_DEV_ST_POS_H,
         &gs_static_dev_st_res},
    {LCD_STATIC_VOLT_POS_X, LCD_STATIC_VOLT_POS_Y, LCD_STATIC_VOLT_POS_W, LCD_STATIC_VOLT_POS_H,
         &gs_static_volt_res},
    {LCD_STATIC_AMT_S_POS_X, LCD_STATIC_AMT_S_POS_Y, LCD_STATIC_AMT_S_POS_W, LCD_STATIC_AMT_S_POS_H,
         &gs_static_amt_s_res},
    {LCD_STATIC_AMT_POS_X, LCD_STATIC_AMT_POS_Y, LCD_STATIC_AMT_POS_W, LCD_STATIC_AMT_POS_H,
         &gs_static_amt_res},
    {LCD_STATIC_DURA_POS_X, LCD_STATIC_DURA_POS_Y, LCD_STATIC_DURA_POS_W, LCD_STATIC_DURA_POS_H,
         &gs_static_dura_res},
    {LCD_STATIC_DIST_POS_X, LCD_STATIC_DIST_POS_Y, LCD_STATIC_DIST_POS_W, LCD_STATIC_DIST_POS_H,
         &gs_static_dist_res},
    {LCD_STATIC_LOGO_POS_X, LCD_STATIC_LOGO_POS_Y, LCD_STATIC_LOGO_POS_W, LCD_STATIC_LOGO_POS_H,
         &gs_static_logo_res},
};

static int print_one_line_to_scrn(const char* str, int size_limit, int pos_x, int pos_y)
{
    int idx = 0;
    char ch = str[idx];
    const unsigned char* img = NULL;
    int img_w, img_h;
    int sum_w = 0;

    while(idx < size_limit && ch)
    {
        if('0' <= ch && ch <= '9')
        {
            img_w = LCD_DIGIT_FONT_W;
            img_h = LCD_DIGIT_FONT_H;
            img = gs_digits_font[ch - '0'];
        }
        else if('a' <= ch && ch <= 'z')
        {
            img_w = LCD_ALPHA_LOW_FONT_W;
            img_h = LCD_ALPHA_LOW_FONT_H;
            img = gs_alpha_low_chars_font[ch - 'a'];
        }
        else if('A' <= ch && ch <= 'Z')
        {
            img_w = LCD_ALPHA_HIGH_FONT_W;
            img_h = LCD_ALPHA_HIGH_FONT_H;
            img = gs_alpha_high_chars_font[ch - 'A'];
        }
        else if('.' == ch)
        {
            img_w = PUNC_DOT_FONT_W;
            img_h = PUNC_DOT_FONT_H;
            img = gs_punc_dot_font;
        }
        else if(':' == ch)
        {
            img_w = PUNC_COLON_FONT_W;
            img_h = PUNC_COLON_FONT_H;
            img = gs_punc_colon_font;
        }
        else
        {
            img_w = LCD_DISPLAY_DEFAULT_CHAR_W;
            img_h = LCD_DISPLAY_DEFAULT_CHAR_H;
            img = gs_lcd_display_def_char;
        }

        sum_w += img_w;
        write_img_to_px_pos(img, img_w, img_h, pos_x + sum_w, pos_y);

        ++idx;
        ch = str[idx];
    }
    return sum_w;
}

static void refresh_hotspot_display(dr_device_st_enum_t st_id)
{
    if(HOTSPOT_DOWN == gs_device_st_pool_of_lcd.hot_spot_st)
    {
        clear_screen_area(gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, 
                gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
    else
    {
        int client_num = (int)(gs_device_st_pool_of_lcd.hot_spot_st - HOTSPOT_NORMAL_0);

        write_img_to_px_rect(gs_lcd_hotspot_res, LCD_HOTSPOT_IMG_W, LCD_HOTSPOT_IMG_H,
              gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);

        if(client_num < 0) client_num = 0;
        if(client_num >= LCD_SMALL_DIGIT_NUM) client_num = LCD_SMALL_DIGIT_NUM - 1;

        write_img_to_px_rect(gs_lcd_small_digit_res[client_num], LCD_SMALL_DIGIT_IMG_W, LCD_SMALL_DIGIT_IMG_H,
            LCD_HOT_SPOT_NUM_POS_X, LCD_HOT_SPOT_NUM_POS_Y, LCD_HOT_SPOT_NUM_POS_W, LCD_HOT_SPOT_NUM_POS_H); 
    }

}

static int bat_lvl_display_map(uint16_t bat_lvl)
{
    int idx = 0;
    while(idx < BATTERY_LEVELS && bat_lvl > gs_lcd_bat_lvls[idx])
    {
        ++idx;
    }
    if(idx > 0) --idx;
    return idx;
}

static void refresh_battery_display(dr_device_st_enum_t st_id)
{
    /* Since bat_chag_st, bat_lvl and bat_chg_full st update all cause this function to be called,
     * we check the update st in function again to minmun the lcd refresh operations.
     * */
    if(gs_device_st_pool_of_lcd.bat_chg_st_upd)
    {
        if(CHARGER_CONNECTED == gs_device_st_pool_of_lcd.bat_chg_st)
        {
            write_img_to_px_rect(gs_lcd_charger_res, LCD_CHARGER_IMG_W, LCD_CHARGER_IMG_H,
                  gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
        }
        else
        {
            clear_screen_area(gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y,
                            gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
        }
    }

    if(gs_device_st_pool_of_lcd.bat_lvl_upd || gs_device_st_pool_of_lcd.bat_chg_st_upd)
    {
        int map_lvl = bat_lvl_display_map(gs_device_st_pool_of_lcd.bat_lvl);
        lcd_battery_img_type img; 
        if((CHARGER_CONNECTED == gs_device_st_pool_of_lcd.bat_chg_st) && !gs_device_st_pool_of_lcd.bat_chg_full)
        {
            img = gs_lcd_bat_lightning_res;
        }
        else
        {
            img = gs_lcd_bat_res;
        }
        write_img_to_px_rect(img[map_lvl], LCD_BAT_IMG_W, LCD_BAT_IMG_H,
            gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);

        gs_device_st_pool_of_lcd.bat_lvl_upd = gs_device_st_pool_of_lcd.bat_chg_st_upd = false;
    }
}

static void refresh_wan_bear_type_display(dr_device_st_enum_t st_id)
{
    if(gs_device_st_pool_of_lcd.wan_bear & WWAN_BEAR_CELLULAR)
    {
        write_img_to_px_rect(gs_lcd_wan_conn_res, LCD_WAN_CONN_IMG_W, LCD_WAN_CONN_IMG_H,
            LCD_CELL_SRV_ST_POS_X + gs_lcd_areas[st_id].pos_x,
            LCD_CELL_SRV_ST_POS_Y + gs_lcd_areas[st_id].pos_y,
            gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
    else
    {
        clear_screen_area(LCD_CELL_SRV_ST_POS_X + gs_lcd_areas[st_id].pos_x,
                          LCD_CELL_SRV_ST_POS_Y + gs_lcd_areas[st_id].pos_y,
                          gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }

    if(gs_device_st_pool_of_lcd.wan_bear & WWAN_BEAR_WIFI)
    {
        write_img_to_px_rect(gs_lcd_wan_conn_res, LCD_WAN_CONN_IMG_W, LCD_WAN_CONN_IMG_H,
            LCD_WIFI_WAN_ST_POS_X + gs_lcd_areas[st_id].pos_x,
            LCD_WIFI_WAN_ST_POS_Y + gs_lcd_areas[st_id].pos_y,
            gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
    else
    {
        clear_screen_area(LCD_WIFI_WAN_ST_POS_X + gs_lcd_areas[st_id].pos_x,
                          LCD_WIFI_WAN_ST_POS_Y +  gs_lcd_areas[st_id].pos_y,
                          gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
}

static void refresh_cellular_srv_st_display(dr_device_st_enum_t st_id)
{
    /* Since cellular_signal_bars and cellular_mode update both cause this function to be called,
     * we check the update st in function again to minmun the lcd refresh operations.
     * */

    if(gs_device_st_pool_of_lcd.cellular_signal_bars_upd)
    {
        int map_lvl = (int)gs_device_st_pool_of_lcd.cellular_signal_bars;

        if(map_lvl < 0) map_lvl = 0;
        if(map_lvl >= MAX_CELL_WAN_LEVELS) map_lvl = MAX_CELL_WAN_LEVELS - 1; 

        write_img_to_px_rect(gs_cell_srv_st_res[map_lvl], LCD_CELL_SRV_ST_IMG_W, LCD_CELL_SRV_ST_IMG_H, 
                gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }

    if(gs_device_st_pool_of_lcd.cellular_mode_upd)
    {
        int mode_idx = (int)gs_device_st_pool_of_lcd.cellular_mode;

        if(mode_idx < 0 || mode_idx >= LCD_CELL_MODE_NUM) mode_idx = 0;
        write_img_to_px_rect(gs_lcd_mode_res[mode_idx], LCD_CELL_MODE_IMG_W, LCD_CELL_MODE_IMG_H,
                gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
        gs_device_st_pool_of_lcd.cellular_mode_upd = false;
    }
}

static void refresh_wifi_wan_display(dr_device_st_enum_t st_id)
{
    int map_lvl = (int)gs_device_st_pool_of_lcd.wifi_wan_st;

    if(map_lvl < 0) map_lvl = 0;
    if(map_lvl >= MAX_WIFI_WAN_LEVELS) map_lvl = MAX_WIFI_WAN_LEVELS - 1; 

    write_img_to_px_rect(gs_wifi_wan_st_res[map_lvl], LCD_WIFI_WAN_ST_IMG_W, LCD_WIFI_WAN_ST_IMG_H, 
                gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
}

static void refresh_sim_card_st_display(dr_device_st_enum_t st_id)
{
    if(SIM_CARD_NORM == gs_device_st_pool_of_lcd.sim_card_st)
    {
        clear_screen_area( gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y,
                gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
    else
    {
        write_img_to_px_rect(gs_sim_card_err_res, LCD_SIM_CARD_ST_IMG_W, LCD_SIM_CARD_ST_IMG_H,
                gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
}

static void refresh_cube_volt_display(dr_device_st_enum_t st_id)
{
    PRINT_NUMBER_WITH_UNIT_TO_SCRN(gs_device_st_pool_of_lcd.expo_volt_kv, LCD_CUBE_VOLT_MAX_INT_CHAR_NUM, "%d",\
                                   gs_LCD_DISPLAY_UNIT_STR_KV, \
                                   gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y);
}

static void refresh_cube_amts_display(dr_device_st_enum_t st_id)
{
    float amts_n = ((float)(gs_device_st_pool_of_lcd.expo_am_ua) / 1000) 
                 * ((float)(gs_device_st_pool_of_lcd.expo_dura_ms) / 1000);
    int max_num_of_number_chars = LCD_CUBE_AMTS_MAX_INT_CHAR_NUM + 1 /*.*/ + LCD_CUBE_AMTS_MAX_FRAC_CHAR_NUM;
    PRINT_NUMBER_WITH_UNIT_TO_SCRN(amts_n, max_num_of_number_chars, "%f",\
                                   gs_LCD_DISPLAY_UNIT_STR_AMTS, \
                                   gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y);
}

static void refresh_expo_st_display(dr_device_st_enum_t st_id)
{
    const lcd_display_resource_t * res = NULL;

    clear_screen_area(gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y,
                            gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);

    if(HV_DSP_DISCONNECTED  == gs_device_st_pool_of_lcd.hv_dsp_conn_st)
    {
        res = &gs_expo_st_hv_disconn_res;
    }
    else
    {
        switch(gs_device_st_pool_of_lcd.expo_st)
        {
            case EXPOSURE_ST_IDLE: 
                res = &gs_expo_st_idle_res;
                break;

            default:
                res = &gs_expo_st_exposing_res;
                break;
        }
    }
    write_img_to_px_rect(res->img, res->img_w, res->img_h, 
            gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y, gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
}

static void refresh_tof_distance_display(dr_device_st_enum_t st_id)
{
    if((int)(gs_device_st_pool_of_lcd.tof_distance) < 0)
    {
        clear_screen_area(gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y,
                       gs_lcd_areas[st_id].pos_w, gs_lcd_areas[st_id].pos_h);
    }
    else
    {
        float dist_in_cm = (float)(gs_device_st_pool_of_lcd.tof_distance) / 10;
        int max_num_of_number_chars = LCD_DISTANCE_MAX_INT_CHAR_NUM + 1 /*.*/ + LCD_DISTANCE_MAX_FRAC_CHAR_NUM;

        PRINT_NUMBER_WITH_UNIT_TO_SCRN(dist_in_cm, max_num_of_number_chars, "%f", \
                                       gs_LCD_DISPLAY_UNIT_STR_CM, \
                                       gs_lcd_areas[st_id].pos_x, gs_lcd_areas[st_id].pos_y);
    }
}

typedef void (*write_info_to_lcd_func_t)(dr_device_st_enum_t st_id);
#undef ST_PARAM_DEF
#define ST_PARAM_DEF(var_t, var_n) write_info_to_lcd_func_t var_n##_to_lcd_func;
#undef COLLECTION_END_FLAG
#define COLLECTION_END_FLAG
typedef struct ST_PARAMS_COLLECTION write_info_to_lcd_funcs_t;
static const write_info_to_lcd_funcs_t gs_write_info_to_lcd_func_list =
{
    /*The function order must be consitent with the element order of the ST_PARAMS_COLLECTION definition in dr_manager.h*/
    refresh_hotspot_display,
    refresh_battery_display,
    refresh_battery_display,
    refresh_battery_display,
    refresh_wan_bear_type_display,
    refresh_cellular_srv_st_display,
    refresh_cellular_srv_st_display,
    refresh_wifi_wan_display,
    refresh_sim_card_st_display,
    refresh_expo_st_display,
    refresh_cube_volt_display,
    refresh_cube_amts_display,
    refresh_cube_amts_display,
    refresh_expo_st_display,
    refresh_tof_distance_display,
};

/*----------------------------------------*/

/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 */
static bool access_g_st_pool_from_lcd_refresh_th(void* buf)
{
    /*copy global device status pool into local buf, and clear the "updated" flag in global pool.*/
    if(buf)
    {
        memcpy(buf, &g_device_st_pool, sizeof(g_device_st_pool));
    }
#undef COLLECTION_END_FLAG
#define COLLECTION_END_FLAG
#undef ST_PARAM_DEF
#define ST_PARAM_DEF(var, ele) g_device_st_pool.ele##_upd = false;
#define ST_PARAM_CLEAR_UPD_ALL ST_PARAMS_COLLECTION;
    ST_PARAM_CLEAR_UPD_ALL;

    return false;
}

#undef COLLECTION_END_FLAG
#define COLLECTION_END_FLAG
#undef ST_PARAM_DEF
#define ST_PARAM_DEF(var_t, var_n) \
{\
    if(gs_device_st_pool_of_lcd.var_n##_upd && gs_write_info_to_lcd_func_list.var_n##_to_lcd_func)\
    {\
        gs_write_info_to_lcd_func_list.var_n##_to_lcd_func(enum_##var_n);\
        gs_device_st_pool_of_lcd.var_n##_upd  = false;\
    }\
}
#define REFRESH_LCD_DISPYAL ST_PARAMS_COLLECTION
static void init_lcd_display()
{
    int i;
    for(i = 0; i < ARRAY_ITEM_CNT(gs_lcd_areas); ++i)
    {
        if(gs_lcd_areas[i].res)
        {
            write_img_to_px_rect(gs_lcd_areas[i].res->img, gs_lcd_areas[i].res->img_w, gs_lcd_areas[i].res->img_h,
                                 gs_lcd_areas[i].pos_x, gs_lcd_areas[i].pos_y,
                                 gs_lcd_areas[i].pos_w, gs_lcd_areas[i].pos_h);
        }
    }
}

void* lcd_refresh_thread_func(void* arg)
{
    lcd_refresh_th_parm_t * parm = (lcd_refresh_th_parm_t*)arg;

    DIY_LOG(LOG_INFO, "%s thread start!\n", g_lcd_refresh_th_desc);

    pthread_cleanup_push(lcd_refresh_thread_cleanup_h, NULL);

    if(!parm)
    {
        DIY_LOG(LOG_ERROR, "Argument passed to lcd_refresh_thread_func is NULL."
               "%s thread exit.\n", g_lcd_refresh_th_desc);
        return NULL;
    }

    gs_lcd_opened = open_lcd_dev(parm->dev_name, parm->dev_addr);
    if(!gs_lcd_opened)
    {
        DIY_LOG(LOG_ERROR, "Open LCD device fails, %s thread exit.\n", g_lcd_refresh_th_desc);
        return NULL;
    }
    clear_screen();
    init_lcd_display();

    while(true)
    {
        pthread_mutex_lock(&gs_lcd_upd_mutex);

        pthread_cond_wait(&gs_lcd_refresh_cond, &gs_lcd_upd_mutex);

        access_device_st_pool(pthread_self(), g_lcd_refresh_th_desc, access_g_st_pool_from_lcd_refresh_th,
                &gs_device_st_pool_of_lcd);

        pthread_mutex_unlock(&gs_lcd_upd_mutex);

        REFRESH_LCD_DISPYAL; 
    }

    pthread_cleanup_pop(1);

    return NULL;
}

int init_lcd_upd_mutex()
{
    int ret;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

    ret = pthread_mutex_init(&gs_lcd_upd_mutex, &attr);
    return ret;
}

int destroy_lcd_upd_mutex()
{
    int ret;
    ret = pthread_mutex_destroy(&gs_lcd_upd_mutex);
    if(ret)
    {
        DIY_LOG(LOG_ERROR, "pthread_mutex_destroy error: %d.\n", ret);
    }
    ret = pthread_cond_destroy(&gs_lcd_refresh_cond);
    return ret;
}

void update_lcd_display(pthread_t pth_id, const char* desc)
{
    int ret;

    DIY_LOG(LOG_INFO, "thread %u ", (uint32_t)pth_id);
    if(desc)
    {
        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "%s ", desc);
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "try to update lcd.\n");

    ret = pthread_mutex_lock(&gs_lcd_upd_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_lcd_upd_mutex);
    }

    pthread_cond_signal(&gs_lcd_refresh_cond);

    pthread_mutex_unlock(&gs_lcd_upd_mutex);

    DIY_LOG(LOG_INFO, "thread %u ", (uint32_t)pth_id);
    if(desc)
    {
        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "%s ", desc);
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "finished updating lcd.\n");
}

