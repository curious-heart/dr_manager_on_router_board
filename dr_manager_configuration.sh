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
mb_tcp_srvr_short_wait_time=0.5
mb_tcp_srvr_debug_flag=0
#monitor (check device state) period.
dev_monitor_period=3
#LCD
lcd_dev_name="/dev/i2c-0"
lcd_i2c_addr=0x3C
#TOF
tof_measure_period=1
tof_dev_name="/dev/i2c-0"
tof_i2c_addr=0x52
#app work mode
app_work_mode_flag=0
#app log level
app_log_level=1
mb_rtu_debug_flag=1
mb_tcp_debug_flag=1

