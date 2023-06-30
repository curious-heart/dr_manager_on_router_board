#ifndef _PTHREAD_HELPER_H_
#define _PTHREAD_HELPER_H_

#include <stdbool.h>
#include <pthread.h>

#include "logger.h"

#define PTHREAD_ERR_CHECK(s, str_h, str_m, str_t, e) \
    if(s != 0)\
    {\
        DIY_LOG(LOG_ERROR, "%s", str_h);\
        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "%s", str_m);\
        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "%s", str_t);\
        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, " :%d.\n", s);\
        if(e) return false;\
    }

typedef void* (*pthread_func_t)(void*);
bool start_assit_thread(const char* desc, pthread_t * th_id, bool detach, pthread_func_t func, void* arg);
void cancel_assit_thread(pthread_t * th_id);

#endif
