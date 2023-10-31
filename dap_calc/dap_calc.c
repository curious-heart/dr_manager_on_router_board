#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "common_tools.h"
#include "logger.h"
#include "dap_calc.h"

static const char* gs_dap_db_file_name = "dap_db.sqlite3";
static const char* gs_dap_tbl_co_name = "coefficient_tbl";
static const char* gs_dap_tbl_data_name = "dap_data_tbl";

/*the order in the array gs_dap_tbl_co_headers should accord with function.*/
static const char* gs_dap_tbl_co_headers =  "co_kV,co_mA,co_sec,co_D,co_size_times";
static const char* gs_dap_tbl_data_col_name_kV = "kV";
static const char* gs_dap_tbl_data_col_name_uA = "uA";
static const char* gs_dap_tbl_data_col_name_ms = "ms";
static const char* gs_dap_tbl_data_col_name_DAP = "DAP_uGym2";

static sqlite3* gs_dap_db = NULL;
static float gs_curr_dap_val;
static const float gs_def_co_kV = 690.0026667, gs_def_co_mA = 34689.19811, gs_def_co_sec = 29106, gs_def_co_D = -67396.55975;
static const float gs_def_co_size_times = 3.9982002504686105; // (pi * 34.5 * 33.2 / (15*15*4))
static float gs_co_kV = gs_def_co_kV, gs_co_mA = gs_def_co_mA, gs_co_sec = gs_def_co_sec, gs_co_D = gs_def_co_D;
static float gs_co_size_times = gs_def_co_size_times;

#define MAX_SQL_STM_LEN 128

static void print_used_coefficient_for_DAP()
{
    DIY_LOG(LOG_INFO, "%s\n", gs_dap_tbl_co_headers);
    DIY_LOG(LOG_INFO + LOG_ONLY_INFO_STR, "%.2f,%.2f,%.2f,%.2f,%.2f\n",
            gs_co_kV, gs_co_mA, gs_co_sec, gs_co_D, gs_co_size_times);
}

static int load_coefficient_cb(void *, int col_num, char ** exec_output, char ** col_names)
{
    static float* const co_v_ptr_arr[] = {&gs_co_kV, &gs_co_mA, &gs_co_sec, &gs_co_D, &gs_co_size_times};
    int idx, dap_tbl_co_col_cnt = ARRAY_ITEM_CNT(co_v_ptr_arr);
    if(dap_tbl_co_col_cnt != col_num)
    {
        DIY_LOG(LOG_ERROR, "error col number: %d, should be:%d\n", col_num, dap_tbl_co_col_cnt);
        return SQLITE_ERROR;
    }
    for(idx = 0; idx < dap_tbl_co_col_cnt; ++idx)
    {
        if(NULL == exec_output[idx])
        {
            DIY_LOG(LOG_ERROR, "The %dth output of sql exec is NULL.\n", idx);
            return SQLITE_ERROR;
        }
        *(co_v_ptr_arr[idx]) = strtof(exec_output[idx], NULL);
    }

    return SQLITE_OK;
}

static bool load_coefficient()
{
    bool use_default_co = false;
    char * err_msg = NULL;
    char sql_stm[MAX_SQL_STM_LEN + 1];
    int sb_size = sizeof(sql_stm), w_len;
    int sql_rc;

    do
    {
        if(NULL == gs_dap_db)
        {
            use_default_co = true;
            break;
        }
        /* SELECT co_kV,co_mA,co_sec,co_D FROM coefficient_tbl; */
        w_len = snprintf(sql_stm, sb_size, "SELECT %s FROM %s;", gs_dap_tbl_co_headers, gs_dap_tbl_co_name);
        if(w_len < 0 || w_len >= sb_size)
        {
            DIY_LOG(LOG_ERROR, "create sql statement error:\n%s\n", sql_stm);
            use_default_co = true;
            break;
        }
        DIY_LOG(LOG_INFO, "Now exec the following sql statement:\n%s\n", sql_stm);
        sql_rc = sqlite3_exec(gs_dap_db, sql_stm, load_coefficient_cb, 0, &err_msg);
        if(sql_rc != SQLITE_OK)
        {
            DIY_LOG(LOG_ERROR, "sql statement exec error!\n");
            if(err_msg) DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "error msg: %s\n", err_msg);

            use_default_co = true;
        }
        else
        {
            /* coefficients are loaded in call back function.*/
            DIY_LOG(LOG_INFO, "Load coefficient for DAP calculation ok!\n");
        }
        sqlite3_free(err_msg);
        break;
    }while(true);

    if(use_default_co)
    {
        DIY_LOG(LOG_WARN, "We use default coefficient for DAP calculation.\n");
        gs_co_kV = gs_def_co_kV; gs_co_mA = gs_def_co_mA; gs_co_sec = gs_def_co_sec; gs_co_D = gs_def_co_D;
        gs_co_size_times = gs_def_co_size_times;
    }

    print_used_coefficient_for_DAP();

    return !use_default_co;
}

