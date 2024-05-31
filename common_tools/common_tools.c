#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common_tools.h"
#include "logger.h"

void print_bytes_arr(unsigned char* buf, ssize_t cnt, ssize_t row_num)
{
    ssize_t idx = 0;
    while(idx < cnt)
    {
        printf("0x%02X, ", buf[idx++]);
        if(idx % row_num == 0)
        {
            printf("\n");
        }

    }
    printf("\n");
}

/*The caller provide the buffer and len*/
static bool get_fmt_datetime_str(char* buf, int len, const char* fmt_str)
{
    struct tm tm_s, *tm_p;
    time_t t;

    t = time(NULL);
    tm_p = localtime_r(&t, &tm_s);
    if(tm_p == NULL) 
    {
        return false;
    }

    if(strftime(buf, len, fmt_str, tm_p) == 0) 
    {
        return false;
    }

    return true;
}

bool get_date_str(char* buf, int len)
{
    if(!buf || (len < DATE_STRING_LEN + 1))
    {
        return false;
    }
    return get_fmt_datetime_str(buf, len, "%Y-%m-%d");
}

bool get_time_str(char* buf, int len)
{
    if(!buf || (len < TIME_STRING_SEC_LEN + 1))
    {
        return false;
    }
    return get_fmt_datetime_str(buf, len, "%H:%M:%S");
}

bool get_datetime_str(char* buf, int len)
{
    /*There is a space between date and time string.*/
    if(!buf || (len < DATE_STRING_LEN + 1 + TIME_STRING_SEC_LEN + 1))
    {
        return false;
    }
    return get_fmt_datetime_str(buf, len, "%Y-%m-%d %H:%M:%S");
}

/*
 * 'r' for read
 * 'w' for write
 * 'c' for cancel
 * */
char choose_read_or_write()
{
#define MAX_CHAR_NUM_READ 3
    char op = 0;
    bool end = false;
    char r_buf[MAX_CHAR_NUM_READ + 1];

    while(!end)
    {
        printf("please choose an operation by input a character:\n");
        printf("'r': read\n");
        printf("'w': write\n");
        printf("'c': cancel\n");
        fgets(r_buf, sizeof(r_buf), stdin);
        sscanf(r_buf, "%c", &op);
        printf("\n");
        switch(op)
        {
            case 'r':
            case 'w':
            case 'c':
                end = true;
                break;
            default:
                printf("read op: 0x%02X: %c, not valid.\n", op, op);
                ;
        }
    }
    return op;
}

bool check_time_out_of_curr_time(time_t last_point, time_t time_out)
{
    return (time(NULL) - last_point >= time_out);
}

#define NANO_FACTOR  1000000000
int fill_timespec_with_known_val(struct timespec * ts, float seconds)
{
    time_t sec;

    if(NULL == ts) return -1;

    sec = (time_t)seconds;
    ts->tv_sec += sec;
    ts->tv_nsec += (long)((seconds - sec) * NANO_FACTOR);
    
    ts->tv_sec += (ts->tv_nsec/NANO_FACTOR);
    ts->tv_nsec = (ts->tv_nsec % NANO_FACTOR);
    return 0;
}

int fill_timespec_struc(struct timespec * ts, float seconds)
{
    int ret;

    if(NULL == ts) return -1;

    ret = clock_gettime(CLOCK_REALTIME, ts);
    if(ret != 0)
    {
        return (int)errno;
    }

    return fill_timespec_with_known_val(ts, seconds);
}

