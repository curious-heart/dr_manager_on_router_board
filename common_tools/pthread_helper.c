#include <stdint.h>
#include "pthread_helper.h"

bool start_assit_thread(const char* desc, pthread_t * th_id, bool detach, pthread_func_t func, void* arg)
{
    int s;
    pthread_attr_t attr;
    pthread_t l_th_id;

    s = pthread_attr_init(&attr);
    PTHREAD_ERR_CHECK(s, "init pthread attribute for ", desc, " fails", true);

    if(detach)
    {
        s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        PTHREAD_ERR_CHECK(s, "Set thread detach state for ", desc, " fails", true);
    }

    s = pthread_create(&l_th_id, &attr, func, arg);
    PTHREAD_ERR_CHECK(s, "Create thread ", desc, " failes", true);

    s= pthread_attr_destroy(&attr);
    PTHREAD_ERR_CHECK(s, "Destory attr for ", desc, " failes", true);

    if(th_id) *th_id = l_th_id;

    DIY_LOG(LOG_INFO, "%s thread created successfully, the id is %u.\n",
            desc, (uint32_t)l_th_id);
    return true;
}

void cancel_assit_thread(pthread_t * th_id)
{
    if(th_id)
    {
        pthread_cancel(*th_id);
    }
}
