#ifndef _MB_TCP_SERVER_EXT_REG_PROCESSOR_H_
#define _MB_TCP_SERVER_EXT_REG_PROCESSOR_H_

#include "hv_registers.h"
#include "mb_tcp_server.h"
#include "mb_tcp_server_internal.h"

/*Functions in this file are all belong to main thread. DO NOT call them from outside of main thread.*/
mb_rw_reg_ret_t mb_tcp_srvr_ext_reg_dose_adj_handler(uint16_t adj_val, bool server_only);
mb_rw_reg_ret_t mb_tcp_srvr_ext_reg_charger_handler(uint16_t in_out);

#endif
