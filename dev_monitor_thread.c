#include <unistd.h>
#include <pthread.h>

#include "logger.h"
#include "dr_manager.h"

const char* g_dev_monitor_th_desc = "Device-State-Monitor";

float gs_dev_sch_period = DEV_MONITOR_DEF_PERIOD;
dr_device_st_pool_t g_device_st_pool;
static pthread_mutex_t gs_dev_st_pool_mutex, gs_lcd_upd_mutex;

/*This global static var should only be accessed from monitor thread.*/
dr_main_dev_st_t gs_main_dev_st;

/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function update_device_st_pool.
 */

static void update_dev_st_pool_from_monitor_th(void* d)
{
    if(!d) return;

    dr_main_dev_st_t * main_dev_st = (dr_main_dev_st_t*)d;

    g_device_st_pool.main_dev_st.bat_chg_st = main_dev_st->bat_chg_st;
    g_device_st_pool.main_dev_st.bat_lvl = main_dev_st->bat_lvl;
    g_device_st_pool.main_dev_st.wan_bear = main_dev_st->wan_bear;
    g_device_st_pool.main_dev_st.cellular_st = main_dev_st->cellular_st;
    g_device_st_pool.main_dev_st.wifi_wan_st = main_dev_st->wifi_wan_st;
    g_device_st_pool.main_dev_st.sim_card_st = main_dev_st->sim_card_st;
}
/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function update_lcd_display.
 */
static void updata_lcd_from_monitor_th(void* arg)
{
    /* TO BE COMPLETED.*/
    /* use gs_main_dev_st (the arg) to update lcd display.*/
    DIY_LOG(LOG_INFO, "bat_chg_st: %d\n",
            gs_main_dev_st.bat_chg_st);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "bat_lvl: %d\n",
            gs_main_dev_st.bat_lvl);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "wan_bear: %d\n",
            gs_main_dev_st.wan_bear);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "cellular_st: %d\n",
            gs_main_dev_st.cellular_st);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "wifi_wan_st: %d\n",
            gs_main_dev_st.wifi_wan_st);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "sim_card_st: %d\n",
            gs_main_dev_st.sim_card_st);
}

void* dev_monitor_thread_func(void* arg)
{
    /* TO BE COMPLETED.
     * periodically read these states and update device status pool.
     *
     */
    dev_monitor_th_parm_t * parm = (dev_monitor_th_parm_t*) arg;

    if(parm != NULL)
    {
        gs_dev_sch_period = parm->sch_period;
    }

    DIY_LOG(LOG_INFO, "%s therad starts, with sch period %f seconds!\n", 
            g_dev_monitor_th_desc, gs_dev_sch_period);
    while(true)
    {
        gs_main_dev_st.bat_chg_st = 0;
        gs_main_dev_st.bat_lvl = (gs_main_dev_st.bat_lvl + 1) % 101;
        gs_main_dev_st.wan_bear += 1;
        gs_main_dev_st.cellular_st += 1;
        gs_main_dev_st.wifi_wan_st += 1;
        gs_main_dev_st.sim_card_st += 1;

        update_device_st_pool(pthread_self(), update_dev_st_pool_from_monitor_th,
                                              &gs_main_dev_st);
        update_lcd_display(pthread_self(), updata_lcd_from_monitor_th, &gs_main_dev_st);

        usleep(gs_dev_sch_period * 1000000);
    }
    return NULL;
}

int init_dev_st_pool_mutex()
{
    return pthread_mutex_init(&gs_dev_st_pool_mutex, NULL);
}

int destroy_dev_st_pool_mutex()
{
    return pthread_mutex_destroy(&gs_dev_st_pool_mutex);
}

int init_lcd_upd_mutex()
{
    return pthread_mutex_init(&gs_lcd_upd_mutex, NULL);
}

int destroy_lcd_upd_mutex()
{
    return pthread_mutex_destroy(&gs_lcd_upd_mutex);
}

/*In current program, g_device_st_pool is not really used. threads use their own 
 * global static var to hold state info.
 * maybe in future the global g_device_st_pool can be used.
 * */
void update_device_st_pool(pthread_t pth_id, update_device_status_pool_func_t func, void* arg)
{
    DIY_LOG(LOG_INFO, "thread %lu try to update device status pool.\n", pth_id);
    if(func)
    {
        pthread_mutex_lock(&gs_dev_st_pool_mutex);
        func(arg);
        pthread_mutex_unlock(&gs_dev_st_pool_mutex);
    }
    DIY_LOG(LOG_INFO, "thread %lu finished updating device status pool.\n", pth_id);
}

void update_lcd_display(pthread_t pth_id, update_lcd_func_t func, void* arg)
{
    DIY_LOG(LOG_INFO, "thread %lu try to update lcd.\n", pth_id);
    if(func)
    {
        pthread_mutex_lock(&gs_lcd_upd_mutex);
        func(arg);
        pthread_mutex_unlock(&gs_lcd_upd_mutex);
    }
    DIY_LOG(LOG_INFO, "thread %lu finished updating lcd.\n", pth_id);
}

