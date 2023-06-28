#include "dr_manager.h"
#include "logger.h"

static pthread_mutex_t gs_lcd_upd_mutex;
static pthread_cond_t gs_lcd_refresh_cond = PTHREAD_COND_INITIALIZER;
static dr_device_st_pool_t gs_device_st_pool_of_lcd;

const char* g_lcd_refresh_th_desc = "LCD-Refresh";

void* lcd_refresh_func(void* arg)
{
    DIY_LOG(LOG_INFO, "%s thread start!\n", g_lcd_refresh_th_desc);
    while(true)
    {
        pthread_mutex_lock(&gs_lcd_upd_mutex);

        pthread_cond_wait(&gs_lcd_refresh_cond, &gs_lcd_upd_mutex);

        copy_device_st_pool(pthread_self(), &gs_device_st_pool_of_lcd);

        pthread_mutex_unlock(&gs_lcd_upd_mutex);


        /*TO BE COMPLETED: fill LCD screen. The data is in gs_device_st_pool_of_lcd.*/
    }
}


int init_lcd_upd_mutex()
{
    return pthread_mutex_init(&gs_lcd_upd_mutex, NULL);
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

void update_lcd_display(pthread_t pth_id)
{
    DIY_LOG(LOG_INFO, "thread %u try to update lcd, send signal to %s thread.\n", (uint32_t)pth_id, g_lcd_refresh_th_desc);
    pthread_mutex_lock(&gs_lcd_upd_mutex);

    pthread_cond_signal(&gs_lcd_refresh_cond);

    pthread_mutex_unlock(&gs_lcd_upd_mutex);
    DIY_LOG(LOG_INFO, "thread %u finished sending signal to %s thread.\n", (uint32_t)pth_id, g_lcd_refresh_th_desc);
}

