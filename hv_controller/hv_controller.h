#ifndef HV_CONTROLLER_H
#define HV_CONTROLLER_H

#include <stdbool.h>
#include <stdint.h>

bool hv_controller_open(const char* dev_name, bool debug_flag);
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

mb_server_exit_code_t mb_server_loop(const char* srv_ip, uint16_t srv_port, bool debug_flag);
void mb_server_exit();

#endif // HV_CONTROLLER_H
