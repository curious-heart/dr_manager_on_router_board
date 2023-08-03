#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

#include <modbus/modbus.h>

#include "logger.h"
#include "gpio_key_processor.h"
#include "hv_registers.h"
#include "mb_tcp_server.h"

extern mb_tcp_client_params_t g_mb_tcp_client_params;
extern key_gpio_cfg_params_s_t g_key_gpio_cfg_params;

static modbus_t * gs_mb_tcp_client_ctx = NULL;
#define CONNECT_FAIL_WAIT_TIME 3 //s
bool begin_key_event_handle()
{
    gs_mb_tcp_client_ctx = modbus_new_tcp(g_mb_tcp_client_params.srvr_ip, g_mb_tcp_client_params.srvr_port);
    if(!gs_mb_tcp_client_ctx)
    {
        DIY_LOG(LOG_ERROR, "modbus_new_tcp fail, %d:%s.\n", errno, modbus_strerror(errno));
        return false;
    }

    if(0 != modbus_set_debug(gs_mb_tcp_client_ctx, g_mb_tcp_client_params.debug_flag))
    {
        DIY_LOG(LOG_WARN, "modbus_set_debug fail, %d:%s\n", errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN + LOG_ONLY_INFO_STR_COMP, "but weago ahead.\n");
    }

    while(modbus_connect(gs_mb_tcp_client_ctx) == -1)
    {
        DIY_LOG(LOG_ERROR, "Connection failed, %d:%s. Wait some time and retry connect.\n", errno, modbus_strerror(errno));
        sleep(CONNECT_FAIL_WAIT_TIME);
        DIY_LOG(LOG_INFO, "Retry to connect.\n");
    }

    /*
    if(modbus_connect(gs_mb_tcp_client_ctx) == -1)
    {
        DIY_LOG(LOG_ERROR, "Connection failed, %d:%s\n", errno, modbus_strerror(errno));
        DIY_LOG(LOG_INFO, "Retry to connect.\n");

        modbus_free(gs_mb_tcp_client_ctx);
        gs_mb_tcp_client_ctx = NULL;
        return false;
    }
    */
    return true;
}

void end_key_event_handle()
{
    if(gs_mb_tcp_client_ctx)
    {
        DIY_LOG(LOG_INFO, "Close modbus connection and free modbus resource.\n");
        modbus_close(gs_mb_tcp_client_ctx);
        modbus_free(gs_mb_tcp_client_ctx);
        gs_mb_tcp_client_ctx = NULL;
    }
}

#define IGNORE_NON_PRESSED_EVT(evt_ptr) \
    if(evt_ptr)\
    {\
        if(evt_ptr->action != key_pressed)\
        {\
            DIY_LOG(LOG_DEBUG, "Not a key_pressed event, ignored.\n");\
            return;\
        }\
    }

