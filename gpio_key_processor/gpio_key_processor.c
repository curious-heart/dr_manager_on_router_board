#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>

#include "logger.h"
#include "get_opt_helper.h"
#include "gpio_key_processor.h"
#include "op_gpio_thu_reg.h"

#define UEVENT_MSG_LEN 1024 //4096

#undef KEY_AND_ACT_NAME
#define KEY_AND_ACT_NAME(k) #k,
#undef GPIO_AND_ACT_NAME
#define GPIO_AND_ACT_NAME(g) #g,
#undef KEY_GPIO_END_FLAG 
#define KEY_GPIO_END_FLAG(k) 
const char* g_key_gpio_name_list[kg_end_flag] = KEY_GPIO_LIST ;
const char* g_key_gpio_act_name_list[kg_action_end_flag] = KEY_GPIO_ACT_LIST ;

/* The definition here SHOULD be consistent with the strings in button_hotplug_fill_event 
 * and button_hotplug_create_event functions of gpio-button-hotplug.c.
 * ("gbh" means "gpio button hotplug")
 */
static const char* gs_cared_subsystem_str = "button";
#undef GBH_UEVENT_ELE
#define GBH_UEVENT_ELE(e) const char* e;
#undef GBH_UEVENT_ELE_END_FLAG
#define GBH_UEVENT_ELE_END_FLAG
#define GBH_UEVENT_LIST \
{\
    GBH_UEVENT_ELE(HOME)\
    GBH_UEVENT_ELE(PATH)\
    GBH_UEVENT_ELE(SUBSYSTEM)\
    GBH_UEVENT_ELE(ACTION)\
    GBH_UEVENT_ELE(BUTTON)\
    GBH_UEVENT_ELE(TYPE)\
    GBH_UEVENT_ELE(SEEN)\
    GBH_UEVENT_ELE(SEQNUM)\
    GBH_UEVENT_ELE_END_FLAG\
}
typedef struct GBH_UEVENT_LIST parsed_gbh_uevt_s_t;

#undef GBH_UEVENT_ELE
#define GBH_UEVENT_ELE(e) #e,
static const char* gs_gbh_uevent_ele_names_arr[] = GBH_UEVENT_LIST; 
#undef GBH_UEVENT_ELE
#define GBH_UEVENT_ELE(e) enum_##e,
#undef GBH_UEVENT_ELE_END_FLAG
#define GBH_UEVENT_ELE_END_FLAG gbh_uevent_ele_end_flag,
typedef enum GBH_UEVENT_LIST gbh_uevent_list_e_t;

static int open_gbh_uevent_recv_socket(void)
{
    struct sockaddr_nl addr;
    int sz = 4 * UEVENT_MSG_LEN; // 64*1024;
    int s;

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (s < 0)
    {
        DIY_LOG(LOG_ERROR, "create socket for gbh uevent reception fails: %d.\n", errno);
        return -1;
    }

    setsockopt(s, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz));

    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) 
    {
        close(s);
        DIY_LOG(LOG_ERROR, "bind socket for gbh uevent reception fails: %d.\n", errno);
        return -1;
    }

    return s;
}

