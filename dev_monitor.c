#include <pthread.h>

#include "logger.h"
#include "dr_manager.h"

dr_device_st_pool_t g_device_st_pool;
static pthread_mutex_t gs_dev_st_pool_mutex, gs_lcd_upd_mutex;


static battery_chg_st_t gs_bat_chg_st;
static uint16_t gs_bat_lvl;
static wwan_bear_type_t gs_wan_bear;
static cellular_srv_st_t gs_cellular_st;
static wifi_wan_st_t gs_wifi_wan_st;
static sim_card_st_t gs_sim_card_st;

/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function update_device_st_pool.
 */

static void update_dev_st_pool_from_monitor_th()
{
    g_device_st_pool.bat_chg_st = gs_bat_chg_st;
    g_device_st_pool.bat_lvl = gs_bat_lvl;
    g_device_st_pool.wan_bear = gs_wan_bear;
    g_device_st_pool.cellular_st = gs_cellular_st;
    g_device_st_pool.wifi_wan_st = gs_wifi_wan_st;
    g_device_st_pool.sim_card_st = gs_sim_card_st;
}

void* dev_monitor_thread(void* arg)
{
    /* TO BE IMPLEMENTED.
     * periodically read these states and update device status pool.
     *
     */
    gs_bat_chg_st = 0;
    gs_bat_lvl = 0;
    gs_wan_bear = 0;
    gs_cellular_st = 0;
    gs_wifi_wan_st = 0;
    gs_sim_card_st = 0;
    update_device_st_pool(pthread_self(), update_dev_st_pool_from_monitor_th);
    return NULL;
}

void init_dev_st_pool_mutex()
{
    pthread_mutex_init(&gs_dev_st_pool_mutex, NULL);
}

void destroy_dev_st_pool_mutex()
{
    pthread_mutex_destroy(&gs_dev_st_pool_mutex);
}

void init_lcd_upd_mutex()
{
    pthread_mutex_init(&gs_lcd_upd_mutex, NULL);
}

void destroy_lcd_upd_mutex()
{
    pthread_mutex_destroy(&gs_lcd_upd_mutex);
}

void update_device_st_pool(pthread_t pth_id, update_device_status_pool_func_t func)
{
    DIY_LOG(LOG_INFO, "thread %lu try to update device status pool.\n", pth_id);
    if(func)
    {
        pthread_mutex_lock(&gs_dev_st_pool_mutex);
        func();
        pthread_mutex_unlock(&gs_dev_st_pool_mutex);
    }
    DIY_LOG(LOG_INFO, "thread %lu finished updating device status pool.\n", pth_id);
}

void update_lcd_display(pthread_t pth_id, update_lcd_func_t func)
{
    DIY_LOG(LOG_INFO, "thread %lu try to update lcd.\n", pth_id);
    if(func)
    {
        pthread_mutex_lock(&gs_lcd_upd_mutex);
        func();
        pthread_mutex_unlock(&gs_lcd_upd_mutex);
    }
    DIY_LOG(LOG_INFO, "thread %lu finished updating lcd.\n", pth_id);
}

