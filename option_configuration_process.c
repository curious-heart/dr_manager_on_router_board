#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "version_def.h"
#include "tof_measure.h"

#include "get_opt_helper.h"
#include "option_configuration_process.h"

/*All configurable-parameters.------------------------------*/
/*modbus rtu parameters*/
static const char* const gs_def_mb_rtu_serialPortName = "/dev/ttyS1";
static char gs_mb_rtu_dev_name[MAX_OPT_STR_SIZE];
static const int32_t gs_def_mb_rtu_serialBaudRate = 9600;
static const char gs_def_mb_rtu_serialParity = 'N';
static const int32_t gs_def_mb_rtu_serialDataBits = 8;
static const int32_t gs_def_mb_rtu_serialStopBits = 1;
static const uint32_t gs_def_mb_rtu_timeout_ms = 999; //1000;
static const int32_t gs_def_mb_rtu_numberOfRetries = 3;
static const int32_t gs_def_mb_rtu_serverAddress = 1;
static const bool gs_def_mb_rtu_debug_flag = false;
/*modbus tcp server parameters*/
static const char* const gs_local_zero_ip = "0.0.0.0";
static char gs_mb_srvr_tcp_ip_str[MAX_OPT_STR_SIZE];
static const uint16_t gs_def_mb_srvr_port = 502;
static const float gs_mb_srvr_long_wait_time = 10, gs_mb_srvr_short_wait_time = 0.5;
static const bool gs_def_mb_srvr_debug_flag= false;
/*monitor (check device state) period.*/
static const float gs_dev_monitor_def_period = 3;
/*LCD*/
static const char* const gs_def_lcd_dev_name = "/dev/i2c-0";
static char gs_lcd_dev_name[MAX_OPT_STR_SIZE];
static const uint8_t gs_def_lcd_i2c_addr = 0x3C;
/*TOF*/
static const float gs_tof_measurement_def_period  = 1;
static const char* const gs_def_tof_dev_name = "/dev/i2c-0";
static char gs_tof_dev_name[MAX_OPT_STR_SIZE];
static const uint8_t gs_def_tof_i2c_addr = 0x52; //8bit addr.
/*app work mode*/
static const app_work_mode_t gs_def_app_work_mode = WORK_MODE_NORMAL;
/*app log level*/
static const uint8_t gs_def_app_log_level = LOG_INFO; //refer to logger.h.
/*------------------------------*/

cmd_line_opt_collection_t g_cmd_line_opt_collection =
{
    .rtu_params =
    {
        .serialPortName = gs_def_mb_rtu_serialPortName,
        .serialBaudRate = gs_def_mb_rtu_serialBaudRate,
        .serialParity = gs_def_mb_rtu_serialParity,
        .serialDataBits = gs_def_mb_rtu_serialDataBits,
        .serialStopBits = gs_def_mb_rtu_serialStopBits,
        .timeout_ms = gs_def_mb_rtu_timeout_ms,
        .numberOfRetries = gs_def_mb_rtu_numberOfRetries,
        .serverAddress = gs_def_mb_rtu_serverAddress,
        .debug_flag = gs_def_mb_rtu_debug_flag,
    },

    .srvr_params =
    {
        .srvr_ip = gs_local_zero_ip,
        .srvr_port = gs_def_mb_srvr_port,
        .long_select_wait_time = gs_mb_srvr_long_wait_time, //in seconds
        .short_select_wait_time = gs_mb_srvr_short_wait_time, //in seconds
        .debug_flag = gs_def_mb_srvr_debug_flag, 
    },

    .dev_monitor_th_parm = 
    {
        .sch_period = gs_dev_monitor_def_period,
    },

    .lcd_refresh_th_parm = 
    {
        .dev_name = gs_def_lcd_dev_name,
        .dev_addr = gs_def_lcd_i2c_addr,
    },

    .tof_th_parm = 
    {
        .measure_period = gs_tof_measurement_def_period,
        .dev_name = gs_def_tof_dev_name,
        .dev_addr = gs_def_tof_i2c_addr,
    },

    .work_mode = gs_def_app_work_mode,
};

