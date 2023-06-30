#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <string.h>

#include "version_def.h"
#include "logger.h"
#include "hv_registers.h"
#include "hv_controller.h"
#include "dr_manager.h"
#include "get_opt_helper.h"
#include "pthread_helper.h"
#include "tof_measure.h"

static const char* const gs_def_mb_rtu_serialPortName = "/dev/ttyS1";
static const int32_t gs_def_mb_rtu_serialBaudRate = 9600;
static const char gs_def_mb_rtu_serialParity = 'N';
static const int32_t gs_def_mb_rtu_serialDataBits = 8;
static const int32_t gs_def_mb_rtu_serialStopBits = 1;
static const uint32_t gs_def_mb_rtu_timeout_ms = 999; //1000;
static const int32_t gs_def_mb_rtu_numberOfRetries = 3;
static const int32_t gs_def_mb_rtu_serverAddress = 1;

static const char* const gs_local_zero_ip = "0.0.0.0";
static const uint16_t gs_def_mb_srvr_port = 502;
static const float gs_mb_srvr_long_wait_time = 10, gs_mb_srvr_short_wait_time = 0.5;

float g_tof_measure_period = TOF_MEASUREMENT_DEF_PERIOD; 
const char* g_main_thread_desc = "Main-thread";

static pthread_t gs_dev_monitor_th_id, gs_lcd_refresh_th_id;

static void mb_reg_only_write(hv_mb_reg_e_t reg_addr)
{
    uint32_t write_data;
    const char* reg_str;
    reg_str = get_hv_mb_reg_str(reg_addr);
    if(!reg_str)
    {
        return;
    }
    printf("please input the data to be written:");
    scanf("%u", &write_data);
    if(hv_controller_write_single_uint16((int)reg_addr, (uint16_t)write_data))
    {
        printf("write register data ok.\n");
    }
}

static uint16_t mb_reg_only_read(hv_mb_reg_e_t reg_addr)
{
    uint16_t read_data = 0xFFFF;
    const char* reg_str;
    reg_str = get_hv_mb_reg_str(reg_addr);
    if(!reg_str)
    {
        return read_data;
    }
    if(hv_controller_read_uint16s((int)reg_addr, &read_data, 1))
    {
        printf("read register %s ok, the value is: %d\n", reg_str, read_data);
    }
    return read_data;
}

static uint16_t mb_reg_read_write(hv_mb_reg_e_t reg_addr)
{
    char op;
    uint16_t read_data = 0xFFFF;

    op = choose_read_or_write();
    switch(op)
    {
        case 'r':
            read_data = mb_reg_only_read(reg_addr);
            break;

        case 'w':
            mb_reg_only_write(reg_addr);
            break;

        default:
            ;
    }
    return read_data;
}

static void rtu_master_test(mb_rtu_params_t* rtu_params )
{
    bool end = false;
    int test_no;
    hv_mb_reg_e_t reg_addr;
    char rw_op;

    if(!hv_controller_open(rtu_params))
    {
        return;
    }

    while(!end)
    {
        printf("\n");
        printf("HSV = 0,                   //软硬件版本\n");
        printf("OTA = 1,                   //OTA升级\n");
        printf("BaudRate = 2,              //波特率\n");
        printf("ServerAddress = 3,         //设备地址\n");
        printf("State = 4,                 //状态\n");
        printf("VoltSet = 5,               //管电压设置值\n");
        printf("FilamentSet = 6,           //管设置值电流 （决定灯丝电流决定管电流）\n");
        printf("ExposureTime = 7,          //曝光时间\n");
        printf("Voltmeter = 8,             //管电压读出值\n");
        printf("Ammeter = 9,               //管电流读出值\n");
        printf("RangeIndicationStatus = 10,//范围指示状态\n");
        printf("ExposureStatus = 11,       //曝光状态\n");
        printf("RangeIndicationStart = 12, //范围指示启动\n");
        printf("ExposureStart = 13,        //曝光启动\n");
        printf("BatteryLevel = 14,         //电池电量\n");
        printf("BatteryVoltmeter = 15,\n");
        printf("OilBoxTemperature = 16,    //电池电压高位\n");
        printf("Poweroff = 17,             //关机请求\n");
        printf("Fixpos = 18,               //校准定义\n");
        printf("Fixval = 19,               //校准值\n");
        printf("Workstatus = 20,           //充能状态\n");
        printf("exposureCount = 21,        //曝光次数\n");
        printf("-1: exit.\n");

        scanf("%d", &test_no);
        if(test_no < 0)
        {
            end = true;
            break;
        }
        if(test_no >= MAX_HV_MB_REG_NUM)
        {
            printf("Invlaid register number!\n");
            continue;
        }

        reg_addr = (hv_mb_reg_e_t)test_no;
        rw_op = get_hv_mb_reg_rw_attr(reg_addr);
        switch(rw_op)
        {
            case HV_MB_REG_RW_ATTR_R:
               mb_reg_only_read(reg_addr);
               break; 

            case HV_MB_REG_RW_ATTR_W:
               mb_reg_only_write(reg_addr);
               break; 

            case HV_MB_REG_RW_ATTR_RW:
               mb_reg_read_write(reg_addr);
               break; 

            default:
               ;
        }
        printf("\n");
    }
    hv_controller_close();
}

