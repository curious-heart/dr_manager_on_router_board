#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "common_tools.h"

enum LOG_LEVEL {
    LOG_DEBUG=0,//调试
    LOG_INFO,   //信息
    LOG_WARN,   //警告
    LOG_ERROR   //错误
};

extern const char* g_log_level_str[];

bool start_log_thread();
void end_log_thread();

#define MAX_LOG_INFO_STR_LEN 320

#define DIY_LOG(level, ...) \
{\
    if(level >= APP_LOG_LEVEL)\
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
            printf("%s [%s]: %s, %d, %s\n", \
                    date_time_str, g_log_level_str[level], __FILE__, __LINE__, __FUNCTION__);\
            printf("\t");\
            printf(log_info_buf);\
        }\
        else\
        {\
            printf("%s [%s]: %s, %d, %s\n",\
                    date_time_str, g_log_level_str[level], __FILE__, __LINE__, __FUNCTION__);\
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