void exp_range_led_key_handler(converted_gbh_uevt_s_t* evt)
{
    uint16_t write_data = true;
    static const char* ls_on_off_str[] = {"OFF", "ON"};

    hv_mb_reg_e_t reg_addr = RangeIndicationStart;
    const char* reg_str;

    DIY_LOG(LOG_INFO, "exp_range_led key handler!\n");

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(gs_mb_tcp_client_ctx)
    {
        write_data = (key_pressed == evt->action);

        if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
        {
            DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n",
                   reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            DIY_LOG(LOG_INFO, "Turn %s exposure range light.\n", ls_on_off_str[write_data ? 1 : 0]); 
            write_data = !write_data;
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
    }
}

static const uint16_t gs_exp_ready = 1, gs_exp_start = 2;
static bool gs_key_hold_processed = false;
static void exp_start_key_hold_handler(int sig)
{
    hv_mb_reg_e_t reg_addr = ExposureStart;
    uint16_t write_data;
    const char* reg_str;

    gs_key_hold_processed = true;

    write_data = gs_exp_start;
    reg_str = get_hv_mb_reg_str(reg_addr);
    if(gs_mb_tcp_client_ctx)
    {
        if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
        {
            DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n",
                   reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            DIY_LOG(LOG_INFO, "Start exposure.\n"); 
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
    }
}

void exp_start_key_handler(converted_gbh_uevt_s_t* evt)
{
    hv_mb_reg_e_t reg_addr = ExposureStart;
    uint16_t write_data;
    const char* reg_str;
    struct itimerval t_v;

    DIY_LOG(LOG_DEBUG, "exp_start key handler!\n");

    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "exposure_start key evt ptr is NULL.\n");
        return;
    }

    signal(SIGALRM, exp_start_key_hold_handler);

    if(key_pressed == evt->action)
    {
        gs_key_hold_processed = false;
        t_v.it_value.tv_sec = g_key_gpio_cfg_params.exp_start_key_hold_time;
        t_v.it_value.tv_usec = 0;
        t_v.it_interval.tv_sec = t_v.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &t_v, NULL);
        DIY_LOG(LOG_DEBUG, "key hold timer parm: %d, timer length: %lld\n",
                g_key_gpio_cfg_params.exp_start_key_hold_time, t_v.it_value.tv_sec);
    }
    else if(key_released == evt->action)
    {
        memset(&t_v, 0, sizeof(t_v)); //disable the timer.
        //setitimer(ITIMER_VIRTUAL, &t_v, NULL);
        setitimer(ITIMER_REAL, &t_v, NULL);
        if(!gs_key_hold_processed)
        {
            write_data = gs_exp_ready;
            reg_str = get_hv_mb_reg_str(reg_addr);
            if(gs_mb_tcp_client_ctx)
            {
                if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
                {
                    DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n",
                           reg_str, errno, modbus_strerror(errno));
                }
                else
                {
                    DIY_LOG(LOG_INFO, "Set exposure ready.\n"); 
                }
            }
            else
            {
                DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
            }
        } 
        else
        {
            gs_key_hold_processed = false;
        }
    }
    else
    {
        DIY_LOG(LOG_WARN, "Unexpected key action: %d", evt->action);
        if(evt->action < kg_action_end_flag)
        {
            DIY_LOG(LOG_WARN + LOG_ONLY_INFO_STR_COMP, ", %s\n", g_key_gpio_act_name_list[evt->action]);
        }
    }

}

void dose_adjust_key_handler(converted_gbh_uevt_s_t* evt)
{
    const char* reg_str;
    hv_mb_reg_e_t reg_addr = EXT_MB_REG_DOSE_ADJ;
    uint16_t write_data;

    IGNORE_NON_PRESSED_EVT(evt);

    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "dose adjust handler, evt ptr is NULL.\n");
        return;
    }

    DIY_LOG(LOG_INFO, "%s key handler!\n", g_key_gpio_name_list[evt->key_gpio]);

    if(key_dose_add == evt->key_gpio)
    {
        write_data = MB_REG_V_DOSE_ADJ_ADD;
    }
    else
    {
        write_data = MB_REG_V_DOSE_ADJ_SUB;
    }

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(gs_mb_tcp_client_ctx)
    {
        if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
        {
            DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n", reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            DIY_LOG(LOG_INFO, "%s: adjust dose ok.\n", g_key_gpio_name_list[evt->key_gpio]); 
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
    }
}

void reset_key_handler(converted_gbh_uevt_s_t* evt)
{
    DIY_LOG(LOG_INFO, "reset key handler!\n");
    IGNORE_NON_PRESSED_EVT(evt);
}

void charger_gpio_handler(converted_gbh_uevt_s_t* evt)
{
    const char* reg_str;
    hv_mb_reg_e_t reg_addr = EXT_MB_REG_CHARGER;
    uint16_t write_data;

    DIY_LOG(LOG_INFO, "charg gpio handler!\n");
    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "evt ptr is NULL.\n");
        return;
    }

    if(key_pressed == evt->action)
    {
        write_data = MB_REG_V_CHARGER_IN;
    }
    else
    {
        write_data = MB_REG_V_CHARGER_OUT;
    }

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(gs_mb_tcp_client_ctx)
    {
        if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
        {
            DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n", reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            DIY_LOG(LOG_INFO, "%s: charger state changed: %hu. \n", g_key_gpio_name_list[evt->key_gpio], write_data); 
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
    }
}

