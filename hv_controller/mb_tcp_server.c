/*
 * Copyright © Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * 20230609 LiLi@ZKXG use this file as a base of dr manager.
 */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <modbus/modbus.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#endif

#include "common_tools.h"
#include "logger.h"
#include "pthread_helper.h"
#include "hv_registers.h"
#include "hv_controller.h"
#include "dr_manager.h"
#include "mb_tcp_server.h"
#include "mb_tcp_server_internal.h"
#include "mb_tcp_server_ext_reg_handlers.h"
#include "option_configuration_process.h"
#include "main_app_used_gpios.h"
#include "dap_calc.h"

/*Do not use it in area out of modbus tcp server code.*/
const char* const gp_mb_server_log_header = "modbus server: ";

/**/
#define NB_CONNECTION 2

static modbus_t *gs_mb_srvr_ctx = NULL;
static modbus_mapping_t *gs_mb_mapping;
static int gs_mb_server_socket = -1;
static int gs_mb_header_len = -1;

static time_t gs_time_point_for_local_regs_refresh;

/*This global static var should only be accessed from main loop thread.*/
static dr_device_st_local_buf_t gs_hv_st;

static mb_tcp_server_params_t * gs_mb_tcp_server_params = NULL;

static uint16_t gs_dsp_sw_ver;
static bool gs_server_ready = false;

void mb_server_exit()
{
    if (gs_mb_server_socket != -1)
    {
        close(gs_mb_server_socket);
        gs_mb_server_socket = -1;
    }
    if(gs_mb_srvr_ctx)
    {
        modbus_close(gs_mb_srvr_ctx);
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
    }
    if(gs_mb_mapping)
    {
        modbus_mapping_free(gs_mb_mapping);
        gs_mb_mapping = NULL;
    }
    gs_mb_header_len = -1;

    close_DAP_db();
}

static const char* mb_exception_string(uint32_t exception_code)
{
    switch(exception_code)
    {
    case MODBUS_EXCEPTION_ILLEGAL_FUNCTION:
        return "MODBUS_EXCEPTION_ILLEGAL_FUNCTION";
    case MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS:
        return "MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS";
    case MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE:
        return "MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE";
    case MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE:
        return "MODBUS_EXCEPTION_SLAVE_OR_SERVER_FAILURE";
    case MODBUS_EXCEPTION_ACKNOWLEDGE:
        return "MODBUS_EXCEPTION_ACKNOWLEDGE";
    case MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY:
        return "MODBUS_EXCEPTION_SLAVE_OR_SERVER_BUSY";
    case MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE:
        return "MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE";
    case MODBUS_EXCEPTION_MEMORY_PARITY:
        return "MODBUS_EXCEPTION_MEMORY_PARITY";
    case MODBUS_EXCEPTION_NOT_DEFINED:
        return "MODBUS_EXCEPTION_NOT_DEFINED";
    case MODBUS_EXCEPTION_GATEWAY_PATH:
        return "MODBUS_EXCEPTION_GATEWAY_PATH";
    case MODBUS_EXCEPTION_GATEWAY_TARGET:
        return "MODBUS_EXCEPTION_GATEWAY_TARGET";

    default:
        ;
    }
    return "MODBUS_EXCEPTION_NOT_DEFINED";
}

static mb_reg_check_ret_t mb_server_check_func_reg_cnt(uint8_t * req_msg, 
        uint8_t * func, uint16_t * addr, uint16_t * cnt, uint32_t * exception_code)
{
    uint8_t func_code;
    uint16_t reg_addr_start, reg_cnt;
    mb_reg_check_ret_t ret;

    if(!req_msg)
    {
        if(exception_code) *exception_code = MODBUS_EXCEPTION_NOT_DEFINED;
        return MB_REG_CHECK_ERROR;
    }

    func_code = req_msg[gs_mb_header_len];
    if(func) *func = func_code;
    switch(func_code)
    {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        {
            reg_addr_start = MODBUS_GET_INT16_FROM_INT8(req_msg, gs_mb_header_len + 1);
            if(func_code != MODBUS_FC_WRITE_SINGLE_REGISTER)
            {
                reg_cnt = MODBUS_GET_INT16_FROM_INT8(req_msg, gs_mb_header_len + 3);
            }
            else
            {
                reg_cnt = 1;
            }

            if(addr) *addr = reg_addr_start;
            if(cnt) *cnt = reg_cnt;
            if(NORMAL_MB_REG_AND_CNT(reg_addr_start, reg_cnt))
            {
                if(exception_code) *exception_code = 0;
                ret = MB_NORMAL_REG;
            }
            else if(EXTEND_MB_REG_AND_CNT(reg_addr_start, reg_cnt))
            {
                if(exception_code) *exception_code = 0;
                ret = MB_EXTEND_REG;
            }
            else
            {
                if(exception_code) *exception_code = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
                ret = MB_INVALID_REG;
            }
        }
        break;

        default:
            if(exception_code) *exception_code = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
            ret = MB_REG_CHECK_ERROR;
            break;
    }

    DIY_LOG(LOG_INFO, "%sfunc_code:%u.\n", gp_mb_server_log_header, func_code); 
    if(ret != MB_REG_CHECK_ERROR)
    {
        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "%sreg_reg_start:%d(%s), reg_cnt:%d.\n",
                gp_mb_server_log_header, reg_addr_start, get_hv_mb_reg_str(reg_addr_start), reg_cnt); 
    }

    return ret;
}

/*
 * NOTE: this variable is like g_device_st_pool defined in dev_monitor_thread.c: it SHOULD be only accessed
 * in thread safe case, e.g. from access_device_st_pool.
 * */
