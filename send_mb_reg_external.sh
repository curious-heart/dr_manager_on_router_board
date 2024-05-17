#!/bin/sh

. /usr/share/libubox/jshn.sh
. dr_manager_configuration.sh

mb_ref_content_file="/tmp/.dr_mb_reg_content"

mcu_exchg_json_val_reg="register"

json_init

json_add_string "$mcu_exchg_json_key_type" "$mcu_exchg_json_val_reg"

while read -t 1 line
do
    reg=$(echo $line | awk -F, '{print $1}')
    val=$(echo $line | awk -F, '{print $2}')
    json_add_string "$reg" "$val"
done < $mb_ref_content_file

json_dump > $mcu_exchg_device
