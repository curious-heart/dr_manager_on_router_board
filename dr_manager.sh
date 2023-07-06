#!/bin/sh

. dr_manager_configuration.sh

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