/*This function supports only single thread app; in multi thread app, it can only be called from one fixed thread.*/
json_process_result_e_t process_json_packets(char* pkt, int pkt_len, json_msg_handler_func_t msg_handler, bool override)
{
    static char ls_process_buffer[MAX_JSON_MESSAGE_LEN + 1], ls_a_message[MAX_JSON_MESSAGE_LEN + 1];
    static bool ls_buffer_holding = false;
    static int ls_buffer_len = 0, ls_buffer_layer = 0;

    json_process_result_e_t ret = JSON_PROC_RET_OK;
    char* a_msg = NULL, *last_msg = NULL;
    int pkt_ch_idx = 0, a_msg_len = 0, last_msg_len = 0;
    int layer = -1;

    if(!pkt || pkt_len <= 0) return JSON_PROC_RET_INVALID_MSG;

#define CLEAR_PROCESS_BUFFER {ls_buffer_holding = false; ls_buffer_len = 0; ls_buffer_layer = -1;}

    if(ls_buffer_holding)
    {
        layer = ls_buffer_layer;
        while(pkt_ch_idx < pkt_len)
        {
            if('{' == pkt[pkt_ch_idx])
            {
                ++layer;
            }
            else if('}' == pkt[pkt_ch_idx])
            {
                if(0 == layer)
                {
                    if(pkt_ch_idx + 1 + ls_buffer_len > MAX_JSON_MESSAGE_LEN)
                    {
                        DIY_LOG(LOG_WARN, "The (jointed) json message is too long and it is discarded.\n");
                        ret |= JSON_PROC_RET_MSG_TOO_LONG;
                        CLEAR_PROCESS_BUFFER;
                    }
                    else
                    {
                        /*join the two parts.*/
                        memcpy(&ls_process_buffer[ls_buffer_len], pkt, pkt_ch_idx + 1);
                        ls_buffer_len += pkt_ch_idx + 1;
                        ls_process_buffer[ls_buffer_len] = 0;
                        last_msg = a_msg = ls_process_buffer;
                        last_msg_len = a_msg_len = ls_buffer_len;
                        if(!override)
                        {
                            if(msg_handler) msg_handler(a_msg, a_msg_len);
                            CLEAR_PROCESS_BUFFER;
                        }
                    }
                    --layer;
                    ++pkt_ch_idx;
                    break;
                }
                else
                {
                    --layer;
                }
            }

            ++pkt_ch_idx;
        }
    }

    while(pkt_ch_idx < pkt_len)
    {
        if('{' == pkt[pkt_ch_idx])
        {
            ++layer;
            if(0 == layer)
            {
                a_msg= &pkt[pkt_ch_idx]; 
            }
        }
        else if('}' == pkt[pkt_ch_idx])
        {
            if(layer < 0)
            {
                DIY_LOG(LOG_WARN, "part of incomplete msg, discard it.\n");
                ++pkt_ch_idx;
                layer = -1;
                continue;
            }
            else if(0 == layer)
            {
                a_msg_len = &pkt[pkt_ch_idx] - a_msg + 1;
                if(a_msg_len > MAX_JSON_MESSAGE_LEN)
                {
                    ret |= JSON_PROC_RET_MSG_TOO_LONG;
                    DIY_LOG(LOG_WARN, "The json message is too long and it is discarded.\n");
                }
                else
                {
                    last_msg = a_msg;
                    last_msg_len = a_msg_len;

                    if(!override && msg_handler)
                    {
                        memcpy(ls_a_message, a_msg,  a_msg_len);
                        ls_a_message[a_msg_len] = 0;
                        msg_handler(ls_a_message, a_msg_len);
                    }
                }
            }
            --layer;
        }

        ++pkt_ch_idx;
    }

    if(override && msg_handler && (last_msg_len > 0) && last_msg)
    {
        /*only process the last complete message in this packet.*/
        if(last_msg_len > MAX_JSON_MESSAGE_LEN)
        {
            ret |= JSON_PROC_RET_MSG_TOO_LONG;
            DIY_LOG(LOG_WARN, "The json message is too long and it is discarded.\n");
        }
        else
        {
            memcpy(ls_a_message, last_msg, last_msg_len);
            ls_a_message[last_msg_len] = 0;
            msg_handler(ls_a_message, last_msg_len);
        }
        CLEAR_PROCESS_BUFFER;
    }

    if(layer >= 0)
    {
        /*there is incomplete msg.*/
        char* part_msg = (a_msg ? a_msg : pkt);
        int part_msg_len = (a_msg ? (pkt_len - (a_msg - pkt)) : pkt_len);
        if(ls_buffer_len + part_msg_len > MAX_JSON_MESSAGE_LEN)
        {
            ret |= JSON_PROC_RET_MSG_TOO_LONG;
            CLEAR_PROCESS_BUFFER;
            DIY_LOG(LOG_WARN, "The (incomplete) json message is too long and it is discarded.");
        }
        else
        {
            ls_buffer_layer = layer;
            memcpy(&ls_process_buffer[ls_buffer_len], part_msg, part_msg_len);
            ls_buffer_len += part_msg_len;
            ls_process_buffer[ls_buffer_len] = 0;
            ls_buffer_holding = true;
        }
    }
    else if(pkt[pkt_ch_idx - 1] != '}')
    {
        ret |= JSON_PROC_RET_INVALID_MSG;
    }

    return ret;
#undef CLEAR_PROCESS_BUFFER
}

