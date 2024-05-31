#!/bin/sh

. /usr/share/libubox/jshn.sh

tmp_recvd_json_file="/tmp/.recvd_json_msg";

tof_key="tof_distance"

key_name_key="key_name"
key_name_adj_add="add"
key_name_adj_sub="sub"
key_val_key="key_val"
key_val_pressed="pressed"
key_val_released="released"

json_load_file $tmp_recvd_json_file

json_get_var distance_val $tof_key

json_get_var key_name $key_name_key
json_get_var key_val $key_val_key

tof_hd=$tof_key
if [ "$distance_val" != "" ]; then
    echo "$tof_hd:$distance_val"
fi

key_evt_hd="key_evt"
if [ "$key_name" != "" ]  && [ "$key_val" != "" ]; then
    echo "$key_evt_hd:$key_name,$key_val"
fi
