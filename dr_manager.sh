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

if [ $dev_monitor_debug_flag = "1" ]; then
    dev_monitor_debug=--dev_monitor_debug
else
    dev_monitor_debug=
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
" --allow_force_exposure $mb_tcp_srvr_allow_force_exposure"\
" --req_tof_dist_wait_time $mb_tcp_srvr_req_tof_dist_wait_time"\
" --expo_tof_measure_wait $mb_tcp_srvr_expo_tof_measure_wait"\
" --srvr_regs_sync_period_int_s $srvr_regs_sync_period_int_s"\
" $mb_tcp_debug"\
" --dev_monitor_peroid $dev_monitor_period"\
" $dev_monitor_debug"\
" --lcd_dev_name $lcd_dev_name"\
" --lcd_dev_addr $lcd_i2c_addr"\
" --send_dev_info_period_int_s $send_dev_info_period_int_s"\
" --tof_measure_period $tof_measure_period"\
" --tof_dev_name $tof_dev_name"\
" --tof_dev_addr $tof_i2c_addr"\
" --tof_mech_cali $tof_mech_cali"\
" --tof_internal_cali $tof_internal_cali"\
" $app_work_mode"\
" --app_log_level $app_log_level"\
" --range_light_auto_off_time $range_light_auto_off_sec"

APP_NAME=dr_manager
echo $APP_NAME $cmd_args $*
$APP_NAME $cmd_args $* &


############################################################
KEY_APP_NAME=gpio_key_monitor
if [ $key_mb_tcp_srvr_debug_flag = "1" ]; then
    key_mb_tcp_debug=--mb_tcp_debug
else
    key_mb_tcp_debug=
fi

if [ $key_exp_start_disabled = "1" ]; then
    key_exp_start_disable_flag=--exp_start_disabled
else
    key_exp_start_disable_flag=
fi

if [ $tof_json_override_flag = "1" ]; then
    tof_json_override_opt=--tof_json_override
else
    tof_json_override_opt=
fi

key_cmd_args=" --exp_start_key_hold $exp_start_key_hold_time"\
" --mb_tcp_srvr_ip_addr $mb_tcp_srvr_ip"\
" --mb_tcp_srvr_port $mb_tcp_srvr_port"\
" --mb_tcp_client_wait_res_timeout_sec $mb_tcp_client_wait_res_timeout_sec"\
" $key_mb_tcp_debug"\
" --app_log_level $key_app_log_level"\
" $key_exp_start_disable_flag"\
" --mcu_exchg_device $mcu_exchg_device"\
" --gpio_clock_tick_sec $gpio_clock_tick_sec"\
" $tof_json_override_opt"\
" --restore_factory_key_hold_time $restore_factory_key_hold_sec"\
" --tof_smooth_range $tof_dist_smooth_range_mm"

echo
echo $KEY_APP_NAME $key_cmd_args $*
$KEY_APP_NAME $key_cmd_args $* &
