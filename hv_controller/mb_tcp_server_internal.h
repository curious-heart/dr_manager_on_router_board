#ifndef _MB_TCP_SERVER_INTERNAL_H_
#define _MB_TCP_SERVER_INTERNAL_H_

#include <modbus/modbus.h>

extern const char* const gp_mb_server_log_header;

typedef enum
{
    MB_RW_REG_RET_ERROR = -1,
    MB_RW_REG_RET_NONE = 0,
    MB_RW_REG_RET_USE_SHORT_WAIT_TIME,
    MB_RW_REG_RET_USE_LONG_WAIT_TIME,
}mb_rw_reg_ret_t;

modbus_t * mb_server_get_ctx();
modbus_mapping_t * mb_server_get_mapping();
mb_rw_reg_ret_t mb_server_read_reg_sniff(uint16_t reg_addr_start, uint16_t * data_arr, uint16_t reg_cnt);
mb_rw_reg_ret_t mb_server_write_reg_sniff(uint16_t reg_addr_start, uint16_t * data_arr, uint16_t reg_cnt);

#endif
