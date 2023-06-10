#include <stdio.h>
#include <errno.h>
#include <modbus/modbus.h>

#include "hv_controller.h"
#include "logger.h"

static const char* gs_mb_rtu_serialPortName="/dev/ttyS1";
static const int32_t gs_mb_rtu_serialBaudRate=9600;
static const char gs_mb_rtu_serialParity='N';
static const int32_t gs_mb_rtu_serialDataBits=8;
static const int32_t gs_mb_rtu_serialStopBits=1;
static const uint32_t gs_mb_rtu_timeout_ms =999; //1000;
static const int32_t gs_mb_rtu_numberOfRetries=3;
static const int32_t gs_mb_rtu_serverAddress=1;

static modbus_t *gs_mb_master_ctx = NULL;
static const char* gs_mb_master_log_header = "modbus master: ";

bool hv_controller_open(const char* dev_name, bool debug_flag)
{
    const char* rtu_dev = dev_name;
    uint32_t resp_timeout_ms = gs_mb_rtu_timeout_ms;

    if(NULL == rtu_dev)
    {
        rtu_dev = gs_mb_rtu_serialPortName;
    }

    gs_mb_master_ctx = modbus_new_rtu(rtu_dev, gs_mb_rtu_serialBaudRate,
            gs_mb_rtu_serialParity, gs_mb_rtu_serialDataBits, gs_mb_rtu_serialStopBits);
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
    if(0 != modbus_set_debug(gs_mb_master_ctx, debug_flag))
    {
        DIY_LOG(LOG_WARN, "%smodbus set debug fail:%d: %s\n",
               gs_mb_master_log_header, errno, modbus_strerror(errno));
        DIY_LOG(LOG_WARN, "%sbut we continue going ahead.\n\n", gs_mb_master_log_header);
    }

    if(0 != modbus_set_response_timeout(gs_mb_master_ctx, 0, resp_timeout_ms * 1000))
    {
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus set response time out (%d s, %d ms) fail:%d: %s\n",
                gs_mb_master_log_header ,0, resp_timeout_ms, errno, modbus_strerror(errno));
        return false;
    }

    if(0 != modbus_set_slave(gs_mb_master_ctx, gs_mb_rtu_serverAddress))
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
