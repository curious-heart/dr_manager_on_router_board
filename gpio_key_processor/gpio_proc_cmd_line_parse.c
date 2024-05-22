#include <string.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "gpio_key_app_version_def.h"
#include "get_opt_helper.h"
#include "hv_controller.h"
#include "mb_tcp_server.h"
#include "gpio_key_processor.h"

static const char* const gs_def_mb_tcp_srvr_ip = "0.0.0.0";
static char gs_mb_tcp_srvr_ip[MAX_OPT_STR_SIZE];
static const uint16_t gs_def_mb_tcp_srvr_port = 502;
static const bool gs_def_mb_tcp_srvr_debug_flag= false;
static const float gs_def_mb_tcp_client_wait_res_timout_sec = 3;
/*app log level*/
static const uint8_t gs_def_app_log_level = LOG_INFO; //refer to logger.h.

static const uint32_t gs_def_exp_start_key_hold_time = 3; //in seconds.
                                                          //
/* This option is used for the purpose that if key does not works properly, we can disable exposure by change configure file
 * to avoid unexpected exposure.*/                                                          
static const bool gs_def_exp_start_key_disabled = false;

static const char* const gs_def_mcu_exchg_device = "/dev/ttyS2";
static char gs_mcu_device_str[MAX_OPT_STR_SIZE];
static const int gs_def_gpio_clock_tick_sec = 1;
static const bool gs_def_tof_json_override = false;

mb_tcp_client_params_t g_mb_tcp_client_params =
{
    .srvr_ip = gs_def_mb_tcp_srvr_ip,
    .srvr_port = gs_def_mb_tcp_srvr_port,
    .debug_flag = gs_def_mb_tcp_srvr_debug_flag,
    .wait_res_timeout_sec = gs_def_mb_tcp_client_wait_res_timout_sec,
};
key_gpio_cfg_params_s_t g_key_gpio_cfg_params = 
{
    .exp_start_key_hold_time = gs_def_exp_start_key_hold_time,
    .exposure_disabled = gs_def_exp_start_key_disabled,
};

const char* g_mcu_exchg_device = gs_def_mcu_exchg_device;
int g_gpio_clock_tick_sec = gs_def_gpio_clock_tick_sec;
bool g_tof_json_override = gs_def_tof_json_override;

static const char* const gs_opt_mb_tcp_srvr_ip_addr_str = "mb_tcp_srvr_ip_addr";
static const char* const gs_opt_mb_tcp_srvr_port_str = "mb_tcp_srvr_port";
static const char* const gs_opt_mb_tcp_client_wait_res_timeout_sec_str = "mb_tcp_client_wait_res_timeout_sec";
static const char* const gs_opt_mb_tcp_srvr_debug_str = "mb_tcp_debug";
#define gs_opt_tcp_debug_c 't'
static const char* const gs_opt_app_log_level_str = "app_log_level";
static const char* const gs_opt_help_str = "help";
#define gs_opt_help_c 'h'
static const char* const gs_opt_version_str = "version";

static const char* const gs_opt_exp_start_key_hold_time_str = "exp_start_key_hold";
static const char* const gs_opt_exp_start_key_disabled_str = "exp_start_disabled";

static const char* const gs_opt_mcu_exchg_device_str = "mcu_exchg_device";
static const char* const gs_opt_gpio_clock_tick_sec_str = "gpio_clock_tick_sec";
static const char* const gs_opt_tof_json_override_str = "tof_json_override";

