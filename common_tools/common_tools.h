#ifndef COMMON_TOOL_FUNC_H
#define COMMON_TOOL_FUNC_H

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define ROUNDUP_UINT16_TO_10(x) ((unsigned short)(((unsigned short)(((x) + 5) / 10)) * 10))
#define ARRAY_ITEM_CNT(a) sizeof(a)/sizeof((a)[0])

void print_bytes_arr(unsigned char* buf, ssize_t cnt, ssize_t row_num);

#define DATE_STRING_LEN 10 //YYYY-mm-dd
#define TIME_STRING_SEC_LEN 8 //HH:MM:SS
 
/*The caller provide the buffer and len*/
bool get_date_str(char* buf, int len);
bool get_time_str(char* buf, int len);
bool get_datetime_str(char* buf, int len);

/*
 * 'r' for read
 * 'w' for write
 * 'c' for cancel
 * */
char choose_read_or_write();

bool check_time_out_of_curr_time(time_t last_point, time_t time_out);
int fill_timespec_struc(struct timespec * ts, float seconds);

#define MAX_JSON_MESSAGE_LEN 512
typedef void json_msg_handler_func_t(char* msg, int msg_len);
typedef enum
{
    JSON_PROC_RET_OK = 0,
    JSON_PROC_RET_MSG_TOO_LONG = 0x01,
    JSON_PROC_RET_INVALID_MSG = 0x02,
}json_process_result_e_t;

/*This function supports only single thread app; in multi thread app, it can only be called from one fixed thread.*/
json_process_result_e_t process_json_packets(char* pkt, int pkt_len, json_msg_handler_func_t msg_handler, bool override);

#endif // COMMON_TOOL_FUNC_H
