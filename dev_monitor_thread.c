#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>

#include "get_opt_helper.h"
#include "dr_manager.h"

const char* g_dev_monitor_th_desc = "Device-State-Monitor";
dr_device_st_pool_t g_device_st_pool = 
{
    .hot_spot_st = HOTSPOT_DOWN, 
    .bat_chg_st = CHARGER_INIT,
    .bat_lvl = -1,
    .cellular_signal_bars = -1,
    .cellular_mode = CELLULAR_MODE_INIT,
    .sim_card_st = SIM_INIT,
    .wifi_wan_st = WIFI_WAN_INIT,
    .hv_dsp_conn_st = HV_DSP_INIT,
    .expo_volt_kv = -1,
    .expo_am_ua = -1,
    .expo_dura_ms = -1,
    .expo_st = EXPOSURE_ST_INIT,
    .tof_distance = (uint16_t)-1,
};

static pthread_mutex_t gs_dev_st_pool_mutex;

/*This global static var should only be accessed from monitor thread.*/
static dr_device_st_local_buf_t gs_main_dev_st;

/* the last 6 digits of MAC address. */
static char gs_mac_tail6_str[6 + 1] = {0};

/*
 * DO NOT call this function directly because it is not thread safe.
 * Use it as a function point parameter of function access_device_st_pool.
 */

static bool update_dev_st_pool_from_monitor_th(void* d)
{
    bool updated = false;
    mb_reg_val_pair_s_t *pair_arr = NULL;
    int pair_cnt = 0, pair_idx;
    uint16_t info_word = 0;

    if(!d) return updated;

    dr_device_st_local_buf_t *main_dev_st = (dr_device_st_local_buf_t*)d;

    ST_PARAM_SET_UPD(g_device_st_pool, wan_bear, main_dev_st->wan_bear);
    ST_PARAM_SET_UPD(g_device_st_pool, cellular_signal_bars, main_dev_st->cellular_signal_bars);
    ST_PARAM_SET_UPD(g_device_st_pool, cellular_mode, main_dev_st->cellular_mode);
    ST_PARAM_SET_UPD(g_device_st_pool, wifi_wan_st, main_dev_st->wifi_wan_st);
    ST_PARAM_SET_UPD(g_device_st_pool, sim_card_st, main_dev_st->sim_card_st);
    ST_PARAM_SET_UPD(g_device_st_pool, hot_spot_st, main_dev_st->hot_spot_st);

    pair_cnt = get_reg_key_val_pair_to_send_external(&pair_arr);
    if(pair_arr && pair_cnt > 0)
    {
        for(pair_idx = 0; pair_idx < pair_cnt; ++pair_idx)
        {
            switch(pair_arr[pair_idx].reg)
            {
                EVAL_EXT_MB_REG_FRO_ST(pair_arr[pair_idx].val)

                default:
                    break;
            }
        }
    }

    return updated;
}

static void prepare_getting_dev_st()
{
    system("init_at_st.sh");
}

typedef struct
{
    const char* mode_str;
    cellular_mode_t mode;
}cell_mode_mapper_t;
static const cell_mode_mapper_t gs_cell_mode_mapper[] =
{
    {"NOSRV",   CELLULAR_MODE_NOSRV},
    {"3G",   CELLULAR_MODE_3G},
    {"4G",     CELLULAR_MODE_4G},
    {"5G", CELLULAR_MODE_5G},
};
static void map_cell_mode_to_pool(const char* mode_str)
{
    cellular_mode_t mode = CELLULAR_MODE_NOSRV;
    int idx;

    for(idx = 0; idx < ARRAY_ITEM_CNT(gs_cell_mode_mapper); ++idx)
    {
        if(!strcmp(mode_str, gs_cell_mode_mapper[idx].mode_str))
        {
            mode = gs_cell_mode_mapper[idx].mode;
            break;
        }
    }
    gs_main_dev_st.cellular_mode = mode;
}

