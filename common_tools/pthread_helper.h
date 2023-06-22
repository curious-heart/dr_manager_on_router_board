#ifndef _PTHREAD_HELPER_H_
#define _PTHREAD_HELPER_H_

#define PTHREAD_ERR_CHECK(s, str_h, str_m, str_t, e) \
    if(s != 0)\
    {\
        DIY_LOG(LOG_ERROR, "%s", str_h);\
        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "%s", str_m);\
        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "%s", str_t);\
        DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, " :%d.\n", s);\
        if(e) return false;\
    }

#endif
