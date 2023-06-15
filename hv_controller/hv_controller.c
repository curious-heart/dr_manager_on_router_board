#include <stdio.h>
#include <errno.h>
#include <modbus/modbus.h>

#include "logger.h"
#include "hv_controller.h"

#include "hv_registers.h"
static const char* gs_hv_mb_reg_str[] = MB_REG_ENUM;
const char* get_hv_mb_reg_str(hv_mb_reg_e_t reg_addr)
{
    if((reg_addr >= MAX_HV_MB_REG_NUM) || (reg_addr < 0))
    {
        DIY_LOG(LOG_ERROR, "register address %d is invalid.\n", reg_addr);
        return NULL;
    }
    return gs_hv_mb_reg_str[reg_addr];
}

static const char gs_hv_mb_reg_rw_attr[] =
{
    HV_MB_REG_RW_ATTR_R, /*C(HSV = 0),                    软硬件版本*/
    HV_MB_REG_RW_ATTR_RW, /*C(OTA = 1),                   OTA升级*/
    HV_MB_REG_RW_ATTR_RW, /*C(BaudRate = 2),              波特率*/
    HV_MB_REG_RW_ATTR_RW, /*C(ServerAddress = 3),         设备地址*/
    HV_MB_REG_RW_ATTR_RW, /*C(State = 4),                 状态*/
    HV_MB_REG_RW_ATTR_RW, /*C(VoltSet = 5),               管电压设置值*/
    HV_MB_REG_RW_ATTR_RW, /*C(FilamentSet = 6),       管设置值电流 （决定灯丝电流决定管电流）*/
    HV_MB_REG_RW_ATTR_RW, /*C(ExposureTime = 7),           曝光时间*/
    HV_MB_REG_RW_ATTR_R, /*C(Voltmeter = 8),               管电压读出值*/
    HV_MB_REG_RW_ATTR_R, /*C(Ammeter = 9),                 管电流读出值*/
    HV_MB_REG_RW_ATTR_R, /*C(RangeIndicationStatus = 10),  范围指示状态*/
    HV_MB_REG_RW_ATTR_R, /*C(ExposureStatus = 11),         曝光状态*/
    HV_MB_REG_RW_ATTR_RW, /*C(RangeIndicationStart = 12),   范围指示启动*/
    HV_MB_REG_RW_ATTR_RW, /*C(ExposureStart = 13),          曝光启动*/
    HV_MB_REG_RW_ATTR_R, /*C(BatteryLevel = 14),           电池电量*/
    HV_MB_REG_RW_ATTR_R, /*C(BatteryVoltmeter = 15), */
    HV_MB_REG_RW_ATTR_R, /*C(OilBoxTemperature = 16),      电池电压高位*/
    HV_MB_REG_RW_ATTR_R, /*C(Poweroff = 17),               17 关机请求*/
    HV_MB_REG_RW_ATTR_W, /*C(Fixpos = 18),                 18 校准定义*/
    HV_MB_REG_RW_ATTR_W, /*C(Fixval = 19),                 19 校准值**/
    HV_MB_REG_RW_ATTR_R, /*C(Workstatus = 20),             20充能状态*/
    HV_MB_REG_RW_ATTR_W, /*C(exposureCount = 21),          曝光次数*/
};
const char get_hv_mb_reg_rw_attr(hv_mb_reg_e_t reg_addr)
{
    if((reg_addr >= MAX_HV_MB_REG_NUM) || (reg_addr < 0))
    {
        DIY_LOG(LOG_ERROR, "register address %d is invalid.\n", reg_addr);
        return 0;
    }
    return gs_hv_mb_reg_rw_attr[reg_addr];
}

static modbus_t *gs_mb_master_ctx = NULL;
static const char* gs_mb_master_log_header = "modbus master: ";


bool hv_controller_open(mb_rtu_params_t* rtu_params)
{
    const char* rtu_dev;
    uint32_t resp_timeout_ms = rtu_params->timeout_ms;

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

    if(0 != modbus_set_response_timeout(gs_mb_master_ctx, 0, resp_timeout_ms * 1000))
    {
        modbus_free(gs_mb_master_ctx);
        gs_mb_master_ctx = NULL;
        DIY_LOG(LOG_ERROR, "%smodbus set response time out (%d s, %d ms) fail:%d: %s\n",
                gs_mb_master_log_header ,0, resp_timeout_ms, errno, modbus_strerror(errno));
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
