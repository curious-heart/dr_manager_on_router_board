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
    C(MAX_HV_NORMAL_MB_REG_NUM), /*normal register end flag.*/\
\
    C(EXT_MB_REG_START_FLAG = 100), /*extend register start flag.*/\
    /*Below are extend register, that is, they are processed internally by server and not passed to hv controller.*/ \
    C(EXT_MB_REG_DOSE_ADJ = 101),                       /*+/- key event*/\
    C(EXT_MB_REG_CHARGER = 102),                       /*charger plug in/pull out*/\
    C(EXT_MB_REG_DAP_HP = 103),                       /*High part of a float of DAP(Dose Area Product), big endian.*/\
    C(EXT_MB_REG_DAP_LP = 104),                       /*Low part of a float of DAP, big endian.*/\
    C(EXT_MB_REG_DISTANCE = 105),                       /* uint16，测距结果。单位mm*/\
    C(EXT_MB_REG_HOTSPOT_ST = 106),           /*uint16，本机Wi-Fi热点状态。*/\
                                              /*0xFFFF表示Wi-Fi热点未开启；*/\
                                              /*其它值表示热点开启，数值指示热点上连接的Client数量*/\
    C(EXT_MB_REG_CELLUAR_ST = 107),              /*uint16，高字节表示蜂窝网的信号格数，有效值0~5；*/\
                                                         /*低字节表示蜂窝网状态：0-无服务；1-3G；2-4G；3-5G*/\
    C(EXT_MB_REG_WIFI_WAN_SIG_AND_BAT_LVL = 108), /*uint16，高字节指示电池电量(BatteryLevel)；*/\
                                                           /*低字节指示WAN侧Wi-Fi信号格数，有效值0~4*/\
    C(EXT_MB_REG_DEV_INFO_BITS = 109), /*uint16的每个bit指示一个设备的二值状态信息：*/\
                                        /*第0位：0-没有连接充电器，1-连接充电器。*/\
                                        /*第1位：0-电池电量未充满，1-电池电量已充满。这一位目前无法使用，因为路由板的GPIO没办法读取到正确的状态。*/\
                                        /*第2位：0-WAN侧Wi-Fi没有连接；1-WAN侧Wi-Fi已连接。*/\
                                        /*第3位：0-WAN侧蜂窝网没有连接；1-WAN侧蜂窝网已连接。*/\
                                        /*第4位：0-SIM卡异常（未插卡，或者卡锁定、卡不识别）；1-SIM卡正常识别。*/\
\
    C(HV_MB_REG_END_FLAG), /*register end flag.*/\
}

#undef C
#define C(a) a
typedef enum MB_REG_ENUM hv_mb_reg_e_t;

#define MB_REG_EXPOSURE_START_CMD 2

#define MB_REG_DEV_INFO_BITS_CHG_CONN ((uint16_t)0x0001)
#define MB_REG_DEV_INFO_BITS_BAT_FULL ((uint16_t)0x0002)
#define MB_REG_DEV_INFO_BITS_WIFI_WAN_CONN ((uint16_t)0x0004)
#define MB_REG_DEV_INFO_BITS_CELL_WAN_CONN ((uint16_t)0x0008)
#define MB_REG_DEV_INFO_BITS_SIM_READY ((uint16_t)0x0010)

#define SET_DEV_INFO_BITS(info, set, mask) \
{\
    if((set) == (info))\
    {                     \
        info_word |= (mask);\
    }                     \
    else                  \
    {                     \
        info_word &= ~(mask);\
    }\
}

#define NORMAL_MB_REG_ADDR(addr) (HSV <= (addr) && (addr) < MAX_HV_NORMAL_MB_REG_NUM)
#define EXTEND_MB_REG_ADDR(addr) (EXT_MB_REG_START_FLAG < (addr) && (addr) < HV_MB_REG_END_FLAG)
#define VALID_MB_REG_ADDR(addr) (NORMAL_MB_REG_ADDR(addr) || EXTEND_MB_REG_ADDR(addr))
#define NORMAL_MB_REG_AND_CNT(addr, cnt) (NORMAL_MB_REG_ADDR(addr) && NORMAL_MB_REG_ADDR((addr) + (cnt) - 1))
#define EXTEND_MB_REG_AND_CNT(addr, cnt) (EXTEND_MB_REG_ADDR(addr) && EXTEND_MB_REG_ADDR((addr) + (cnt) - 1))
#define VALID_MB_REG_AND_CNT(addr, cnt) (VALID_MB_REG_ADDR(addr) && VALID_MB_REG_ADDR((addr) + (cnt) - 1))

/*those registers that need to communicate with dsp.*/
#define MB_REG_COMM_DSP(addr, cnt) (NORMAL_MB_REG_AND_CNT(addr, cnt) || (EXT_MB_REG_DOSE_ADJ == addr))

/* The number of registers, including the two flag: MAX_HV_NORMAL_MB_REG_NUM and EXT_MB_REG_START_FLAG,
 * but not including the HV_MB_REG_END_FLAG.
 */
#define ALL_MB_REG_NUM (MAX_HV_NORMAL_MB_REG_NUM + (HV_MB_REG_END_FLAG - EXT_MB_REG_START_FLAG))

/* The extend reg addr to its order idx.*/
#define EXTEND_MB_REG_ADDR2IDX(ext_reg_addr) (MAX_HV_NORMAL_MB_REG_NUM + 1 + ((ext_reg_addr) - EXT_MB_REG_START_FLAG)) 
/* The reg addr to its order idx. refer to its use in function get_hv_mb_reg_str.*/
#define MB_REG_ADDR2IDX(reg_addr) (NORMAL_MB_REG_ADDR(reg_addr) ? (reg_addr) : EXTEND_MB_REG_ADDR2IDX(reg_addr))

#define HV_MB_REG_RW_ATTR_R 'r'
#define HV_MB_REG_RW_ATTR_W 'w'
#define HV_MB_REG_RW_ATTR_RW '+'
const char* get_hv_mb_reg_str(hv_mb_reg_e_t reg_addr);
const char get_hv_mb_reg_rw_attr(hv_mb_reg_e_t reg_addr);

/*reg_addr should be uint16_t array of at least 2 items, and the fist two items contain the high and low part of DAP.*/
#define LOWEST_BYTE_ADDR_OF_DAP_IN_REG_ARR(reg_arr) ((uint8_t*)(&((reg_arr)[1])) + 1)
#define HIGHEST_BYTE_ADDR_OF_DAP_IN_REG_ARR(reg_arr) ((uint8_t*)(&(reg_arr)[0]))
float get_float_DAP_from_reg_arr(uint16_t * dap_high_reg_ptr);

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
