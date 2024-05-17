#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "version_def.h"
#include "logger.h"
#include "hv_registers.h"
#include "hv_controller.h"
#include "mb_tcp_server.h"
#include "dr_manager.h"
#include "get_opt_helper.h"
#include "pthread_helper.h"

#ifdef MANAGE_LCD_AND_TOF_HERE
#include "tof_measure.h"
#endif

#include "option_configuration_process.h"
#include "main_app_used_gpios.h"

#ifdef MANAGE_LCD_AND_TOF_HERE
extern const char* const g_def_lcd_dev_name;
extern const unsigned char g_def_LCD_I2C_ADDR;
#endif

const char* g_main_thread_desc = "Main-thread";

static pthread_t gs_dev_monitor_th_id;
static bool gs_dev_monitor_th_started = false;

#ifdef MANAGE_LCD_AND_TOF_HERE
static pthread_t gs_lcd_refresh_th_id, gs_tof_th_id;
static bool gs_lcd_refresh_th_started =false, gs_tof_th_started = false;
#endif

#define MAX_USR_INPUT_LEN 16
static void mb_reg_only_write(hv_mb_reg_e_t reg_addr)
{
    uint32_t write_data;
    const char* reg_str;
    char r_buf[MAX_USR_INPUT_LEN + 1];

    reg_str = get_hv_mb_reg_str(reg_addr);
    if(!reg_str)
    {
        return;
    }
    printf("please input the data to be written:\n");
    fgets(r_buf, sizeof(r_buf), stdin);
    sscanf(r_buf, "%u", &write_data);
    printf("\n");
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
    char r_buf[MAX_USR_INPUT_LEN + 1];
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
        printf("\n");

        printf("please input the register number (-1 to exit):\n");

        fgets(r_buf, sizeof(r_buf), stdin);
        sscanf(r_buf, "%d", &test_no);
        printf("\n");

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
    hv_controller_close();
}

static void print_modbus_params(mb_rtu_params_t * rtu_params, mb_tcp_server_params_t * srvr_params)
{
    DIY_LOG(LOG_INFO, "++++++++++++++++++++\nRTU info:\nserialPortName: %s\n",
            rtu_params->serialPortName);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, 
            "serialBaudRate: %d\nserialParity: %c\nserialDataBits: %d\nserialStopBits: %d\n"
            "timeout_ms: %u\nnumberOfRetries: %d\n"
            "serverAddress: %d\ndebug_flag:%d\n",
            rtu_params->serialBaudRate, rtu_params->serialParity,
            rtu_params->serialDataBits, rtu_params->serialStopBits,
            rtu_params->timeout_ms, rtu_params->numberOfRetries,
            rtu_params->serverAddress, rtu_params->debug_flag);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP,
            "====================\nTCP server info:\n"
            "ip address: %s\ntcp port: %u\ndebug_flag:%d\n\t--------------------\n",
            srvr_params->srvr_ip, srvr_params->srvr_port, srvr_params->debug_flag);
}

static void main_th_exit_handler()
{
    /*Nothing to do now.*/
}

static bool clear_threads()
{
    /*
       If we call the following functions (indeed it is the cancel_assit_thread), when kill the process from shell,
       there is segment fault. I don't know why...
       But it does not matter we cancel threads or not now, since there is no resource leak. So we jut comment the following
       code parts, and in future when I get to understand more about the pthrea mechanism, I'll refresh this function.
    */

    /*
    int s;
    cancel_assit_thread(gs_dev_monitor_th_started , &gs_dev_monitor_th_id);
    gs_dev_monitor_th_started = false;

    cancel_assit_thread(gs_lcd_refresh_th_started, &gs_lcd_refresh_th_id);
    gs_lcd_refresh_th_started = false;

    cancel_assit_thread(gs_tof_th_started, &gs_tof_th_id);
    gs_tof_th_started = false;

    s = destroy_dev_st_pool_mutex();
    PTHREAD_ERR_CHECK(s, "destory_dev_st_pool_mutex", "", " failes", false, false);

    s = destroy_tof_th_measure_syncs();
    PTHREAD_ERR_CHECK(s, "destory_tof_th_measure_mutex", "", " failes", false, false);

    */

    main_th_exit_handler();
    return true;
}