static mb_reg_val_pair_s_t gs_mb_reg_val_pairs_sent_external[] =
{
    REGS_TO_SEND_EXTERNAL
};
static const int gs_mb_reg_cnt_to_send_external = ARRAY_ITEM_CNT(gs_mb_reg_val_pairs_sent_external);
int get_reg_key_val_pair_to_send_external(mb_reg_val_pair_s_t * pairs[])
{
    if(pairs) (*pairs) = gs_mb_reg_val_pairs_sent_external;
    return gs_mb_reg_cnt_to_send_external;
}
/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 */
static bool update_dev_st_pool_from_main_loop_th(void* d)
{
    bool updated = false;
    mb_reg_val_pair_s_t *pair_arr = NULL;
    int pair_cnt = 0, pair_idx;
    uint16_t info_word = 0;
    bool becare;

    if(!d)
    {
        return updated;
    }

    dr_device_st_local_buf_t * hv_st = (dr_device_st_local_buf_t*)d;

    ST_PARAM_SET_UPD(g_device_st_pool, bat_lvl, hv_st->bat_lvl);
    ST_PARAM_SET_UPD(g_device_st_pool, bat_chg_st, hv_st->bat_chg_st);
    ST_PARAM_SET_UPD(g_device_st_pool, bat_chg_full, hv_st->bat_chg_full);
    ST_PARAM_SET_UPD(g_device_st_pool, hv_dsp_conn_st, hv_st->hv_dsp_conn_st);
    ST_PARAM_SET_UPD(g_device_st_pool, expo_volt_kv, hv_st->expo_volt_kv);
    ST_PARAM_SET_UPD(g_device_st_pool, expo_dura_ms, hv_st->expo_dura_ms);
    ST_PARAM_SET_UPD(g_device_st_pool, expo_am_ua, hv_st->expo_am_ua);
    ST_PARAM_SET_UPD(g_device_st_pool, expo_st, hv_st->expo_st);
    ST_PARAM_SET_UPD(g_device_st_pool, range_light_on, hv_st->range_light_on);

#ifndef MANAGE_LCD_AND_TOF_HERE
    /* TOF distance is written into mb register by external user, it has been written into local buffer, and now we 
     * update it to global buffer. In fact, global buffer is not used now since LCD refresh thread does not exist now...
     * */
    ST_PARAM_SET_UPD(g_device_st_pool, tof_distance, hv_st->tof_distance);

#endif
    pair_cnt = get_reg_key_val_pair_to_send_external(&pair_arr);
    if(pair_arr && pair_cnt > 0)
    {
        for(pair_idx = 0; pair_idx < pair_cnt; ++pair_idx)
        {
            if(!VALID_MB_REG_ADDR(pair_arr[pair_idx].reg)) continue;

            becare = true;
            if(EXTEND_MB_REG_ADDR(pair_arr[pair_idx].reg))
            {
                switch(pair_arr[pair_idx].reg)
                {
                    case EXT_MB_REG_DISTANCE:
#ifdef MANAGE_LCD_AND_TOF_HERE
                        gs_mb_mapping->tab_registers[EXT_MB_REG_DISTANCE] = ST_PARAM_GET(g_device_st_pool, tof_distance);
#endif
                        break;

                    EVAL_EXT_MB_REG_FRO_ST(gs_mb_mapping->tab_registers[pair_arr[pair_idx].reg])

                    default:
                        becare = false;
                        break;
                }
            }
            if(becare) pair_arr[pair_idx].val = gs_mb_mapping->tab_registers[pair_arr[pair_idx].reg];
        }
    }

    return updated;
}
/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 * */
static bool update_local_extend_regs(void* reg)
{
    hv_mb_reg_e_t start_reg, end_reg, cur_reg;
    uint16_t info_word = 0;

    if(!reg)
    {
        /*update all extend regs.*/
        start_reg = EXT_MB_REG_DISTANCE;
        end_reg = HV_MB_REG_END_FLAG;
    }
    else
    {
        /*update a single reg.*/
        start_reg = *((hv_mb_reg_e_t*)reg);
        end_reg = start_reg + 1;
    }
    for(cur_reg = start_reg; cur_reg < end_reg; ++cur_reg)
    {
        switch(cur_reg)
        {
            case EXT_MB_REG_DISTANCE:
#ifdef MANAGE_LCD_AND_TOF_HERE
                gs_mb_mapping->tab_registers[EXT_MB_REG_DISTANCE] = ST_PARAM_GET(g_device_st_pool, tof_distance);
#endif
                break;

            EVAL_EXT_MB_REG_FRO_ST(gs_mb_mapping->tab_registers[cur_reg])

            default:
                break;
        }
    }
    return true;
}

/*----------------------------------------------------------------------------------------------*/

static void update_local_st_from_regs()
{
    gs_hv_st.bat_lvl = gs_mb_mapping->tab_registers[BatteryLevel];
    gs_hv_st.expo_volt_kv = gs_mb_mapping->tab_registers[VoltSet];
    gs_hv_st.expo_am_ua = gs_mb_mapping->tab_registers[FilamentSet];
    gs_hv_st.expo_dura_ms = gs_mb_mapping->tab_registers[ExposureTime];
    gs_hv_st.expo_st = gs_mb_mapping->tab_registers[ExposureStatus];
    gs_hv_st.range_light_on = gs_mb_mapping->tab_registers[RangeIndicationStatus];
}

extern cmd_line_opt_collection_t g_cmd_line_opt_collection;

/*DO NOT call this function from outside of main thread. So DO NOT export it in .h file, but declare it as necessary.*/
void refresh_global_dev_st_info_from_main_th()
{
    bool upd = access_device_st_pool(pthread_self(),g_main_thread_desc, update_dev_st_pool_from_main_loop_th, &gs_hv_st);
    if(upd)
    {
        update_lcd_display(pthread_self(), g_main_thread_desc);
    }
}

/*Return true if the charge full st is updated.*/
static bool check_bat_chg_full_pin()
{

    /*2023-09-24 去掉对满电状态的判断。参考main_app_used_gpios.h中的注释说明*/
    gs_hv_st.bat_chg_full = true;
    return false;

    /*
    int chg_full_st;

    chg_full_st = app_read_gpio_value(GPIO_CHARGER_FULL_IND); 

    DIY_LOG(LOG_DEBUG, "........chg_full_st: %d, gs_hv_st.bat_chg_full:%d\n", chg_full_st, gs_hv_st.bat_chg_full);

    if((bool)chg_full_st != gs_hv_st.bat_chg_full)
    {
        gs_hv_st.bat_chg_full = (bool)chg_full_st;
        return true;
    }
    else
    {
        return false;
    }
    */
}

/*This functin should only be called from main loop thread. So DO NOT export it in .h file, but declare it as necessary.*/
void set_bat_chg_state(battery_chg_st_t st)
{
    gs_hv_st.bat_chg_st = st;
}