/*for type DDL_LIST_HEADER_P_HT_NO_LOOP and DDL_LIST_HEADER_P_HT_LOOP, header should be adjust by user.*/
bool insert_a_ddl_node(double_dirc_list_header_t * curr_node, double_dirc_list_header_t *new_node, bool after)
{
    if(!curr_node || !new_node) return false;

    if(after)
    {
        new_node->prev = curr_node;
        new_node->next = curr_node->next;
        curr_node->next = new_node;
        if(new_node->next) new_node->next->prev = new_node;
    }
    else
    {
        new_node->prev = curr_node->prev;
        new_node->next = curr_node;
        curr_node->prev = new_node;
        if(new_node->prev) new_node->prev->next = new_node;
    }
    return true;
}

/*for type DDL_LIST_NO_HEADER_NOLOOP and DDL_LIST_NO_HEADER_LOOP, header should be adjust by user.*/
bool delete_a_ddl_node(double_dirc_list_header_t * header, double_dirc_list_header_t *node,
                            ddl_list_type_e_t list_type, bool free_node)
{
    double_dirc_list_header_t * p, *first, *last;

    if(!node || !header) return false;

#define DELETE_A_NODE(n) \
{\
    if((n)->prev) (n)->prev->next = (n)->next;\
    if((n)->next) (n)->next->prev = (n)->prev;\
}
    switch(list_type)
    {
        case DDL_LIST_NO_HEADER_NOLOOP:
            p = header;
            while(p && p != node) p = p->next;
            if(!p) return false;

            DELETE_A_NODE(p);
            node->next = node->prev = NULL;
            break;

        case DDL_LIST_NO_HEADER_LOOP:
            p = header;
            if(p == node) break; else p = p->next;
            while(p != header && p != node) p = p->next;
            if(p == header) return false;

            DELETE_A_NODE(p);
            node->next = node->prev = node;
            break;

        case DDL_LIST_HEADER_NOLOOP:
            p = header->next;
            while(p && p != node) p = p->next;
            if(!p) return false;

            DELETE_A_NODE(p);
            node->next = node->prev = NULL;
            break;

        case DDL_LIST_HEADER_LOOP:
            p = header->next;
            while(p != header && p!= node) p = p->next;
            if(p == header) return false;

            DELETE_A_NODE(p);
            node->next = node->prev = node;
            break;

        case DDL_LIST_HEADER_P_HT_NO_LOOP: 
            first = header->next;
            last = header->prev;
            p = header->next;
            while(p && p != node) p = p->next;
            if(!p) return false;

            DELETE_A_NODE(p);
            if(p == first) header->next = p->next;
            if(p == last) header->prev = p->prev;
            node->next = node->prev = NULL;
            break;

        case DDL_LIST_HEADER_P_HT_LOOP:
        default:
            first = header->next;
            last = header->prev;
            p = header->next;
            if(!p) return false;
            if(p == node) break; else p = p->next;
            while(p != header->next && p != node) p = p->next;
            if(p == header->next) return false;

            DELETE_A_NODE(p);
            if(p == first) header->next = p->next;
            if(p == last) header->prev = p->prev;
            node->next = node->prev = node;
            break;
    }

    if(free_node) free(node);
    return true;
}