static void print_modbus_params(mb_rtu_params_t * rtu_params, mb_server_params_t * srvr_params)
{
    DIY_LOG(LOG_INFO, "++++++++++++++++++++\nRTU info:\nserialPortName: %s\n",
            rtu_params->serialPortName);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, 
            "serialBaudRate: %d\nserialParity: %c\nserialDataBits: %d\nserialStopBits: %d\n"
            "timeout_ms: %u\nnumberOfRetries: %d\n"
            "serverAddress: %d\n",
            rtu_params->serialBaudRate, rtu_params->serialParity,
            rtu_params->serialDataBits, rtu_params->serialStopBits,
            rtu_params->timeout_ms, rtu_params->numberOfRetries,
            rtu_params->serverAddress);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP,
            "====================\nTCP server info:\n"
            "ip address: %s\ntcp port: %u\n\t--------------------\n",
            srvr_params->srvr_ip, srvr_params->srvr_port);
}

static void get_mb_rtu_params(mb_rtu_params_t * rtu_params)
{
    if(rtu_params)
    {
        rtu_params->serialPortName = gs_def_mb_rtu_serialPortName;
        rtu_params->serialBaudRate = gs_def_mb_rtu_serialBaudRate;
        rtu_params->serialParity = gs_def_mb_rtu_serialParity;
        rtu_params->serialDataBits = gs_def_mb_rtu_serialDataBits;
        rtu_params->serialStopBits = gs_def_mb_rtu_serialStopBits;
        rtu_params->timeout_ms = gs_def_mb_rtu_timeout_ms;
        rtu_params->numberOfRetries = gs_def_mb_rtu_numberOfRetries;
        rtu_params->serverAddress = gs_def_mb_rtu_serverAddress;
        rtu_params->debug_flag = false;
    }
}

static bool clear_threads()
{
    int s;
    bool ret = true;
    void check_and_cancel_tof_th();

    cancel_assit_thread(&gs_dev_monitor_th_id);
    cancel_assit_thread(&gs_lcd_refresh_th_id);
    check_and_cancel_tof_th();

    s = destroy_dev_st_pool_mutex();
    PTHREAD_ERR_CHECK(s, "destory_dev_st_pool_mutex", "", " failes", false);
    ret = ret && s;
    s = destroy_lcd_upd_mutex();
    PTHREAD_ERR_CHECK(s, "destory_lcd_upd_mutex", "", " failes", false);
    ret = ret && s;

    return ret;
}

static void clear_for_exit()
{
    mb_server_exit();
    hv_controller_close();
    clear_threads();
}

static void close_sigint(int dummy)
{
    DIY_LOG(LOG_INFO, "SIGINT received, now exit...\n");
    clear_for_exit();
    exit(dummy);
}