bool init_DAP_db()
{
    int rc;
    bool ret;

    rc = sqlite3_open_v2(gs_dap_db_file_name, &gs_dap_db, SQLITE_OPEN_READONLY, NULL);
    if(rc != SQLITE_OK)
    {
        DIY_LOG(LOG_ERROR, "open sqlite3 db %s error:%s.\n", gs_dap_db_file_name, sqlite3_errmsg(gs_dap_db));
        sqlite3_close_v2(gs_dap_db);
        gs_dap_db = NULL;
    }

    ret = load_coefficient();
    return ret;
}

void close_DAP_db()
{
    int rc = sqlite3_close_v2(gs_dap_db);
    if(rc != SQLITE_OK)
    {
        DIY_LOG(LOG_ERROR, "close sqlite3 db %s error:%s.\n", gs_dap_db_file_name, sqlite3_errmsg(gs_dap_db));
    }
    else
    {
        DIY_LOG(LOG_INFO, "sqlite db %s closed.\n", gs_dap_db_file_name);
    }
    gs_dap_db = NULL;
}

static int load_DAP_cb(void *, int col_num, char ** exec_output, char ** col_names)
{
    if(col_num < 1 || (NULL == exec_output[0])) 
    {
        DIY_LOG(LOG_ERROR, "exec error, col_num is %d, the exec_output[0] is 0x%08X\n", col_num, exec_output[0]);
        return SQLITE_ERROR;
    }
    gs_curr_dap_val = strtof(exec_output[0], NULL);
    return SQLITE_OK;
}

float calculate_DAP_value(uint16_t kV, uint32_t uA, uint16_t ms)
{
    float mA = ((float)uA) / 1000, seconds = ((float)ms) / 1000, DAP_v;
    bool dap_exist_in_db = false;
    char * err_msg = NULL;
    char sql_stm[MAX_SQL_STM_LEN + 1];
    int sb_size = sizeof(sql_stm), w_len;
    int sql_rc;

    do
    {
        if(NULL == gs_dap_db)
        {
            break;
        }
        /* SELECT DAP_uGym2 FROM dap_data_tbl WHERE kV=90 and uA=5000 and ms=1400; */
        w_len = snprintf(sql_stm, sb_size, "SELECT %s FROM %s WHERE %s=%d and %s=%d and %s=%d;",
                                            gs_dap_tbl_data_col_name_DAP, gs_dap_tbl_data_name, 
                                            gs_dap_tbl_data_col_name_kV, kV,
                                            gs_dap_tbl_data_col_name_uA, uA,
                                            gs_dap_tbl_data_col_name_ms, ms);
        if(w_len < 0 || w_len >= sb_size)
        {
            DIY_LOG(LOG_ERROR, "create sql statement error:\n%s\n", sql_stm);
            break;
        }
        DIY_LOG(LOG_INFO, "Now exec the following sql statement:\n%s\n", sql_stm);
        sql_rc = sqlite3_exec(gs_dap_db, sql_stm, load_DAP_cb, 0, &err_msg);
        if(SQLITE_OK == sql_rc)
        {
            dap_exist_in_db = true;
            DIY_LOG(LOG_INFO, "Found a DAP value in db.\n");
        }
        else
        {
            DIY_LOG(LOG_ERROR, "sql statement exec error!\n");
            if(err_msg) DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "error msg: %s\n", err_msg);
            DIY_LOG(LOG_WARN, "No DAP found in db, we calculate it!\n");
        }
        sqlite3_free(err_msg);

        break;
    }while(true);

    if(dap_exist_in_db)
    {
        DAP_v = gs_curr_dap_val;
    }
    else
    {
        DAP_v = (gs_co_kV * kV + gs_co_mA * mA + gs_co_sec * seconds + gs_co_D) * gs_co_size_times / 100000;
    }
    DIY_LOG(LOG_INFO,
        "\n%s\t%s\t%s\t%s\t\n%d\t%d\t%d\t%f\n\n", 
        gs_dap_tbl_data_col_name_kV, gs_dap_tbl_data_col_name_uA, gs_dap_tbl_data_col_name_ms, gs_dap_tbl_data_col_name_DAP,
        kV, uA, ms, DAP_v);

    return DAP_v;
}

