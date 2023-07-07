#include <errno.h>
#include <modbus/modbus.h>

#include "logger.h"
#include "gpio_key_processor.h"
#include "hv_registers.h"
#include "mb_tcp_server.h"

extern mb_tcp_client_params_t g_mb_tcp_client_params;
extern key_gpio_cfg_params_s_t g_key_gpio_cfg_params;

static modbus_t * gs_mb_tcp_client_ctx = NULL;
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

    if(modbus_connect(gs_mb_tcp_client_ctx) == -1)
    {
         DIY_LOG(LOG_ERROR, "Connection failed, %d:%s\n", errno, modbus_strerror(errno));
         modbus_free(gs_mb_tcp_client_ctx);
         return false;
    }
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
    static uint16_t write_data = true;
    static const char* ls_on_off_str[] = {"OFF", "ON"};

    hv_mb_reg_e_t reg_addr = RangeIndicationStart;
    const char* reg_str;

    DIY_LOG(LOG_INFO, "exp_reand_led key handler!\n");
    IGNORE_NON_PRESSED_EVT(evt);

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
            DIY_LOG(LOG_INFO, "Turn %s exposure range light.\n", ls_on_off_str[write_data ? 1 : 0]); 
            write_data = !write_data;
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
    }
}

void exp_start_key_handler(converted_gbh_uevt_s_t* evt)
{
    static const uint16_t exp_ready = 1, exp_start = 2;
    hv_mb_reg_e_t reg_addr = ExposureStart;
    uint16_t write_data = exp_start;
    const char* reg_str;

    DIY_LOG(LOG_INFO, "exp_reand_led key handler!\n");
    IGNORE_NON_PRESSED_EVT(evt);

    if(evt)
    {
        if(evt->seen >= g_key_gpio_cfg_params.exp_start_key_hold_time)
        {
            write_data = exp_start;
        }
        else
        {
            write_data = exp_ready;
        } 
    }

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

void dose_add_key_handler(converted_gbh_uevt_s_t* evt)
{
    DIY_LOG(LOG_INFO, "dose_add key handler!\n");
    IGNORE_NON_PRESSED_EVT(evt);
}

void dose_sub_key_handler(converted_gbh_uevt_s_t* evt)
{
    DIY_LOG(LOG_INFO, "dose_sub key handler!\n");
    IGNORE_NON_PRESSED_EVT(evt);
}

void reset_key_handler(converted_gbh_uevt_s_t* evt)
{
    DIY_LOG(LOG_INFO, "reset key handler!\n");
    IGNORE_NON_PRESSED_EVT(evt);
}

void charger_gpio_handler(converted_gbh_uevt_s_t* evt)
{
    DIY_LOG(LOG_INFO, "charg gpio handler!\n");
}
