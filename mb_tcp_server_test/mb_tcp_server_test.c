#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <errno.h>
#include <modbus/modbus.h>

#include "logger.h"
#include "hv_registers.h"

static const char* gs_local_loop_ip = "127.0.0.1";
static const uint16_t gs_def_mb_srvr_port = 502;
static modbus_t * gs_mb_tcp_client_ctx = NULL;

static void mb_reg_only_write(hv_mb_reg_e_t reg_addr)
{
    uint32_t write_data;
    const char* reg_str;
    reg_str = get_hv_mb_reg_str(reg_addr);
    if(!reg_str)
    {
        return;
    }

    if(gs_mb_tcp_client_ctx)
    {
        printf("please input the data to be written:");
        scanf("%u", &write_data);
        if(modbus_write_register(gs_mb_tcp_client_ctx, reg_addr, write_data) <= 0)
        {
            printf("modbus write register %s error:%d, %s\n",
                   reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            printf("modbus write register %s ok!\n", reg_str); 
        }
    }
    else
    {
        printf("modbus ctx is NULL.\n");
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

    if(gs_mb_tcp_client_ctx)
    {
        if(modbus_read_registers(gs_mb_tcp_client_ctx, reg_addr, 1, &read_data) <= 0)
        {
            printf("modbus read register %s error:%d, %s\n",
                    reg_str, errno, modbus_strerror(errno));
        }
        else
        {
            printf("read register %s ok, the value is: %d\n", reg_str, read_data);
        }
    }
    else
    {
        printf("modbus ctx is NULL.\n");
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

static void mb_tcp_server_test()
{
    bool end = false;
    int test_no;
    hv_mb_reg_e_t reg_addr;
    char rw_op;

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
}

static void print_modbus_params(const char* ip_addr, uint16_t tcp_port)
{
    printf("====================\nTCP server info:\n"
            "ip address: %s\ntcp port: %u\n--------------------\n",
            ip_addr, tcp_port
            );
}

static void print_usage()
{
    /*should be consistent with options defined in main function.*/
    printf("mb_tcp_test_client  [--ip_addr|-a modbus_server_ip]"
           "[--port|-p modbus_server_port] [--tcp_debug|-t]"
           "[--help|-h]\n");
    printf("The defaul arguments:\n");
    printf("--ip_addr %s\n", gs_local_loop_ip);
    printf("--port %u\n", gs_def_mb_srvr_port);
}

static const char* gs_opt_ip_addr_str = "ip_addr";
#define gs_opt_ip_addr_c 'a'
static const char* gs_opt_port_str = "port";
#define gs_opt_port_c 'p'
static const char* gs_opt_tcp_debug_str = "tcp_debug";
#define gs_opt_tcp_debug_c 't'
static const char* gs_opt_help_str = "help";
#define gs_opt_help_c 'h'
#define MAX_IP_ADDR_STR_SIZE 17 
char gs_mb_tcp_ip_str[MAX_IP_ADDR_STR_SIZE];
int main(int argc, char *argv[])
{
    /*should be consistent with usage string in print_usage function.*/
    const char* s_opt_chars = "a:p:th";
    struct option l_opt_arr[] =
    {
        {gs_opt_ip_addr_str, required_argument, 0, gs_opt_ip_addr_c},
        {gs_opt_port_str, required_argument, 0, gs_opt_port_c},
        {gs_opt_tcp_debug_str, no_argument, 0, gs_opt_tcp_debug_c}, 
        {gs_opt_help_str, no_argument, 0, gs_opt_help_c}, 
        {0, 0, 0, 0},
    };
    int opt_c;
    bool arg_parse_result;

    const char* srvr_ip = gs_local_loop_ip;
    uint16_t srvr_port = gs_def_mb_srvr_port;
    bool mb_tcp_debug_flag = false;
    struct in_addr srvr_ip_in_addr;

    arg_parse_result = true;
    while((opt_c = getopt_long(argc, argv, s_opt_chars, l_opt_arr, NULL)) >= 0)
    {
        switch(opt_c)
        {
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

            case (int)gs_opt_tcp_debug_c: 
                mb_tcp_debug_flag = true;
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
    print_modbus_params(srvr_ip, srvr_port);

    gs_mb_tcp_client_ctx = modbus_new_tcp(srvr_ip, srvr_port);
    if(!gs_mb_tcp_client_ctx)
    {
        printf("modbus_new_tcp fail, %d:%s.\n", errno, modbus_strerror(errno));
        return -1;
    }

    if(0 != modbus_set_debug(gs_mb_tcp_client_ctx, mb_tcp_debug_flag))
    {
        printf("modbus_set_debug fail, %d:%s\n", errno, modbus_strerror(errno));
        printf("but weago ahead.\n");
    }

    if(modbus_connect(gs_mb_tcp_client_ctx) == -1)
    {
         printf("Connection failed, %d:%s\n", errno, modbus_strerror(errno));
         modbus_free(gs_mb_tcp_client_ctx);
         return -1;
     }
    mb_tcp_server_test();

    modbus_close(gs_mb_tcp_client_ctx);
    modbus_free(gs_mb_tcp_client_ctx);

    return 0;
}
