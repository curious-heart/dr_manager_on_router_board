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
    bool debug_flag;
}mb_rtu_params_t;


bool hv_controller_open(mb_rtu_params_t* rtu_params);
bool hv_controller_close();
bool hv_controller_write_single_uint16(int reg_addr, uint16_t value);
bool hv_controller_write_uint16s(int reg_addr_start, uint16_t *buf, int len);
bool hv_controller_read_uint16s(int reg_addr_start, uint16_t * buf, int len);
#endif // HV_CONTROLLER_H
