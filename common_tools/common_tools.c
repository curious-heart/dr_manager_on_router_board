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
int fill_timespec_struc(struct timespec * ts, float seconds)
{
    int ret;
    time_t sec;

    if(NULL == ts) return -1;

    ret = clock_gettime(CLOCK_REALTIME, ts);
    if(ret != 0)
    {
        return (int)errno;
    }
    sec = (time_t)seconds;
    ts->tv_sec += sec;
    ts->tv_nsec += (long)((seconds - sec) * NANO_FACTOR);
    
    ts->tv_sec += (ts->tv_nsec/NANO_FACTOR);
    ts->tv_nsec = (ts->tv_nsec % NANO_FACTOR);

    return 0;
}

/*This function support only single thread app; in multi thread app, it can only be called from one fixed thread.*/
json_process_result_e_t process_json_packets(char* pkt, int pkt_len, json_msg_handler_func_t msg_handler, bool override)
{
    static char ls_process_buffer[MAX_JSON_MESSAGE_LEN + 1], ls_a_message[MAX_JSON_MESSAGE_LEN + 1];
    static bool ls_buffer_holding = false;
    static int ls_buffer_len = 0, ls_buffer_layer = 0;

    json_process_result_e_t ret = 0;
    char* a_msg = NULL, *last_msg = NULL;
    int pkt_ch_idx = 0, a_msg_len = 0, last_msg_len = 0;
    int layer = -1;

    if(!pkt || pkt_len <= 0) return JSON_PROC_RET_INVALID_MSG;

#define CLEAR_PROCESS_BUFFER {ls_buffer_len = 0; ls_buffer_layer = -1; ls_buffer_holding = false; }

    if(!ls_buffer_holding)
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
                        DIY_LOG(LOG_WARN, "The (jointed) json message is too long and it is discarded.");
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
                        if(!override && msg_handler)
                        {
                            msg_handler(a_msg, a_msg_len);
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
        if(layer != -1)
        {
            DIY_LOG(LOG_WARN, "the (joint) msg is too long, discard its part.");
            ret |= JSON_PROC_RET_MSG_TOO_LONG;
            CLEAR_PROCESS_BUFFER;
            return ret;
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
            if(0 == layer)
            {
                a_msg_len = &pkt[pkt_ch_idx] - a_msg + 1;
                if(a_msg_len > MAX_JSON_MESSAGE_LEN)
                {
                    ret |= JSON_PROC_RET_MSG_TOO_LONG;
                    DIY_LOG(LOG_WARN, "The json message is too long and it is discarded.");
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
        a_msg = last_msg;
        a_msg_len = last_msg_len;;
        if(a_msg_len > MAX_JSON_MESSAGE_LEN)
        {
            ret |= JSON_PROC_RET_MSG_TOO_LONG;
            DIY_LOG(LOG_WARN, "The json message is too long and it is discarded.");
        }
        else
        {
            memcpy(ls_a_message, a_msg, a_msg_len);
            ls_a_message[a_msg_len] = 0;
            msg_handler(ls_a_message, a_msg_len);
        }
    }

    if(layer >= 0)
    {
        /*there is incomplete msg.*/
        if(pkt_len - (a_msg - pkt) > MAX_JSON_MESSAGE_LEN)
        {
            DIY_LOG(LOG_WARN, "The (incomplete) json message is too long and it is discarded.");
        }
        else
        {
            ls_buffer_layer = layer;
            ls_buffer_len = pkt_len - (a_msg - pkt);
            memcpy(ls_process_buffer, a_msg, ls_buffer_len);
            ls_process_buffer[ls_buffer_len] = 0;
            ls_buffer_holding = true;
        }
    }
    else
    {
        ret |= JSON_PROC_RET_INVALID_MSG;
    }

    return ret;
#undef CLEAR_PROCESS_BUFFER
}
