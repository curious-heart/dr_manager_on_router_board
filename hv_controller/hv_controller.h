#ifndef HV_CONTROLLER_H
#define HV_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    const char* serialPortName;
    int32_t serialBaudRate;
    char serialParity;
    int32_t serialDataBits;
    int32_t serialStopBits;
    uint32_t timeout_ms;
    int32_t numberOfRetries;
    int32_t serverAddress;
    int debug_flag;
}mb_rtu_params_t;

typedef struct
{
    const char* srvr_ip;
    uint16_t srvr_port;

    float long_select_wait_time, short_select_wait_time; //in seconds
    int debug_flag;
}mb_server_params_t;

bool hv_controller_open(mb_rtu_params_t* rtu_params);
bool hv_controller_close();
bool hv_controller_write_single_uint16(int reg_addr, uint16_t value);
bool hv_controller_write_uint16s(int reg_addr_start, uint16_t *buf, int len);
bool hv_controller_read_uint16s(int reg_addr_start, uint16_t * buf, int len);
const char* mb_exception_string(uint32_t exception_code);

typedef enum
{
    MB_SERVER_EXIT_UNKNOWN = 0,
    MB_SERVER_EXIT_INIT_FAIL,
    MB_SERVER_EXIT_COMM_FATAL_ERROR, 
}mb_server_exit_code_t;

mb_server_exit_code_t mb_server_loop(mb_server_params_t * srvr_params, bool server_only);
void mb_server_exit();

#endif // HV_CONTROLLER_H
