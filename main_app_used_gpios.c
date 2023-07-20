#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include "common_tools.h"
#include "logger.h"
#include "main_app_used_gpios.h"

static const gpio_cfg_tbl_s_t gs_app_op_gpio_table[] =
{
    {GPIO_CHARGER_FULL_IND, "in"},
};

int get_effective_gpio_number(int n)
{
    static const int gpio_bank[] = 
    {
        GPIO_BANK_0_BASE, 
        GPIO_BANK_1_BASE, 
        GPIO_BANK_2_BASE, 
    };
    int idx = n / GPIO_CNT_PER_BANK, bias = n % GPIO_CNT_PER_BANK;
    if(idx < ARRAY_ITEM_CNT(gpio_bank))
    {
        return gpio_bank[idx] + bias;
    }
    else
    {
        return -1;
    }
}

void export_gpios_to_app()
{
    int idx, eff_gpio;
    char ptn_str[GPIO_MAX_PTN_STR_LEN + 1]; 
    struct stat sb;
    FILE* export_fp = NULL, *op_fp = NULL;

    for(idx = 0; idx < ARRAY_ITEM_CNT(gs_app_op_gpio_table); ++idx)
    {
        eff_gpio = get_effective_gpio_number(gs_app_op_gpio_table[idx].number);
        if(eff_gpio < 0)
        {
            DIY_LOG(LOG_ERROR, "The gpio number %d is invalid.\n", gs_app_op_gpio_table[idx].number);
            continue;
        }
        snprintf(ptn_str, sizeof(ptn_str), "%s/gpio%d", GPIO_CTL_PTH,  eff_gpio);
        if(stat(ptn_str, &sb) < 0)
        {
            if(NULL == export_fp)
            {
                snprintf(ptn_str, sizeof(ptn_str), "%s/export", GPIO_CTL_PTH);  
                export_fp = fopen(ptn_str, "w");
                if(NULL == export_fp)
                {
                    DIY_LOG(LOG_ERROR, "open %s error: %d.\n", ptn_str, errno);
                    break;
                }
            }
            fprintf(export_fp, "%d", eff_gpio);
            sleep(1);
        }
        snprintf(ptn_str, sizeof(ptn_str), "%s/gpio%d/direction", GPIO_CTL_PTH, eff_gpio) ;
        op_fp = fopen(ptn_str, "w");
        if(NULL == op_fp)
        {
            DIY_LOG(LOG_ERROR, "open %s error: %d.\n", ptn_str, errno);
            continue;
        }
        else
        {
            fprintf(op_fp, "%s", gs_app_op_gpio_table[idx].direction);
            fclose(op_fp);
            op_fp = NULL;
        }
    }

    if(export_fp != NULL)
    {
        fclose(export_fp);
    }
}

int app_read_gpio_value(int n)
{
    int eff_gpio;
    char ptn_str[GPIO_MAX_PTN_STR_LEN + 1]; 
    char value_str[GPIO_MAX_VALUE_DIGITS + 1]; 
    int value;
    int read_len;
    FILE* op_fp = NULL;

    eff_gpio = get_effective_gpio_number(n);
    if(eff_gpio < 0)
    {
        DIY_LOG(LOG_ERROR, "The gpio number %d is invalid.\n", n);
        return -1;
    }
    snprintf(ptn_str, sizeof(ptn_str), "%s/gpio%d/value", GPIO_CTL_PTH, eff_gpio);
    op_fp = fopen(ptn_str, "r");
    if(NULL == op_fp)
    {
        DIY_LOG(LOG_ERROR, "open %s error: %d.\n", ptn_str, errno);
        return -1;
    }
    memset(value_str, 0, sizeof(value_str));
    read_len = fread(value_str, GPIO_MAX_VALUE_DIGITS, 1, op_fp);
    if(read_len <= 0)
    {
        DIY_LOG(LOG_ERROR, "read from %s error, %d bytes read.\n", ptn_str, read_len);
        fclose(op_fp);
        return -1;
    }
    value = (int)strtol(value_str, NULL, 0);

    fclose(op_fp);
    return value;
}

int app_write_gpio_value(int n, int value)
{
    int eff_gpio;
    char ptn_str[GPIO_MAX_PTN_STR_LEN + 1]; 
    FILE* op_fp = NULL;

    eff_gpio = get_effective_gpio_number(n);
    if(eff_gpio < 0)
    {
        DIY_LOG(LOG_ERROR, "The gpio number %d is invalid.\n", n);
        return -1;
    }
    snprintf(ptn_str, sizeof(ptn_str), "%s/gpio%d/value", GPIO_CTL_PTH, eff_gpio);
    op_fp = fopen(ptn_str, "w");
    if(NULL == op_fp)
    {
        DIY_LOG(LOG_ERROR, "open %s error: %d.\n", ptn_str, errno);
        return -1;
    }
    fprintf(op_fp, "%d", value);

    fclose(op_fp);
    return 0;
}
