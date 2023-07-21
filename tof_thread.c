#include <errno.h>
#include <pthread.h>

#include "logger.h"
#include "dr_manager.h"
#include "tof_measure.h"

const char* g_tof_th_desc = "TOF-Measurement";

static bool gs_tof_opened = false;
static bool gs_tof_th_measure_now = false;
static pthread_mutex_t gs_tof_th_measure_mutex;

static bool check_tof_th_measure_flag()
{
    int ret;
    bool flag;
    ret = pthread_mutex_lock(&gs_tof_th_measure_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_measure_mutex);
    }
    flag = gs_tof_th_measure_now;
    pthread_mutex_unlock(&gs_tof_th_measure_mutex);
    return flag;
}

void set_tof_th_measure_flag(bool flag)
{
    int ret;
    ret = pthread_mutex_lock(&gs_tof_th_measure_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_measure_mutex);
    }
    gs_tof_th_measure_now = flag;
    pthread_mutex_unlock(&gs_tof_th_measure_mutex);
}

static void tof_th_cleanup_h(void* arg)
{
    DIY_LOG(LOG_INFO, "%s thread exit cleanup!\n", g_tof_th_desc);
    if(gs_tof_opened)
    {
        tof_close();
        gs_tof_opened = false;
    }
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

int init_tof_th_measure_mutex()
{
    int ret;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

    ret = pthread_mutex_init(&gs_tof_th_measure_mutex, &attr);
    return ret;
}

int destroy_tof_th_measure_mutex()
{
    return pthread_mutex_destroy(&gs_tof_th_measure_mutex);
}

void* tof_thread_func(void* arg)
{
    tof_thread_parm_t * parm = (tof_thread_parm_t*)arg;
    float period;
    int ret;
    unsigned short distance;

    pthread_cleanup_push(tof_th_cleanup_h, NULL);

    if(!parm)
    {
        DIY_LOG(LOG_ERROR, "Arguments passed to tof_thread_func is NULL.\n"
                "Thread %s exit.\n", g_tof_th_desc);
        return NULL;
    }

    period = parm->measure_period;
    DIY_LOG(LOG_INFO, "%s thread starts, with measure period %f seconds!\n", 
            g_tof_th_desc, period);

    ret = tof_open(parm->dev_name, parm->dev_addr);
    if(0 != ret)
    {
        DIY_LOG(LOG_ERROR, "%s thread exit due to open tof error: %d.\n", g_tof_th_desc, ret);
        return NULL;
    }
    gs_tof_opened = true;

    while(true)
    {
        if(check_tof_th_measure_flag())
        {
            distance = tof_single_measure(); 
            DIY_LOG(LOG_INFO, "%s distance:%u\n", g_tof_th_desc, distance);
        }
        else
        {
            distance = -1; 
        }

        if(access_device_st_pool(pthread_self(), g_tof_th_desc, upd_g_st_pool_from_tof_th, &distance))
        {
            update_lcd_display(pthread_self(), g_tof_th_desc);
        }

        usleep(period * 1000000);
    }

    pthread_cleanup_pop(1);

    return NULL;
}