#define MAX_PIPE_READ_STR_LEN 20
typedef struct
{
    const char* tag;
    data_type_id_t data_type;
    void* value;
}pipe_read_helper_t;
static const char* gs_state_str_cell_conn = "CONNECT";
static const char* gs_state_str_cell_no_conn = "NOCONN";
static char gs_cell_mode_str[MAX_PIPE_READ_STR_LEN + 1];
static char gs_cell_state_str[MAX_PIPE_READ_STR_LEN + 1];
static int gs_cell_signal_bars, gs_cell_rsrp, gs_cell_rsrq, gs_cell_rscp, gs_cell_ecno, gs_cell_sinr, gs_cell_rssi;
static char gs_ori_cell_mode_str[MAX_PIPE_READ_STR_LEN + 1];
static pipe_read_helper_t gs_pipe_read_helper[] =
{
    {"cell_mode", type_charp,  gs_cell_mode_str},
    {"cell_state", type_charp, gs_cell_state_str},
    {"signal_bars", type_int, &gs_cell_signal_bars},
    {"rsrp", type_int, &gs_cell_rsrp},
    {"rsrq", type_int, &gs_cell_rsrq},
    {"rscp", type_int, &gs_cell_rscp},
    {"ecno", type_int, &gs_cell_ecno},
    {"sinr", type_int, &gs_cell_sinr},
    {"rssi", type_int, &gs_cell_rssi},
    {"ori_cell_mode", type_charp, gs_ori_cell_mode_str},
};
#define NO_SIGNAL_VALUE (-200)
static void init_cell_info_var()
{
    gs_cell_mode_str[0] = gs_cell_state_str[0] = '\0';
    gs_cell_signal_bars = 0;
    gs_cell_rsrp = gs_cell_rsrq = gs_cell_rscp = gs_cell_ecno = gs_cell_sinr = gs_cell_rssi = NO_SIGNAL_VALUE;

    gs_ori_cell_mode_str[0] = '\0';
}

static void get_cellular_st(bool debug_flag)
{
    static const char* get_cell_info_sh = "get_cell_info.sh";
    static const char* get_cell_info_sh_debug = "get_cell_info.sh -d";
    FILE* r_stream = NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;
    int tag_len, idx;

    if(debug_flag)
    {
        r_stream = popen(get_cell_info_sh_debug, "r");
    }
    else
    {
        r_stream = popen(get_cell_info_sh, "r");
    }
    if(NULL == r_stream)
    {
        DIY_LOG(LOG_ERROR, "popen %s error.\n", get_cell_info_sh);
        return;
    }

    init_cell_info_var();
    DIY_LOG(LOG_DEBUG, "cellular info line:\n");
    while((nread = getline(&line, &len, r_stream)) != -1)
    {
        line[strcspn(line, "\r\n")] = '\0';
        DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "%s\n", line);
        for(idx = 0; idx < ARRAY_ITEM_CNT(gs_pipe_read_helper); ++idx)
        {
            tag_len = strlen(gs_pipe_read_helper[idx].tag);
            if(!strncmp(gs_pipe_read_helper[idx].tag, line, tag_len) && (nread > tag_len + 1))
            {
                switch(gs_pipe_read_helper[idx].data_type)
                {
                    case type_charp:
                        CONVERT_FUNC_STRCPY((char*)gs_pipe_read_helper[idx].value, &line[tag_len+1]);//skip ":"
                        break;
                    case type_int:
                    default:
                        CONVERT_FUNC_ATOI((*(int*)(gs_pipe_read_helper[idx].value)), &line[tag_len+1]);//skip ":"
                        break;
                }
                break;
            }
        }
    }

    /*cell mode.*/
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "cellular cell mode str: %s\n", gs_cell_mode_str);
    map_cell_mode_to_pool(gs_cell_mode_str);
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "cellular mode: %d\n", gs_main_dev_st.cellular_mode);

    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "cellular conn state str: %s\n", gs_cell_state_str);
    /*wan connection.*/
    if(!strcmp(gs_cell_state_str, gs_state_str_cell_conn)) 
    {
        gs_main_dev_st.wan_bear |= WWAN_BEAR_CELLULAR;
    }
    else
    {
        gs_main_dev_st.wan_bear &= ~WWAN_BEAR_CELLULAR;
        if(strcmp(gs_cell_state_str, gs_state_str_cell_no_conn))
        {
            /*cell_state str is neither connect or noconn, then there is no service.*/
            gs_cell_signal_bars = 0;
            gs_main_dev_st.cellular_mode = CELLULAR_MODE_NOSRV;
        }
    }
    DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR, "wan bear: 0x%02X\n", gs_main_dev_st.wan_bear);

    /* since rsrp and other signal values are already obtained, in future, there can be more accurate signal-bar map 
     * implement here. Now, we just use the bar from shell script, which is much coarse...
     */
    /*cell_state, i.e. signal bars.*/
    gs_main_dev_st.cellular_signal_bars = gs_cell_signal_bars;

    free(line);
    pclose(r_stream);
}

