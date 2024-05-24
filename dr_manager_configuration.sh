#!/bin/sh

#modbus rtu parameters
mb_rtu_serialPortName="/dev/ttyS1"
mb_rtu_serialBaudRate=9600
mb_rtu_serialParity='N'
mb_rtu_serialDataBits=8
mb_rtu_serialStopBits=1
mb_rtu_timeout_ms=999
mb_rtu_numberOfRetries=3
mb_rtu_serverAddress=1
mb_rtu_debug_flag=0
#modbus tcp server parameters
mb_tcp_srvr_ip=0.0.0.0
mb_tcp_srvr_port=502
mb_tcp_srvr_long_wait_time=10
mb_tcp_srvr_short_wait_time=1
mb_tcp_srvr_debug_flag=0
mb_tcp_srvr_allow_force_exposure=0
mb_tcp_srvr_req_tof_dist_wait_time=1.5
mb_tcp_srvr_expo_tof_measure_wait=0
#monitor (check device state) period.
dev_monitor_period=3
dev_monitor_debug_flag=0
#LCD
lcd_dev_name="/dev/i2c-0"
lcd_i2c_addr=0x3C
#TOF
tof_measure_period=1
tof_dev_name="/dev/i2c-0"
tof_i2c_addr=0x52
tof_mech_cali=100
tof_internal_cali=-40
#app work mode
app_work_mode_flag=0
#app log level
app_log_level=3
mb_rtu_debug_flag=0
mb_tcp_debug_flag=0


#the fowllowing are key app configuration.
exp_start_key_hold_time=5
key_app_log_level=3
key_mb_tcp_srvr_debug_flag=0
key_exp_start_disabled=0
#this timeout should not be too short or there may be sequence of rw errors following one "invalid data" error (due to timeout)
mb_tcp_client_wait_res_timeout_sec=3

#used by exchange info with external mcu
mcu_exchg_device="/dev/ttyS2"
mcu_exchg_json_key_type="json_type"
tof_json_override_flag=1

#gpio clock tick. should be int.
gpio_clock_tick_sec=1

#should be uint
range_light_auto_off_sec=20

#should be uint
restore_factory_key_hold_sec=10
