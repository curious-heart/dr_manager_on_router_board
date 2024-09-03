#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "logger.h"
#include "dr_manager.h"
#include "tof_measure.h"

const char* g_tof_th_desc = "TOF-Measurement";

static bool gs_tof_opened = false;
static tof_requester_e_t gs_tof_th_measure_now = TOF_REQUESTER_NONE;
static pthread_mutex_t gs_tof_th_measure_mutex;
static sem_t gs_tof_th_wait_sem, gs_tof_th_render_sem;

static bool gs_tof_th_wait_sem_inited = false, gs_tof_th_render_sem_inited = false;
static unsigned short gs_distance = (uint16_t)-1;
static tof_thread_parm_t * gs_tof_thread_parm = NULL;

static tof_requester_e_t check_tof_th_measure_flag()
{
    int ret;
    tof_requester_e_t flag;
    ret = pthread_mutex_lock(&gs_tof_th_measure_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_measure_mutex);
    }
    flag = gs_tof_th_measure_now;
    pthread_mutex_unlock(&gs_tof_th_measure_mutex);
    return flag;
}

void set_tof_th_measure_flag(tof_requester_e_t requester)
{
    int ret;
    ret = pthread_mutex_lock(&gs_tof_th_measure_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_measure_mutex);
    }
    gs_tof_th_measure_now |= requester;
    pthread_mutex_unlock(&gs_tof_th_measure_mutex);
}

void unset_tof_th_measure_flag(tof_requester_e_t requester)
{
    int ret;
    ret = pthread_mutex_lock(&gs_tof_th_measure_mutex);
    if(EOWNERDEAD == ret)
    {
        pthread_mutex_consistent(&gs_tof_th_measure_mutex);
    }
    gs_tof_th_measure_now &= (~requester);
    pthread_mutex_unlock(&gs_tof_th_measure_mutex);
}

bool inform_tof_th_to_measure()
{
    if(gs_tof_th_wait_sem_inited)
    {
        sem_post(&gs_tof_th_wait_sem);
        return true;
    }
    return false;
}

static void inform_tof_measurement()
{
    if(gs_tof_th_render_sem_inited)
    {
        sem_post(&gs_tof_th_render_sem);
    }
}

/*This functoin should be called from other thread, currently main thread.*/
/*Return distance in mm.*/
uint16_t request_tof_distance(tof_requester_e_t requester, float seconds, bool wait_measure)
{
    struct timespec ts;
    int sem_value = -10;

    if(gs_tof_th_render_sem_inited)
    {
        sem_getvalue(&gs_tof_th_render_sem, &sem_value);
        while(sem_value > 0)
        {
            /*clear gs_tof_th_render_sem to assure the action order: tof_thread measusre, then here get the result.*/
            DIY_LOG(LOG_INFO, "gs_tof_th_render_sem value:%d\n", sem_value);
            sem_wait(&gs_tof_th_render_sem);
            sem_getvalue(&gs_tof_th_render_sem, &sem_value);
        }

        set_tof_th_measure_flag(requester);
        inform_tof_th_to_measure();

        if(wait_measure)
        {
            if(0 == fill_timespec_struc(&ts, seconds))
            {
                sem_getvalue(&gs_tof_th_render_sem, &sem_value);
                DIY_LOG(LOG_INFO, "gs_tof_th_render_sem value:%d\n", sem_value);

                sem_timedwait(&gs_tof_th_render_sem, &ts);
            }
            else
            {
                usleep(seconds * 1000000);
            }
        }
        return gs_distance;
    }
    else
    {
        return (uint16_t)-1;
    }
}

static void tof_th_cleanup_h(void* arg)
{
    DIY_LOG(LOG_INFO, "%s thread exit cleanup!\n", g_tof_th_desc);
    if(gs_tof_opened)
    {
        tof_close();
        gs_tof_opened = false;
    }

    if(gs_tof_th_wait_sem_inited)
    {
        sem_destroy(&gs_tof_th_wait_sem);
        gs_tof_th_wait_sem_inited = false;
    }
    if(gs_tof_th_render_sem_inited)
    {
        sem_destroy(&gs_tof_th_render_sem);
        gs_tof_th_render_sem_inited = false;
    }
}
/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 */
static bool upd_g_st_pool_from_tof_th(void* arg)
{
    bool updated = false;
    mb_reg_val_pair_s_t *pair_arr = NULL;
    int pair_cnt = 0, pair_idx;

    if(arg)
    {
        ST_PARAM_SET_UPD(g_device_st_pool, tof_distance, (*(uint16_t*)arg));

        pair_cnt = get_reg_key_val_pair_to_send_external(&pair_arr);
        if(pair_arr && pair_cnt > 0)
        {
            for(pair_idx = 0; pair_idx < pair_cnt; ++pair_idx)
            {
                switch(pair_arr[pair_idx].reg)
                {
                    case EXT_MB_REG_DISTANCE:
                        pair_arr[pair_idx].val = ST_PARAM_GET(g_device_st_pool, tof_distance);
                        break;

                    default:
                        break;
                }
            }
        }
    }
    return updated;
}