/*timer list type: DDL_LIST_HEADER_LOOP*/
static double_dirc_list_header_t gs_app_timer_list = {&gs_app_timer_list, &gs_app_timer_list};
static void insert_app_timer_node(app_timer_node_s_t * new_timer)
{
    double_dirc_list_header_t * p;
    app_timer_node_s_t * timer;

    if(!new_timer) return;

    p = gs_app_timer_list.next;
    while(p != &gs_app_timer_list)
    {
        timer = (app_timer_node_s_t*)p;
        if(TIME_A_IS_EARLIER_THAN_B(&(timer->t), &(new_timer->t))) p = p->next;
        else break;
    }
    p = p->prev;
    insert_a_ddl_node(p, (double_dirc_list_header_t *)new_timer, true);
}

app_timer_node_s_t * add_a_new_app_timer(uint32_t interval_ms, bool single, 
        app_timer_function_t timeout_func, void* to_param,  app_timer_extra_check_func_t extra_check_func, void* ec_param)
{
    int err;
    app_timer_node_s_t * new_timer = (app_timer_node_s_t*)malloc(sizeof(app_timer_node_s_t));
    if(!new_timer)
    {
        DIY_LOG(LOG_ERROR, "alloc new timer error!\n");
        return NULL;
    }
    err = fill_timespec_struc(&(new_timer->t), ((float)interval_ms)/1000);
    if(err != 0)
    {
        DIY_LOG(LOG_ERROR, "get clock time error %d:%s, so create timer fails.\n", err, strerror(err));
        free(new_timer);
        return NULL;
    }
    new_timer->timeout_func = timeout_func; new_timer->to_param = to_param;
    new_timer->extra_check_func = extra_check_func; new_timer->ec_param = ec_param;
    new_timer->ddl_header.next = new_timer->ddl_header.prev = (double_dirc_list_header_t*)new_timer;
    new_timer->interval_ms = (single ? 0 : interval_ms);

    insert_app_timer_node(new_timer);

    return new_timer;
}

bool delete_an_app_timer(app_timer_node_s_t * tp, bool free_timer)
{
    return delete_a_ddl_node(&gs_app_timer_list, (double_dirc_list_header_t*)tp, DDL_LIST_HEADER_LOOP, free_timer);
}

void check_and_process_app_timers()
{
    int ret;
    struct timespec curr_time;
    app_timer_node_s_t *timer;
    double_dirc_list_header_t * p, *next;;

    p = gs_app_timer_list.next;
    if(p == &gs_app_timer_list) return;

    ret = clock_gettime(CLOCK_REALTIME, &curr_time);
    if(ret != 0)
    {
        DIY_LOG(LOG_ERROR, "get current time fail in timer process: %d, %s\n", errno, strerror(errno));
        return;
    }

    while(p != &gs_app_timer_list)
    {
        timer = (app_timer_node_s_t*)p;
        if(TIME_A_IS_EARLIER_THAN_B(&(timer->t), &curr_time))
        {
            bool extra_check = true;
            if(timer->extra_check_func) extra_check = timer->extra_check_func(timer->ec_param);
            if(extra_check && timer->timeout_func) timer->timeout_func(timer->to_param);

            next = p->next;
            if(0 == timer->interval_ms)
            {
                /*single shot timer*/
                delete_an_app_timer(timer, true);
            }
            else
            {
                delete_an_app_timer(timer, false);

                timer->t = curr_time;
                fill_timespec_with_known_val(&(timer->t), (((float)(timer->interval_ms))/1000));
                insert_app_timer_node(timer);
            }
            p = next;
        }
        else
        {
            /*the list is in ascending order, so when encounting the first timer yet expires, just exit.*/
            break;
        }
    }
}
void clear_app_timer_list()
{
    double_dirc_list_header_t * p = gs_app_timer_list.next;

    while(p != &gs_app_timer_list)
    {
        delete_an_app_timer((app_timer_node_s_t*)p, true);
        p = gs_app_timer_list.next;
    }
}