/*This functin should only be called from main loop thread. So DO NOT export it in .h file, but declare it as necessary.*/
battery_chg_st_t get_local_rec_bat_chg_state()
{
    return gs_hv_st.bat_chg_st;
}

static mb_rw_reg_ret_t mb_server_pre_check_write_reg(uint16_t reg_addr_start, uint16_t * data_arr, uint16_t reg_cnt)
{
    mb_rw_reg_ret_t ret = MB_RW_REG_RET_NONE; 
    uint16_t idx;
    bool exposure_start = false;
    uint16_t exposure_start_cmd = 0;
#ifdef MANAGE_LCD_AND_TOF_HERE
    uint16_t  turn_on_exposure_range_led = 0;
    bool exposure_range_led = false;
#endif

    for(idx = 0; idx < reg_cnt; ++idx)
    {
        if(ExposureStart == (reg_addr_start + idx))
        {
            exposure_start = true;
            exposure_start_cmd = data_arr[idx];
        }
#ifdef MANAGE_LCD_AND_TOF_HERE
        else if(RangeIndicationStart == (reg_addr_start + idx))
        {
            exposure_range_led = true;
            turn_on_exposure_range_led = data_arr[idx];
        }
#endif
    }

    if(exposure_start && (MB_REG_EXPOSURE_START_CMD == exposure_start_cmd))
    {
        if(!gs_mb_tcp_server_params->allow_force_exposure)
        {
            int distance; //unit is mm.

            DIY_LOG(LOG_INFO, "%s pre-check exposure start command.\n", gp_mb_server_log_header);

#ifdef MANAGE_LCD_AND_TOF_HERE
            distance = (int)request_tof_distance(TOF_REQUESTER_EXPOSURE, 
                    gs_mb_tcp_server_params->req_tof_dist_wait_time, gs_mb_tcp_server_params->expo_tof_measure_wait);
            gs_mb_mapping->tab_registers[EXT_MB_REG_DISTANCE] = distance;
#else
            /*distance is set by external user to mb reg, and already updated to local buffer.*/
            distance = gs_hv_st.tof_distance;
#endif
            if(distance < MIN_ALLOWED_FSD_IN_CM * 10)
            {
                DIY_LOG(LOG_WARN, "%sdistance %d is too small to start exposure.\n", gp_mb_server_log_header, distance);
                ret = MB_RW_REG_RET_REJ_TOO_CLOSE;
            }
            else
            {
                DIY_LOG(LOG_WARN, "%sdistance %d is enough to start exposure\n", gp_mb_server_log_header, distance);
            }
#ifdef MANAGE_LCD_AND_TOF_HERE
            unset_tof_th_measure_flag(TOF_REQUESTER_EXPOSURE);
#endif
        }
        else
        {
            DIY_LOG(LOG_INFO, "allow_force_exposure is enabled, go exposuring.\n");
        }
    }
#ifdef MANAGE_LCD_AND_TOF_HERE
    else if(exposure_range_led)
    {
        if(turn_on_exposure_range_led) //turn on range indicator command
        {
            set_tof_th_measure_flag(TOF_REQUESTER_RANGE_LED);
            inform_tof_th_to_measure();
        }
        else //turn off range indicator command
        {
            unset_tof_th_measure_flag(TOF_REQUESTER_RANGE_LED);
        }
    }
#endif

    return ret;
}

#define LOWEST_BYTE_ADDR_OF_DAP_IN_MAPPING_REGS ((uint8_t*)(&gs_mb_mapping->tab_registers[(EXT_MB_REG_DAP_LP)]) + 1)
#define HIGHEST_BYTE_ADDR_OF_DAP_IN_MAPPING_REGS ((uint8_t*)(&gs_mb_mapping->tab_registers[(EXT_MB_REG_DAP_HP)]))
static void set_float_DAP_to_mapping_reg(float DAP_v)
{
    uint8_t * reg_B_3, *float_B_0;
    int idx, float_size = 4;

    reg_B_3 = LOWEST_BYTE_ADDR_OF_DAP_IN_MAPPING_REGS;
    float_B_0 = (uint8_t*)&DAP_v;
    for(idx = 0; idx < float_size; ++idx)
    {
        *(reg_B_3 - idx) = *(float_B_0 + idx);
    }
}

static void range_light_timeout_handler(void* to_param)
{
    DIY_LOG(LOG_INFO, "range light timer times out.\n");

    /*turn off range light*/
    hv_controller_write_single_uint16(RangeIndicationStart, 0);

#ifdef MANAGE_LCD_AND_TOF_HERE
    /*stop tof measure.*/
    unset_tof_th_measure_flag(TOF_REQUESTER_RANGE_LED);
#endif

    /*to_param should point to the ls_range_light_timer in function mb_server_write_reg_sniff*/
    if(to_param) *(app_timer_node_s_t**)to_param = NULL;
}