static void clear_for_exit()
{
    static bool exited = false;

    if(!exited)
    {
        DIY_LOG(LOG_INFO, "clear for exit.................\n");
        mb_server_exit();
        hv_controller_close();

        clear_threads();

        exited = true;
    }
    else
    {
        DIY_LOG(LOG_INFO, "app already  cleared and exited.\n");
    }
}

static void close_sigint(int dummy)
{
    DIY_LOG(LOG_INFO, "close_signint(dummy: %d), now exit...\n", dummy);
    clear_for_exit();
    exit(dummy);
}

static void init_thread_syncs()
{
    init_dev_st_pool_mutex();
#ifdef MANAGE_LCD_AND_TOF_HERE
    init_lcd_upd_sync_mech();
    init_tof_th_measure_syncs();
#endif
}

/* version format:
 *     "v" + SW_V + " " + "v" + SW_V + "." + dsp_ver + "." + fw_ver + "." + app_ver
 * where
 *     The beginning "v" + SW_V is the "release version"
 *     SW_V: 1~3 digits, "ss".
 *     dsp_ver: 6 digits, "hhhlll", where hhh is the high version and lll is low version, both are decima number.
 *     fw_ver: 3 digits, "fff", from openwrt_version file.
 *     app_ver: 6~8  digits, "ddd-ggg-", the concatenation of the version str of dr_manager and gpio_processor app.
 *              "-" are optional, indicating this is a debugging version.
 *
 * Note: since dsp_sw_v needs read registers from hv-controlling-dsp, this function should be called after gs_server_ready
 * getting true (check by mb_server_is_ready()), or dsp_sw_v is zero.
 * */
const char* get_whole_fw_version_string(int* s_len)
{
    extern const char * g_gpio_processor_APP_VER_STR;
    static char ver_str[MAX_FW_V_LINE_LEN + 1] = {0};
    static int buf_size = sizeof(ver_str);
    int ver_str_len = 0, w_len = 0,  cur_size;
    FILE* fw_v_file;
    char fw_v_line_buf[MAX_FW_V_LINE_LEN + 1], *line_ptr, *fw_v_pos, *invalid_char_pos;
    uint16_t dsp_sw_v = get_dsp_sw_ver();

    /* generate the string.*/
    cur_size = buf_size;
    do
    {
        w_len = snprintf(&ver_str[ver_str_len], cur_size, "v%u  v%u.%03u%03u.",
                            g_SW_VER_NUMBER, g_SW_VER_NUMBER, (dsp_sw_v & 0xFF00)>>8, (dsp_sw_v & 0x00FF));
        if(w_len <= 0 || w_len >= cur_size) break;
        ver_str_len += w_len; cur_size = buf_size - ver_str_len;

        fw_v_file = fopen("/etc/openwrt_version", "r");
        if(NULL == fw_v_file) { DIY_LOG(LOG_ERROR, "Open openwrt_version error.\n"); break; }
        line_ptr = fgets(fw_v_line_buf, sizeof(fw_v_line_buf), fw_v_file);
        if(NULL == line_ptr) { fclose(fw_v_file); DIY_LOG(LOG_ERROR, "read openwrt_version gets NULL.\n"); break; }
        fclose(fw_v_file);
        fw_v_pos = strstr(fw_v_line_buf, ", ");
        if(NULL == fw_v_pos) { DIY_LOG(LOG_ERROR, "no fw version number found.\n"); break; }
        fw_v_pos += 2; //skip the ", "
        invalid_char_pos = strpbrk(fw_v_pos, "\r\n");
        if(NULL != invalid_char_pos) *invalid_char_pos = '\0';
        w_len = snprintf(&ver_str[ver_str_len], cur_size, "%s.", fw_v_pos);
        if(w_len <= 0) break;
        ver_str_len += w_len; cur_size = buf_size - ver_str_len;
        if(cur_size <= 0) break;

        w_len = snprintf(&ver_str[ver_str_len], cur_size, "%s%s", g_APP_VER_STR, g_gpio_processor_APP_VER_STR);
        if(w_len <= 0 || w_len >= cur_size) break;
        ver_str_len += w_len; cur_size = buf_size - ver_str_len;

        break;
    }while(true);
    DIY_LOG(LOG_INFO, "The version string to be displayd:\n%s\n", ver_str);

    if(s_len) *s_len = ver_str_len;
    return ver_str;
}

