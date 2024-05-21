#!/bin/sh

. /usr/share/libubox/jshn.sh

tmp_json_file="/tmp/.tof_json_msg"

tof_key="tof_distance"

json_load_file $tmp_json_file
json_get_var distance_val $tof_key 
echo $distance_val