mb_rw_reg_ret_t mb_server_write_reg_sniff(uint16_t reg_addr_start, uint16_t * data_arr, uint16_t reg_cnt, bool server_only)
{
    bool becare = false;
    uint16_t idx = 0;
    mb_rw_reg_ret_t ret = MB_RW_REG_RET_NONE; 
    float DAP_v;
    static app_timer_node_s_t* ls_range_light_timer = NULL;

    if(gs_hv_st.hv_dsp_conn_st != HV_DSP_CONNECTED)
    {
        gs_hv_st.hv_dsp_conn_st = HV_DSP_CONNECTED;
        becare = true;
    }

    for(idx = 0; idx < reg_cnt; ++idx)
    {
        switch(reg_addr_start + idx)
        {
            case VoltSet:     // 5,          /*5管电压设置值*/
                gs_hv_st.expo_volt_kv = data_arr[idx];
                becare = true;

                DAP_v = calculate_DAP_value(gs_hv_st.expo_volt_kv, gs_hv_st.expo_am_ua, gs_hv_st.expo_dura_ms);
                set_float_DAP_to_mapping_reg(DAP_v);
                break;

            case FilamentSet:     // 6,      /*6 管设置值电流 （决定灯丝电流决定管电流）*/
                gs_hv_st.expo_am_ua = data_arr[idx];
                becare = true;

                DAP_v = calculate_DAP_value(gs_hv_st.expo_volt_kv, gs_hv_st.expo_am_ua, gs_hv_st.expo_dura_ms);
                set_float_DAP_to_mapping_reg(DAP_v);
                break;

            case ExposureTime:     // 7,     /*曝光时间*/
                gs_hv_st.expo_dura_ms = data_arr[idx];
                becare = true;

                DAP_v = calculate_DAP_value(gs_hv_st.expo_volt_kv, gs_hv_st.expo_am_ua, gs_hv_st.expo_dura_ms);
                set_float_DAP_to_mapping_reg(DAP_v);
                break;

            case RangeIndicationStart:
                gs_hv_st.range_light_on = data_arr[idx];
                becare = true;

                if(gs_hv_st.range_light_on && !ls_range_light_timer)
                {
                    /*light is turned on, start timer count.*/
                    ls_range_light_timer = add_a_new_app_timer(g_cmd_line_opt_collection.range_light_auto_off_time_s * 1000,
                            true, range_light_timeout_handler, &ls_range_light_timer, NULL, NULL);
                    if(ls_range_light_timer)
                    {
                        DIY_LOG(LOG_INFO, "start range light timer (%u sec) success.\n",
                                g_cmd_line_opt_collection.range_light_auto_off_time_s);
                    }
                    else
                    {
                        DIY_LOG(LOG_ERROR, "start range light timer (%u sec) fail!\n", 
                                    g_cmd_line_opt_collection.range_light_auto_off_time_s);
                    }
                }
                else if(!gs_hv_st.range_light_on && ls_range_light_timer)
                {
                    /*light is turned off, stop timer count.*/
                    delete_an_app_timer(ls_range_light_timer, true);
                    ls_range_light_timer = NULL;
                    DIY_LOG(LOG_INFO, "stop range light timer on key press.\n");
                }

                /*update RangeIndicationStatus in local buffer.*/
                if(!hv_controller_read_uint16s(RangeIndicationStatus,
                            &gs_mb_mapping->tab_registers[RangeIndicationStatus], 1))
                {
                    gs_mb_mapping->tab_registers[RangeIndicationStatus] = gs_hv_st.range_light_on;
                }
                break;

            case ExposureStart:     // 13,   /*曝光启动*/
                if(!server_only)
                {
                    gs_hv_st.expo_st = EXPOSURE_ST_ALARM; 
                    becare = true;
                    ret = MB_RW_REG_RET_USE_SHORT_WAIT_TIME;
                }

#ifdef MANAGE_LCD_AND_TOF_HERE
                unset_tof_th_measure_flag(TOF_REQUESTER_EXPOSURE);
#endif
                break;

            default:
                ;
        }
    }
    if(becare)
    {
        refresh_global_dev_st_info_from_main_th();
    }

    return ret;
}

mb_rw_reg_ret_t mb_server_read_reg_sniff(uint16_t reg_addr_start, uint16_t * data_arr, uint16_t reg_cnt, bool server_only)
{
    bool becare = false;
    uint16_t idx = 0;
    mb_rw_reg_ret_t ret = MB_RW_REG_RET_NONE; 

    if(server_only)
    {
        return ret;
    }

    if(gs_hv_st.hv_dsp_conn_st != HV_DSP_CONNECTED)
    {
        gs_hv_st.hv_dsp_conn_st = HV_DSP_CONNECTED;
        becare = true;
    }

    for(idx = 0; idx < reg_cnt; ++idx)
    {
        switch(reg_addr_start + idx)
        {
            case ExposureStatus:    // = 11),                /*曝光状态*/
                if(gs_hv_st.expo_st != data_arr[idx])
                {
                    gs_hv_st.expo_st = data_arr[idx];
                    becare = true;

                    if(EXPOSURE_ST_IDLE == gs_hv_st.expo_st)
                    {
                        ret = MB_RW_REG_RET_USE_LONG_WAIT_TIME;
                    }
                    else
                    {
                        ret = MB_RW_REG_RET_USE_SHORT_WAIT_TIME;
                    }
                }
                break;


            case BatteryLevel:
                if(gs_hv_st.bat_lvl != data_arr[idx])
                {
                    gs_hv_st.bat_lvl = data_arr[idx];
                    becare = true;
                }
                if(check_bat_chg_full_pin())
                {
                    becare = true;
                }
                break;

            default:
                break;
        }
    }
    if(becare)
    {
        refresh_global_dev_st_info_from_main_th();
    }

    return ret;
}

static mb_rw_reg_ret_t read_hv_st_from_internal()
{
    mb_rw_reg_ret_t process_ret = MB_RW_REG_RET_NONE; 
    bool becare = false;

    if(hv_controller_read_uint16s(HSV, &gs_mb_mapping->tab_registers[HSV], (OilBoxTemperature - HSV + 1))
        && hv_controller_read_uint16s(Workstatus, &gs_mb_mapping->tab_registers[Workstatus],
                                        (exposureCount - Workstatus + 1)))
    {
        gs_time_point_for_local_regs_refresh = time(NULL);

        if(gs_hv_st.hv_dsp_conn_st != HV_DSP_CONNECTED)
        {
            becare = true;
            gs_hv_st.hv_dsp_conn_st = HV_DSP_CONNECTED;
        }

        if(gs_hv_st.expo_st != gs_mb_mapping->tab_registers[ExposureStatus])
        {
            becare = true;
            gs_hv_st.expo_st = gs_mb_mapping->tab_registers[ExposureStatus];
            if(EXPOSURE_ST_IDLE == gs_hv_st.expo_st)
            {
                process_ret = MB_RW_REG_RET_USE_LONG_WAIT_TIME;
            }
            else
            {
                process_ret = MB_RW_REG_RET_USE_SHORT_WAIT_TIME;
            }
        }

        if(gs_hv_st.bat_lvl != gs_mb_mapping->tab_registers[BatteryLevel])
        {
            becare = true;
            gs_hv_st.bat_lvl = gs_mb_mapping->tab_registers[BatteryLevel];
        }

        if(check_bat_chg_full_pin())
        {
            becare = true;
        }

        update_local_st_from_regs();
    }
    else
    {
        gs_hv_st.hv_dsp_conn_st = HV_DSP_DISCONNECTED;
        process_ret = MB_RW_REG_RET_ERROR;
        becare = true;
        DIY_LOG(LOG_ERROR, "%sread hv state from internall error.\n", gp_mb_server_log_header);
    }

    if(becare)
    {
        refresh_global_dev_st_info_from_main_th();
    }

    return process_ret;
}

