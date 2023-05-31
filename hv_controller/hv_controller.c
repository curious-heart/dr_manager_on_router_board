#include <stdio.h>
#include <errno.h>
#include <modbus/modbus.h>

#include "hv_controller.h"
#include "logger.h"
#include "type_defs.h"

static const char* gs_mb_rtu_serialPortName="/dev/ttyS0";
static const int gs_mb_rtu_serialBaudRate=9600;
static const char gs_mb_rtu_serialParity='N';
static const int gs_mb_rtu_serialDataBits=8;
static const int gs_mb_rtu_serialStopBits=1;
static const int gs_mb_rtu_timeout=1000;
static const int gs_mb_rtu_numberOfRetries=3;
static const int gs_mb_rtu_serverAddress=1;

static modbus_t *gs_mb_master_ctx = NULL;

bool hv_controller_open()
{
    gs_mb_master_ctx = modbus_new_rtu(gs_mb_rtu_serialPortName, gs_mb_rtu_serialBaudRate,
            gs_mb_rtu_serialParity, gs_mb_rtu_serialDataBits, gs_mb_rtu_serialStopBits);
    if(gs_mb_master_ctx)
    {
        DIY_LOG(LOG_INFO, "modbus init ok.\n");
    }
    else
    {
        const char* err_msg = modbus_strerror(errno);
        DIY_LOG(LOG_ERROR, "modbus init fail: %s\n", err_msg);
        return false;
    }

    if(-1 == modbus_connect(gs_mb_master_ctx))
    {
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "modbus connect fail:%s\n", modbus_strerror(errno));
        return false;
    }

    if(-1 == modbus_set_slave(gs_mb_master_ctx, gs_mb_rtu_serverAddress))
    {
        modbus_close(gs_mb_master_ctx);
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "modbus set slave fail:%s\n", modbus_strerror(errno));
        return false;
    }
    return true;
}

bool hv_controller_close()
{
    if(gs_mb_master_ctx)
    {
        modbus_close(gs_mb_master_ctx);
        modbus_free(gs_mb_master_ctx);
    }
    else
    {
        DIY_LOG(LOG_WARN, "modbus ctx is NULL.\n");
    }
    return true;
}

bool hv_controller_write_uint16(int reg_addr, uint16 value)
{
    if(gs_mb_master_ctx)
    {
        if(modbus_write_register(gs_mb_master_ctx, reg_addr, value) < 0)
        {
            DIY_LOG(LOG_ERROR, "modbus write register error:%s\n",
                    modbus_strerror(errno));
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        DIY_LOG(LOG_ERROR, "modbus ctx is NULL.\n");
        return false;
    }
}

bool hv_controller_read_data(int reg_addr, uint16 * buf, int len)
{
    if(gs_mb_master_ctx)
    {
        if(modbus_read_registers(gs_mb_master_ctx, reg_addr, len, buf) <= 0)
        {
            DIY_LOG(LOG_ERROR, "modbus read register error:%s\n",
                    modbus_strerror(errno));
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