static void init_thread_syncs()
{
    init_dev_st_pool_mutex();
    init_lcd_upd_mutex();
}

typedef enum
{
    WORK_MODE_NORMAL,
    WORK_MODE_RTU_MASTER_ONLY,
    WORK_MODE_TCP_SERVER_ONLY,
}work_mode_t;

static const char* const gs_opt_com_dev_str = "com_dev";
#define gs_opt_com_dev_c 'c'
static const char* const gs_opt_mb_rtu_serialBaudRate_str = "mb_rtu_serialBaudRate";
static const char* const gs_opt_mb_rtu_serialParity_str = "mb_rtu_serialParity";
static const char* const gs_opt_mb_rtu_serialDataBits_str = "mb_rtu_serialDataBits";
static const char* const gs_opt_mb_rtu_serialStopBits_str = "mb_rtu_serialStopBits";
static const char* const gs_opt_mb_rtu_timeout_ms_str = "mb_rtu_timeout_ms";
static const char* const gs_opt_mb_rtu_numberOfRetries_str = "mb_rtu_numberOfRetries";
static const char* const gs_opt_mb_rtu_serverAddress_str = "mb_rtu_serverAddress";
static const char* const gs_opt_mb_tcp_srvr_ip_addr_str = "mb_tcp_srvr_ip_addr";
#define gs_opt_mb_tcp_srvr_ip_addr_c 'a'
static const char* const gs_opt_port_str = "mb_tcp_srvr_port";
#define gs_opt_port_c 'p'
static const char* const gs_opt_rtu_debug_str = "rtu_debug";
#define gs_opt_rtu_debug_c 'r'
static const char* const gs_opt_tcp_debug_str = "tcp_debug";
#define gs_opt_tcp_debug_c 't'
static const char* const gs_opt_help_str = "help";
#define gs_opt_help_c 'h'
static const char* const gs_opt_rtu_master_only_str = "rtu_master_only";
#define gs_opt_rtu_master_only_c 'm'
static const char* const gs_opt_tcp_server_only_str = "tcp_server_only";
#define gs_opt_tcp_server_only_c 's'
static const char* const gs_opt_version_str = "version";
static const char* const gs_opt_dev_monitor_th_period_str = "dev_monitor_peroid";
static const char* const gs_opt_mb_server_w_long_time_str = "mb_server_long_time";
static const char* const gs_opt_mb_server_w_short_time_str = "mb_server_short_time";
static const char* const gs_opt_tof_measure_period_str = "tof_measure_period";
static const char* const gs_opt_app_log_level_str = "app_log_level";

 /* The "val_type" in below APP_CMD_OPT_ARRAY macro MUST 
  * be consistent with the TYPE_LIST macro in get_opt_helper.h.
  */