static void mb_tcp_server_fill_mapping_tab_reg_from_msg(uint8_t *req_msg, int offset, uint16_t reg_addr_start, uint16_t cnt, bool check_rw)
{
    uint16_t idx;
    for(idx = 0; idx < cnt; ++idx)
    {
        if(VALID_MB_REG_ADDR(reg_addr_start + idx))
        {
            if(check_rw && (HV_MB_REG_RW_ATTR_R == get_hv_mb_reg_rw_attr(reg_addr_start + idx)))
            {
                DIY_LOG(LOG_ERROR, "%sthe register %d is read-only, can't be written.\n",
                        gp_mb_server_log_header, (reg_addr_start + idx));
                continue;
            }

            gs_mb_mapping->tab_registers[(reg_addr_start + idx)]
                = MODBUS_GET_INT16_FROM_INT8(req_msg, gs_mb_header_len + offset  + 2 * idx);
        }
        else
        {
            DIY_LOG(LOG_ERROR, "Invalid reg addr: %d, not fill into tab_registers.\n", reg_addr_start + idx);
        }
    }
}

#define MODBUS_REPLY(ctx, req_msg, req_msg_len, mapping) \
{\
        int ret_len;\
        ret_len = modbus_reply(ctx, req_msg, req_msg_len, mapping); \
        if(ret_len <= 0)\
        {\
             DIY_LOG(LOG_ERROR, "%sreply error with ret_len %d, %d:%s.\n",\
                 gp_mb_server_log_header, ret_len, errno, modbus_strerror(errno));\
             return MB_RW_REG_RET_ERROR;\
        }\
}

static mb_rw_reg_ret_t mb_server_process_extend_reg(uint8_t * req_msg, int req_msg_len, 
        uint8_t func, uint16_t reg_addr_start, uint16_t reg_cnt, bool server_only)
{
#ifdef MANAGE_LCD_AND_TOF_HERE
    bool becare = false;
#endif
    mb_rw_reg_ret_t ret = MB_RW_REG_RET_NONE;

    DIY_LOG(LOG_INFO, "%sProcess extend register.\n", gp_mb_server_log_header);
    switch(func)
    {
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            int offset;
            uint16_t idx;

            if(MODBUS_FC_WRITE_SINGLE_REGISTER == func)
            {
                offset = MB_REQ_MSG_SINGLE_VALUE_OFFSET_AFTER_HDR;
            }
            else
            {
                offset = MB_REQ_MSG_MULTI_VALUE_OFFSET_AFTER_HDR;
            }

            for(idx = 0; idx < reg_cnt; ++idx)
            {
                if(EXTEND_MB_REG_ADDR(reg_addr_start + idx))
                {
                    uint16_t write_data;

                    if((HV_MB_REG_RW_ATTR_R == get_hv_mb_reg_rw_attr(reg_addr_start + idx)))
                    {
                        DIY_LOG(LOG_ERROR, "%sthe register %d is read-only, can't be written.\n",
                                gp_mb_server_log_header, (reg_addr_start + idx));
                        ret = MB_RW_REG_RET_ERROR;
                        break;
                    }

                    gs_mb_mapping->tab_registers[(reg_addr_start + idx)]
                        = MODBUS_GET_INT16_FROM_INT8(req_msg, gs_mb_header_len + offset  + 2 * idx);

                    write_data = gs_mb_mapping->tab_registers[(reg_addr_start + idx)];
                    switch(reg_addr_start + idx)
                    {
                        case EXT_MB_REG_DOSE_ADJ:
                            ret = mb_tcp_srvr_ext_reg_dose_adj_handler(write_data, server_only);
                            break;

                        case EXT_MB_REG_CHARGER:
                            ret = mb_tcp_srvr_ext_reg_charger_handler(write_data);
                            break;

                        case EXT_MB_REG_DISTANCE:
#ifdef MANAGE_LCD_AND_TOF_HERE
                            if(gs_hv_st.tof_distance != write_data)
                            {
                                becare = true;
                            }
#endif
                            gs_hv_st.tof_distance = write_data;
                            DIY_LOG(LOG_INFO, "tof distance: %u\n", write_data);
                            break;

                        default:
                            DIY_LOG(LOG_WARN + LOG_ONLY_INFO_STR_COMP, "But nothing to do now...\n");
                            break;
                    }
                    if(ret != MB_RW_REG_RET_NONE)
                    {
                        break;
                    }

                }
                else
                {
                    DIY_LOG(LOG_ERROR, "%sInvalid extend reg addr: %d.\n", gp_mb_server_log_header, reg_addr_start + idx);
                    ret = MB_RW_REG_RET_ERROR;
                    break;
                }
            }

            if(MB_RW_REG_RET_ERROR == ret)
            {
                DIY_LOG(LOG_ERROR, "%sprocess extend register(s) %s write error, reg_cnt:%d.\n",
                                    gp_mb_server_log_header, get_hv_mb_reg_str(reg_addr_start), reg_cnt);
                modbus_reply_exception(gs_mb_srvr_ctx, req_msg, MODBUS_EXCEPTION_NOT_DEFINED);
            }
            else
            {
                MODBUS_REPLY(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
            }
        }
        break;

        case MODBUS_FC_READ_HOLDING_REGISTERS:
        {
            int idx;
            access_device_st_pool(pthread_self(),g_main_thread_desc, update_local_extend_regs, NULL);
            for(idx = 0; idx < reg_cnt; ++idx)
            {
                if(!EXTEND_MB_REG_ADDR(reg_addr_start + idx))
                {
                    DIY_LOG(LOG_ERROR, "%sInvalid extend reg addr: %d.\n", gp_mb_server_log_header, reg_addr_start + idx);
                    ret = MB_RW_REG_RET_ERROR;
                    break;
                }
                else if(HV_MB_REG_RW_ATTR_W == get_hv_mb_reg_rw_attr(reg_addr_start + idx))
                {
                    DIY_LOG(LOG_ERROR, "%sthe register %d is write-only, can't be read.\n",
                            gp_mb_server_log_header, (reg_addr_start + idx));
                    ret =  MB_RW_REG_RET_ERROR ;
                    break;
                }
            }

            if(MB_RW_REG_RET_ERROR == ret)
            {
                modbus_reply_exception(gs_mb_srvr_ctx, req_msg, MODBUS_EXCEPTION_NOT_DEFINED);
            }
            else
            {
                MODBUS_REPLY(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
            }
        }
        break;

        default:
            DIY_LOG(LOG_WARN + LOG_ONLY_INFO_STR_COMP, "But nothing to do now...\n");
            break;
    }

#ifdef MANAGE_LCD_AND_TOF_HERE
    if(becare)
    {
        refresh_global_dev_st_info_from_main_th();
    }
#endif

    return ret;
}

#if 0
static void exchange_info_with_mb_reg(bool read_reg)
{
    uint16_t info_word = 0;
#ifdef MANAGE_LCD_AND_TOF_HERE
    if(!read_reg)
    {
        /*TOF distance is acquired by tof thread which updated it into g_device_st_pool, and here we update it into mb reg.*/
        gs_mb_mapping->tab_registers[EXT_MB_REG_DISTANCE] = ST_PARAM_GET(g_device_st_pool, tof_distance);
    }
#else
    if(read_reg)
    {
        /*TOF distance is written into mb register by external user, we should update it into local buffer.*/
        gs_hv_st.tof_distance = gs_mb_mapping->tab_registers[EXT_MB_REG_DISTANCE];
    }
#endif
    if(!read_reg)
    {
        gs_mb_mapping->tab_registers[EXT_MB_REG_HOTSPOT_ST] = ST_PARAM_GET(g_device_st_pool, hot_spot_st); 

        gs_mb_mapping->tab_registers[EXT_MB_REG_CELLUAR_ST] 
            = (((uint16_t)(ST_PARAM_GET(g_device_st_pool,cellular_signal_bars))) << 8) 
                | ((uint16_t)(ST_PARAM_GET(g_device_st_pool,cellular_mode)) & 0xFF); 

        gs_mb_mapping->tab_registers[EXT_MB_REG_WIFI_WAN_SIG_AND_BAT_LVL] 
            = (((uint16_t)(ST_PARAM_GET(g_device_st_pool, bat_lvl))) << 8) 
                | ((uint16_t)(ST_PARAM_GET(g_device_st_pool, wifi_wan_st)) & 0xFF); 

        SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, bat_chg_st), CHARGER_CONNECTED, MB_REG_DEV_INFO_BITS_CHG_CONN)
        SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, bat_chg_full), true, MB_REG_DEV_INFO_BITS_BAT_FULL)
        SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, wan_bear) & WWAN_BEAR_WIFI, 
                          WWAN_BEAR_WIFI, MB_REG_DEV_INFO_BITS_WIFI_WAN_CONN)
        SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool, wan_bear) & WWAN_BEAR_CELLULAR, 
                          WWAN_BEAR_CELLULAR, MB_REG_DEV_INFO_BITS_CELL_WAN_CONN)
        SET_DEV_INFO_BITS(ST_PARAM_GET(g_device_st_pool,sim_card_st), SIM_CARD_NORM, MB_REG_DEV_INFO_BITS_SIM_READY)

        gs_mb_mapping->tab_registers[EXT_MB_REG_DEV_INFO_BITS] = info_word;
    }
}
#endif