static void get_sim_card_st(bool debug_flag)
{
    static const char* tag_str= "imsi:";
    static const int tag_len = 5; //len of tag_str;
    static const char* imsi_char_set = "0123456789";
    static const int imsi_len = 15;
    static const char* get_sim_card_st_sh = "get_sim_card_st.sh";
    static const char* get_sim_card_st_sh_debug = "get_sim_card_st.sh -d";
    FILE* r_stream = NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    if(debug_flag)
    {
        r_stream = popen(get_sim_card_st_sh_debug, "r");
    }
    else
    {
        r_stream = popen(get_sim_card_st_sh, "r");
    }
    if(NULL == r_stream)
    {
        DIY_LOG(LOG_ERROR, "popen %s error.\n", get_sim_card_st_sh);
        return;
    }
    gs_main_dev_st.sim_card_st = SIM_NO_CARD;
    while((nread = getline(&line, &len, r_stream)) != -1)
    {
        int spn;
        spn = strspn(&line[tag_len], imsi_char_set);
        if(!strncmp(tag_str, line, tag_len) && (nread >= tag_len + imsi_len) && (spn == imsi_len))
        {
            DIY_LOG(LOG_DEBUG, "%s\n", line);
            gs_main_dev_st.sim_card_st = SIM_CARD_NORM;
            break;
        }
    }
    if(SIM_CARD_NORM != gs_main_dev_st.sim_card_st)
    {
        DIY_LOG(LOG_DEBUG, "NO SIM card.\n");
    }

    free(line);
    pclose(r_stream);
}

static void get_wifi_info(bool static_info, bool debug_flag)
{
    static const char* get_static_sh = "get_wifi_static_info.sh";
    static const char* get_static_sh_debug= "get_wifi_static_info.sh -d";
    static const char* get_dynamic_sh = "get_wifi_dynamic_info.sh";
    static const char* get_dynamic_sh_debug= "get_wifi_dynamic_info.sh -d";
    const char* curr_sh;

    FILE* r_stream = NULL;
    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    if(static_info)
    {
        curr_sh = debug_flag ? get_static_sh : get_static_sh_debug;
    }
    else
    {
        curr_sh = debug_flag ? get_dynamic_sh : get_dynamic_sh_debug;
    }

    r_stream = popen(curr_sh, "r");
    if(NULL == r_stream)
    {
        DIY_LOG(LOG_ERROR, "popen %s error.\n", curr_sh);
        return;
    }
    if((nread = getline(&line, &len, r_stream)) != -1)
    {
        if(static_info)
        {
            snprintf(gs_mac_tail6_str, sizeof(gs_mac_tail6_str), "%s", line);
            DIY_LOG(LOG_INFO, "mac addr last 6 digits:%s\n", gs_mac_tail6_str);
        }
        else
        {
            /*dynamic info: assoc_number,is_client,client_signal,client_signal_bars*/
            int assoc_number = 0, is_client = 0, client_signal = 0, client_signal_bars = 0;
            int line_len = strlen(line);
            char* n_s;

            do
            {
                /* sscanf may not work as I expected... maybe in future when I'm more fammiliar with this function,
                 * I can use it.
                   sscanf(line, "%d,%d,%d,%d", &assoc_number, &is_client, &client_signal, &client_signal_bars);
                 */
                n_s = line;
                CONVERT_FUNC_ATOI(assoc_number, n_s);
                n_s = strstr(n_s, ","); if((n_s != NULL) && (n_s + 1 < line + line_len)) ++n_s; else break; //skip the ","
                CONVERT_FUNC_ATOI(is_client, n_s);
                n_s = strstr(n_s, ","); if((n_s != NULL) && (n_s + 1 < line + line_len)) ++n_s; else break;//skip the ","
                CONVERT_FUNC_ATOI(client_signal, n_s);
                n_s = strstr(n_s, ","); if((n_s != NULL) && (n_s + 1 < line + line_len)) ++n_s; else break;//skip the ","
                CONVERT_FUNC_ATOI(client_signal_bars, n_s);

                break;
            }while(true);

            gs_main_dev_st.hot_spot_st = assoc_number;
            if(0 == is_client)
            {
                gs_main_dev_st.wifi_wan_st = WIFI_WAN_DISCONNECTED;
                gs_main_dev_st.wan_bear &= ~WWAN_BEAR_WIFI;
            }
            else
            {
                gs_main_dev_st.wifi_wan_st = (wifi_wan_st_t)client_signal_bars;
                gs_main_dev_st.wan_bear |= WWAN_BEAR_WIFI;
            }

            DIY_LOG(LOG_INFO,
                "Wi-Fi dynamic info: association host nubmer:%d, client_conn:%d, client_signal:%d, client_signal_bars:%d\n",
                assoc_number, is_client, client_signal, client_signal_bars);
        }
    }

    free(line);
    pclose(r_stream);
}

