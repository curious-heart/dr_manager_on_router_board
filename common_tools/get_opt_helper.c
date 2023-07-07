#include "logger.h"
#include "get_opt_helper.h"

const char* type_id_str_arr[] = TYPE_LIST;

/*This function does not check the buffer for security. The caller should guarantee the buffer size.*/
void construct_short_opt_chars_str(char* buf, struct option * long_opt_arr, int arr_cnt)
{
    int i, pos;
    for(i = 0, pos = 0; i < arr_cnt; ++i)
    {
        if(long_opt_arr[i].val != 0)
        {
            buf[pos++] = (char)long_opt_arr[i].val;
            if(required_argument == long_opt_arr[i].has_arg)
            {
                buf[pos++] = ':';
            }
            else if(optional_argument == long_opt_arr[i].has_arg)
            {
                buf[pos++] = ':';
                buf[pos++] = ':';
            }
        }
    }
    buf[pos++] = 0;

    DIY_LOG(LOG_DEBUG, "short option chars string: %s\n", buf);
}

void print_app_cmd_line_usage(const char* app_name, struct option* long_opt_arr, cmd_opt_desc_val_t* opt_desc_arr, int arr_cnt)
{
    int i;
    printf("\nusage:\n%s ", app_name);
    for(i = 0; i < arr_cnt; ++i)
    {
        printf("[");
        if(long_opt_arr[i].name) printf("--%s", long_opt_arr[i].name);
        if(long_opt_arr[i].val) printf("|-%c", (char)long_opt_arr[i].val);
        if(required_argument == long_opt_arr[i].has_arg && opt_desc_arr[i].desc)
        {
            printf(" %s", opt_desc_arr[i].desc);
        }
        printf("] ");
    }
    printf("\n");
    printf("\n");

    print_app_cmd_line_parameters(DEFAULT_PARAMS, long_opt_arr, opt_desc_arr, arr_cnt);
}

void print_app_cmd_line_parameters(default_or_working_params_ind_t ind, struct option* long_opt_arr,
                                   cmd_opt_desc_val_t *opt_desc_arr, int arr_cnt)
{
    int i;
    const void* ptr = NULL;

    printf("\n");
    if(DEFAULT_PARAMS == ind)
    {
        printf("The defaul arguments:\n");
    }
    else
    {
        printf("The working arguments:\n");
    }

    for(i = 0; i < arr_cnt; ++i)
    {
        if(DEFAULT_PARAMS == ind)
        {
            ptr = opt_desc_arr[i].def_val_ptr;
        }
        else
        {
            ptr = opt_desc_arr[i].work_val_ptr;
        }
        if(!ptr)
        {
            continue;
        }

        if(long_opt_arr[i].name && required_argument == long_opt_arr[i].has_arg)
        {
            printf("--%s: ", long_opt_arr[i].name);
            switch(opt_desc_arr[i].type_ind)
            {
                case type_bool:
                    printf("%d", *(bool*)ptr);
                    break;

                case type_c_charp:
                case type_charp:
                    printf("%s", *(const char**)ptr);
                    break;
                case type_char:
                    printf("%c", *(const char*)ptr);
                    break;
                case type_short:
                    printf("%d", *(const short*)ptr);
                    break;
                case type_int:
                    printf("%d", *(const int*)ptr);
                    break;
                case type_int8_t:
                    printf("%d", *(const int8_t*)ptr);
                    break;
                case type_int16_t:
                    printf("%d", *(const int16_t*)ptr);
                    break;
                case type_int32_t:
                    printf("%d", *(const int32_t*)ptr);
                    break;
                case type_uint8_t:
                    printf("%u", *(const uint8_t*)ptr);
                    break;
                case type_uint16_t:
                    printf("%u", *(const uint16_t*)ptr);
                    break;
                case type_uint32_t:
                    printf("%u", *(const uint32_t*)ptr);
                    break;
                case type_float:
                    printf("%f", *(const float*)ptr);
                    break;
                case type_double:
                    printf("%f", *(const double*)ptr);
                    break;

                default:
                    ;
            }
            printf("\n");
        }
    }
    printf("\n");
}

