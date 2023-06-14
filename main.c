#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

#include "logger.h"
#include "hv_registers.h"
#include "hv_controller.h"

static const char* gs_def_mb_rtu_serialPortName="/dev/ttyS1";
static const int32_t gs_def_mb_rtu_serialBaudRate=9600;
static const char gs_def_mb_rtu_serialParity='N';
static const int32_t gs_def_mb_rtu_serialDataBits=8;
static const int32_t gs_def_mb_rtu_serialStopBits=1;
static const uint32_t gs_def_mb_rtu_timeout_ms =999; //1000;
static const int32_t gs_def_mb_rtu_numberOfRetries=3;
static const int32_t gs_def_mb_rtu_serverAddress=1;

static const char* gs_local_loop_ip = "127.0.0.1";
static const uint16_t gs_def_mb_srvr_port = 1502;

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
    const char* reg_str;
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

static void clear_for_exit()
{
    mb_server_exit();
    hv_controller_close();
}

static void close_sigint(int dummy)
{
    clear_for_exit();
    exit(dummy);
}

static void print_modbus_params(mb_rtu_params_t * rtu_params,
        const char* ip_addr, uint16_t tcp_port)
{
    DIY_LOG(LOG_INFO, "++++++++++++++++++++\nRTU info:\n"
            "serialPortName: %s\n"
            "serialBaudRate: %d\nserialParity: %c\nserialDataBits: %d\nserialStopBits: %d\n"
            "timeout_ms: %d\nnumberOfRetries: %d\n"
            "serverAddress: %d\n"
            "====================\nTCP server info:\n"
            "ip address: %s\ntcp port: %u\n--------------------\n",
            rtu_params->serialPortName, 
            rtu_params->serialBaudRate, rtu_params->serialParity,
            rtu_params->serialDataBits, rtu_params->serialStopBits,
            rtu_params->timeout_ms, rtu_params->numberOfRetries,
            rtu_params->serverAddress,
            ip_addr, tcp_port
            );
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

static void print_usage()
{
    /*should be consistent with options defined in main function.*/
    printf("dr_manager [--com_dev|-c /dev/ttySN] [--ip_addr|-a modbus_server_ip]\
[--port|-p modbus_server_port] [--rtu_debug|-r] [--tcp_debug|-t] \
[--rtu_master_only|-m] [--tcp_server_only|-s] \
[--help|-h]\n");
    printf("The defaul arguments:\n");
    printf("--com_dev /dev/ttyS1\n");
    printf("--ip_addr 127.0.0.1\n");
    printf("--port 1502\n");
}

typedef enum
{
    WORK_MODE_NORMAL,
    WORK_MODE_RTU_MASTER_ONLY,
    WORK_MODE_TCP_SERVER_ONLY,
}work_mode_t;
static const char* gs_opt_com_dev_str = "com_dev";
#define gs_opt_com_dev_c 'c'
static const char* gs_opt_ip_addr_str = "ip_addr";
#define gs_opt_ip_addr_c 'a'
static const char* gs_opt_port_str = "port";
#define gs_opt_port_c 'p'
static const char* gs_opt_rtu_debug_str = "rtu_debug";
#define gs_opt_rtu_debug_c 'r'
static const char* gs_opt_tcp_debug_str = "tcp_debug";
#define gs_opt_tcp_debug_c 't'
static const char* gs_opt_help_str = "help";
#define gs_opt_help_c 'h'
static const char* gs_opt_rtu_master_only_str = "rtu_master_only";
#define gs_opt_rtu_master_only_c 'm'
static const char* gs_opt_tcp_server_only_str = "tcp_server_only";
#define gs_opt_tcp_server_only_c 's'
#define MAX_TTY_DEV_NAME_SIZE 17 
#define MAX_IP_ADDR_STR_SIZE 17 
char gs_mb_rtu_dev_name[MAX_TTY_DEV_NAME_SIZE];
char gs_mb_tcp_ip_str[MAX_IP_ADDR_STR_SIZE];
int main(int argc, char *argv[])
{
    /*should be consistent with usage string in print_usage function.*/
    const char* s_opt_chars = "c:a:p:rth";
    struct option l_opt_arr[] =
    {
        {gs_opt_com_dev_str, required_argument, 0, gs_opt_com_dev_c},
        {gs_opt_ip_addr_str, required_argument, 0, gs_opt_ip_addr_c},
        {gs_opt_port_str, required_argument, 0, gs_opt_port_c},
        {gs_opt_rtu_debug_str, no_argument, 0, gs_opt_rtu_debug_c}, 
        {gs_opt_tcp_debug_str, no_argument, 0, gs_opt_tcp_debug_c}, 
        {gs_opt_rtu_master_only_str, no_argument, 0, gs_opt_rtu_master_only_c}, 
        {gs_opt_tcp_server_only_str, no_argument, 0, gs_opt_tcp_server_only_c}, 
        {gs_opt_help_str, no_argument, 0, gs_opt_help_c}, 
        {0, 0, 0, 0},
    };
    int opt_c;
    work_mode_t work_mode = WORK_MODE_NORMAL;
    bool server_only = false;
    bool arg_parse_result;

    mb_rtu_params_t rtu_params;
    const char* srvr_ip = gs_local_loop_ip;
    uint16_t srvr_port = gs_def_mb_srvr_port;
    bool mb_tcp_debug_flag = false;
    struct in_addr srvr_ip_in_addr;
    mb_server_exit_code_t mb_server_ret;

    get_mb_rtu_params(&rtu_params);
    arg_parse_result = true;
    while((opt_c = getopt_long(argc, argv, s_opt_chars, l_opt_arr, NULL)) >= 0)
    {
        switch(opt_c)
        {
            case gs_opt_com_dev_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    snprintf(gs_mb_rtu_dev_name, MAX_TTY_DEV_NAME_SIZE, optarg);
                    rtu_params.serialPortName = gs_mb_rtu_dev_name;
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "if option -%c or --%s are provided, modbus rtu device name is necessary.\n", 
                            gs_opt_com_dev_c, gs_opt_com_dev_str);
                    arg_parse_result = false;
                }
                break;

            case (int)gs_opt_ip_addr_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    snprintf(gs_mb_tcp_ip_str, MAX_IP_ADDR_STR_SIZE, optarg);
                    if(0 == inet_aton(gs_mb_tcp_ip_str, &srvr_ip_in_addr))
                    {
                        DIY_LOG(LOG_ERROR,
                                "The modbus server ip address %s is invalid.\n",
                                gs_mb_tcp_ip_str);
                        arg_parse_result = false;
                    }
                    else
                    {
                        srvr_ip = gs_mb_tcp_ip_str;
                    }
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "if option -%c or --%s are provided, modbus server ip addr is necessary.\n", 
                            gs_opt_ip_addr_c, gs_opt_ip_addr_str);
                    arg_parse_result = false;
                }
                break;

            case (int)gs_opt_port_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    srvr_port = (uint16_t)atoi(optarg);
                    if(0 == srvr_port)
                    {
                        DIY_LOG(LOG_ERROR, "The modbus server port %s is invalid.\n", optarg);
                        arg_parse_result = false;
                    }
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "if option -%c or --%s are provided, modbus server port is necessary.\n", 
                            gs_opt_port_c, gs_opt_port_str);
                    arg_parse_result = false;
                }
                break;

            case (int)gs_opt_rtu_debug_c: 
                rtu_params.debug_flag = true;
                break;

            case (int)gs_opt_tcp_debug_c: 
                mb_tcp_debug_flag = true;
                break;

            case (int)gs_opt_rtu_master_only_c:
                work_mode = WORK_MODE_RTU_MASTER_ONLY;
                break;

            case (int)gs_opt_tcp_server_only_c:
                work_mode = WORK_MODE_TCP_SERVER_ONLY;
                break;

            case (int)gs_opt_help_c: 
                print_usage();
                return 0;
            default:
                arg_parse_result = false;
        }
    }
    if(!arg_parse_result)
    {
        print_usage();
        return -1;
    }

    print_modbus_params(&rtu_params, srvr_ip, srvr_port);
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
            return -1;;
        }
    }

    mb_server_ret = mb_server_loop(srvr_ip, srvr_port, mb_tcp_debug_flag, server_only);
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
