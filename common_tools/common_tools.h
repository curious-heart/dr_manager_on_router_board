#ifndef COMMON_TOOL_FUNC_H
#define COMMON_TOOL_FUNC_H

#include <unistd.h>
#include <stdbool.h>

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
#endif // COMMON_TOOL_FUNC_H