#undef APP_CMD_OPT_ITEM
#define APP_CMD_OPT_ITEM(long_o_s, has_arg, flag, val, desc, def_val, val_type) {long_o_s, has_arg, flag, val}
#define APP_CMD_OPT_ARRAY \
{\
    APP_CMD_OPT_ITEM(gs_opt_com_dev_str, required_argument, 0, gs_opt_com_dev_c, \
            "modbus_com", gs_def_mb_rtu_serialPortName, c_charp),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialBaudRate_str, required_argument, 0, 0,\
            "modbus_com_baudrate", gs_def_mb_rtu_serialBaudRate, int32_t),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialParity_str, required_argument, 0, 0,\
            "modbus_com_parity", gs_def_mb_rtu_serialParity, char),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialDataBits_str, required_argument, 0, 0,\
            "modbus_com_databits_num", gs_def_mb_rtu_serialDataBits, int32_t),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serialStopBits_str, required_argument, 0, 0,\
            "modbus_com_stopbits", gs_def_mb_rtu_serialStopBits, int32_t),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_timeout_ms_str, required_argument, 0, 0,\
            "modbus_com_timeout_ms", gs_def_mb_rtu_timeout_ms, uint32_t),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_numberOfRetries_str, required_argument, 0, 0,\
            "modbus_com_retry_times", gs_def_mb_rtu_numberOfRetries, int32_t),\
    APP_CMD_OPT_ITEM(gs_opt_mb_rtu_serverAddress_str, required_argument, 0, 0,\
            "modbus_com_server_addr", gs_def_mb_rtu_serverAddress, int32_t),\
    APP_CMD_OPT_ITEM(gs_opt_mb_tcp_srvr_ip_addr_str, required_argument, 0, gs_opt_mb_tcp_srvr_ip_addr_c, \
            "mb_srvr_ip", gs_local_zero_ip, c_charp),\
    APP_CMD_OPT_ITEM(gs_opt_port_str, required_argument, 0, gs_opt_port_c, \
            "mb_srvr_port", gs_def_mb_srvr_port, uint16_t),\
    APP_CMD_OPT_ITEM(gs_opt_rtu_debug_str, no_argument, 0, gs_opt_rtu_debug_c, \
            "mb_rtu_debug", 0, int), \
    APP_CMD_OPT_ITEM(gs_opt_tcp_debug_str, no_argument, 0, gs_opt_tcp_debug_c, \
            "mb_tcp_debug", 0, int), \
    APP_CMD_OPT_ITEM(gs_opt_rtu_master_only_str, no_argument, 0, gs_opt_rtu_master_only_c, \
            "mb_rtu_only_test", 0, int), \
    APP_CMD_OPT_ITEM(gs_opt_tcp_server_only_str, no_argument, 0, gs_opt_tcp_server_only_c, \
            "mb_tcp_only_test", 0, int), \
    APP_CMD_OPT_ITEM(gs_opt_help_str, no_argument, 0, gs_opt_help_c, \
            "help", 0, int), \
    APP_CMD_OPT_ITEM(gs_opt_version_str, no_argument, 0, 0, \
            "version", 0, int), \
    APP_CMD_OPT_ITEM(gs_opt_dev_monitor_th_period_str, required_argument, 0, 0,\
            "dev_st_query_period(s)", DEV_MONITOR_DEF_PERIOD, float), \
    APP_CMD_OPT_ITEM(gs_opt_mb_server_w_long_time_str, required_argument, 0, 0,\
            "mb_srvr_wait_time_long(s)", gs_mb_srvr_long_wait_time, float), \
    APP_CMD_OPT_ITEM(gs_opt_mb_server_w_short_time_str, required_argument, 0, 0, \
            "mb_srvr_wait_time_short(s)", gs_mb_srvr_short_wait_time, float), \
    APP_CMD_OPT_ITEM(gs_opt_tof_measure_period_str, required_argument, 0, 0,\
            "tof_measure_period(s)", TOF_MEASUREMENT_DEF_PERIOD, float), \
    APP_CMD_OPT_ITEM(gs_opt_app_log_level_str, required_argument, 0, 0,\
            "app_log_level", LOG_INFO, int), \
    APP_CMD_OPT_ITEM(0, 0, 0, 0, 0, 0, int),\
}
static struct option gs_long_opt_arr[] = APP_CMD_OPT_ARRAY;

