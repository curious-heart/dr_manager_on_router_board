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

typedef struct _double_dirc_list_header_t
{
    struct _double_dirc_list_header_t * prev, *next;
}double_dirc_list_header_t;
typedef enum
{
    DDL_LIST_NO_HEADER_NOLOOP,
    DDL_LIST_NO_HEADER_LOOP,
    DDL_LIST_HEADER_NOLOOP,
    DDL_LIST_HEADER_LOOP,
    //header next and prev points to the 1st and last item respectively, and the rest nodes does not points to header.
    DDL_LIST_HEADER_P_HT_NO_LOOP, 
    DDL_LIST_HEADER_P_HT_LOOP,
}ddl_list_type_e_t;

/*for type DDL_LIST_HEADER_P_HT_NO_LOOP and DDL_LIST_HEADER_P_HT_LOOP, header should be adjust by user.*/
bool insert_a_ddl_node(double_dirc_list_header_t * curr_node, double_dirc_list_header_t *new_node, bool after);
/*for type DDL_LIST_NO_HEADER_NOLOOP and DDL_LIST_NO_HEADER_LOOP, header should be adjust by user.*/
bool delete_a_ddl_node(double_dirc_list_header_t * header, double_dirc_list_header_t *node,
                            ddl_list_type_e_t list_type, bool free_node);

/*a and b should pointer to struce timespec*/
#define TIME_A_IS_EARLIER_THAN_B(a,b) \
    (((a)->tv_sec < (b)->tv_sec) || (((a)->tv_sec == (b)->tv_sec) && ((a)->tv_nsec < (b)->tv_nsec)))

typedef void (*app_timer_function_t)(void* param);
typedef bool (*app_timer_extra_check_func_t)(void* param);
typedef struct
{
    double_dirc_list_header_t ddl_header;
    struct timespec t; //time to action.
    uint32_t interval_ms; //if 0, this is a single timer.
    app_timer_function_t timeout_func; void* to_param;
    app_timer_extra_check_func_t extra_check_func; void *ec_param;
}app_timer_node_s_t;
app_timer_node_s_t * add_a_new_app_timer(uint32_t interval_ms, bool single, 
        app_timer_function_t timeout_func, void* to_param,  app_timer_extra_check_func_t extra_check_func, void* ec_param);
bool delete_an_app_timer(app_timer_node_s_t * tp, bool free_timer);
void check_and_process_app_timers();
void clear_app_timer_list();

#endif // COMMON_TOOL_FUNC_H