const char* get_wifi_mac_tail6()
{
    static bool already_got = false;

    if(!already_got)
    {
        get_wifi_info(true, false);
        if(strlen(gs_mac_tail6_str) == 6)
        {
            already_got = true;
            return gs_mac_tail6_str;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return gs_mac_tail6_str;
    }
}

void* dev_monitor_thread_func(void* arg)
{
    dev_monitor_th_parm_t * parm = (dev_monitor_th_parm_t*) arg;
    bool sh_debug_flag;

    if(NULL == parm)
    {
        DIY_LOG(LOG_ERROR, "Arguments passed to %s thread is NULL. Thread exit.\n", g_dev_monitor_th_desc);
        return NULL;
    }

    sh_debug_flag = parm->sh_script_debug;

    prepare_getting_dev_st();

    DIY_LOG(LOG_INFO, "%s therad starts, with sch period %f seconds!\n", 
            g_dev_monitor_th_desc, parm->sch_period);
    while(true)
    {
        get_cellular_st(sh_debug_flag);
        get_sim_card_st(sh_debug_flag);
        get_wifi_info(false, sh_debug_flag);

        if(access_device_st_pool(pthread_self(), g_dev_monitor_th_desc, update_dev_st_pool_from_monitor_th,
                                              &gs_main_dev_st))
        {
            update_lcd_display(pthread_self(), g_dev_monitor_th_desc);
        }

        usleep(parm->sch_period * 1000000);
    }
    return NULL;
}

int init_dev_st_pool_mutex()
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    return pthread_mutex_init(&gs_dev_st_pool_mutex, &attr);
}

int destroy_dev_st_pool_mutex()
{
    return pthread_mutex_destroy(&gs_dev_st_pool_mutex);
}

bool access_device_st_pool(pthread_t pth_id, const char* desc, access_device_status_pool_func_t func, void* arg)
{
    /* every thread that may update device status should have its own private function to write 
     * the global pool, and use this function to call that private funciton.
     * */
    bool upd = false;

    if(func)
    {
        int ret;
        ret = pthread_mutex_lock(&gs_dev_st_pool_mutex);
        if(EOWNERDEAD == ret)
        {
            pthread_mutex_consistent(&gs_dev_st_pool_mutex);
        }
        upd = func(arg);
        pthread_mutex_unlock(&gs_dev_st_pool_mutex);
    }

    if(upd)
    {
        DIY_LOG(LOG_DEBUG, "thread %u ", (uint32_t)pth_id);
        if(desc)
        {
            DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR_COMP, "%s ", desc);
        }
        DIY_LOG(LOG_DEBUG + LOG_ONLY_INFO_STR_COMP, "finished updating device status pool.\n");
    }
    return upd;
}