static void parse_gbh_uevent(const char *msg, int max_msg_len, const char** helper[])
{
    int idx;
    int ele_name_len;
    int msg_parsed_len;

    if(!msg || !helper)
    {
        DIY_LOG(LOG_ERROR, "NULL msg or parse helper array.\n");
        return;
    }

    if(*msg)
    {
        DIY_LOG(LOG_DEBUG, "Received gbh msg:\n");
    }
    msg_parsed_len = 0;
    while(*msg && msg_parsed_len < max_msg_len)
    {
        DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "%s\n", msg);
        for(idx = enum_HOME; idx < gbh_uevent_ele_end_flag; ++idx)
        {
            ele_name_len = strlen(gs_gbh_uevent_ele_names_arr[idx]);
            if(!strncmp(msg, gs_gbh_uevent_ele_names_arr[idx], ele_name_len)
                    && '=' == msg[ele_name_len])
            {
                /* + 1 for the "=". */
                ele_name_len += 1;
                msg += ele_name_len;
                msg_parsed_len += ele_name_len;

                if(helper[idx]) *(helper[idx]) = msg;
                break;
            }
        }
        while(*msg && msg_parsed_len < max_msg_len)
        {
            ++msg; ++msg_parsed_len;
        }
        if(msg_parsed_len < max_msg_len)
        {
            ++msg; ++msg_parsed_len;
        }

        /*
        if (!strncmp(msg, "ACTION=", 7)) {
            msg += 7;
            parsed_gbh_uevt->action = msg;
        } else if (!strncmp(msg, "DEVPATH=", 8)) {
            msg += 8;
            parsed_gbh_uevt->path = msg;
        } else if (!strncmp(msg, "SUBSYSTEM=", 10)) {
            msg += 10;
            parsed_gbh_uevt->subsystem = msg;
        } else if (!strncmp(msg, "FIRMWARE=", 9)) {
            msg += 9;
            parsed_gbh_uevt->firmware = msg;
        } else if (!strncmp(msg, "MAJOR=", 6)) {
            msg += 6;
            parsed_gbh_uevt->major = atoi(msg);
        } else if (!strncmp(msg, "MINOR=", 6)) {
            msg += 6;
            parsed_gbh_uevt->minor = atoi(msg);
        }

        while(*msg++)
            ;
        */
    }
}

static void convert_gbh_string_uevent(parsed_gbh_uevt_s_t * string_uevvt, converted_gbh_uevt_s_t * converted_uevt)
{
    int idx;
    if(!string_uevvt || !converted_uevt)
    {
        DIY_LOG(LOG_ERROR, "string_uevent or converted_uevt is NULL.\n");
        return;
    }
    if(strcmp(string_uevvt->SUBSYSTEM, gs_cared_subsystem_str))
    {
        DIY_LOG(LOG_INFO, "We do not care about the event of sub-system %s.\n", string_uevvt->SUBSYSTEM);
        return;
    }

    converted_uevt->key_gpio = kg_end_flag;
    for(idx = key_exp_range_led; idx < kg_end_flag; ++idx)
    {
        if(!strcmp(string_uevvt->BUTTON, g_key_gpio_name_list[idx]))
        {
            converted_uevt->key_gpio = idx;
            break;
        }
    }
    converted_uevt->action = kg_action_end_flag;
    for(idx = key_released; idx < kg_action_end_flag; ++idx)
    {
        if(!strcmp(string_uevvt->ACTION, g_key_gpio_act_name_list[idx]))
        {
            converted_uevt->action = idx;
            break;
        }
    }
    converted_uevt->seen = strtol(string_uevvt->SEEN, NULL, 10);
    converted_uevt->seqnum = strtol(string_uevvt->SEQNUM, NULL, 10);
    if(converted_uevt->key_gpio < kg_end_flag && converted_uevt->action < kg_action_end_flag)
    {
        converted_uevt->valid = true;
    }
    else
    {
        DIY_LOG(LOG_WARN, "Unknown key/gpio or action.\n");
    }
}

static key_gpio_handler_t gs_key_gpio_handler_arr[kg_end_flag] =
{
    /*The order MUST be consistent with the element order defined in macro KEY_GPIO_LIST. */
   [key_exp_range_led] = exp_range_led_key_handler,
   [key_exp_start] = exp_start_key_handler,
   [key_dose_add] = dose_adjust_key_handler,
   [key_dose_sub] = dose_adjust_key_handler,
   [key_reset] = exp_start_key_handler, //reset_key_handler,
   [gpio_charger] = charger_gpio_handler,
};
static void process_gbh_uevent(converted_gbh_uevt_s_t * evt)
{
    if(!evt)
    {
        DIY_LOG(LOG_ERROR, "event is NULL.!\n");
        return;
    }
    if(evt->key_gpio >= kg_end_flag)
    {
        DIY_LOG(LOG_WARN, "Unknown key/gpio: %d\n", evt->key_gpio);
        return;
    }
    if(evt->action >= kg_action_end_flag)
    {
        DIY_LOG(LOG_WARN, "Unknown key/gpio action: %d\n", evt->action);
        return;
    }

    DIY_LOG(LOG_DEBUG, "Converted gbh uevent info:\n");
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "key/gpio: %s\n", g_key_gpio_name_list[evt->key_gpio]);
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "action: %s\n", g_key_gpio_act_name_list[evt->action]);
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "seen: %u\n", evt->seen);
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "seqnum: %u\n", evt->seqnum);
    if(!gs_key_gpio_handler_arr[evt->key_gpio])
    {
        DIY_LOG(LOG_WARN, "The key/gpio %s has no handler.\n", g_key_gpio_name_list[evt->key_gpio]);
        return;
    }
    gs_key_gpio_handler_arr[evt->key_gpio](evt);
}

