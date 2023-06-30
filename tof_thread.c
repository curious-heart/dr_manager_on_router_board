#include <pthread.h>

#include "logger.h"
#include "dr_manager.h"
#include "tof_measure.h"

const char* gs_tof_th_desc = "TOF-Measurement";

static bool gs_tof_opened = false;

static void tof_th_cleanup_h(void* arg)
{
    DIY_LOG(LOG_INFO, "%s thread exit cleanup!\n", gs_tof_th_desc);
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
static void upd_g_st_pool_from_tof_th(void* arg)
{
    if(arg)
    {
        ST_PARAM_SET_UPD(g_device_st_pool, tof_distance, (*(uint16_t*)arg));
    }
}

void* tof_thread_func(void* arg)
{
    tof_thread_parm_t * parm = (tof_thread_parm_t*)arg;
    float period = TOF_MEASUREMENT_DEF_PERIOD; 
    int ret;
    unsigned short distance;

    if(parm) period = parm->measure_period;
    DIY_LOG(LOG_INFO, "%s therad starts, with measure period %f seconds!\n", 
            gs_tof_th_desc, period);

    pthread_cleanup_push(tof_th_cleanup_h, NULL);

    ret = tof_open();
    if(0 != ret)
    {
        DIY_LOG(LOG_ERROR, "%s thread open tof error: %d\n", gs_tof_th_desc, ret);
        return NULL;
    }
    gs_tof_opened = true;

    while(true)
    {
        distance = tof_single_measure(); 
        access_device_st_pool(pthread_self(), gs_tof_th_desc, upd_g_st_pool_from_tof_th, &distance);
        update_lcd_display(pthread_self(), gs_tof_th_desc);

        DIY_LOG(LOG_INFO, "%s distance:%u\n", gs_tof_th_desc, distance);

        usleep(period * 1000000);
    }

    pthread_cleanup_pop(1);

    return NULL;
}
