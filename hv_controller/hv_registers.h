#ifndef _HV_REGISTERS_H_
#define _HV_REGISTERS_H_

#include <stdint.h>

#define MB_REG_ENUM \
{\
    C(HSV = 0),                            /*软硬件版本*/\
    C(OTA = 1),                            /*OTA升级*/\
    C(BaudRate = 2),                       /*波特率*/\
    C(ServerAddress = 3),                  /*设备地址*/\
    C(State = 4),                          /*状态*/\
    C(VoltSet = 5),                        /*5管电压设置值*/\
    C(FilamentSet = 6),                    /*6 管设置值电流 （决定灯丝电流决定管电流）*/\
    C(ExposureTime = 7),                   /*曝光时间*/\
    C(Voltmeter = 8),                      /*管电压读出值*/\
    C(Ammeter = 9),                        /*管电流读出值*/\
    C(RangeIndicationStatus = 10),         /*范围指示状态*/\
    C(ExposureStatus = 11),                /*曝光状态*/\
    C(RangeIndicationStart = 12),          /*范围指示启动*/\
    C(ExposureStart = 13),                 /*曝光启动*/\
    C(BatteryLevel = 14),                  /*电池电量*/\
    C(BatteryVoltmeter = 15),\
    C(OilBoxTemperature = 16),             /*电池电压高位*/\
    C(Poweroff = 17),                      /*关机请求*/\
    C(Fixpos = 18),                        /*校准定义*/\
    C(Fixval = 19),                        /*校准值*/\
    C(Workstatus = 20),                    /*充能状态*/\
    C(exposureCount = 21),                 /*曝光次数*/\
\
    C(MAX_HV_MB_REG_NUM), /*normal register end flag.*/\
\
    /*Below are extend register, that is, they are processed internally by server and not passed to hv controller.*/ \
    C(EXT_MB_REG_DOSE_ADJ),                       /*+/- key event*/\
    C(EXT_MB_REG_CHARGER),                       /*charger plug in/pull out*/\
\
    C(HV_MB_REG_END_FLAG), /*register end flag.*/\
}

#undef C
#define C(a) a
typedef enum MB_REG_ENUM hv_mb_reg_e_t;

#define NORMAL_MB_REG_ADDR(addr) (HSV <= (addr) && (addr) < MAX_HV_MB_REG_NUM)
#define EXTEND_MB_REG_ADDR(addr) (MAX_HV_MB_REG_NUM < (addr) && (addr) < HV_MB_REG_END_FLAG)
#define VALID_MB_REG_ADDR(addr) (NORMAL_MB_REG_ADDR(addr) ||EXTEND_MB_REG_ADDR(addr))
#define NORMAL_MB_REG_AND_CNT(addr, cnt) (NORMAL_MB_REG_ADDR(addr) && NORMAL_MB_REG_ADDR((addr) + (cnt) - 1))
#define EXTEND_MB_REG_AND_CNT(addr, cnt) (EXTEND_MB_REG_ADDR(addr) && EXTEND_MB_REG_ADDR((addr) + (cnt) - 1))
#define VALID_MB_REG_AND_CNT(addr, cnt) (VALID_MB_REG_ADDR(addr) && VALID_MB_REG_ADDR((addr) + (cnt) - 1))

#define HV_MB_REG_RW_ATTR_R 'r'
#define HV_MB_REG_RW_ATTR_W 'w'
#define HV_MB_REG_RW_ATTR_RW '+'
const char* get_hv_mb_reg_str(hv_mb_reg_e_t reg_addr);
const char get_hv_mb_reg_rw_attr(hv_mb_reg_e_t reg_addr);

typedef enum
{
   MB_NORMAL_REG = 0,
   MB_EXTEND_REG,
   MB_INVALID_REG,
   MB_REG_CHECK_ERROR,
}mb_reg_check_ret_t;

#define MB_REG_V_DOSE_ADJ_ADD 0 
#define MB_REG_V_DOSE_ADJ_SUB 1

#define MB_REG_V_CHARGER_IN 1
#define MB_REG_V_CHARGER_OUT 0

#endif
