#ifndef _GET_OPT_HELPER_H_
#define _GET_OPT_HELPER_H_

#include <stdio.h>
#include "logger.h"

#undef TYPE_ID
#define TYPE_ID(t) t
#define TYPE_LIST \
{\
    TYPE_ID(type_char_p), TYPE_ID(type_int), TYPE_ID(type_uint16_t),\
    TYPE_ID(type_uint32_t), TYPE_ID(type_float), TYPE_ID(type_double),\
    TYPE_ID(type_known),\
}
typedef enum TYPE_LIST data_type_id_t;
#undef TYPE_ID
#define TYPE_ID(t) #t

extern const char* type_id_str_arr[];

#define SHOULD_BE_NE_0(x) ((x) != 0 ? 1 : 0)
#define SHOULD_BE_EQ_0(x) ((x) == 0 ? 1 : 0)
#define SHOULD_BE_GT_0(x) ((x) > 0 ? 1 : 0)
#define SHOULD_BE_GE_0(x) ((x) >= 0 ? 1 : 0)
#define SHOULD_BE_LT_0(x) ((x) < 0 ? 1 : 0)
#define SHOULD_BE_LE_0(x) ((x) <= 0 ? 1 : 0)
#define SHOULD_BE_IN_INCLUDED(x, l, r) (((l) <= (x)) && ((x) <= (r)) ? 1 : 0)
#define SHOULD_BE_IN_EXCLUDED(x, l, r) ((l) < (x)) && ((x) < (r)) ? 1 : 0

/* Be careful when using the following macro CONVERT_FUNC.
 * Improper use may leads to subtle error or memory leak (when x is of type char*).
 */
#ifndef _Generic
#define TYPE_STR_OF_VAR(x, type_id) type_id_str_arr[type_id]
#define CONVERT_FUNC_STRDUP(var, value) strdup(value)
#define CONVERT_FUNC_ATOI(var, value) atoi(value)
#define CONVERT_FUNC_ATOUINT16(var, value) (uint16_t)atoi(value)
#define CONVERT_FUNC_ATOUINT32(var, value) (uint32_t)atoi(value)
#define CONVERT_FUNC_ATOF(var, value) (float)atof(value)
#define CONVERT_FUNC_ATODB(var, value) (double)atof(value)

#else
#define TYPE_STR_OF_VAR(x, type_id) _Generic((x), \
        char*: "char*",\
        int: "int",\
        uint16_t: "uint16_t",\
        uint32_t: "uint32_t",\
        float: "float",\
        double: "double",\
        default: "int")
#define CONVERT_FUNC(var, value) _Generic((var),\
        char*: strdup(value),\
        int: (int)atoi(value),\
        uint16_t: (uint16_t)atoi(value),\
        uint32_t: (uint32_t)atoi(value),\
        float: (float)atof(value),\
        double: (double)atof(value),\
        default: (int)atoi(value)
#define CONVERT_FUNC_STRDUP(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOI(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOUINT16(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOUINT32(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATOF(var, value) CONVERT_FUNC(var, value)
#define CONVERT_FUNC_ATODB(var, value) CONVERT_FUNC(var, value)
#endif
/*
 * This macro should be used with getoptlong. See the main.c in dr_manager. 
 */
#define OPT_CHECK_AND_DRAW(option_arr,check_str, var, value_check, draw_value, type_id) \
if(!strcmp(option_arr[longindex].name, check_str))\
{\
    if(optarg && optarg[0] != ':' && optarg[0] != '?')\
    {\
        (var) = draw_value;\
        if(!value_check)\
        {\
            DIY_LOG(LOG_ERROR,\
                    "The --%s param value %s is invalid.\n",\
                    check_str, optarg);\
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
