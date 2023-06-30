#ifndef _GET_OPT_HELPER_H_
#define _GET_OPT_HELPER_H_

#include <stdint.h>
#include <stdio.h>
#include "logger.h"

#undef TYPE_ID
#define TYPE_ID(t, name) type_##name,
#define TYPE_LIST \
{\
    TYPE_ID(const char*, c_charp) TYPE_ID(char*, charp)\
    TYPE_ID(char, char)  TYPE_ID(short, short)  TYPE_ID(int, int)  \
    TYPE_ID(int8_t, int8_t)  TYPE_ID(int16_t, int16_t) TYPE_ID(int32_t, int32_t) \
    TYPE_ID(uint8_t, uint8_t)  TYPE_ID(uint16_t, uint16_t)  TYPE_ID(uint32_t, uint32_t) \
    TYPE_ID(float, float)  TYPE_ID(double, double) \
}
typedef enum TYPE_LIST data_type_id_t;

#undef TYPE_ID
#define TYPE_ID(t, name) t name##_val;
typedef struct
{
    const char* desc;
    union TYPE_LIST def_val;
    data_type_id_t type_ind;
}cmd_opt_desc_val_t;

#undef TYPE_ID
#define TYPE_ID(t, name) #name
extern const char* type_id_str_arr[];

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

#define CHECK_COM_PARITY(c) ((c == 'N' || c == 'E' || c == 'O') ? 1: 0)
#define CHECK_COM_PARITY_LOG DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, " should be of \'N\' or \'E\' or \'O\'.")
#define MIN_SERIAL_DATA_BITS 5
#define MAX_SERIAL_DATA_BITS 8
#define MIN_SERIAL_STOP_BITS 1
#define MAX_SERIAL_STOP_BITS 2

#define TYPE_STR_OF_VAR(x, type_id) type_id_str_arr[type_id]
/* Be careful when using the following macro CONVERT_FUNC.
 * Improper use may leads to subtle error or memory leak (when x is of type char*).
 */
#ifndef _Generic
#define CONVERT_FUNC_STRDUP(var, value) strdup(value)
#define CONVERT_FUNC_ATOC(var, value) (char)(value)[0]
#define CONVERT_FUNC_ATOS(var, value) (short)atoi(value)
#define CONVERT_FUNC_ATOI(var, value) atoi(value)
#define CONVERT_FUNC_ATOINT8(var, value) (int8_t)atoi(value)
#define CONVERT_FUNC_ATOINT16(var, value) (int16_t)atoi(value)
#define CONVERT_FUNC_ATOINT32(var, value) (int32_t)atoi(value)
#define CONVERT_FUNC_ATOUINT8(var, value) (uint8_t)atoi(value)
#define CONVERT_FUNC_ATOUINT16(var, value) (uint16_t)atoi(value)
#define CONVERT_FUNC_ATOUINT32(var, value) (uint32_t)atoi(value)
#define CONVERT_FUNC_ATOF(var, value) (float)atof(value)
#define CONVERT_FUNC_ATODB(var, value) (double)atof(value)

#else
#define CONVERT_FUNC(var, value) _Generic((var),\
        char*: strdup(value),\
        char: (char)(value)[0],\
        short: (short)atoi(value),\
        int: (int)atoi(value),\
        uint8_t: (uint8_t)atoi(value),\
        uint16_t: (uint16_t)atoi(value),\
        uint32_t: (uint32_t)atoi(value),\
        float: (float)atof(value),\
        double: (double)atof(value),\
        default: (int)atoi(value)
#define CONVERT_FUNC_STRDUP(var, value) CONVERT_FUNC(var, value)
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
 * This macro should be used with getoptlong. See the main.c in dr_manager. 
 */
#define OPT_CHECK_AND_DRAW(option_arr,check_str, var, value_check, check_log, draw_value, type_id) \
if(!strcmp(option_arr[longindex].name, check_str))\
{\
    if(optarg && optarg[0] != ':' && optarg[0] != '?')\
    {\
        (var) = draw_value;\
        if(!value_check)\
        {\
            DIY_LOG(LOG_ERROR, "The --%s param value %s is invalid:", check_str, optarg);\
            check_log;\
            DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR_COMP, "\n");\
            arg_parse_result = false;\
        }\
    }\
    else\
    {\
        DIY_LOG(LOG_ERROR,\
                "if option --%s are provided, an %s parameter is necessary.\n", \
                check_str, TYPE_STR_OF_VAR(var, type_id));\
        arg_parse_result = false;\
    }\
    break;\
}

#endif
