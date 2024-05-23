#include <stdio.h>
#include <errno.h>
#include <modbus/modbus.h>

#include "logger.h"
#include "hv_controller.h"

#include "hv_registers.h"

static modbus_t *gs_mb_master_ctx = NULL;
static const char* gs_mb_master_log_header = "modbus master: ";

bool hv_controller_open(mb_rtu_params_t* rtu_params)
{
    const char* rtu_dev;
    uint32_t resp_timeout_s = rtu_params->timeout_ms / 1000;
    uint32_t resp_timeout_us = (rtu_params->timeout_ms - resp_timeout_s * 1000) * 1000;

    if(!rtu_params || !rtu_params->serialPortName)
    {
        DIY_LOG(LOG_ERROR, "%slacking rtu info.\n", gs_mb_master_log_header);
        return false;
    }
    rtu_dev = rtu_params->serialPortName;

    gs_mb_master_ctx = modbus_new_rtu(rtu_dev, rtu_params->serialBaudRate,
            rtu_params->serialParity, rtu_params->serialDataBits,
            rtu_params->serialStopBits);
    if(gs_mb_master_ctx)
    {
        DIY_LOG(LOG_INFO, "%smodbus init ok.\n", gs_mb_master_log_header );
    }
    else
    {
        const char* err_msg = modbus_strerror(errno);
        DIY_LOG(LOG_ERROR, "%smodbus init fail: %d:%s\n",
                gs_mb_master_log_header, errno, err_msg);
        return false;
    }
    if(0 != modbus_set_debug(gs_mb_master_ctx, rtu_params->debug_flag))
    {
        DIY_LOG(LOG_WARN, "%smodbus set debug fail:%d: %s\n",
               gs_mb_master_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gs_mb_master_log_header);
    }

    if(0!= modbus_set_error_recovery(gs_mb_master_ctx,
            MODBUS_ERROR_RECOVERY_LINK | MODBUS_ERROR_RECOVERY_PROTOCOL))
    {
        DIY_LOG(LOG_WARN, "%sset error recovery mode fail:%d: %s, ",
                gs_mb_master_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gs_mb_master_log_header);
    }

    if(0 != modbus_set_response_timeout(gs_mb_master_ctx, resp_timeout_s, resp_timeout_us))
    {
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus set response time out (%d s, %d us) fail:%d: %s\n",
                gs_mb_master_log_header , resp_timeout_s, resp_timeout_us, errno, modbus_strerror(errno));
        return false;
    }

    if(0 != modbus_set_slave(gs_mb_master_ctx, rtu_params->serverAddress))
    {
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus set slave fail:%d, %s\n",
               gs_mb_master_log_header, errno, modbus_strerror(errno));
        return false;
    }

    if(0 != modbus_connect(gs_mb_master_ctx))
    {
        modbus_close(gs_mb_master_ctx);
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus connect %s fail:%d, %s\n",
                gs_mb_master_log_header, rtu_dev, errno, modbus_strerror(errno));
        return false;
    }
    DIY_LOG(LOG_INFO, "%smodbus rtu connect to %s ok.\n", gs_mb_master_log_header, rtu_dev);

    return true;
}

bool hv_controller_close()
{
    if(gs_mb_master_ctx)
    {
        modbus_close(gs_mb_master_ctx);
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
    }
    else
    {
        DIY_LOG(LOG_WARN, "%smodbus ctx is NULL.\n", gs_mb_master_log_header);
    }
    return true;
}

bool hv_controller_write_single_uint16(int reg_addr, uint16_t value)
{
    if(gs_mb_master_ctx)
    {
        if(modbus_write_register(gs_mb_master_ctx, reg_addr, value) <= 0)
        {
            DIY_LOG(LOG_ERROR, "%smodbus write register error:%d, %s\n",
                   gs_mb_master_log_header, errno, modbus_strerror(errno));
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "%smodbus ctx is NULL.\n", gs_mb_master_log_header);
        return false;
    }
}

bool hv_controller_write_uint16s(int reg_addr_start, uint16_t *buf, int len)
{
    if(gs_mb_master_ctx)
    {
        if(modbus_write_registers(gs_mb_master_ctx, reg_addr_start, len, buf) <= 0)
        {
            DIY_LOG(LOG_ERROR, "%smodbus write multi-registers error:%d, %s.\n",
                    gs_mb_master_log_header,  errno, modbus_strerror(errno));
            return false;
        }
        else
        {
            return true;
        }

    }
    else
    {
        DIY_LOG(LOG_ERROR, "%smodbus ctx is NULL.\n", gs_mb_master_log_header);
        return false;
    }
}

bool hv_controller_read_uint16s(int reg_addr, uint16_t * buf, int len)
{
    if(gs_mb_master_ctx)
    {
        if(modbus_read_registers(gs_mb_master_ctx, reg_addr, len, buf) <= 0)
        {
            DIY_LOG(LOG_ERROR, "%smodbus read register error:%d, %s\n",
                    gs_mb_master_log_header, errno, modbus_strerror(errno));
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
}
