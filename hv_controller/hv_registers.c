#include "logger.h"
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