static const char* const gs_opt_com_dev_str = "com_dev";
#define gs_opt_com_dev_c 'c'
static const char* const gs_opt_mb_rtu_serialBaudRate_str = "mb_rtu_serialBaudRate";
static const char* const gs_opt_mb_rtu_serialParity_str = "mb_rtu_serialParity";
static const char* const gs_opt_mb_rtu_serialDataBits_str = "mb_rtu_serialDataBits";
static const char* const gs_opt_mb_rtu_serialStopBits_str = "mb_rtu_serialStopBits";
static const char* const gs_opt_mb_rtu_timeout_ms_str = "mb_rtu_timeout_ms";
static const char* const gs_opt_mb_rtu_numberOfRetries_str = "mb_rtu_numberOfRetries";
static const char* const gs_opt_mb_rtu_serverAddress_str = "mb_rtu_serverAddress";
static const char* const gs_opt_rtu_debug_str = "mb_rtu_debug";
#define gs_opt_rtu_debug_c 'r'
static const char* const gs_opt_mb_tcp_srvr_ip_addr_str = "mb_tcp_srvr_ip_addr";
#define gs_opt_mb_tcp_srvr_ip_addr_c 'a'
static const char* const gs_opt_port_str = "mb_tcp_srvr_port";
#define gs_opt_port_c 'p'
static const char* const gs_opt_mb_server_w_long_time_str = "mb_server_long_time";
static const char* const gs_opt_mb_server_w_short_time_str = "mb_server_short_time";
static const char* const gs_opt_tcp_debug_str = "mb_tcp_debug";
#define gs_opt_tcp_debug_c 't'
static const char* const gs_opt_dev_monitor_th_period_str = "dev_monitor_peroid";
static const char* const gs_opt_lcd_dev_name_str = "lcd_dev_name";
static const char* const gs_opt_lcd_dev_addr_str = "lcd_dev_addr";
static const char* const gs_opt_tof_measure_period_str = "tof_measure_period";
static const char* const gs_opt_tof_dev_name_str = "tof_dev_name";
static const char* const gs_opt_tof_dev_addr_str = "tof_dev_addr";
static const char* const gs_opt_rtu_master_only_str = "rtu_master_only";
#define gs_opt_rtu_master_only_c 'm'
static const char* const gs_opt_tcp_server_only_str = "tcp_server_only";
#define gs_opt_tcp_server_only_c 's'
static const char* const gs_opt_app_log_level_str = "app_log_level";
static const char* const gs_opt_help_str = "help";
#define gs_opt_help_c 'h'
static const char* const gs_opt_version_str = "version";

#undef APP_CMD_OPT_ITEM
#define APP_CMD_OPT_ITEM(long_o_s, has_arg, flag, val) {long_o_s, has_arg, flag, val},
#undef APP_CMD_OPT_VALUE
#define APP_CMD_OPT_VALUE(desc, def_val_ptr, work_val_ptr, val_type)
#define APP_CMD_OPT_ARRAY \
{\
    APP_CMD_OPT_ITEM(gs_opt_com_dev_str, required_argument, 0, gs_opt_com_dev_c) \
    APP_CMD_OPT_VALUE("modbus_com", \
            &gs_def_mb_rtu_serialPortName, &g_cmd_line_opt_collection.rtu_params.serialPortName, c_charp)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialBaudRate_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_baudrate",\
            &gs_def_mb_rtu_serialBaudRate, &g_cmd_line_opt_collection.rtu_params.serialBaudRate, int32_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialParity_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_parity",\
            &gs_def_mb_rtu_serialParity, &g_cmd_line_opt_collection.rtu_params.serialParity, char)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialDataBits_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_databits_num",\
            &gs_def_mb_rtu_serialDataBits, &g_cmd_line_opt_collection.rtu_params.serialDataBits, int32_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialStopBits_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_stopbits",\
            &gs_def_mb_rtu_serialStopBits, &g_cmd_line_opt_collection.rtu_params.serialStopBits, int32_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_timeout_ms_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_timeout_ms",\
            &gs_def_mb_rtu_timeout_ms, &g_cmd_line_opt_collection.rtu_params.timeout_ms, uint32_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_numberOfRetries_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_retry_times",\
            &gs_def_mb_rtu_numberOfRetries, &g_cmd_line_opt_collection.rtu_params.numberOfRetries, int32_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serverAddress_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("modbus_com_server_addr", \
            &gs_def_mb_rtu_serverAddress, &g_cmd_line_opt_collection.rtu_params.serverAddress, int32_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_rtu_debug_str, no_argument, 0, gs_opt_rtu_debug_c) \
    APP_CMD_OPT_VALUE("mb_rtu_debug", \
            &gs_def_mb_rtu_debug_flag, &g_cmd_line_opt_collection.rtu_params.debug_flag, int) \
