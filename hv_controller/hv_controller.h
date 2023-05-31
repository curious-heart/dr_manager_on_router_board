#ifndef HV_CONTROLLER_H
#define HV_CONTROLLER_H

#include <stdbool.h>

typedef enum
{
    HSV = 0,                            //软硬件版本
    OTA = 1,                            //OTA升级
    BaudRate = 2,                       //波特率
    ServerAddress = 3,                  //设备地址
    State = 4,                          //状态
    VoltSet = 5,  					 	//5管电压设置值
    FilamentSet = 6,					//6 管设置值电流 （决定灯丝电流决定管电流）
    ExposureTime = 7,                   //曝光时间
    Voltmeter = 8,                      //管电压读出值
    Ammeter = 9,                        //管电流读出值
    RangeIndicationStatus = 10,         //范围指示状态
    ExposureStatus = 11,                //曝光状态
    RangeIndicationStart = 12,          //范围指示启动
    ExposureStart = 13,                 //曝光启动
    BatteryLevel = 14,                  //电池电量
    BatteryVoltmeter = 15,
    OilBoxTemperature = 16,             //电池电压高位
    Poweroff = 17,							//17 关机请求
    Fixpos = 18, 								//18 油盒温度低位
    Fixval = 19, 								//19 充能状态
    Workstatus = 20,						//20充能状态
    exposureCount = 21,                     //曝光次数
}hv_mb_reg_e_t;

bool hv_controller_init();
bool hv_controller_connect();
bool hv_controller_write_uint16(int reg_addr, unsigned short value);
bool hv_controller_read_data();

#endif // HV_CONTROLLER_H
