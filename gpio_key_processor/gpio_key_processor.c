#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <signal.h>

#include "logger.h"
#include "get_opt_helper.h"
#include "gpio_key_processor.h"

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
   [key_reset] = dose_adjust_key_handler, //exp_start_key_handler, //reset_key_handler,
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

    DIY_LOG(LOG_INFO, "Converted gbh uevent info:\n");
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "key/gpio: %s\n", g_key_gpio_name_list[evt->key_gpio]);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "action: %s\n", g_key_gpio_act_name_list[evt->action]);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "seen: %u\n", evt->seen);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "seqnum: %u\n", evt->seqnum);
    if(!gs_key_gpio_handler_arr[evt->key_gpio])
    {
        DIY_LOG(LOG_WARN, "The key/gpio %s has no handler.\n", g_key_gpio_name_list[evt->key_gpio]);
        return;
    }
    gs_key_gpio_handler_arr[evt->key_gpio](evt);
}

static int gs_gbh_uevent_recv_sock = -1;
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

    device_fd = open_gbh_uevent_recv_socket();
    if(device_fd < 0)
    {
        return -1;
    }
    gs_gbh_uevent_recv_sock  = device_fd;

    if(!begin_key_event_handle())
    {
        return -1;
    }

    do 
    {
        while((n = recv(device_fd, msg, UEVENT_MSG_LEN, 0)) > 0)
        {
            DIY_LOG(LOG_DEBUG, "recv %d bytes.\n", n);

            if(n == UEVENT_MSG_LEN)
                continue;

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
    } while(true);

    return 0;
}