\
    APP_CMD_OPT_ITEM(gs_opt_mb_tcp_srvr_ip_addr_str, required_argument, 0, gs_opt_mb_tcp_srvr_ip_addr_c)\
    APP_CMD_OPT_VALUE("mb_srvr_ip", \
            &gs_local_zero_ip, &g_cmd_line_opt_collection.srvr_params.srvr_ip, c_charp)\
\
    APP_CMD_OPT_ITEM(gs_opt_port_str, required_argument, 0, gs_opt_port_c) \
    APP_CMD_OPT_VALUE("mb_srvr_port",\
           &gs_def_mb_srvr_port, &g_cmd_line_opt_collection.srvr_params.srvr_port, uint16_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_server_w_long_time_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("mb_srvr_wait_time_long(s)",\
            &gs_mb_srvr_long_wait_time, &g_cmd_line_opt_collection.srvr_params.long_select_wait_time, float) \
\
    APP_CMD_OPT_ITEM(gs_opt_mb_server_w_short_time_str, required_argument, 0, 0) \
    APP_CMD_OPT_VALUE("mb_srvr_wait_time_short(s)", \
            &gs_mb_srvr_short_wait_time, &g_cmd_line_opt_collection.srvr_params.short_select_wait_time, float) \
\
    APP_CMD_OPT_ITEM(gs_opt_tcp_debug_str, no_argument, 0, gs_opt_tcp_debug_c) \
    APP_CMD_OPT_VALUE("mb_tcp_debug",\
            &gs_def_mb_srvr_debug_flag, &g_cmd_line_opt_collection.srvr_params.debug_flag, int) \
\
    APP_CMD_OPT_ITEM(gs_opt_dev_monitor_th_period_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("dev_st_monitor_period(s)",\
            &gs_dev_monitor_def_period, &g_cmd_line_opt_collection.dev_monitor_th_parm.sch_period, float) \
\
    APP_CMD_OPT_ITEM(gs_opt_lcd_dev_name_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("lcd_device_name",\
            &gs_def_lcd_dev_name, &g_cmd_line_opt_collection.lcd_refresh_th_parm.dev_name, c_charp) \
\
    APP_CMD_OPT_ITEM(gs_opt_lcd_dev_addr_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("lcd_device_addr(0x..)", \
            &gs_def_lcd_i2c_addr, &g_cmd_line_opt_collection.lcd_refresh_th_parm.dev_addr, uint8_t) \
\
    APP_CMD_OPT_ITEM(gs_opt_tof_measure_period_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("tof_measure_period(s)",\
            &gs_tof_measurement_def_period, &g_cmd_line_opt_collection.tof_th_parm.measure_period, float) \
\
    APP_CMD_OPT_ITEM(gs_opt_tof_dev_name_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("tof_device_name",\
            &gs_def_tof_dev_name, &g_cmd_line_opt_collection.tof_th_parm.dev_name, c_charp) \
\
    APP_CMD_OPT_ITEM(gs_opt_tof_dev_addr_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("tof_device_addr(0x..)",\
            &gs_def_tof_i2c_addr, &g_cmd_line_opt_collection.tof_th_parm.dev_addr, uint8_t) \
\
    APP_CMD_OPT_ITEM(gs_opt_rtu_master_only_str, no_argument, 0, gs_opt_rtu_master_only_c) \
    APP_CMD_OPT_VALUE("mb_rtu_master_only_test",\
           &gs_def_app_work_mode, &g_cmd_line_opt_collection.work_mode, uint8_t) \
\
    APP_CMD_OPT_ITEM(gs_opt_tcp_server_only_str, no_argument, 0, gs_opt_tcp_server_only_c) \
    APP_CMD_OPT_VALUE("mb_tcp_server_only_test", NULL, NULL, uint8_t) \
\
    APP_CMD_OPT_ITEM(gs_opt_app_log_level_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("app_log_level", \
           &gs_def_app_log_level, &g_APP_LOG_LEVEL, uint8_t) \
\
    APP_CMD_OPT_ITEM(gs_opt_help_str, no_argument, 0, gs_opt_help_c) \
    APP_CMD_OPT_VALUE("help", NULL, NULL, int) \
\
    APP_CMD_OPT_ITEM(gs_opt_version_str, no_argument, 0, 0) \
    APP_CMD_OPT_VALUE("version", NULL, NULL, int) \
\
    APP_CMD_OPT_ITEM(0, 0, 0, 0)\
}

static struct option gs_long_opt_arr[] = APP_CMD_OPT_ARRAY;

#undef APP_CMD_OPT_ITEM
#define APP_CMD_OPT_ITEM(long_o_s, has_arg, flag, val)
#undef APP_CMD_OPT_VALUE
#define APP_CMD_OPT_VALUE(desc, def_val_ptr, work_val_ptr, val_type) {desc, def_val_ptr, work_val_ptr, type_##val_type},
static cmd_opt_desc_val_t gs_cmd_opt_desc_val[] = APP_CMD_OPT_ARRAY;

/*This function does not check the buffer for security. The caller should guarantee the buffer size.*/
static void construct_short_opt_chars_str(char* buf)
{
    int i, pos;
    for(i = 0, pos = 0; i < ARRAY_ITEM_CNT(gs_long_opt_arr) - 1; ++i)
    {
        if(gs_long_opt_arr[i].val != 0)
        {
            buf[pos++] = (char)gs_long_opt_arr[i].val;
            if(required_argument == gs_long_opt_arr[i].has_arg)
            {
                buf[pos++] = ':';
            }
            else if(optional_argument == gs_long_opt_arr[i].has_arg)
            {
                buf[pos++] = ':';
                buf[pos++] = ':';
            }
        }
    }
    buf[pos++] = 0;

    DIY_LOG(LOG_DEBUG, "short option chars string: %s\n", buf);
}

typedef enum
{
    DEFAULT_PARAMS,
    WORKING_PARAMS,
}default_or_working_params_ind_t;
static void print_parameters(default_or_working_params_ind_t ind)
{
    int i;
    const void* ptr = NULL;

    if(DEFAULT_PARAMS == ind)
    {
        printf("The defaul arguments:\n");
    }
    else
    {
        printf("The working arguments:\n");
    }

    for(i = 0; i < ARRAY_ITEM_CNT(gs_long_opt_arr) - 1; ++i)
    {
        if(DEFAULT_PARAMS == ind)
        {
            ptr = gs_cmd_opt_desc_val[i].def_val_ptr;
        }
        else
        {
            ptr = gs_cmd_opt_desc_val[i].work_val_ptr;
        }
        if(!ptr)
        {
            continue;
        }

        if(gs_long_opt_arr[i].name && required_argument == gs_long_opt_arr[i].has_arg)
        {
            printf("--%s: ", gs_long_opt_arr[i].name);
            switch(gs_cmd_opt_desc_val[i].type_ind)
            {
                case type_c_charp:
                case type_charp:
                    printf("%s", *(const char**)ptr);
                    break;
                case type_char:
                    printf("%c", *(const char*)ptr);
                    break;
                case type_short:
                    printf("%d", *(const short*)ptr);
                    break;
                case type_int:
                    printf("%d", *(const int*)ptr);
                    break;
                case type_int8_t:
                    printf("%d", *(const int8_t*)ptr);
                    break;
                case type_int16_t:
                    printf("%d", *(const int16_t*)ptr);
                    break;
                case type_int32_t:
                    printf("%d", *(const int32_t*)ptr);
                    break;
                case type_uint8_t:
                    printf("%u", *(const uint8_t*)ptr);
                    break;
                case type_uint16_t:
                    printf("%u", *(const uint16_t*)ptr);
                    break;
                case type_uint32_t:
                    printf("%u", *(const uint32_t*)ptr);
                    break;
                case type_float:
                    printf("%f", *(const float*)ptr);
                    break;
                case type_double:
                    printf("%f", *(const double*)ptr);
                    break;

                default:
                    ;
            }
            printf("\n");
        }
    }
}

static void print_usage()
{
    int i;
    printf("\nusage:\ndr_manager ");
    for(i = 0; i < ARRAY_ITEM_CNT(gs_long_opt_arr) - 1; ++i)
    {
        printf("[");
        if(gs_long_opt_arr[i].name) printf("--%s", gs_long_opt_arr[i].name);
        if(gs_long_opt_arr[i].val) printf("|-%c", (char)gs_long_opt_arr[i].val);
        if(required_argument == gs_long_opt_arr[i].has_arg && gs_cmd_opt_desc_val[i].desc)
        {
            printf(" %s", gs_cmd_opt_desc_val[i].desc);
        }
        printf("] ");
    }
    printf("\n");

    print_parameters(DEFAULT_PARAMS);
}

option_process_ret_t process_cmd_options(int argc, char *argv[])
{
    char short_opt_chars[3 * ARRAY_ITEM_CNT(gs_long_opt_arr) + 1];
    int opt_c;
    int longindex;
    struct in_addr srvr_ip_in_addr;
    bool options_valid = true;

    construct_short_opt_chars_str(short_opt_chars);
    while((opt_c = getopt_long(argc, argv, short_opt_chars, gs_long_opt_arr, &longindex)) >= 0)
    {
        switch(opt_c)
        {
            case gs_opt_com_dev_c:
                OPT_CHECK_AND_DRAW(gs_opt_com_dev_str, gs_opt_com_dev_str,
                        {CONVERT_FUNC_STRCPY(gs_mb_rtu_dev_name, optarg); \
                        g_cmd_line_opt_collection.rtu_params.serialPortName = gs_mb_rtu_dev_name;},
                        SHOULD_BE_NON_NULL_STR(g_cmd_line_opt_collection.rtu_params.serialPortName),
                        SHOULD_BE_NON_NULL_STR_LOG,
                        type_c_charp);
                break;

            case gs_opt_mb_tcp_srvr_ip_addr_c:
                OPT_CHECK_AND_DRAW(gs_opt_mb_tcp_srvr_ip_addr_str, gs_opt_mb_tcp_srvr_ip_addr_str,
                        {CONVERT_FUNC_STRCPY(gs_mb_srvr_tcp_ip_str, optarg);\
                        g_cmd_line_opt_collection.srvr_params.srvr_ip = gs_mb_srvr_tcp_ip_str;},
                        VALID_IP_STR(optarg, &srvr_ip_in_addr), INVALID_IP_STR_LOG(optarg),
                        type_c_charp);
                break;

            case gs_opt_port_c:
                OPT_CHECK_AND_DRAW(gs_opt_port_str, gs_opt_port_str,
                        CONVERT_FUNC_ATOUINT16(g_cmd_line_opt_collection.srvr_params.srvr_port, optarg),
                        SHOULD_BE_NE_0(g_cmd_line_opt_collection.srvr_params.srvr_port), SHOULD_BE_NE_0_LOG,
                        type_uint16_t);
                break;

            case gs_opt_rtu_debug_c: 
                g_cmd_line_opt_collection.rtu_params.debug_flag = true;
                break;

            case gs_opt_tcp_debug_c: 
                g_cmd_line_opt_collection.srvr_params.debug_flag = true;
                break;

            case gs_opt_rtu_master_only_c:
                g_cmd_line_opt_collection.work_mode = WORK_MODE_RTU_MASTER_ONLY;
                break;

            case gs_opt_tcp_server_only_c:
                g_cmd_line_opt_collection.work_mode = WORK_MODE_TCP_SERVER_ONLY;
                break;

            case gs_opt_help_c: 
                print_usage();
                return OPTION_PROCESS_EXIT_NORMAL;

            case 0:
                if(!strcmp(gs_long_opt_arr[longindex].name, gs_opt_version_str))
                {
                    printf("%s\n", g_APP_VER_STR);
                    return OPTION_PROCESS_EXIT_NORMAL;
                }
                {
                    uint8_t p_lvl;
                    OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_app_log_level_str,
                            {CONVERT_FUNC_ATOUINT8(p_lvl, optarg); \
                            if(SHOULD_BE_IN_INCLUDED(p_lvl, LOG_DEBUG, LOG_ERROR)) g_APP_LOG_LEVEL = (enum LOG_LEVEL)p_lvl;},
                            SHOULD_BE_IN_INCLUDED(p_lvl, LOG_DEBUG, LOG_ERROR),
                            SHOULD_BE_IN_INCLUDED_INT_LOG(LOG_DEBUG, LOG_ERROR),
                            type_uint8_t);
                }
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_dev_monitor_th_period_str,
                        CONVERT_FUNC_ATOF(g_cmd_line_opt_collection.dev_monitor_th_parm.sch_period, optarg),
                        SHOULD_BE_GT_0(g_cmd_line_opt_collection.dev_monitor_th_parm.sch_period), SHOULD_BE_GT_0_LOG, 
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_server_w_long_time_str,
                        CONVERT_FUNC_ATOF(g_cmd_line_opt_collection.srvr_params.long_select_wait_time, optarg),
                        SHOULD_BE_GT_0(g_cmd_line_opt_collection.srvr_params.long_select_wait_time), SHOULD_BE_GT_0_LOG, 
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_server_w_short_time_str,
                        CONVERT_FUNC_ATOF(g_cmd_line_opt_collection.srvr_params.short_select_wait_time, optarg),
                        SHOULD_BE_GT_0(g_cmd_line_opt_collection.srvr_params.short_select_wait_time), SHOULD_BE_GT_0_LOG, 
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_tof_measure_period_str,
                        CONVERT_FUNC_ATOF(g_cmd_line_opt_collection.tof_th_parm.measure_period, optarg),
                        SHOULD_BE_IN_INCLUDED(g_cmd_line_opt_collection.tof_th_parm.measure_period,\
                            TOF_CONTI_MEAS_MIN_INTERVAL, TOF_CONTI_MEAS_MAX_INTERVAL),
                        SHOULD_BE_IN_INCLUDED_FLOAT_LOG(TOF_CONTI_MEAS_MIN_INTERVAL, TOF_CONTI_MEAS_MAX_INTERVAL),
                        type_float);
                OPT_CHECK_AND_DRAW(gs_opt_com_dev_str, gs_opt_tof_dev_name_str,
                        {CONVERT_FUNC_STRCPY(gs_tof_dev_name, optarg); \
                        g_cmd_line_opt_collection.tof_th_parm.dev_name = gs_tof_dev_name;},
                        SHOULD_BE_NON_NULL_STR(g_cmd_line_opt_collection.tof_th_parm.dev_name),
                        SHOULD_BE_NON_NULL_STR_LOG,
                        type_c_charp);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_tof_dev_addr_str ,
                        CONVERT_FUNC_ATOUINT8(g_cmd_line_opt_collection.tof_th_parm.dev_addr, optarg),
                        true, NULL,
                        type_uint8_t);

                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_serialBaudRate_str,
                        CONVERT_FUNC_ATOINT32(g_cmd_line_opt_collection.rtu_params.serialBaudRate, optarg),
                        SHOULD_BE_GT_0(g_cmd_line_opt_collection.rtu_params.serialBaudRate), SHOULD_BE_GT_0_LOG,
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_serialParity_str,
                        CONVERT_FUNC_ATOC(g_cmd_line_opt_collection.rtu_params.serialParity, optarg),
                        CHECK_COM_PARITY(g_cmd_line_opt_collection.rtu_params.serialParity), CHECK_COM_PARITY_LOG, 
                        type_char);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_serialDataBits_str,
                        CONVERT_FUNC_ATOINT32(g_cmd_line_opt_collection.rtu_params.serialDataBits, optarg),
                        SHOULD_BE_IN_INCLUDED(g_cmd_line_opt_collection.rtu_params.serialDataBits\
                            , MIN_SERIAL_DATA_BITS, MAX_SERIAL_DATA_BITS),
                        SHOULD_BE_IN_INCLUDED_INT_LOG(MIN_SERIAL_DATA_BITS, MAX_SERIAL_DATA_BITS),
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_serialStopBits_str,
                        CONVERT_FUNC_ATOINT32(g_cmd_line_opt_collection.rtu_params.serialStopBits, optarg),
                        SHOULD_BE_IN_INCLUDED(g_cmd_line_opt_collection.rtu_params.serialStopBits, \
                            MIN_SERIAL_STOP_BITS, MAX_SERIAL_STOP_BITS),
                        SHOULD_BE_IN_INCLUDED_INT_LOG(MIN_SERIAL_STOP_BITS, MAX_SERIAL_STOP_BITS),
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_timeout_ms_str,
                        CONVERT_FUNC_ATOUINT32(g_cmd_line_opt_collection.rtu_params.timeout_ms, optarg),
                        SHOULD_BE_GT_0(g_cmd_line_opt_collection.rtu_params.timeout_ms), SHOULD_BE_GT_0_LOG,
                        type_uint32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_numberOfRetries_str,
                        CONVERT_FUNC_ATOINT32(g_cmd_line_opt_collection.rtu_params.numberOfRetries, optarg),
                        SHOULD_BE_GT_0(g_cmd_line_opt_collection.rtu_params.numberOfRetries), SHOULD_BE_GT_0_LOG,
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_rtu_serverAddress_str,
                        CONVERT_FUNC_ATOINT32(g_cmd_line_opt_collection.rtu_params.serverAddress, optarg),
                        SHOULD_BE_GE_0(g_cmd_line_opt_collection.rtu_params.serverAddress), SHOULD_BE_GE_0_LOG,
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_lcd_dev_name_str,
                        {CONVERT_FUNC_STRCPY(gs_lcd_dev_name, optarg); \
                        g_cmd_line_opt_collection.lcd_refresh_th_parm.dev_name = gs_lcd_dev_name;},
                        SHOULD_BE_NON_NULL_STR(g_cmd_line_opt_collection.lcd_refresh_th_parm.dev_name),
                        SHOULD_BE_NON_NULL_STR_LOG,
                        type_c_charp);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_lcd_dev_addr_str,
                        CONVERT_FUNC_ATOUINT8(g_cmd_line_opt_collection.lcd_refresh_th_parm.dev_addr, optarg),
                        true, NULL,
                        type_uint8_t);
                break;

            default:
                options_valid = false;
        }
    }

    if(options_valid)
    {
        print_parameters(WORKING_PARAMS);
        return OPTION_PROCESS_GOON;
    }
    else
    {
        print_usage();
        return OPTION_PROCESS_EXIT_ERROR;
    } 
}
