#include "logger.h"
#include "hv_registers.h"

#undef C
#define C(a) #a
static const char* gs_hv_mb_reg_str[] = MB_REG_ENUM;

const char* get_hv_mb_reg_str(hv_mb_reg_e_t reg_addr)
{
    if(VALID_MB_REG_ADDR(reg_addr))
    {
        return gs_hv_mb_reg_str[MB_REG_ADDR2IDX(reg_addr)];
    }
    else
    {
        DIY_LOG(LOG_ERROR, "register address %d is invalid.\n", reg_addr);
        return NULL;
    }
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
    HV_MB_REG_RW_ATTR_RW, /*C(Poweroff = 17),               17 关机请求*/
    HV_MB_REG_RW_ATTR_W, /*C(Fixpos = 18),                 18 校准定义*/
    HV_MB_REG_RW_ATTR_W, /*C(Fixval = 19),                 19 校准值**/
    HV_MB_REG_RW_ATTR_R, /*C(Workstatus = 20),             20充能状态*/
    HV_MB_REG_RW_ATTR_W, /*C(exposureCount = 21),          曝光次数*/
    0, /*C(MAX_HV_NORMAL_MB_REG_NUM) */
    0, /*C(EXT_MB_REG_START_FLAG)*/
    HV_MB_REG_RW_ATTR_W, /*C(EXT_DOSE_ADJ),                       +/- key event*/
    HV_MB_REG_RW_ATTR_RW, /*C(EXT_MB_REG_CHARGER),             charger plug in/pull out*/
    HV_MB_REG_RW_ATTR_R, /*(EXT_MB_REG_DAP_HP = 103*/
    HV_MB_REG_RW_ATTR_R, /*EXT_MB_REG_DAP_LP = 104*/
    HV_MB_REG_RW_ATTR_R, /* EXT_MB_REG_DISTANCE = 105),distance: TOF test result.*/\
    /*C(HV_MB_REG_END_FLAG), */
};

const char get_hv_mb_reg_rw_attr(hv_mb_reg_e_t reg_addr)
{
    if(VALID_MB_REG_ADDR(reg_addr))
    {
        return gs_hv_mb_reg_rw_attr[MB_REG_ADDR2IDX(reg_addr)];
    }
    else
    {
        DIY_LOG(LOG_ERROR, "register address %d is invalid.\n", reg_addr);
        return 0;
    }
}

/* dap_high_reg_ptr should be uint16_t array of at least 2 items, and the fist two items contain
 * the high and low part of DAP.
 * */
float get_float_DAP_from_reg_arr(uint16_t * dap_high_reg_ptr)
{
    float DAP_v;
    uint8_t * reg_B_3, *float_B_0;
    int idx, float_size = 4;

    reg_B_3 = LOWEST_BYTE_ADDR_OF_DAP_IN_REG_ARR(dap_high_reg_ptr);
    float_B_0 = (uint8_t*)&DAP_v;
    for(idx = 0; idx < float_size; ++idx)
    {
        *(float_B_0 + idx) = *(reg_B_3 - idx) ;
    }
    return DAP_v;
}
