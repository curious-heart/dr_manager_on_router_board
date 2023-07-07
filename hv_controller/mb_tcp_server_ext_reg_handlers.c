#include "logger.h"
#include "mb_tcp_server_ext_reg_handlers.h"
#include "hv_controller.h"

#define MB_CTX_AND_MAPPING_CHECK \
    if(!mb_ctx)\
    {\
        DIY_LOG(LOG_ERROR, "%smodbus contex is NULL!\n", gp_mb_server_log_header);\
        return MB_RW_REG_RET_ERROR;\
    }\
    if(!reg_mappings)\
    {\
        DIY_LOG(LOG_ERROR, "%smodbus mappings is NULL!\n", gp_mb_server_log_header);\
        return MB_RW_REG_RET_ERROR;\
    }

static void mb_tcp_srvr_adjust_dose(uint16_t adj_val, uint16_t* reg_buf)
{
    /* TO BE COMPLETED...The following are just for test...*/
    struct exp_parms_tuple_t
    {
        uint16_t vol, amt, dura;
    };
#define NUM_OF_LEVELS 12
    static const struct exp_parms_tuple_t exp_parms_tuple_arr[NUM_OF_LEVELS] =
    {
        [0] =  {70, 1000, 7 },
        [1] =  {70, 2000, 10},
        [2] =  {70, 3000, 14},
        [3] =  {70, 5000, 16},
        [4] =  {80, 1000, 7 },
        [5] =  {80, 2000, 10},
        [6] =  {80, 3000, 14},
        [7] =  {80, 5000, 16},
        [8] =  {90, 1000, 7 },
        [9] =  {90, 2000, 10},
        [10] = {90, 3000, 14},
        [11] = {90, 5000, 16},
    };

    static int curr_level = 0;

    if(!reg_buf)
    {
        DIY_LOG(LOG_ERROR, "register buf is NULL.\n");
    }

    if(MB_REG_V_DOSE_ADJ_ADD == adj_val)
    {
        curr_level = (curr_level + 1) % NUM_OF_LEVELS;
    }
    else
    {
        curr_level = (curr_level - 1 < 0) ? NUM_OF_LEVELS - 1: (curr_level - 1);
    }
    reg_buf[VoltSet] = exp_parms_tuple_arr[curr_level].vol;
    reg_buf[VoltSet + 1] = exp_parms_tuple_arr[curr_level].amt;
    reg_buf[VoltSet + 2] = exp_parms_tuple_arr[curr_level].dura;
}

mb_rw_reg_ret_t mb_tcp_srvr_ext_reg_dose_adj_handler(uint16_t adj_val)
{
    mb_rw_reg_ret_t process_ret = MB_RW_REG_RET_NONE;
    modbus_t * mb_ctx = mb_server_get_ctx();
    modbus_mapping_t * reg_mappings = mb_server_get_mapping();
    uint16_t reg_addr_start =  VoltSet;
    uint16_t reg_cnt = 3; //VolSet, FilamentSet, ExposureTime

    MB_CTX_AND_MAPPING_CHECK; 

    mb_tcp_srvr_adjust_dose(adj_val, reg_mappings->tab_registers);

    if(hv_controller_write_uint16s(reg_addr_start, &reg_mappings->tab_registers[reg_addr_start], reg_cnt))
    {
       process_ret = mb_server_write_reg_sniff(reg_addr_start, &reg_mappings->tab_registers[reg_addr_start], reg_cnt);
    }
    else
    {
        uint16_t idx;
        DIY_LOG(LOG_ERROR, "%swrite the following registers to hv_controller error:.\n", gp_mb_server_log_header);

        for(idx = reg_addr_start; idx < reg_addr_start + reg_cnt; ++idx)
        {
            DIY_LOG(LOG_ERROR + LOG_ONLY_INFO_STR, "\t%s: %d\n", get_hv_mb_reg_str(idx), reg_mappings->tab_registers[idx]);
        }
        process_ret = MB_RW_REG_RET_ERROR;
    }

    return process_ret; 
}
