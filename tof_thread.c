#include <errno.h>
#include <pthread.h>

#include "logger.h"
#include "dr_manager.h"
#include "tof_measure.h"

const char* gs_tof_th_desc = "TOF-Measurement";

static bool gs_tof_opened = false;
static bool gs_tof_th_running = false;
static pthread_mutex_t gs_tof_th_check_mutex;

bool get_tof_th_running_flag()
{
    int ret;
    bool flag;
    ret = pthread_mutex_lock(&gs_tof_th_check_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_check_mutex);
    }
    flag = gs_tof_th_running;
    pthread_mutex_unlock(&gs_tof_th_check_mutex);
    return flag;
}

static void set_tof_th_running_flag(bool flag)
{
    int ret;
    ret = pthread_mutex_lock(&gs_tof_th_check_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_check_mutex);
    }
    gs_tof_th_running = flag;
    pthread_mutex_unlock(&gs_tof_th_check_mutex);
}

static void tof_th_cleanup_h(void* arg)
{
    DIY_LOG(LOG_INFO, "%s thread exit cleanup!\n", gs_tof_th_desc);
    if(gs_tof_opened)
    {
        tof_close();
        gs_tof_opened = false;
    }
    set_tof_th_running_flag(false);
}
/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 */
static bool upd_g_st_pool_from_tof_th(void* arg)
{
    bool updated = false;

    if(arg)
    {
        ST_PARAM_SET_UPD(g_device_st_pool, tof_distance, (*(uint16_t*)arg));
    }
    return updated;
}

int init_tof_th_check_mutex()
{
    int ret;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

    ret = pthread_mutex_init(&gs_tof_th_check_mutex, &attr);
    return ret;
}

void* tof_thread_func(void* arg)
{
    tof_thread_parm_t * parm = (tof_thread_parm_t*)arg;
    float period;
    int ret;
    unsigned short distance;

    set_tof_th_running_flag(true);

    pthread_cleanup_push(tof_th_cleanup_h, NULL);

    if(!parm)
    {
        DIY_LOG(LOG_ERROR, "Arguments passed to tof_thread_func is NULL.\n"
                "Thread %s exit.\n", gs_tof_th_desc);
        set_tof_th_running_flag(false);
        return NULL;
    }

    period = parm->measure_period;
    DIY_LOG(LOG_INFO, "%s thread starts, with measure period %f seconds!\n", 
            gs_tof_th_desc, period);

    ret = tof_open(parm->dev_name, parm->dev_addr);
    if(0 != ret)
    {
        DIY_LOG(LOG_ERROR, "%s thread exit due to open tof error: %d.\n", gs_tof_th_desc, ret);
        set_tof_th_running_flag(false);
        return NULL;
    }
    gs_tof_opened = true;

    while(true)
    {
        distance = tof_single_measure(); 
        if(access_device_st_pool(pthread_self(), gs_tof_th_desc, upd_g_st_pool_from_tof_th, &distance))
        {
            update_lcd_display(pthread_self(), gs_tof_th_desc);
        }

        DIY_LOG(LOG_INFO, "%s distance:%u\n", gs_tof_th_desc, distance);

        usleep(period * 1000000);
    }

    pthread_cleanup_pop(1);

    return NULL;
}