#undef APP_CMD_OPT_ITEM
#define APP_CMD_OPT_ITEM(long_o_s, has_arg, flag, val, desc, def_val, val_type)\
{desc, {.val_type##_val = def_val}, type_##val_type}
static cmd_opt_desc_val_t gs_cmd_opt_desc_val[] = APP_CMD_OPT_ARRAY;
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

    printf("The defaul arguments:\n");
    for(i = 0; i < ARRAY_ITEM_CNT(gs_long_opt_arr) - 1; ++i)
    {
        if(gs_long_opt_arr[i].name && required_argument == gs_long_opt_arr[i].has_arg)
        {
            printf("--%s: ", gs_long_opt_arr[i].name);
            switch(gs_cmd_opt_desc_val[i].type_ind)
            {
                case type_c_charp:
                case type_charp:
                    printf("%s", gs_cmd_opt_desc_val[i].def_val.charp_val);
                    break;
                case type_char:
                    printf("%c", gs_cmd_opt_desc_val[i].def_val.char_val);
                    break;
                case type_short:
                    printf("%d", gs_cmd_opt_desc_val[i].def_val.short_val);
                    break;
                case type_int:
                    printf("%d", gs_cmd_opt_desc_val[i].def_val.int_val);
                    break;
                case type_int8_t:
                    printf("%u", gs_cmd_opt_desc_val[i].def_val.int8_t_val);
                    break;
                case type_int16_t:
                    printf("%u", gs_cmd_opt_desc_val[i].def_val.int16_t_val);
                    break;
                case type_int32_t:
                    printf("%u", gs_cmd_opt_desc_val[i].def_val.int32_t_val);
                    break;
                case type_uint8_t:
                    printf("%u", gs_cmd_opt_desc_val[i].def_val.uint8_t_val);
                    break;
                case type_uint16_t:
                    printf("%u", gs_cmd_opt_desc_val[i].def_val.uint16_t_val);
                    break;
                case type_uint32_t:
                    printf("%u", gs_cmd_opt_desc_val[i].def_val.uint32_t_val);
                    break;
                case type_float:
                    printf("%f", gs_cmd_opt_desc_val[i].def_val.float_val);
                    break;
                case type_double:
                    printf("%f", gs_cmd_opt_desc_val[i].def_val.double_val);
                    break;

                default:
                    ;
            }
            printf("\n");
        }
    }
}

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

#define MAX_TTY_DEV_NAME_SIZE 32 
#define MAX_IP_ADDR_STR_SIZE 17 
static char gs_mb_rtu_dev_name[MAX_TTY_DEV_NAME_SIZE];
static char gs_mb_srvr_tcp_ip_str[MAX_IP_ADDR_STR_SIZE];

int main(int argc, char *argv[])
{
    char short_opt_chars[3 * ARRAY_ITEM_CNT(gs_long_opt_arr) + 1];
    int opt_c;
    work_mode_t work_mode = WORK_MODE_NORMAL;
    bool server_only = false;
    bool arg_parse_result;
    int longindex;

    mb_rtu_params_t rtu_params;
    mb_server_params_t srvr_params
        = {gs_local_zero_ip, gs_def_mb_srvr_port, gs_mb_srvr_long_wait_time, gs_mb_srvr_short_wait_time};
    bool mb_tcp_debug_flag = false;
    struct in_addr srvr_ip_in_addr;
    mb_server_exit_code_t mb_server_ret;

    dev_monitor_th_parm_t dev_monitor_th_parm = {DEV_MONITOR_DEF_PERIOD};

    construct_short_opt_chars_str(short_opt_chars);
    get_mb_rtu_params(&rtu_params);
    arg_parse_result = true;
    while((opt_c = getopt_long(argc, argv, short_opt_chars, gs_long_opt_arr, &longindex)) >= 0)
    {
        switch(opt_c)
        {
            case gs_opt_com_dev_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    snprintf(gs_mb_rtu_dev_name, MAX_TTY_DEV_NAME_SIZE, "%s", optarg);
                    rtu_params.serialPortName = gs_mb_rtu_dev_name;
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "if option -%c or --%s are provided, modbus rtu device name is necessary.\n", 
                            gs_opt_com_dev_c, gs_opt_com_dev_str);
                    arg_parse_result = false;
                }
                break;

            case gs_opt_mb_tcp_srvr_ip_addr_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    snprintf(gs_mb_srvr_tcp_ip_str, MAX_IP_ADDR_STR_SIZE, "%s",  optarg);
                    if(0 == inet_aton(gs_mb_srvr_tcp_ip_str, &srvr_ip_in_addr))
                    {
                        DIY_LOG(LOG_ERROR,
                                "The modbus server ip address %s is invalid.\n",
                                gs_mb_srvr_tcp_ip_str);
                        arg_parse_result = false;
                    }
                    else
                    {
                        srvr_params.srvr_ip = gs_mb_srvr_tcp_ip_str;
                    }
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "if option -%c or --%s are provided, modbus server ip addr is necessary.\n", 
                            gs_opt_mb_tcp_srvr_ip_addr_c, gs_opt_mb_tcp_srvr_ip_addr_str);
                    arg_parse_result = false;
                }
                break;

            case gs_opt_port_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    srvr_params.srvr_port = (uint16_t)atoi(optarg);
                    if(0 == srvr_params.srvr_port)
                    {
                        DIY_LOG(LOG_ERROR, "The modbus server port %s is invalid.\n", optarg);
                        arg_parse_result = false;
                    }
                }
                else
                {
                    DIY_LOG(LOG_ERROR,
                            "if option -%c or --%s are provided,"
                            "modbus server port is necessary.\n", 
                            gs_opt_port_c, gs_opt_port_str);
                    arg_parse_result = false;
                }
                break;

            case gs_opt_rtu_debug_c: 
                rtu_params.debug_flag = true;
                break;

            case gs_opt_tcp_debug_c: 
                mb_tcp_debug_flag = true;
                break;

            case gs_opt_rtu_master_only_c:
                work_mode = WORK_MODE_RTU_MASTER_ONLY;
                break;

            case gs_opt_tcp_server_only_c:
                work_mode = WORK_MODE_TCP_SERVER_ONLY;
                break;

            case gs_opt_help_c: 
                print_usage();
                return 0;

            case 0:
                if(!strcmp(gs_long_opt_arr[longindex].name, gs_opt_version_str))
                {
                    printf("%s\n", APP_VER_STR);
                    return 0;
                }
                {
                    int p_lvl;
                    OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_app_log_level_str,
                            p_lvl, 
                            SHOULD_BE_IN_INCLUDED(p_lvl, LOG_DEBUG, LOG_ERROR),
                            SHOULD_BE_IN_INCLUDED_INT_LOG(LOG_DEBUG, LOG_ERROR),
                            CONVERT_FUNC_ATOI(p_lvl, optarg),
                            type_int);
                    if(arg_parse_result)
                    {
                        g_APP_LOG_LEVEL = (enum LOG_LEVEL)p_lvl;
                    }
                }
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_dev_monitor_th_period_str,
                        dev_monitor_th_parm.sch_period, 
                        SHOULD_BE_GT_0(dev_monitor_th_parm.sch_period), SHOULD_BE_GT_0_LOG, 
                        CONVERT_FUNC_ATOF(dev_monitor_th_parm.sch_period, optarg),
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_server_w_long_time_str,
                        srvr_params.long_select_wait_time, 
                        SHOULD_BE_GT_0(srvr_params.long_select_wait_time), SHOULD_BE_GT_0_LOG, 
                        CONVERT_FUNC_ATOF(srvr_params.long_select_wait_time, optarg),
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_server_w_short_time_str,
                        srvr_params.short_select_wait_time,
                        SHOULD_BE_GT_0(srvr_params.short_select_wait_time), SHOULD_BE_GT_0_LOG, 
                        CONVERT_FUNC_ATOF(srvr_params.short_select_wait_time, optarg),
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_tof_measure_period_str,
                        g_tof_measure_period, 
                        SHOULD_BE_IN_INCLUDED(g_tof_measure_period, TOF_CONTI_MEAS_MIN_INTERVAL, TOF_CONTI_MEAS_MAX_INTERVAL),
                        SHOULD_BE_IN_INCLUDED_FLOAT_LOG(TOF_CONTI_MEAS_MIN_INTERVAL, TOF_CONTI_MEAS_MAX_INTERVAL),
                        CONVERT_FUNC_ATOF(g_tof_measure_period, optarg),
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_serialBaudRate_str,
                        rtu_params.serialBaudRate,
                        SHOULD_BE_GT_0(rtu_params.serialBaudRate), SHOULD_BE_GT_0_LOG,
                        CONVERT_FUNC_ATOINT32(rtu_params.serialBaudRate, optarg),
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_serialParity_str,
                        rtu_params.serialParity,
                        CHECK_COM_PARITY(rtu_params.serialParity),
                        CHECK_COM_PARITY_LOG, 
                        CONVERT_FUNC_ATOC(rtu_params.serialParity, optarg),
                        type_char);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_serialDataBits_str,
                        rtu_params.serialDataBits,
                        SHOULD_BE_IN_INCLUDED(rtu_params.serialDataBits, MIN_SERIAL_DATA_BITS, MAX_SERIAL_DATA_BITS),
                        SHOULD_BE_IN_INCLUDED_INT_LOG(MIN_SERIAL_DATA_BITS, MAX_SERIAL_DATA_BITS),
                        CONVERT_FUNC_ATOINT32(rtu_params.serialDataBits, optarg),
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_serialStopBits_str,
                        rtu_params.serialStopBits,
                        SHOULD_BE_IN_INCLUDED(rtu_params.serialStopBits, MIN_SERIAL_STOP_BITS, MAX_SERIAL_STOP_BITS),
                        SHOULD_BE_IN_INCLUDED_INT_LOG(MIN_SERIAL_STOP_BITS, MAX_SERIAL_STOP_BITS),
                        CONVERT_FUNC_ATOINT32(rtu_params.serialStopBits, optarg),
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_timeout_ms_str,
                        rtu_params.timeout_ms,
                        SHOULD_BE_GT_0(rtu_params.timeout_ms), SHOULD_BE_GT_0_LOG,
                        CONVERT_FUNC_ATOUINT32(rtu_params.timeout_ms, optarg),
                        type_uint32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_numberOfRetries_str,
                        rtu_params.numberOfRetries,
                        SHOULD_BE_GT_0(rtu_params.numberOfRetries), SHOULD_BE_GT_0_LOG,
                        CONVERT_FUNC_ATOINT32(rtu_params.numberOfRetries, optarg),
                        type_int32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr, gs_opt_mb_rtu_serverAddress_str,
                        rtu_params.serverAddress,
                        SHOULD_BE_GE_0(rtu_params.serverAddress), SHOULD_BE_GE_0_LOG,
                        CONVERT_FUNC_ATOINT32(rtu_params.serverAddress, optarg),
                        type_int32_t);
                break;

            default:
                arg_parse_result = false;
        }
    }
    if(!arg_parse_result)
    {
        print_usage();
        return -1;
    }

    print_modbus_params(&rtu_params, &srvr_params);

    /*Init necessary mutexes or other elements for synchronization.*/
    init_thread_syncs();
    /*start other threads. ++++++++++++++++++++++++++++++*/
    if(!start_assit_thread(g_dev_monitor_th_desc, &gs_dev_monitor_th_id, 
            true, dev_monitor_thread_func, &dev_monitor_th_parm))
    {
        return -1;
    }
    if(!start_assit_thread(g_lcd_refresh_th_desc, &gs_lcd_refresh_th_id, true, lcd_refresh_thread_func, NULL))
    {
        return -1;
    }
    /*------------------------------*/

    switch(work_mode)
    {
        case WORK_MODE_RTU_MASTER_ONLY:
            rtu_master_test(&rtu_params);
            return 0;

        case WORK_MODE_TCP_SERVER_ONLY:
            server_only = true;
            break;

        default:
        ;
    }

    signal(SIGINT, close_sigint);

    if(!server_only)
    {
        if(!hv_controller_open(&rtu_params))
        {
            DIY_LOG(LOG_ERROR, "Connecting high volatage board fails.\n");
            clear_for_exit();
            return -1;;
        }
    }

    mb_server_ret = mb_server_loop(&srvr_params, mb_tcp_debug_flag, server_only);
    switch(mb_server_ret)
    {
        case MB_SERVER_EXIT_INIT_FAIL:
            DIY_LOG(LOG_ERROR, "modbus server startup fails.!\n");
            break;

        case MB_SERVER_EXIT_COMM_FATAL_ERROR: 
            DIY_LOG(LOG_ERROR, "modbus server communication fatal error.!\n");
            close_sigint(mb_server_ret);
            break;

         default:
            DIY_LOG(LOG_ERROR, "modbus server exit for unknow reason.!\n");
            close_sigint(mb_server_ret);
    }

    clear_for_exit();
    return 0;
}