/*
 * Note: since get_whole_fw_version_string needs read registers from hv-controlling-dsp, this function should be called 
 * after gs_server_ready getting true (check by mb_server_is_ready()), or dsp version is zero.
 * */
void write_version_str_to_file()
{
#define MAX_CMD_LINE 64
    const char* tmp_file = "/tmp/.dr_manager_version";
    const char* ver_str = get_whole_fw_version_string(NULL);
    char cmd_line[MAX_CMD_LINE + MAX_FW_V_LINE_LEN + 1];
    snprintf(cmd_line, sizeof(cmd_line), "echo \"%s\" > %s", ver_str, tmp_file);
    system(cmd_line);
}

#ifndef MANAGE_LCD_AND_TOF_HERE
bool send_dev_info_external()
{
    static const char* curr_sh = "/usr/bin/send_dev_info_external.sh";
    FILE* r_stream = NULL;
    char* line = NULL;
    size_t len = 0;
    bool ret = false;
    char true_char = '1';

    r_stream = popen(curr_sh, "r");
    if(NULL == r_stream)
    {
        DIY_LOG(LOG_ERROR, "popen %s error.\n", curr_sh);
        return ret;
    }

    if(getline(&line, &len, r_stream) > 0)
    {
        ret = (line[0] == true_char) ? true : false;
    }

    free(line);
    pclose(r_stream);

    return ret;
}
#endif

extern cmd_line_opt_collection_t g_cmd_line_opt_collection;
int main(int argc, char *argv[])
{
    bool server_only = false;
    option_process_ret_t arg_parse_result;

    mb_server_exit_code_t mb_server_ret;

    arg_parse_result = process_cmd_options(argc, argv);

    switch(arg_parse_result)
    {
        case OPTION_PROCESS_EXIT_NORMAL:
            return 0;
        case OPTION_PROCESS_EXIT_ERROR:
            return -1;
        default:
            ;
    }

    print_modbus_params(&g_cmd_line_opt_collection.rtu_params, &g_cmd_line_opt_collection.srvr_params);

    switch(g_cmd_line_opt_collection.work_mode)
    {
        case WORK_MODE_RTU_MASTER_ONLY:
            rtu_master_test(&g_cmd_line_opt_collection.rtu_params);
            return 0;

        case WORK_MODE_TCP_SERVER_ONLY:
            server_only = true;
            break;

        default:
        ;
    }

    export_gpios_to_app();

    /*Init necessary mutexes or other elements for synchronization.*/
    init_thread_syncs();
    /*start other threads. ++++++++++++++++++++++++++++++*/
    if(!start_assit_thread(g_dev_monitor_th_desc, &gs_dev_monitor_th_id, true,
                dev_monitor_thread_func, &g_cmd_line_opt_collection.dev_monitor_th_parm))
    {
        return -1;
    }
    gs_dev_monitor_th_started = true;

#ifdef MANAGE_LCD_AND_TOF_HERE
    if(!start_assit_thread(g_lcd_refresh_th_desc, &gs_lcd_refresh_th_id, true,
                lcd_refresh_thread_func, &g_cmd_line_opt_collection.lcd_refresh_th_parm))
    {
        return -1;
    }
    gs_lcd_refresh_th_started = true;

    if(!start_assit_thread(g_tof_th_desc, &gs_tof_th_id, true,
            tof_thread_func, &g_cmd_line_opt_collection.tof_th_parm))
    {
        return -1;
    }
    gs_tof_th_started = true;
#endif
    /*------------------------------*/

    if(!server_only)
    {
        if(!hv_controller_open(&g_cmd_line_opt_collection.rtu_params))
        {
            DIY_LOG(LOG_ERROR, "Connecting high volatage board fails.\n");
            return -1;;
        }
    }

    atexit(clear_for_exit);
    signal(SIGINT, close_sigint);
    signal(SIGTERM, close_sigint);

    mb_server_ret = mb_server_loop(&g_cmd_line_opt_collection.srvr_params, server_only);
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

    return 0;
}
