#include <stdio.h>
#include <errno.h>

#include "common_tools.h"

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