#undef APP_CMD_OPT_ITEM
#define APP_CMD_OPT_ITEM(long_o_s, has_arg, flag, val) {long_o_s, has_arg, flag, val},
#undef APP_CMD_OPT_VALUE
#define APP_CMD_OPT_VALUE(desc, def_val_ptr, work_val_ptr, val_type)
#define APP_CMD_OPT_ARRAY \
{\
    APP_CMD_OPT_ITEM(gs_opt_mb_tcp_srvr_ip_addr_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("mb_srvr_ip", \
            &gs_def_mb_tcp_srvr_ip, &g_mb_tcp_client_params.srvr_ip, c_charp)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_tcp_srvr_port_str, required_argument, 0, 0) \
    APP_CMD_OPT_VALUE("mb_srvr_port",\
           &gs_def_mb_tcp_srvr_port, &g_mb_tcp_client_params.srvr_port, uint16_t)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_tcp_client_wait_res_timeout_sec_str, required_argument, 0, 0) \
    APP_CMD_OPT_VALUE("mb_client_wait_res_timeout_sec",\
           &gs_def_mb_tcp_client_wait_res_timout_sec, &g_mb_tcp_client_params.wait_res_timeout_sec, float)\
\
    APP_CMD_OPT_ITEM(gs_opt_mb_tcp_srvr_debug_str, no_argument, 0, gs_opt_tcp_debug_c) \
    APP_CMD_OPT_VALUE("mb_tcp_debug",\
            &gs_def_mb_tcp_srvr_debug_flag, &g_mb_tcp_client_params.debug_flag, bool) \
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
    APP_CMD_OPT_ITEM(gs_opt_exp_start_key_hold_time_str , required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("exposure_start_key_hold_time", \
           &gs_def_exp_start_key_hold_time, &g_key_gpio_cfg_params.exp_start_key_hold_time, uint32_t) \
\
    APP_CMD_OPT_ITEM(gs_opt_exp_start_key_disabled_str, no_argument, 0, 0)\
    APP_CMD_OPT_VALUE("disable exposure key", \
           &gs_def_exp_start_key_disabled, &g_key_gpio_cfg_params.exposure_disabled, bool) \
\
    APP_CMD_OPT_ITEM(gs_opt_mcu_exchg_device_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("tty device for mcu comm", \
           &gs_def_mcu_exchg_device, &g_mcu_exchg_device, c_charp) \
\
    APP_CMD_OPT_ITEM(gs_opt_gpio_clock_tick_sec_str, required_argument, 0, 0)\
    APP_CMD_OPT_VALUE("tick interval for internal clock(in sec)", \
           &gs_def_gpio_clock_tick_sec, &g_gpio_clock_tick_sec, int) \
\
    APP_CMD_OPT_ITEM(gs_opt_tof_json_override_str, no_argument, 0, 0)\
    APP_CMD_OPT_VALUE("for a seq of msg, the last override the formers", \
           &gs_def_tof_json_override, &g_tof_json_override, bool) \
\
    APP_CMD_OPT_ITEM(0, 0, 0, 0)\
}

static struct option gs_long_opt_arr[] = APP_CMD_OPT_ARRAY;

#undef APP_CMD_OPT_ITEM
#define APP_CMD_OPT_ITEM(long_o_s, has_arg, flag, val)
#undef APP_CMD_OPT_VALUE
#define APP_CMD_OPT_VALUE(desc, def_val_ptr, work_val_ptr, val_type) {desc, def_val_ptr, work_val_ptr, type_##val_type},
static cmd_opt_desc_val_t gs_cmd_opt_desc_val[] = APP_CMD_OPT_ARRAY;

option_process_ret_t process_cmd_line(int argc, char* argv[])
{
    char short_opt_chars[3 * ARRAY_ITEM_CNT(gs_long_opt_arr) + 1];
    int opt_c;
    int longindex;
    struct in_addr srvr_ip_in_addr;
    bool options_valid = true;

    construct_short_opt_chars_str(short_opt_chars, gs_long_opt_arr, ARRAY_ITEM_CNT(gs_long_opt_arr));
    while((opt_c = getopt_long(argc, argv, short_opt_chars, gs_long_opt_arr, &longindex)) >= 0)
    {
        switch(opt_c)
        {
            case gs_opt_help_c: 
                print_app_cmd_line_usage(g_gpio_processor_APP_NAME, gs_long_opt_arr, gs_cmd_opt_desc_val,
                        ARRAY_ITEM_CNT(gs_long_opt_arr) - 1);
                return OPTION_PROCESS_EXIT_NORMAL;

            case gs_opt_tcp_debug_c: 
                g_mb_tcp_client_params.debug_flag = true;
                break;

            case 0:
                if(!strcmp(gs_long_opt_arr[longindex].name, gs_opt_version_str))
                {
                    printf("%s-%s.%s-%s\n", g_gpio_processor_APP_NAME, g_gpio_processor_APP_VER_STR, BUILD_DATE_STR, BUILD_TYPE_STR);
                    return OPTION_PROCESS_EXIT_NORMAL;
                }
                if(!strcmp(gs_long_opt_arr[longindex].name, gs_opt_exp_start_key_disabled_str))
                {
                    g_key_gpio_cfg_params.exposure_disabled = true;
                    DIY_LOG(LOG_INFO, "Exposure key disabled.\n");
                    break;
                }

                if(!strcmp(gs_long_opt_arr[longindex].name, gs_opt_tof_json_override_str))
                {
                    g_tof_json_override = true;
                    break;
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
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_tcp_srvr_ip_addr_str,
                        {CONVERT_FUNC_STRCPY(gs_mb_tcp_srvr_ip, optarg);\
                        g_mb_tcp_client_params.srvr_ip = gs_mb_tcp_srvr_ip;},
                        VALID_IP_STR(optarg, &srvr_ip_in_addr), INVALID_IP_STR_LOG(optarg),
                        type_c_charp);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_tcp_srvr_port_str,
                        CONVERT_FUNC_ATOUINT16(g_mb_tcp_client_params.srvr_port, optarg),
                        SHOULD_BE_NE_0(g_mb_tcp_client_params.srvr_port), SHOULD_BE_NE_0_LOG,
                        type_uint16_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mb_tcp_client_wait_res_timeout_sec_str,
                        CONVERT_FUNC_ATOF(g_mb_tcp_client_params.wait_res_timeout_sec, optarg),
                        SHOULD_BE_GT_0(g_mb_tcp_client_params.wait_res_timeout_sec), SHOULD_BE_GT_0_LOG,
                        type_float);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_exp_start_key_hold_time_str,
                        CONVERT_FUNC_ATOUINT32(g_key_gpio_cfg_params.exp_start_key_hold_time, optarg),
                        SHOULD_BE_GE_0(g_key_gpio_cfg_params.exp_start_key_hold_time), SHOULD_BE_GE_0_LOG,
                        type_uint32_t);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_mcu_exchg_device_str,
                        {CONVERT_FUNC_STRCPY(gs_mcu_device_str, optarg); g_mcu_exchg_device = gs_mcu_device_str;},
                        SHOULD_BE_NON_NULL_STR(g_mcu_exchg_device), SHOULD_BE_NON_NULL_STR_LOG,
                        type_c_charp);
                OPT_CHECK_AND_DRAW(gs_long_opt_arr[longindex].name, gs_opt_gpio_clock_tick_sec_str,
                        CONVERT_FUNC_ATOI(g_gpio_clock_tick_sec, optarg),
                        SHOULD_BE_GT_0(g_gpio_clock_tick_sec), SHOULD_BE_GT_0_LOG,
                        type_int);
                break;

            default:
                options_valid = false;
        }
    }
    if(options_valid)
    {
        print_app_cmd_line_parameters(WORKING_PARAMS,
                gs_long_opt_arr, gs_cmd_opt_desc_val, ARRAY_ITEM_CNT(gs_long_opt_arr) - 1);
        return OPTION_PROCESS_GOON;
    }
    else
    {
        print_app_cmd_line_usage(g_gpio_processor_APP_NAME, gs_long_opt_arr, gs_cmd_opt_desc_val, ARRAY_ITEM_CNT(gs_long_opt_arr) - 1);
        return OPTION_PROCESS_EXIT_ERROR;
    } 
}
