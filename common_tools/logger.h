#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "common_tools.h"

enum LOG_LEVEL {
    LOG_DEBUG=0,//调试
    LOG_INFO,   //信息
    LOG_WARN,   //警告
    LOG_ERROR,   //错误

    LOG_ONLY_INFO_STR = 0x80,
    LOG_ONLY_INFO_STR_COMP = 0x90,
};
extern uint8_t g_APP_LOG_LEVEL;

extern const char* g_log_level_str[];

bool start_log_thread();
void end_log_thread();

#define MAX_LOG_INFO_STR_LEN 256 

#define DIY_LOG(level, ...) \
{\
    bool only_info_str = false, compact = false;\
    int v_level = level;\
    if(v_level >= LOG_ONLY_INFO_STR_COMP)\
    {\
        only_info_str = true;\
        v_level -= LOG_ONLY_INFO_STR_COMP;\
        compact = true;\
    }\
    else if(v_level >= LOG_ONLY_INFO_STR)\
    {\
        only_info_str = true;\
        v_level -= LOG_ONLY_INFO_STR;\
    }\
    if(v_level >= g_APP_LOG_LEVEL)\
    {\
        char log_info_buf[MAX_LOG_INFO_STR_LEN + 1];\
        char date_time_str[DATE_STRING_LEN + 1 + TIME_STRING_SEC_LEN + 1];\
        int l_p = snprintf(log_info_buf, sizeof(log_info_buf),  __VA_ARGS__);\
        if(!get_datetime_str(date_time_str, sizeof(date_time_str)))\
        {\
            date_time_str[0] = 0;\
        }\
        if(l_p > 0)\
        {\
            if(!only_info_str)\
            {\
                printf("%s [%s]: %s, %d, %s\n", \
                        date_time_str, g_log_level_str[v_level], \
                        __FILE__, __LINE__, __FUNCTION__);\
            }\
            if(!compact)\
            {\
                printf("\t");\
            }\
            printf("%s", log_info_buf);\
        }\
        else\
        {\
            if(!only_info_str)\
            {\
                printf("%s [%s]: %s, %d, %s\n",\
                        date_time_str, g_log_level_str[v_level],\
                        __FILE__, __LINE__, __FUNCTION__);\
            }\
            printf("snprintf error.\n");\
        }\
    }\
}

extern const char* log_dir_str, *log_file_str;
/*
 * Use example:
 *
*/

#endif // LOGGER_H