int init_tof_th_measure_syncs()
{
    int ret;
    pthread_mutexattr_t attr;

    ret = sem_init(&gs_tof_th_wait_sem, 0, 1);
    if(ret != 0)
    {
        DIY_LOG(LOG_ERROR, "sem_init for gs_tof_th_wait_sem error:%d\n", errno);
        return ret;
    }
    gs_tof_th_wait_sem_inited = true;

    ret = sem_init(&gs_tof_th_render_sem, 0, 0);
    if(ret != 0)
    {
        DIY_LOG(LOG_ERROR, "sem_init for gs_tof_th_render_sem error:%d\n", errno);
        return ret;
    }
    gs_tof_th_render_sem_inited = true;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);

    ret = pthread_mutex_init(&gs_tof_th_measure_mutex, &attr);


    return ret;
}

int destroy_tof_th_measure_syncs()
{
    return pthread_mutex_destroy(&gs_tof_th_measure_mutex);
}

void* tof_thread_func(void* arg)
{
    tof_thread_parm_t * gs_tof_thread_parm = (tof_thread_parm_t*)arg;
    float period;
    int ret;
    struct timespec w_ts;
    static uint16_t last_raw_distance = (uint16_t)-1, raw_distance = (uint16_t)-1;
    static bool ls_thread_startup = true; //take a measurement at the thread startup.
    tof_requester_e_t requester = TOF_REQUESTER_NONE;

    pthread_cleanup_push(tof_th_cleanup_h, NULL);

    if(!gs_tof_thread_parm)
    {
        DIY_LOG(LOG_ERROR, "Arguments passed to tof_thread_func is NULL.\n"
                "Thread %s exit.\n", g_tof_th_desc);
        return NULL;
    }

    period = gs_tof_thread_parm->measure_period;
    DIY_LOG(LOG_INFO, "%s thread starts, with measure period %f seconds!\n", 
            g_tof_th_desc, period);

    ret = tof_open(gs_tof_thread_parm->dev_name, gs_tof_thread_parm->dev_addr);
    if(0 != ret)
    {
        DIY_LOG(LOG_ERROR, "%s thread exit due to open tof error: %d.\n", g_tof_th_desc, ret);
        gs_distance = 21000;
        access_device_st_pool(pthread_self(), g_tof_th_desc, upd_g_st_pool_from_tof_th, &gs_distance);
        update_lcd_display(pthread_self(), g_tof_th_desc);
        return NULL;
    }
    gs_tof_opened = true;

    ret = tof_single_measure_prepare();
    if(0 != ret)
    {
        DIY_LOG(LOG_ERROR, "%s thread exit due to single measure prepare error: %d.\n", g_tof_th_desc, ret);
        tof_close();
        gs_tof_opened = false;
        gs_distance = 22000;
        access_device_st_pool(pthread_self(), g_tof_th_desc, upd_g_st_pool_from_tof_th, &gs_distance);
        update_lcd_display(pthread_self(), g_tof_th_desc);
        return NULL;
    }

    gs_distance = 0;
    access_device_st_pool(pthread_self(), g_tof_th_desc, upd_g_st_pool_from_tof_th, &gs_distance);
    update_lcd_display(pthread_self(), g_tof_th_desc);

    while(true)
    {
        if(!ls_thread_startup)
        {
            ret = fill_timespec_struc(&w_ts, period);
            if(0 != ret)
            {
                DIY_LOG(LOG_ERROR, "fill timespec error:%d\n", ret);
                
                usleep(period * 1000000);
            }
            else
            {
                sem_timedwait(&gs_tof_th_wait_sem, &w_ts);
            }

            requester = check_tof_th_measure_flag();
        }

        if((ls_thread_startup) || (TOF_REQUESTER_NONE != requester))
        {
            raw_distance = tof_single_measure(); 
            gs_distance = (uint16_t)((int)raw_distance + 
                    gs_tof_thread_parm->tof_mech_cali + gs_tof_thread_parm->tof_internal_cali);
            if(requester & TOF_REQUESTER_EXPOSURE)
            {
                inform_tof_measurement();
            }
            DIY_LOG(LOG_INFO, "%s raw distance: %u, distance:%u\n", g_tof_th_desc, raw_distance, gs_distance);

            if((last_raw_distance != raw_distance)
                    && access_device_st_pool(pthread_self(), g_tof_th_desc, upd_g_st_pool_from_tof_th, &gs_distance))
            {
                update_lcd_display(pthread_self(), g_tof_th_desc);
            }
            last_raw_distance = raw_distance;

            ls_thread_startup = false;
        }
    }

    pthread_cleanup_pop(1);

    return NULL;
}