static int gs_gbh_uevent_recv_sock = -1;

#ifndef MANAGE_LCD_AND_TOF_HERE
static int gs_mcu_dev_fd = -1;
#endif

static void clear_for_exit()
{
    static bool already_called = false;

    if(!already_called)
    {
        already_called = true;

        if(gs_gbh_uevent_recv_sock > 0)
        {
            close(gs_gbh_uevent_recv_sock);
            gs_gbh_uevent_recv_sock = -1;
        }
        end_key_event_handle();

#ifndef MANAGE_LCD_AND_TOF_HERE
        if(gs_mcu_dev_fd > 0)
        {
            close(gs_mcu_dev_fd);
            gs_mcu_dev_fd = -1;
        }
#endif
        clear_app_timer_list();

        DIY_LOG(LOG_INFO, "clear_for_exit!\n");
    }
    else
    {
        DIY_LOG(LOG_WARN, "clear_for_exit already called.\n");
    }
}

static void close_sigint(int dummy)
{
    DIY_LOG(LOG_INFO, "SIGINT received, now exit...\n");
    clear_for_exit();
    exit(dummy);
}

#ifndef MANAGE_LCD_AND_TOF_HERE
extern const char* g_mcu_exchg_device;
extern bool g_tof_json_override;
static char gs_mcu_json_packet[MAX_JSON_MESSAGE_LEN + 1];
static void process_tof_json_msg(char* msg, int msg_len)
{
    static const char* tmp_recvd_json_file="/tmp/.recvd_json_msg";
    static const char* get_json_data_sh = "get_data_from_recvd_json.sh";
    static const char* tof_hd = "tof_distance";
    static const char* key_evt_hd = "key_evt";
    static const char* key_name_add = "add";
    static const char* key_name_sub = "sub";
    static const char* key_val_pressed = "pressed";
    static const char* key_val_released = "released";
    static uint32_t ls_key_seq = 0;
    FILE * msg_file, *d_stream;
    int written_len;
    char *info_line;
    size_t info_line_len;
    ssize_t read_cnt;
    bool goon = true;
    uint16_t dist;
    static uint16_t ls_last_dist;

    DIY_LOG(LOG_INFO, "json msg: %s\n", msg);

    msg_file = fopen(tmp_recvd_json_file, "w");
    if(!msg_file)
    {
        DIY_LOG(LOG_ERROR, "create file %s error!\n", tmp_recvd_json_file);
        return;
    }
    written_len = fwrite(msg, msg_len, 1, msg_file);
    if(written_len != 1)
    {
        DIY_LOG(LOG_ERROR, "write to file %s abnormal: %d chars to be written, but actual %d chars.\n",
                tmp_recvd_json_file, msg_len, written_len * msg_len);
        goon = false;
    }
    fflush(msg_file);
    fclose(msg_file);

    if(!goon) return;

    d_stream = popen(get_json_data_sh, "r");
    if(!d_stream)
    {
        DIY_LOG(LOG_ERROR, "open %s error: %s\n", get_json_data_sh, strerror(errno));
        return;
    }

    info_line = NULL; info_line_len = 0;
    while((read_cnt = getline(&info_line, &info_line_len, d_stream)) != -1)
    {
        if(info_line && read_cnt > 0)
        {
            char* colon_cp, *hd_ptr, *vstr_ptr;
            info_line[strcspn(info_line, "\r\n")] = '\0';
            colon_cp = strstr(info_line, ":");
            if(!colon_cp || (colon_cp - info_line + 1 >= info_line_len))
            {
                DIY_LOG(LOG_ERROR, "ignore invalid info from json data sh: %s\n", info_line);
                continue;
            }
            hd_ptr = info_line; *colon_cp = 0;
            vstr_ptr = colon_cp + 1;
            if(!strcmp(hd_ptr, tof_hd))
            {
                CONVERT_FUNC_ATOUINT16(dist, vstr_ptr);
                DIY_LOG(LOG_INFO, "tof distance read from json: %s, last distance %u.", vstr_ptr, ls_last_dist);
                {
                    extern uint16_t g_tof_dist_smooth_range_mm;
                    uint16_t diff = (ls_last_dist >= dist) ? (ls_last_dist - dist) : (dist - ls_last_dist);
                    if(diff >= g_tof_dist_smooth_range_mm)
                    {
                        ls_last_dist = dist;
                        update_tof_distance(dist);
                        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, " update.\n");
                    }
                    else
                    {
                        DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR_COMP, " diff < %u, so do not update.\n",
                                g_tof_dist_smooth_range_mm);
                    }
                }
            }
            else if(!strcmp(hd_ptr, key_evt_hd))
            {
                converted_gbh_uevt_s_t converted_gbh_uevt;
                char* semicol_ptr, *key_name_ptr, *key_val_ptr;
                DIY_LOG(LOG_INFO, "key event read from json: %s\n", vstr_ptr);
                semicol_ptr = strstr(vstr_ptr, ",");
                if(!semicol_ptr || (semicol_ptr - vstr_ptr + 1) >= strlen(vstr_ptr))
                {
                    DIY_LOG(LOG_ERROR, "ignore invalid key event from json data sh:%s\n", vstr_ptr);
                    continue;
                }
                key_name_ptr = vstr_ptr; *semicol_ptr = 0;
                key_val_ptr = semicol_ptr + 1;
                if(!strcmp(key_name_ptr, key_name_add))
                {
                    converted_gbh_uevt.key_gpio = key_dose_add;
                }
                else if(!strcmp(key_name_ptr, key_name_sub))
                {
                    converted_gbh_uevt.key_gpio = key_dose_sub;
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "ignore key event because of invalid key name: %s\n", key_name_ptr);
                    continue;
                }

                if(!strcmp(key_val_ptr, key_val_pressed))
                {
                    converted_gbh_uevt.action = key_pressed;
                }
                else if(!strcmp(key_val_ptr, key_val_released))
                {
                    converted_gbh_uevt.action = key_released;
                }
                else
                {
                    DIY_LOG(LOG_ERROR, "ignore key event because of invalid key value: %s\n", key_val_ptr);
                    continue;
                }
                converted_gbh_uevt.seen = 0;
                converted_gbh_uevt.seqnum = ls_key_seq; ++ls_key_seq;
                converted_gbh_uevt.valid = true;

                process_gbh_uevent(&converted_gbh_uevt);
            }
            else
            {
                DIY_LOG(LOG_ERROR, "unknown info from json data sh and it is ignored:%s:%s\n", hd_ptr, vstr_ptr);
            }
        }
    }

    free(info_line);
    pclose(d_stream);
}
#endif
extern int g_gpio_clock_tick_sec;

