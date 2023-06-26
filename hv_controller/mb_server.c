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

#include "logger.h"
#include "hv_registers.h"
#include "hv_controller.h"
#include "dr_manager.h"
#include "common_tools.h"

/**/
#define NB_CONNECTION 2

static modbus_t *gs_mb_srvr_ctx = NULL;
static modbus_mapping_t *gs_mb_mapping;
static int gs_mb_server_socket = -1;
static int gs_mb_header_len = -1;
static const char* gs_mb_server_log_header = "modbus server: ";

static time_t gs_time_point_for_conn_check;

/*This global static var should only be accessed from main loop thread.*/
static dr_hv_st_t gs_hv_st;

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
}

const char* mb_exception_string(uint32_t exception_code)
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

static bool mb_server_check_func_reg_cnt(uint8_t * req_msg, 
        uint8_t * func, uint16_t * addr, uint16_t * cnt, uint32_t * exception_code)
{
    uint8_t func_code;
    uint16_t reg_addr_start, reg_cnt;
    bool ret = false;

    if(!req_msg)
    {
        if(exception_code) *exception_code = MODBUS_EXCEPTION_NOT_DEFINED;
        return false;
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
            if(reg_addr_start + reg_cnt < MAX_HV_MB_REG_NUM)
            {
                if(exception_code) *exception_code = 0;
                ret = true;
            }
            else
            {
                if(exception_code) *exception_code = MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;
                ret = false;
            }
        }
        break;

        default:
            if(exception_code) *exception_code = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
            ret = false;
            break;
    }
    return ret;
}

/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function update_device_st_pool.
 */
static void update_dev_st_pool_from_main_loop_th(void* d)
{
    dr_hv_st_t * hv_st = (dr_hv_st_t*)d;

    g_device_st_pool.hv_st.hv_dsp_conn_st = hv_st->hv_dsp_conn_st;
    g_device_st_pool.hv_st.expo_volt_kv = hv_st->expo_volt_kv;
    g_device_st_pool.hv_st.expo_dura_ms = hv_st->expo_dura_ms;
    g_device_st_pool.hv_st.expo_am_ua = hv_st->expo_am_ua;
    g_device_st_pool.hv_st.expo_st = hv_st->expo_st;
}
/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function update_lcd_display.
 */
static void updata_lcd_from_main_loop_th(void* arg)
{
    /* TO BE COMPLETED.*/
    /* use gs_hv_st(the arg) to update lcd display.*/
}

typedef enum
{
    MB_RW_REG_RET_ERROR = -1,
    MB_RW_REG_RET_NONE = 0,
    MB_RW_REG_RET_USE_SHORT_WAIT_TIME,
    MB_RW_REG_RET_USE_LONG_WAIT_TIME,
}mb_rw_reg_ret_t;

static mb_rw_reg_ret_t mb_server_write_reg_sniff(uint16_t reg_addr_start,
                                     uint16_t * data_arr, uint16_t reg_cnt)
{
    bool becare = false;
    uint16_t idx = 0;
    mb_rw_reg_ret_t ret = MB_RW_REG_RET_NONE; 

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
                break;

            case FilamentSet:     // 6,      /*6 管设置值电流 （决定灯丝电流决定管电流）*/
                gs_hv_st.expo_am_ua = data_arr[idx];
                becare = true;
                break;

            case ExposureTime:     // 7,     /*曝光时间*/
                gs_hv_st.expo_dura_ms = data_arr[idx];
                becare = true;
                break;

            case ExposureStart:     // 13,   /*曝光启动*/
                gs_hv_st.expo_st = EXPOSURE_ST_ALARM; 
                becare = true;
                ret = MB_RW_REG_RET_USE_SHORT_WAIT_TIME;
                break;

            case RangeIndicationStatus:
                /*TO BE COMPLEMENTED: start/stop TOF thread.*/
                break;

            default:
                ;
        }
    }
    if(becare)
    {
        update_device_st_pool(pthread_self(), 
                update_dev_st_pool_from_main_loop_th, &gs_hv_st);
        update_lcd_display(pthread_self(), updata_lcd_from_main_loop_th, &gs_hv_st);
    }

    gs_time_point_for_conn_check = time(NULL);
    return ret;
}

