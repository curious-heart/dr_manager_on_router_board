#include <signal.h>

#include "logger.h"
#include "hv_registers.h"
#include "hv_controller.h"

void exposure_test(const char* dev_name)
{
    bool end = false;
    int test_no;
    hv_mb_reg_e_t reg_addr;
    uint16_t read_data;

    if(!hv_controller_open(dev_name, true))
    {
        return;
    }

    while(!end)
    {
        printf("\n");
        printf("HSV = 0,                   //软硬件版本\n");
        printf("OTA = 1,                   //OTA升级\n");
        printf("BaudRate = 2,              //波特率\n");
        printf("ServerAddress = 3,         //设备地址\n");
        printf("State = 4,                 //状态\n");
        printf("VoltSet = 5,               //5管电压设置值\n");
        printf("FilamentSet = 6,           //6 管设置值电流 （决定灯丝电流决定管电流）\n");
        printf("ExposureTime = 7,          //曝光时间\n");
        printf("Voltmeter = 8,             //管电压读出值\n");
        printf("Ammeter = 9,               //管电流读出值\n");
        printf("RangeIndicationStatus = 10,//范围指示状态\n");
        printf("ExposureStatus = 11,       //曝光状态\n");
        printf("RangeIndicationStart = 12, //范围指示启动\n");
        printf("ExposureStart = 13,        //曝光启动\n");
        printf("BatteryLevel = 14,         //电池电量\n");
        printf("BatteryVoltmeter = 15,\n");
        printf("OilBoxTemperature = 16,    //电池电压高位\n");
        printf("Poweroff = 17,             //17 关机请求\n");
        printf("Fixpos = 18,               //18 油盒温度低位\n");
        printf("Fixval = 19,               //19 充能状态\n");
        printf("Workstatus = 20,           //20充能状态\n");
        printf("exposureCount = 21,        //曝光次数\n");
        printf("-1: exit.\n");

        scanf("%d", &test_no);
        if(test_no < 0)
        {
            end = true;
            break;
        }
        reg_addr = (hv_mb_reg_e_t)test_no;
        switch(reg_addr)
        {
            case HSV:
            case OTA:
            case BaudRate:
            case ServerAddress:
            case State:
            case VoltSet:
            case FilamentSet:
                break;

            case ExposureTime:
                read_data = 0xFFFF;
                hv_controller_read_uint16s((int)reg_addr, &read_data, 1);
                printf("ExposureTime: %d\n", read_data);
                break;

            case Voltmeter:
            case Ammeter:
            case RangeIndicationStatus:
                break;

            case ExposureStatus:
                hv_controller_read_uint16s((int)reg_addr, &read_data, 1);
                printf("ExposureStatus: %d\n", read_data);
                break;

            case RangeIndicationStart:
                break;

            case ExposureStart:
                hv_controller_write_single_uint16((int)ExposureStart, 2);
                break;

            case BatteryLevel:
            case BatteryVoltmeter:
            case OilBoxTemperature:
            case Poweroff:
            case Fixpos:
            case Fixval:
            case Workstatus:
            case exposureCount:
            default:
                printf("Invlaid input value.\n");
                break;
        }
        printf("\n");
    }
    hv_controller_close();
}

static void clear_for_exit()
{
    mb_server_exit();
    hv_controller_close();
}

static void close_sigint(int dummy)
{
    clear_for_exit();
    exit(dummy);
}

int main(int argc, char *argv[])
{
    const char* dev_name = NULL;
    mb_server_exit_code_t mb_server_ret;

    if(argc > 1)
    {
        dev_name = argv[1];
    }

    signal(SIGINT, close_sigint);

    if(!hv_controller_open(dev_name, true))
    {
        DIY_LOG(LOG_ERROR, "Connecting high volatage board fails.\n");
        return -1;;
    }

    mb_server_ret = mb_server_loop("127.0.0.1", 1502, true);
    switch(mb_server_ret)
    {
        case MB_SERVER_EXIT_INIT_FAIL:
            DIY_LOG(LOG_ERROR, "modbus server startup fails.!\n");
            break;

        case MB_SERVER_EXIT_COMM_FATAL_ERROR: 
            DIY_LOG(LOG_ERROR, "modbus server communication fatal error.!\n");
            close_sigint(mb_server_ret);
            break;

         default:
            DIY_LOG(LOG_ERROR, "modbus server exit for unknow reason.!\n");
            close_sigint(mb_server_ret);
    }

    clear_for_exit();
    return 0;
}
