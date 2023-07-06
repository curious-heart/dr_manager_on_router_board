#ifndef _OPTION_CONFIGURATION_PROCESS_H_
#define _OPTION_CONFIGURATION_PROCESS_H_

#include "hv_controller.h"
#include "mb_tcp_server.h"
#include "dr_manager.h"
#include "get_opt_helper.h"

typedef enum
{
    WORK_MODE_NORMAL = 0,
    WORK_MODE_RTU_MASTER_ONLY,
    WORK_MODE_TCP_SERVER_ONLY,

    WORK_MODE_MAX = 0xFF,
}app_work_mode_t;

typedef struct
{
    mb_rtu_params_t rtu_params;
    mb_tcp_server_params_t srvr_params;

    dev_monitor_th_parm_t dev_monitor_th_parm; 
    lcd_refresh_th_parm_t lcd_refresh_th_parm; 
    tof_thread_parm_t tof_th_parm;

    app_work_mode_t work_mode;
}cmd_line_opt_collection_t;


option_process_ret_t process_cmd_options(int argc, char *argv[]);

#endif
