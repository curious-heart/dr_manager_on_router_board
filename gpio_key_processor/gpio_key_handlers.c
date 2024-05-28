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

static bool gs_restory_factory_key1_pressed = false, gs_restory_factory_key2_pressed = false;
static app_timer_node_s_t* gs_restore_factory_key_timer = NULL;
extern uint32_t g_restore_factory_time_hold_key_s;

static modbus_t * gs_mb_tcp_client_ctx = NULL;
#define CONNECT_FAIL_WAIT_TIME 3 //s
bool begin_key_event_handle()
{
    uint32_t res_t_sec, res_t_us;

    gs_mb_tcp_client_ctx = modbus_new_tcp(g_mb_tcp_client_params.srvr_ip, g_mb_tcp_client_params.srvr_port);
    if(!gs_mb_tcp_client_ctx)
    {
        DIY_LOG(LOG_ERROR, "modbus_new_tcp fail, %d:%s.\n", errno, modbus_strerror(errno));
        return false;
    }

    if(0 != modbus_set_debug(gs_mb_tcp_client_ctx, g_mb_tcp_client_params.debug_flag))
    {
        DIY_LOG(LOG_WARN, "modbus_set_debug fail, %d:%s\n", errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN + LOG_ONLY_INFO_STR_COMP, "but we go ahead.\n");
    }

    res_t_sec = (uint32_t)g_mb_tcp_client_params.wait_res_timeout_sec;
    res_t_us = (g_mb_tcp_client_params.wait_res_timeout_sec - res_t_sec) * 1000000;
    if(0 != modbus_set_response_timeout(gs_mb_tcp_client_ctx, res_t_sec, res_t_us))
    {
        DIY_LOG(LOG_ERROR, "modbus_set_response_timeout fail, %d:%s\n", errno, modbus_strerror(errno));
        modbus_free(gs_mb_tcp_client_ctx);
        gs_mb_tcp_client_ctx = NULL;
        return false;
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
    uint16_t write_data = 1;
    static const char* ls_on_off_str[] = {"OFF", "ON"};
    static bool ls_light_switch = false;

    hv_mb_reg_e_t reg_addr = RangeIndicationStart;
    const char* reg_str;

    IGNORE_NON_PRESSED_EVT(evt);

    DIY_LOG(LOG_INFO, "exp_range_led key handler!");

    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "\nexposure range led key evt ptr is NULL.\n");
        return;
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "action: %s\n", g_key_gpio_act_name_list[evt->action]);

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(gs_mb_tcp_client_ctx)
    {
        if(modbus_read_registers(gs_mb_tcp_client_ctx, RangeIndicationStatus, 1, &write_data) <= 0)
        {
            DIY_LOG(LOG_ERROR, "read RangeIndicationStatus error, use local recorded status for range light cmd.");
            ls_light_switch = !ls_light_switch;
            write_data = (uint16_t)ls_light_switch;
        }
        ls_light_switch = (bool)write_data;


        if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
        {
            DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n",
                   reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            DIY_LOG(LOG_INFO, "Turn %s exposure range light.\n", ls_on_off_str[write_data ? 1 : 0]); 
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
    }
}

static const uint16_t gs_exp_ready = 1, gs_exp_start = 2;
static bool gs_key_hold_processed = true;
static void exp_start_key_hold_handler(int sig)
{
    hv_mb_reg_e_t reg_addr = ExposureStart;
    uint16_t write_data;
    const char* reg_str;

    gs_key_hold_processed = true;

    if(g_key_gpio_cfg_params.exposure_disabled)
    {
        DIY_LOG(LOG_INFO, "exp_start key hold timeout. But exposure by key is disabled.\n");
        return;
    }

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

    DIY_LOG(LOG_INFO, "exp_start key handler!");

    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "\nexposure_start key evt ptr is NULL.\n");
        return;
    }

    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "action: %s\n", g_key_gpio_act_name_list[evt->action]);

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

static void restore_factory_settings(void* param)
{
    /*to_param should point to gs_restore_factory_key_timer */
    if(param) *(app_timer_node_s_t**)param = NULL;
    DIY_LOG(LOG_INFO, "restore factor settings now...\n");

    system("firstboot -y");
    sleep(3);
    system("reboot");
}
static bool restore_factory_setting_check_keys(void* param)
{
    return (gs_restory_factory_key1_pressed && gs_restory_factory_key2_pressed);
}

void dose_adjust_key_handler(converted_gbh_uevt_s_t* evt)
{
    const char* reg_str;
    hv_mb_reg_e_t reg_addr = EXT_MB_REG_DOSE_ADJ;
    uint16_t write_data;

    //IGNORE_NON_PRESSED_EVT(evt);

    DIY_LOG(LOG_INFO, "dose adjust key handler!");

    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "\ndose adjust handler, evt ptr is NULL.\n");
        return;
    }

    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, 
            "%s, action: %s\n", g_key_gpio_name_list[evt->key_gpio], g_key_gpio_act_name_list[evt->action]);


    if(key_pressed == evt->action)
    {
        write_data = (key_dose_add == evt->key_gpio) ? MB_REG_V_DOSE_ADJ_ADD : MB_REG_V_DOSE_ADJ_SUB;

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

        if(key_dose_add == evt->key_gpio)
        {
            gs_restory_factory_key1_pressed = true;
            gs_restore_factory_key_timer = add_a_new_app_timer(g_restore_factory_time_hold_key_s * 1000, true,
                    restore_factory_settings, &gs_restore_factory_key_timer,
                    restore_factory_setting_check_keys, NULL);
            DIY_LOG(LOG_INFO, "start restore factory setting timer on key press, %u sec.\n",
                    g_restore_factory_time_hold_key_s);
        }
        else
        {
            gs_restory_factory_key2_pressed = true;
        }
    }
    else
    {
        if(key_dose_add == evt->key_gpio)
        {
            gs_restory_factory_key1_pressed = false;
            delete_an_app_timer(gs_restore_factory_key_timer, true);
            gs_restore_factory_key_timer = NULL;
            DIY_LOG(LOG_INFO, "stop restore factory setting timer on key press.\n");
        }
        else
        {
            gs_restory_factory_key2_pressed = false;
        }
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

    DIY_LOG(LOG_INFO, "charg gpio handler!");
    
    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "\nevt ptr is NULL.\n");
        return;
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, "action: %s\n", g_key_gpio_act_name_list[evt->action]);

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

void update_tof_distance(uint16_t dist)
{
    const char* reg_str = get_hv_mb_reg_str(EXT_MB_REG_DISTANCE);
    if(modbus_write_register(gs_mb_tcp_client_ctx, EXT_MB_REG_DISTANCE, dist) <= 0)
    {
        DIY_LOG(LOG_ERROR, "modbus write register %s error:%d, %s\n", reg_str, errno, modbus_strerror(errno));
    }
}