option_process_ret_t process_cmd_line(int argc, char* argv[]);
int main(int argc, char* argv[])
{
    converted_gbh_uevt_s_t converted_gbh_uevt;
    parsed_gbh_uevt_s_t parsed_gbh_uevt;
#undef GBH_UEVENT_ELE
#define GBH_UEVENT_ELE(e) &(parsed_gbh_uevt.e),
#undef GBH_UEVENT_ELE_END_FLAG
#define GBH_UEVENT_ELE_END_FLAG
    const char** parse_helper[] = GBH_UEVENT_LIST;
    int device_fd = -1;
    char msg[UEVENT_MSG_LEN+2];
    int n;
    option_process_ret_t arg_parse_result;
    fd_set fds, ref_fds;
    int fd_max;
    struct timeval timeout;

    arg_parse_result = process_cmd_line(argc, argv);
    switch(arg_parse_result)
    {
        case OPTION_PROCESS_EXIT_NORMAL:
            return 0;
        case OPTION_PROCESS_EXIT_ERROR:
            return -1;

        default:
            ;
    }

    atexit(clear_for_exit);
    signal(SIGINT, close_sigint);
    signal(SIGTERM, close_sigint);

    device_fd = open_gbh_uevent_recv_socket();
    if(device_fd < 0)
    {
        return -1;
    }
    gs_gbh_uevent_recv_sock  = device_fd;

    FD_ZERO(&ref_fds);
    FD_SET(gs_gbh_uevent_recv_sock, &ref_fds);
    fd_max = gs_gbh_uevent_recv_sock;

    if(!begin_key_event_handle())
    {
        return -1;
    }

    /* Read charger gpio at start. Because if device is powered on with charger plugged, no uevent would be 
     * broadcast up.
     * */
    if(0 == gpio_mmap_for_read())
    {
#define CHARGER_GPIO_NUM 41 //refer to the table in comment in file main_app_used_gpios.h
        int gpio_value = gpio_pin_read(CHARGER_GPIO_NUM);
        if(gpio_value >= 0)
        {
            converted_gbh_uevt.key_gpio = gpio_charger;
            converted_gbh_uevt.action = gpio_value ? key_pressed : key_released;
            converted_gbh_uevt.seen = converted_gbh_uevt.seqnum = 0; //this two element are not used yet.
            converted_gbh_uevt.valid = true;
            charger_gpio_handler(&converted_gbh_uevt);
        }
        else
        {
            DIY_LOG(LOG_ERROR, "invalid value read from charger pin.\n");
        }
        gpio_munmap_for_read();
    }


#ifndef MANAGE_LCD_AND_TOF_HERE
    /*prepare for mcu msg read.*/
    gs_mcu_dev_fd = open(g_mcu_exchg_device, O_RDONLY | O_NOCTTY | O_NDELAY);
    if(gs_mcu_dev_fd < 0)
    {
        DIY_LOG(LOG_WARN, "open %s error:%s. tof distance is fixed at -1.\n", g_mcu_exchg_device, strerror(errno));
        update_tof_distance((uint16_t)-1);
    }
    else
    {
        FD_SET(gs_mcu_dev_fd, &ref_fds);
        if(gs_mcu_dev_fd > fd_max) fd_max = gs_mcu_dev_fd;
    }
#endif

    do 
    {
        int select_ret;

        fds = ref_fds;
        timeout.tv_sec = g_gpio_clock_tick_sec;
        timeout.tv_usec = 0;

        select_ret = select(fd_max + 1, &fds, NULL, NULL, &timeout);
        if(select_ret < 0)
        {
            DIY_LOG(LOG_ERROR, "select error: %s.\n", strerror(errno));
            continue;
        }
        else if(0 == select_ret)
        {
            check_and_process_app_timers();
            continue;
        }

#ifndef MANAGE_LCD_AND_TOF_HERE
        if(FD_ISSET(gs_mcu_dev_fd, &fds))
        {
            int pkt_len;

            pkt_len = read(gs_mcu_dev_fd, gs_mcu_json_packet, MAX_JSON_MESSAGE_LEN);
            if(pkt_len < 0)
            {
                DIY_LOG(LOG_ERROR, "read json packet error:%s\n", strerror(errno));
            }
            else
            {
                process_json_packets(gs_mcu_json_packet, pkt_len, process_tof_json_msg, g_tof_json_override);
            }
        }
#endif

        if(FD_ISSET(gs_gbh_uevent_recv_sock, &fds))
        {
            if((n = recv(device_fd, msg, UEVENT_MSG_LEN, 0)) > 0)
            {
                DIY_LOG(LOG_DEBUG, "recv %d bytes.\n", n);

                /*
                if(n == UEVENT_MSG_LEN)
                    continue;
                */
                if(n > UEVENT_MSG_LEN) n = UEVENT_MSG_LEN;

                msg[n] = '\0';
                msg[n+1] = '\0';

                memset(&parsed_gbh_uevt, 0, sizeof(parsed_gbh_uevt));
                parse_gbh_uevent(msg, n, parse_helper);

                converted_gbh_uevt.valid = false;
                convert_gbh_string_uevent(&parsed_gbh_uevt, &converted_gbh_uevt);
                if(converted_gbh_uevt.valid)
                {
                    process_gbh_uevent(&converted_gbh_uevt);
                }
            }
        }

        check_and_process_app_timers();
    } while(true);

    return 0;
}

