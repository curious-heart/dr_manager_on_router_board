#ifndef _GET_OPT_HELPER_H_
#define _GET_OPT_HELPER_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdint.h>
#include <stdio.h>
#include <getopt.h>
#include "logger.h"

#undef TYPE_ID
#define TYPE_ID(t, name) type_##name,
#define TYPE_LIST \
{\
    TYPE_ID(bool, bool)\
    TYPE_ID(const char*, c_charp) TYPE_ID(char*, charp)\
    TYPE_ID(char, char)  TYPE_ID(short, short)  TYPE_ID(int, int)  \
    TYPE_ID(int8_t, int8_t)  TYPE_ID(int16_t, int16_t) TYPE_ID(int32_t, int32_t) \
    TYPE_ID(uint8_t, uint8_t)  TYPE_ID(uint16_t, uint16_t)  TYPE_ID(uint32_t, uint32_t) \
    TYPE_ID(float, float)  TYPE_ID(double, double) \
}
typedef enum TYPE_LIST data_type_id_t;

#undef TYPE_ID
#define TYPE_ID(t, name) #name
extern const char* type_id_str_arr[];

#define SHOULD_BE_NON_NULL_STR(x) (((x) && (x)[0]) ? 1 : 0)
#define SHOULD_BE_NON_NULL_STR_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be non-empty string.")
#define SHOULD_BE_NE_0(x) ((x) != 0 ? 1 : 0)
#define SHOULD_BE_NE_0_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be <> 0.")
#define SHOULD_BE_EQ_0(x) ((x) == 0 ? 1 : 0)
#define SHOULD_BE_EQ_0_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be 0.")
#define SHOULD_BE_GT_0(x) ((x) > 0 ? 1 : 0)
#define SHOULD_BE_GT_0_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be > 0.")
#define SHOULD_BE_GE_0(x) ((x) >= 0 ? 1 : 0)
#define SHOULD_BE_GE_0_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be >= 0.")
#define SHOULD_BE_LT_0(x) ((x) < 0 ? 1 : 0)
#define SHOULD_BE_LT_0_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be < 0.")
#define SHOULD_BE_LE_0(x) ((x) <= 0 ? 1 : 0)
#define SHOULD_BE_LE_0_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be <= 0.")
#define SHOULD_BE_IN_INCLUDED(x, l, r) (((l) <= (x)) && ((x) <= (r)) ? 1 : 0)
#define SHOULD_BE_IN_INCLUDED_INT_LOG(l, r) \
    DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be in range [%d, %d].", (l), (r))
#define SHOULD_BE_IN_INCLUDED_FLOAT_LOG(l, r) \
    DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be in range [%f, %f].", (l), (r))
#define SHOULD_BE_IN_EXCLUDED(x, l, r) (((l) < (x)) && ((x) < (r)) ? 1 : 0)
#define SHOULD_BE_IN_EXCLUDED_INT_LOG(l, r) \
    DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be in range (%d, %d).", (l), (r))
#define SHOULD_BE_IN_EXCLUDED_FLOAT_LOG(l, r) \
    DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be in range (%f, %f).", (l), (r))
#define VALID_IP_STR(ip_str, in_addr_ptr) (inet_aton(ip_str, in_addr_ptr))
#define INVALID_IP_STR_LOG(ip_str) DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " not a valid IP address.")

#define CHECK_COM_PARITY(c) ((c == 'N' || c == 'E' || c == 'O') ? 1: 0)
#define CHECK_COM_PARITY_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be of \'N\' or \'E\' or \'O\'.")
#define MIN_SERIAL_DATA_BITS 5
#define MAX_SERIAL_DATA_BITS 8
#define MIN_SERIAL_STOP_BITS 1
#define MAX_SERIAL_STOP_BITS 2

#define MAX_OPT_STR_SIZE 32 
#define MAX_IP_ADDR_STR_SIZE 17 

#define TYPE_STR_OF_VAR(type_id) type_id_str_arr[type_id]
/* Be careful when using the following macro CONVERT_FUNC.
 * Improper use may leads to subtle error.
 */
