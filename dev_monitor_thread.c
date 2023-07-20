#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "logger.h"
#include "dr_manager.h"

const char* g_dev_monitor_th_desc = "Device-State-Monitor";
dr_device_st_pool_t g_device_st_pool;

static pthread_mutex_t gs_dev_st_pool_mutex;

/*This global static var should only be accessed from monitor thread.*/
static dr_device_st_local_buf_t gs_main_dev_st;

/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 */

static void update_dev_st_pool_from_monitor_th(void* d)
{
    if(!d) return;

    dr_device_st_local_buf_t *main_dev_st = (dr_device_st_local_buf_t*)d;

    ST_PARAM_SET_UPD(g_device_st_pool, wan_bear, main_dev_st->wan_bear);
    ST_PARAM_SET_UPD(g_device_st_pool, cellular_st, main_dev_st->cellular_st);
    ST_PARAM_SET_UPD(g_device_st_pool, wifi_wan_st, main_dev_st->wifi_wan_st);
    ST_PARAM_SET_UPD(g_device_st_pool, sim_card_st, main_dev_st->sim_card_st);
}

void* dev_monitor_thread_func(void* arg)
{
    /* TO BE COMPLETED.
     * periodically read these states and update device status pool.
     *
     */
    dev_monitor_th_parm_t * parm = (dev_monitor_th_parm_t*) arg;

    if(NULL == parm)
    {
        DIY_LOG(LOG_ERROR, "Arguments passed to %s thread is NULL. Thread exit.\n", g_dev_monitor_th_desc);
        return NULL;
    }

    DIY_LOG(LOG_INFO, "%s therad starts, with sch period %f seconds!\n", 
            g_dev_monitor_th_desc, parm->sch_period);
    while(true)
    {
        gs_main_dev_st.wan_bear += 1;
        gs_main_dev_st.cellular_st += 1;
        gs_main_dev_st.wifi_wan_st += 1;
        gs_main_dev_st.sim_card_st += 1;

        access_device_st_pool(pthread_self(), g_dev_monitor_th_desc, update_dev_st_pool_from_monitor_th,
                                              &gs_main_dev_st);
        update_lcd_display(pthread_self(), g_dev_monitor_th_desc);

        usleep(parm->sch_period * 1000000);
    }
    return NULL;
}

int init_dev_st_pool_mutex()
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    return pthread_mutex_init(&gs_dev_st_pool_mutex, &attr);
}

int destroy_dev_st_pool_mutex()
{
    return pthread_mutex_destroy(&gs_dev_st_pool_mutex);
}

void access_device_st_pool(pthread_t pth_id, const char* desc, access_device_status_pool_func_t func, void* arg)
{
    /* every thread that may update device status should have its own private function to write 
     * the global pool, and use this function to call that private funciton.
     * */
    DIY_LOG(LOG_INFO, "thread %u ", (uint32_t)pth_id);
    if(desc)
    {
        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "%s ", desc);
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "try to update device status pool.\n");

    if(func)
    {
        int ret;
        ret = pthread_mutex_lock(&gs_dev_st_pool_mutex);
        if(EOWNERDEAD == ret)
        {
            pthread_mutex_consistent(&gs_dev_st_pool_mutex);
        }
        func(arg);
        pthread_mutex_unlock(&gs_dev_st_pool_mutex);
    }

    DIY_LOG(LOG_INFO, "thread %u ", (uint32_t)pth_id);
    if(desc)
    {
        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "%s ", desc);
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "finished updating device status pool.\n");
}