static mb_rw_reg_ret_t mb_server_read_reg_sniff(uint16_t reg_addr_start,
                                     uint16_t * data_arr, uint16_t reg_cnt)
{
    bool becare = false;
    uint16_t idx = 0;
    mb_rw_reg_ret_t ret = MB_RW_REG_RET_NONE; 

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

            default:
                break;
        }
    }
    if(becare)
    {
        update_device_st_pool(pthread_self(), 
                update_dev_st_pool_from_main_loop_th, &gs_hv_st);
        update_lcd_display(pthread_self(), updata_lcd_from_main_loop_th, &gs_hv_st);
    }

    gs_time_point_for_conn_check = time(NULL);
    return ret;
}

static mb_rw_reg_ret_t read_hv_st_from_internal(float timeout_sec)
{
    mb_rw_reg_ret_t process_ret = MB_RW_REG_RET_ERROR; 
    bool becare = false;

    if(hv_controller_read_uint16s(ExposureStatus,
                &gs_mb_mapping->tab_registers[ExposureStatus], 1))
    {
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
        else
        {
            process_ret = MB_RW_REG_RET_NONE;
        }
        gs_time_point_for_conn_check = time(NULL);
    }
    else
    {
        DIY_LOG(LOG_ERROR, "%sread hv state from internall error.\n", gs_mb_server_log_header);
    }

    if(process_ret != MB_RW_REG_RET_ERROR)
    {
        if(hv_controller_read_uint16s(BatteryLevel,
                &gs_mb_mapping->tab_registers[BatteryLevel], 1))
        {
            if(gs_hv_st.bat_lvl != gs_mb_mapping->tab_registers[BatteryLevel])
            {
                becare = true;
                gs_hv_st.bat_lvl = gs_mb_mapping->tab_registers[BatteryLevel];
            }

        }
        else
        {
            DIY_LOG(LOG_ERROR, "%sread battery level from internall error.\n",
                    gs_mb_server_log_header);
        }
    }

    if(process_ret == MB_RW_REG_RET_ERROR)
    {
        if(check_time_out_of_curr_time(gs_time_point_for_conn_check, (time_t)timeout_sec))
        {
            gs_hv_st.hv_dsp_conn_st = HV_DSP_DISCONNECTED;
            becare = true;

            gs_time_point_for_conn_check = time(NULL);
        }
    }

    if(becare)
    {
        update_device_st_pool(pthread_self(), 
                update_dev_st_pool_from_main_loop_th, &gs_hv_st);
        update_lcd_display(pthread_self(), updata_lcd_from_main_loop_th, &gs_hv_st);
    }

    return process_ret;
}