#define CONVERT_FUNC_ATOBOOL(var, value) (var) = (bool)strtol((value), NULL, 0)
#define CONVERT_FUNC_STRSHAR(var, value) (var) = (value)
#ifndef _Generic
#define CONVERT_FUNC_STRCPY(var, value) snprintf((var), MAX_OPT_STR_SIZE, "%s", (value))
#define CONVERT_FUNC_ATOC(var, value) (var) = (char)((value))[0]
#define CONVERT_FUNC_ATOS(var, value) (var) = (short)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOI(var, value) (var) = (int)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOINT8(var, value) (var) = (int8_t)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOINT16(var, value) (var) = (int16_t)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOINT32(var, value) (var) = (int32_t)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOUINT8(var, value) (var) = (uint8_t)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOUINT16(var, value) (var) = (uint16_t)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOUINT32(var, value) (var) = (uint32_t)strtol((value), NULL, 0)
#define CONVERT_FUNC_ATOF(var, value) (var) = strtof((value), NULL)
#define CONVERT_FUNC_ATODB(var, value) (var) = strtod((value), NULL)

#else
#define CONVERT_FUNC(var, value) _Generic((var),\
        char*: snprintf((var), MAX_OPT_STR_SIZE, "%s", (value)), \
        char: (var) = (char)((value))[0],\
        short: (var) = (short)strtol((value), NULL, 0),\
        int: (var) = (int)strtol((value), NULL, 0),\
        uint8_t: (var) = (uint8_t)strtol((value), NULL, 0),\
        uint16_t: (var) = (uint16_t)strtol((value), NULL, 0),\
        uint32_t: (var) = (uint32_t)strtol((value), NULL, 0),\
        float: (var) = (float)atof((value)),\
        double: (var) = (double)atof((value)),\
        default: (var) = (int)strtol((value), NULL, 0)
#define CONVERT_FUNC_STRCPY(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOC(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOS(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOI(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOINT8(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOINT16(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOINT32(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOUINT8(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOUINT16(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOUINT32(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOF(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATODB(var, value) CONVERT_FUNC(var, value)
#endif
/*
 * This macro should be used with getoptlong. refer to option_configuration_process.c for the usage.
 */
#define OPT_CHECK_AND_DRAW(option_str,check_str, draw_value, value_check, check_log, type_id) \
if(!strcmp(option_str, check_str))\
{\
    if(optarg && optarg[0] != ':' && optarg[0] != '?')\
    {\
        draw_value;\
        if(!value_check)\
        {\
            DIY_LOG(LOG_ERROR, "The --%s param value %s is invalid:", check_str, optarg);\
            check_log;\
            DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, "\n");\
            options_valid = false;\
        }\
    }\
    else\
    {\
        DIY_LOG(LOG_ERROR,\
                "if option --%s are provided, an parameter of type %s is necessary.\n", \
                check_str, TYPE_STR_OF_VAR(type_id));\
        options_valid = false;\
    }\
    break;\
}

typedef enum
{
    OPTION_PROCESS_GOON = 0,
    OPTION_PROCESS_EXIT_NORMAL,
    OPTION_PROCESS_EXIT_ERROR,
}option_process_ret_t;

typedef enum
{
    DEFAULT_PARAMS,
    WORKING_PARAMS,
}default_or_working_params_ind_t;

typedef struct
{
    const char* desc;
    const void* def_val_ptr;
    const void* work_val_ptr;
    data_type_id_t type_ind;
}cmd_opt_desc_val_t;

/*The definition of long_opt_arr and opt_desc_arr MUST be in accordance. Refer to the usage in option_configuration_process.c*/
void construct_short_opt_chars_str(char* buf, struct option * long_opt_arr, int arr_cnt);
void print_app_cmd_line_usage(const char* app_name, struct option* long_opt_arr, cmd_opt_desc_val_t* opt_desc_arr,
        int arr_cnt);
void print_app_cmd_line_parameters(default_or_working_params_ind_t ind, struct option* long_opt_arr,
                                   cmd_opt_desc_val_t *opt_desc_arr, int arr_cnt);

#endif