static mb_rw_reg_ret_t mb_server_process_req(uint8_t * req_msg, int req_msg_len, bool * comm_with_dsp,
                                             bool server_only)
{
    mb_rw_reg_ret_t process_ret = MB_RW_REG_RET_NONE; 
    mb_reg_check_ret_t check_ret;
    uint8_t func_code = 255;
    uint16_t reg_addr_start = 0, reg_cnt = 0;
    uint32_t exception_code;

    if(NULL == req_msg)
    {
        DIY_LOG(LOG_ERROR, "%smodbus server: receive NULL msg.\n",
                gp_mb_server_log_header );
        return MB_RW_REG_RET_ERROR;
    }

    check_ret = mb_server_check_func_reg_cnt(req_msg, 
                                      &func_code, &reg_addr_start, &reg_cnt, &exception_code);
    if(MB_INVALID_REG == check_ret || MB_REG_CHECK_ERROR == check_ret)
    {
        DIY_LOG(LOG_ERROR, "%sinvalid req, %u:%s\n", gp_mb_server_log_header, 
                            exception_code,
                            mb_exception_string(exception_code));
        modbus_reply_exception(gs_mb_srvr_ctx, req_msg, exception_code);
        return MB_RW_REG_RET_ERROR;
    }

    //exchange_info_with_mb_reg(false);

    if(comm_with_dsp && (server_only || MB_REG_COMM_DSP(reg_addr_start, reg_cnt)))
    {
        *comm_with_dsp = true;
    }

    if(MB_EXTEND_REG == check_ret)
    {
        /*extend register process.*/
        process_ret = mb_server_process_extend_reg(req_msg, req_msg_len, func_code, reg_addr_start, reg_cnt, server_only);
        //exchange_info_with_mb_reg(true);
        return process_ret;
    }

    /*normal register process.*/
    switch(func_code)
    {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        {
            if(server_only)
            {
                MODBUS_REPLY(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
                process_ret = mb_server_read_reg_sniff(reg_addr_start,
                                        &gs_mb_mapping->tab_registers[(reg_addr_start)],
                                        reg_cnt, server_only);
            }
            else
            {
                if(hv_controller_read_uint16s(reg_addr_start,
                            &gs_mb_mapping->tab_registers[(reg_addr_start)],
                            reg_cnt))
                {
                    MODBUS_REPLY(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
                    process_ret = mb_server_read_reg_sniff(reg_addr_start,
                                            &gs_mb_mapping->tab_registers[(reg_addr_start)],
                                            reg_cnt, server_only);

                }
                else
                {
                    DIY_LOG(LOG_ERROR, "%sread registers from hv_controller error.\n",
                                        gp_mb_server_log_header);
                    modbus_reply_exception(gs_mb_srvr_ctx, req_msg, 
                                            MODBUS_EXCEPTION_NOT_DEFINED);
                    process_ret = MB_RW_REG_RET_ERROR;
                }
            }
        }
        break;

        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            int offset;

            if(MODBUS_FC_WRITE_SINGLE_REGISTER == func_code)
            {
                offset = MB_REQ_MSG_SINGLE_VALUE_OFFSET_AFTER_HDR;
            }
            else
            {
                offset = MB_REQ_MSG_MULTI_VALUE_OFFSET_AFTER_HDR;
            }

            /*the operation is transfered to hv-dsp, so we does not control rw attritbute here.*/
            mb_tcp_server_fill_mapping_tab_reg_from_msg(req_msg, offset, reg_addr_start, reg_cnt, false);

            process_ret = mb_server_pre_check_write_reg(reg_addr_start, 
                                        &gs_mb_mapping->tab_registers[(reg_addr_start)], reg_cnt);
            if(process_ret < MB_RW_REG_RET_NONE)
            {
                DIY_LOG(LOG_ERROR, "%spre check write reg error.\n",gp_mb_server_log_header);
                modbus_reply_exception(gs_mb_srvr_ctx, req_msg, MODBUS_EXCEPTION_NOT_DEFINED);
                break;
            }
            
            if(server_only)
            {
                MODBUS_REPLY(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
                process_ret = mb_server_write_reg_sniff(reg_addr_start,
                                    &gs_mb_mapping->tab_registers[(reg_addr_start)], reg_cnt, server_only);
            }
            else
            {
                if(hv_controller_write_uint16s(reg_addr_start,
                            &gs_mb_mapping->tab_registers[(reg_addr_start)],
                            reg_cnt))
                {
                    MODBUS_REPLY(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
                    process_ret = mb_server_write_reg_sniff(reg_addr_start,
                                        &gs_mb_mapping->tab_registers[(reg_addr_start)], reg_cnt, server_only);
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "%swrite register(s) to hv_controller error.\n", gp_mb_server_log_header);
                    modbus_reply_exception(gs_mb_srvr_ctx, req_msg, 
                                            MODBUS_EXCEPTION_NOT_DEFINED);
                    process_ret = MB_RW_REG_RET_ERROR;
                }
            }
        }
        break;

        default:
            DIY_LOG(LOG_WARN, "%sthe function %d is not supported.\n", gp_mb_server_log_header, func_code);
            modbus_reply_exception(gs_mb_srvr_ctx, req_msg, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
            process_ret = MB_RW_REG_RET_ERROR;
            break;
    }

    //exchange_info_with_mb_reg(true);
    return process_ret;
}

modbus_t * mb_server_get_ctx()
{
    return gs_mb_srvr_ctx;
}

modbus_mapping_t * mb_server_get_mapping()
{
    return gs_mb_mapping;
}

static void init_reg_refresh()
{
    mb_rw_reg_ret_t process_ret; 

    process_ret = read_hv_st_from_internal();
    if(process_ret >= MB_RW_REG_RET_NONE)
    {
        gs_dsp_sw_ver = gs_mb_mapping->tab_registers[HSV];
    }
    else
    {
        DIY_LOG(LOG_ERROR, "%sinitial modbus registers read error.\n", gp_mb_server_log_header);
    }
}

uint16_t get_dsp_sw_ver()
{
    return gs_dsp_sw_ver;
}
bool mb_server_is_ready()
{
    return gs_server_ready;
}

void write_version_str_to_file();
mb_server_exit_code_t  mb_server_loop(mb_tcp_server_params_t * srvr_params, bool server_only)
{
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    socklen_t addrlen;
    struct sockaddr_in clientaddr;
    /* Maximum file descriptor number */
    int fdmax;
    uint32_t resp_timeout_ms = 500;
    struct timeval timeout;
    bool timeout_updated = false, ex_client_active = false;
    mb_rw_reg_ret_t process_ret;

    if(!srvr_params)
    {
        DIY_LOG(LOG_ERROR, "%sserver parameters pointer is NULL.\n", gp_mb_server_log_header);
        return MB_SERVER_EXIT_INIT_FAIL;
    }
    gs_mb_tcp_server_params = srvr_params;

    timeout.tv_sec = 0;
    timeout.tv_usec = (suseconds_t)(srvr_params->long_select_wait_time * 1000000);

    gs_mb_srvr_ctx = modbus_new_tcp(srvr_params->srvr_ip, srvr_params->srvr_port);
    if(NULL == gs_mb_srvr_ctx)
    {
        DIY_LOG(LOG_ERROR, "%smodbus_new_tcp error: %d: %s.\n",
                gp_mb_server_log_header, errno, modbus_strerror(errno));
        return MB_SERVER_EXIT_INIT_FAIL;
    }

    if(0 != modbus_set_debug(gs_mb_srvr_ctx, srvr_params->debug_flag))
    {
        DIY_LOG(LOG_WARN, "%sset debug fail:%d: %s, ",
                gp_mb_server_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gp_mb_server_log_header);
    }

    /*
    if(0!= modbus_set_error_recovery(gs_mb_srvr_ctx,
            MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL))
    {
        DIY_LOG(LOG_WARN, "%sset error recovery mode fail:%d: %s, ",
                gp_mb_server_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gp_mb_server_log_header);
    }
    */

    if(0 != modbus_set_response_timeout(gs_mb_srvr_ctx, 0, resp_timeout_ms * 1000))
    {
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus set response time out (%d s, %d ms) fail:%d: %s\n",
                gp_mb_server_log_header, 0, resp_timeout_ms, errno, modbus_strerror(errno));
        return false;
    }

    gs_mb_header_len = modbus_get_header_length(gs_mb_srvr_ctx);
    if(gs_mb_header_len < 0)
    {
        DIY_LOG(LOG_ERROR, "%smodbus_get_header_length error: %d: %s.\n",
                gp_mb_server_log_header, errno, modbus_strerror(errno));
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
        return MB_SERVER_EXIT_INIT_FAIL;
    }
    DIY_LOG(LOG_INFO, "%sheader lengthi is: %d.\n",
            gp_mb_server_log_header, gs_mb_header_len);

    gs_mb_mapping =
        modbus_mapping_new(0, 0, HV_MB_REG_END_FLAG, 0);
    if(gs_mb_mapping == NULL)
    {
        DIY_LOG(LOG_ERROR, "%sFailed to allocate the mapping:%d: %s\n",
                gp_mb_server_log_header, errno, modbus_strerror(errno));
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
        gs_mb_header_len = -1;
        return MB_SERVER_EXIT_INIT_FAIL;
    }

    gs_mb_server_socket = modbus_tcp_listen(gs_mb_srvr_ctx, NB_CONNECTION);
    if (gs_mb_server_socket == -1)
    {
        DIY_LOG(LOG_ERROR, "%sUnable to listen TCP connection: %d: %s.\n",
                gp_mb_server_log_header, errno, modbus_strerror(errno));
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;

        modbus_mapping_free(gs_mb_mapping);
        gs_mb_mapping = NULL;
        gs_mb_header_len = -1;
        return MB_SERVER_EXIT_INIT_FAIL;
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "\n\n");
    DIY_LOG(LOG_INFO, "%smodbus server listenning on %s:%d, socket fd: %d...\n",
            gp_mb_server_log_header, srvr_params->srvr_ip, srvr_params->srvr_port,
            gs_mb_server_socket);


    init_DAP_db();

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(gs_mb_server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = gs_mb_server_socket;

    init_reg_refresh();
    gs_server_ready = true;

    write_version_str_to_file();

    for (;;) 
    {
        int select_ret;

        rdset = refset;
        select_ret = select(fdmax + 1, &rdset, NULL, NULL, &timeout);
        if (-1 == select_ret)
        {
            DIY_LOG(LOG_ERROR, "%sServer select() failure.\n", gp_mb_server_log_header);
            /*leaving for caller to release resource.*/
            return MB_SERVER_EXIT_COMM_FATAL_ERROR;
        }
        if(0 == select_ret)
        {
            check_and_process_app_timers();
            continue;
        }

        timeout_updated = false;
        ex_client_active = false;

        /* Run through the existing connections looking for data to be read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++)
        {
            if (!FD_ISSET(master_socket, &rdset))
            {
                continue;
            }

            if(master_socket == gs_mb_server_socket)
            {
                /* A client is asking a new connection */
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                //newfd = accept(gs_mb_server_socket, (struct sockaddr *) &clientaddr, &addrlen);
                newfd = modbus_tcp_accept(gs_mb_srvr_ctx, &gs_mb_server_socket);
                if (newfd == -1)
                {
                    DIY_LOG(LOG_ERROR, "%smodbus_tcp_accept error.\n",
                            gp_mb_server_log_header);
                }
                else
                {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax)
                    {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    if(getpeername(newfd, (struct sockaddr*)&clientaddr, &addrlen) < 0)
                    {
                        DIY_LOG(LOG_WARN,
                                "%sNew connection coming on socket %d,"
                                " but can't obtain its addr:%d\n",
                                gp_mb_server_log_header,
                                newfd,
                                errno);
                    }
                    else
                    {
                        DIY_LOG(LOG_INFO, "%sNew connection on socket %d, %s:%u.\n",
                               gp_mb_server_log_header, 
                               newfd,
                               inet_ntoa(clientaddr.sin_addr),
                               clientaddr.sin_port);
                    }
                }
            }
            else
            {
                int get_peer_name_ret = 0;
                get_peer_name_ret = getpeername(master_socket,
                                                (struct sockaddr*)&clientaddr, &addrlen);
                modbus_set_socket(gs_mb_srvr_ctx, master_socket);
                rc = modbus_receive(gs_mb_srvr_ctx, query);
                if (rc > 0)
                {
                    addrlen = sizeof(clientaddr);
                    memset(&clientaddr, 0, sizeof(clientaddr));

                    if(get_peer_name_ret < 0)
                    {
                        DIY_LOG(LOG_WARN,
                                "%sdata received, but can't obtain its addr, errno:%d\n",
                                gp_mb_server_log_header,
                                errno);
                    }
                    else
                    {
                        DIY_LOG(LOG_INFO, "%sdata received on socket %d, %s:%u.\n",
                               gp_mb_server_log_header, 
                               master_socket,
                               inet_ntoa(clientaddr.sin_addr),
                               clientaddr.sin_port);
                    }

                    ex_client_active = true;
                    process_ret = mb_server_process_req(query, rc, NULL, server_only);
                    if(MB_RW_REG_RET_USE_SHORT_WAIT_TIME == process_ret)
                    {
                        timeout.tv_sec = 0;
                        timeout.tv_usec 
                            = (suseconds_t)(srvr_params->short_select_wait_time * 1000000);

                        timeout_updated = true;
                    }
                    else if(MB_RW_REG_RET_USE_LONG_WAIT_TIME == process_ret)
                    {
                        timeout.tv_sec = 0;
                        timeout.tv_usec 
                            = (suseconds_t)(srvr_params->long_select_wait_time * 1000000);
                        timeout_updated = true;
                    }
                }
                else if (rc == -1)
                {
                    /* This example server in ended on connection closing or any errors. */
                    DIY_LOG(LOG_ERROR, "%sConnection closed on socket %d, ",
                           gp_mb_server_log_header, master_socket);
                    if(get_peer_name_ret < 0)
                    {
                        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR,
                                "but can't obtain the remote addr, errno: %d.\n",
                                errno);
                    }
                    else
                    {
                        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "%s:%u.\n",
                               inet_ntoa(clientaddr.sin_addr),
                               clientaddr.sin_port);
                    }
                    //close(master_socket);
                    modbus_close(gs_mb_srvr_ctx);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax)
                    {
                        fdmax--;
                    }
                }
            }
        }

        if(!ex_client_active || check_time_out_of_curr_time(gs_time_point_for_local_regs_refresh,
                                                            (time_t)(srvr_params->srvr_regs_sync_period_int_s)))
        {
            process_ret = read_hv_st_from_internal();
            if((MB_RW_REG_RET_USE_SHORT_WAIT_TIME == process_ret) || MB_RW_REG_RET_ERROR == process_ret)
            {
                timeout.tv_sec = 0;
                timeout.tv_usec 
                    = (suseconds_t)(srvr_params->short_select_wait_time * 1000000);

                timeout_updated = true;
            }
            else if(MB_RW_REG_RET_USE_LONG_WAIT_TIME == process_ret)
            {
                timeout.tv_sec = 0;
                timeout.tv_usec 
                    = (suseconds_t)(srvr_params->long_select_wait_time * 1000000);
                timeout_updated = true;
            }
        }

        if(!timeout_updated)
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = (suseconds_t)(srvr_params->long_select_wait_time * 1000000);
        }

        check_and_process_app_timers();
    }

    return MB_SERVER_EXIT_UNKNOWN;
}

