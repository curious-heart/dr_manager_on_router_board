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

#------------------------------
if [ $mb_rtu_debug_flag = "1" ]; then
    mb_rtu_debug=--mb_rtu_debug
else
    mb_rtu_debug=
fi

if [ $mb_tcp_srvr_debug_flag = "1" ]; then
    mb_tcp_debug=--mb_tcp_debug
else
    mb_tcp_debug=
fi

if [ $app_work_mode_flag = "0" ]; then
    app_work_mode=
elif [ $app_work_mode_flag = "1" ]; then
    app_work_mode=--rtu_master_only
else
    app_work_mode=--tcp_server_only
fi

cmd_args=" --com_dev $mb_rtu_serialPortName"\
" --mb_rtu_serialBaudRate $mb_rtu_serialBaudRate"\
" --mb_rtu_serialParity $mb_rtu_serialParity"\
" --mb_rtu_serialDataBits $mb_rtu_serialDataBits"\
" --mb_rtu_serialStopBits $mb_rtu_serialStopBits"\
" --mb_rtu_timeout_ms $mb_rtu_timeout_ms"\
" --mb_rtu_numberOfRetries $mb_rtu_numberOfRetries"\
" --mb_rtu_serverAddress $mb_rtu_serverAddress"\
" $mb_rtu_debug"\
" --mb_tcp_srvr_ip_addr $mb_tcp_srvr_ip"\
" --mb_tcp_srvr_port $mb_tcp_srvr_port"\
" --mb_tcp_srvr_long_time $mb_tcp_srvr_long_wait_time"\
" --mb_tcp_srvr_short_time $mb_tcp_srvr_short_wait_time"\
" $mb_tcp_debug"\
" --dev_monitor_peroid $dev_monitor_period"\
" --lcd_dev_name $lcd_dev_name"\
" --lcd_dev_addr $lcd_i2c_addr"\
" --tof_measure_period $tof_measure_period"\
" --tof_dev_name $tof_dev_name"\
" --tof_dev_addr $tof_i2c_addr"\
"$app_work_mode"\
" --app_log_level $app_log_level"

APP_NAME=dr_manager
echo $APP_NAME $cmd_args $*
$APP_NAME $cmd_args $*
