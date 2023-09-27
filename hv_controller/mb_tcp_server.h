#ifndef _MODBUS_TCP_SERVER_H_
#define _MODBUS_TCP_SERVER_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    const char* srvr_ip;
    uint16_t srvr_port;

    float long_select_wait_time, short_select_wait_time; //in seconds
    bool debug_flag;
    bool allow_force_exposure; //true: allow exposrue even the focus-to-skin distance is less than MIN_ALLOWED_FSD_IN_CM
    float req_tof_dist_wait_time;
}mb_tcp_server_params_t;

typedef struct
{
    const char* srvr_ip;
    uint16_t srvr_port;
    bool debug_flag;
    float wait_res_timeout_sec;
}mb_tcp_client_params_t;

typedef enum
{
    MB_SERVER_EXIT_UNKNOWN = 0,
    MB_SERVER_EXIT_INIT_FAIL,
    MB_SERVER_EXIT_COMM_FATAL_ERROR, 
}mb_server_exit_code_t;

mb_server_exit_code_t mb_server_loop(mb_tcp_server_params_t * srvr_params, bool server_only);
void mb_server_exit();

#define MB_REQ_MSG_SINGLE_VALUE_OFFSET_AFTER_HDR 3
#define MB_REQ_MSG_MULTI_VALUE_OFFSET_AFTER_HDR 6

#endif