static mb_rw_reg_ret_t mb_server_process_req(uint8_t * req_msg, int req_msg_len,
                                             bool server_only)
{
    mb_rw_reg_ret_t process_ret = MB_RW_REG_RET_NONE; 
    bool check_ret;
    uint8_t func_code = 255;
    uint16_t reg_addr_start = 0, reg_cnt = 0;
    uint32_t exception_code;
    int ret_len;

    if(NULL == req_msg)
    {
        DIY_LOG(LOG_ERROR, "%smodbus server: receive NULL msg.\n",
                gs_mb_server_log_header );
        return MB_RW_REG_RET_ERROR;
    }

    check_ret = mb_server_check_func_reg_cnt(req_msg, 
                                      &func_code, &reg_addr_start, &reg_cnt, &exception_code);
    DIY_LOG(LOG_INFO, "%sfunc_code:%u.\n", gs_mb_server_log_header, func_code); 
    switch(func_code)
    {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "%sreg_reg_start:%d, reg_cnt:%d.\n",
                    gs_mb_server_log_header, reg_addr_start, reg_cnt); 
            break;

        default:
            ;
    }

    if(!check_ret)
    {
        DIY_LOG(LOG_ERROR, "%sinvalid req, %u:%s\n", gs_mb_server_log_header, 
                            exception_code,
                            mb_exception_string(exception_code));
        modbus_reply_exception(gs_mb_srvr_ctx, req_msg, exception_code);
        return MB_RW_REG_RET_ERROR;
    }

    if(server_only)
    {
        return process_ret;
    }

    switch(func_code)
    {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        {
            if(hv_controller_read_uint16s(reg_addr_start,
                        &gs_mb_mapping->tab_registers[reg_addr_start],
                        reg_cnt))
            {
               ret_len = modbus_reply(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
               if(ret_len <= 0)
               {
                    DIY_LOG(LOG_ERROR, "%sreply error with ret_len %d, %d:%s.\n",
                        gs_mb_server_log_header, ret_len, errno, modbus_strerror(errno));
                    return MB_RW_REG_RET_ERROR ;
               }
               process_ret = mb_server_read_reg_sniff(reg_addr_start,
                                        &gs_mb_mapping->tab_registers[reg_addr_start],
                                        reg_cnt);

            }
            else
            {
                DIY_LOG(LOG_ERROR, "%sread registers from hv_controller error.\n",
                                    gs_mb_server_log_header);
                modbus_reply_exception(gs_mb_srvr_ctx, req_msg, 
                                        MODBUS_EXCEPTION_NOT_DEFINED);
                return MB_RW_REG_RET_ERROR ;
            }
        }
        break;

        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        {
            uint16_t write_data;
            write_data = MODBUS_GET_INT16_FROM_INT8(req_msg, gs_mb_header_len + 3);
            gs_mb_mapping->tab_registers[reg_addr_start] = write_data;
            if(hv_controller_write_single_uint16(reg_addr_start, write_data))
            {
                ret_len = modbus_reply(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
                if(ret_len <= 0)
                {
                     DIY_LOG(LOG_ERROR, "%sreply error with ret_len %d, %d:%s.\n",
                         gs_mb_server_log_header, ret_len, errno, modbus_strerror(errno));
                     return MB_RW_REG_RET_ERROR;
                }
                process_ret = mb_server_write_reg_sniff(reg_addr_start,
                                        &gs_mb_mapping->tab_registers[reg_addr_start], 1);
            }
            else
            {
                DIY_LOG(LOG_ERROR, "%swrite registers to hv_controller error.\n",
                                    gs_mb_server_log_header);
                modbus_reply_exception(gs_mb_srvr_ctx, req_msg, 
                                        MODBUS_EXCEPTION_NOT_DEFINED);
                return MB_RW_REG_RET_ERROR;
            }
        }
        break;

        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            uint16_t idx;
            for(idx = 0; idx < reg_cnt; ++idx)
            {
                gs_mb_mapping->tab_registers[idx]
                    = MODBUS_GET_INT16_FROM_INT8(req_msg, gs_mb_header_len + 6 + 2 * idx);
            }
            if(hv_controller_write_uint16s(reg_addr_start,
                        &gs_mb_mapping->tab_registers[reg_addr_start],
                        reg_cnt))
            {
               ret_len = modbus_reply(gs_mb_srvr_ctx, req_msg, req_msg_len, gs_mb_mapping);
               if(ret_len <= 0)
               {
                    DIY_LOG(LOG_ERROR, "%sreply error with ret_len %d, %d:%s.\n",
                        gs_mb_server_log_header, ret_len, errno, modbus_strerror(errno));
                    return MB_RW_REG_RET_ERROR;
               }
               process_ret = mb_server_write_reg_sniff(reg_addr_start,
                                    &gs_mb_mapping->tab_registers[reg_addr_start], reg_cnt);
            }
            else
            {
                DIY_LOG(LOG_ERROR, "%swrite multi-registers to hv_controller error.\n",
                                    gs_mb_server_log_header);
                modbus_reply_exception(gs_mb_srvr_ctx, req_msg, 
                                        MODBUS_EXCEPTION_NOT_DEFINED);
                return MB_RW_REG_RET_ERROR;
            }
        }
        break;

        default:
            DIY_LOG(LOG_WARN, "%sthe function %d is not supported.\n", 
                    gs_mb_server_log_header, func_code);
            modbus_reply_exception(gs_mb_srvr_ctx, req_msg, 
                    MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
            return MB_RW_REG_RET_ERROR;
    }

    return process_ret;
}

mb_server_exit_code_t  mb_server_loop(mb_server_params_t * srvr_params, bool debug_flag, bool server_only)
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
        DIY_LOG(LOG_ERROR, "%sserver parameters pointer is NULL.\n", gs_mb_server_log_header);
        return MB_SERVER_EXIT_INIT_FAIL;
    }

    timeout.tv_sec = 0;
    timeout.tv_usec = (suseconds_t)(srvr_params->long_select_wait_time * 1000000);

    gs_mb_srvr_ctx = modbus_new_tcp(srvr_params->srvr_ip, srvr_params->srvr_port);
    if(NULL == gs_mb_srvr_ctx)
    {
        DIY_LOG(LOG_ERROR, "%smodbus_new_tcp error: %d: %s.\n",
                gs_mb_server_log_header, errno, modbus_strerror(errno));
        return MB_SERVER_EXIT_INIT_FAIL;
    }

    if(0 != modbus_set_debug(gs_mb_srvr_ctx, debug_flag))
    {
        DIY_LOG(LOG_WARN, "%sset debug fail:%d: %s, ",
                gs_mb_server_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gs_mb_server_log_header);
    }

    /*
    if(0!= modbus_set_error_recovery(gs_mb_srvr_ctx,
            MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL))
    {
        DIY_LOG(LOG_WARN, "%sset error recovery mode fail:%d: %s, ",
                gs_mb_server_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gs_mb_server_log_header);
    }
    */

    if(0 != modbus_set_response_timeout(gs_mb_srvr_ctx, 0, resp_timeout_ms * 1000))
    {
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus set response time out (%d s, %d ms) fail:%d: %s\n",
                gs_mb_server_log_header, 0, resp_timeout_ms, errno, modbus_strerror(errno));
        return false;
    }

    gs_mb_header_len = modbus_get_header_length(gs_mb_srvr_ctx);
    if(gs_mb_header_len < 0)
    {
        DIY_LOG(LOG_ERROR, "%smodbus_get_header_length error: %d: %s.\n",
                gs_mb_server_log_header, errno, modbus_strerror(errno));
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
        return MB_SERVER_EXIT_INIT_FAIL;
    }
    DIY_LOG(LOG_INFO, "%sheader lengthi is: %d.\n",
            gs_mb_server_log_header, gs_mb_header_len);

    gs_mb_mapping =
        modbus_mapping_new(0, 0, MAX_HV_MB_REG_NUM, 0);
    if(gs_mb_mapping == NULL)
    {
        DIY_LOG(LOG_ERROR, "%sFailed to allocate the mapping:%d: %s\n",
                gs_mb_server_log_header, errno, modbus_strerror(errno));
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;
        gs_mb_header_len = -1;
        return MB_SERVER_EXIT_INIT_FAIL;
    }

    gs_mb_server_socket = modbus_tcp_listen(gs_mb_srvr_ctx, NB_CONNECTION);
    if (gs_mb_server_socket == -1)
    {
        DIY_LOG(LOG_ERROR, "%sUnable to listen TCP connection: %d: %s.\n",
                gs_mb_server_log_header, errno, modbus_strerror(errno));
        modbus_free(gs_mb_srvr_ctx);
        gs_mb_srvr_ctx = NULL;

        modbus_mapping_free(gs_mb_mapping);
        gs_mb_mapping = NULL;
        gs_mb_header_len = -1;
        return MB_SERVER_EXIT_INIT_FAIL;
    }
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "\n\n");
    DIY_LOG(LOG_INFO, "%smodbus server listenning on %s:%d, socket fd: %d...\n",
            gs_mb_server_log_header, srvr_params->srvr_ip, srvr_params->srvr_port,
            gs_mb_server_socket);

    gs_time_point_for_conn_check = time(NULL);
    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(gs_mb_server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = gs_mb_server_socket;

    for (;;) 
    {
        rdset = refset;
        if (select(fdmax + 1, &rdset, NULL, NULL, &timeout) == -1)
        {
            DIY_LOG(LOG_ERROR, "%sServer select() failure.\n", gs_mb_server_log_header);
            /*leaving for caller to release resource.*/
            return MB_SERVER_EXIT_COMM_FATAL_ERROR;
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
                            gs_mb_server_log_header);
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
                                gs_mb_server_log_header,
                                newfd,
                                errno);
                    }
                    else
                    {
                        DIY_LOG(LOG_INFO, "%sNew connection on socket %d, %s:%u.\n",
                               gs_mb_server_log_header, 
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
                                gs_mb_server_log_header,
                                errno);
                    }
                    else
                    {
                        DIY_LOG(LOG_INFO, "%sdata received on socket %d, %s:%u.\n",
                               gs_mb_server_log_header, 
                               master_socket,
                               inet_ntoa(clientaddr.sin_addr),
                               clientaddr.sin_port);
                    }

                    process_ret = mb_server_process_req(query, rc, server_only);
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
                    ex_client_active = true;
                }
                else if (rc == -1)
                {
                    /* This example server in ended on connection closing or any errors. */
                    DIY_LOG(LOG_ERROR, "%sConnection closed on socket %d, ",
                           gs_mb_server_log_header, master_socket);
                    if(get_peer_name_ret < 0)
                    {
                        DIY_LOG(LOG_WARN + LOG_ONLY_INFO_STR,
                                "but can't obtain the remote addr, errno: %d.\n",
                                errno);
                    }
                    else
                    {
                        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "%s:%u.\n",
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

        if(!ex_client_active)
        {
            process_ret = read_hv_st_from_internal(srvr_params->long_select_wait_time);
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

        if(!timeout_updated)
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = (suseconds_t)(srvr_params->long_select_wait_time * 1000000);
        }
    }

    return MB_SERVER_EXIT_UNKNOWN;
}

