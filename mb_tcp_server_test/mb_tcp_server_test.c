#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <errno.h>
#include <modbus/modbus.h>
#include <string.h>

#include "logger.h"
#include "hv_registers.h"

static const char* gs_APP_NAME = "mb_tcp_test_client";
static const char * gs_APP_VER_STR = "1.0.1";

#define MAX_CHAR_NUM_READ 16
static const char* gs_local_loop_ip = "127.0.0.1";
static const uint16_t gs_def_mb_srvr_port = 502;
static modbus_t * gs_mb_tcp_client_ctx = NULL;

static void mb_reg_only_write(hv_mb_reg_e_t reg_addr)
{
    uint16_t write_data;
    const char* reg_str;
    char r_buf[MAX_CHAR_NUM_READ + 1];

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(!reg_str)
    {
        return;
    }
    printf("...............%s.\n", reg_str);
    if(gs_mb_tcp_client_ctx)
    {
        printf("please input the data to be written:\n");
        fgets(r_buf, sizeof(r_buf), stdin);
        sscanf(r_buf, "%hu", &write_data);
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
    uint16_t read_data[2] = {0xFFFF, 0xFFFF}, translated_addr;
    float DAP_v;
    const char* reg_str;
    int mb_read_ret;

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(!reg_str)
    {
        return read_data[0];
    }

    if(gs_mb_tcp_client_ctx)
    {
        if(EXT_MB_REG_DAP_HP == reg_addr || EXT_MB_REG_DAP_LP == reg_addr )
        {
            translated_addr = EXT_MB_REG_DAP_HP;
            mb_read_ret = modbus_read_registers(gs_mb_tcp_client_ctx, translated_addr, 2, &read_data[0]);
            if(mb_read_ret <= 0)
            {
                printf("modbus read register %s error:%d, %s\n", reg_str, errno, modbus_strerror(errno));
            }
            else
            {
                DAP_v = get_float_DAP_from_reg_arr(read_data);
                printf("DAP High part: 0x%04X, Low Part: 0x%04X; DAP value: %f\n",
                        read_data[0], read_data[1], DAP_v);
            }
        }
        else
        {
            mb_read_ret = modbus_read_registers(gs_mb_tcp_client_ctx, reg_addr, 1, &read_data[0]);
            if(mb_read_ret <= 0)
            {
                printf("modbus read register %s error:%d, %s\n", reg_str, errno, modbus_strerror(errno));
            }
            else
            {
                printf("read register %s ok, the value is: %d\n", reg_str, read_data[0]);
            }
        }
    }
    else
    {
        printf("modbus ctx is NULL.\n");
    }

    return read_data[0];
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

static void print_register_tips()
{
    hv_mb_reg_e_t idx;
    const char* reg_str;
    char attr_char;
    for(idx = HSV; idx < HV_MB_REG_END_FLAG; ++idx)
    {
        if(!VALID_MB_REG_ADDR(idx))
        {
            continue;
        }

        reg_str = get_hv_mb_reg_str(idx);
        if(reg_str) printf("%s", reg_str);

        attr_char = get_hv_mb_reg_rw_attr(idx);
        if(attr_char) printf("\t\t\trw:\"%c\"", attr_char);

        printf("\n");
    }
}

static void mb_tcp_server_test()
{
    bool end = false;
    int test_no;
    hv_mb_reg_e_t reg_addr;
    char r_buf[MAX_CHAR_NUM_READ + 1]; 
    char rw_op;

    while(!end)
    {
        printf("\n");
        print_register_tips();
        printf("-1: exit.\n");

        fgets(r_buf, sizeof(r_buf), stdin);
        sscanf(r_buf, "%d", &test_no);
        if(test_no < 0)
        {
            end = true;
            break;
        }
        if(!VALID_MB_REG_ADDR(test_no))
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
           "[--help|-h] [--version]\n");
    printf("The defaul arguments:\n");
    printf("--ip_addr %s\n", gs_local_loop_ip);
    printf("--port %u\n", gs_def_mb_srvr_port);
}

static const char* const gs_opt_version_str = "version";
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
        {gs_opt_version_str, no_argument, 0, 0}, 
        {0, 0, 0, 0},
    };
    int opt_c;
    int longindex;
    bool arg_parse_result;

    const char* srvr_ip = gs_local_loop_ip;
    uint16_t srvr_port = gs_def_mb_srvr_port;
    bool mb_tcp_debug_flag = false;
    struct in_addr srvr_ip_in_addr;

    arg_parse_result = true;
    while((opt_c = getopt_long(argc, argv, s_opt_chars, l_opt_arr, &longindex)) >= 0)
    {
        switch(opt_c)
        {
            case (int)gs_opt_ip_addr_c:
                if(optarg && optarg[0] != ':' && optarg[0] != '?')
                {
                    snprintf(gs_mb_tcp_ip_str, MAX_IP_ADDR_STR_SIZE,"%s", optarg);
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

            case 0:
                if(!strcmp(l_opt_arr[longindex].name, gs_opt_version_str))
                {
                    printf("%s-%s.%s-%s\n", gs_APP_NAME, gs_APP_VER_STR, BUILD_DATE_STR, BUILD_TYPE_STR);
                    return 0;
                }
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
