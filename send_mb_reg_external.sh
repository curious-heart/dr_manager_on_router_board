#!/bin/sh

. /usr/share/libubox/jshn.sh
. dr_manager_configuration.sh

mb_ref_content_file="/tmp/.dr_mb_reg_content"

mcu_exchg_json_val_reg="register"

json_init

json_add_string "$mcu_exchg_json_key_type" "$mcu_exchg_json_val_reg"

while read -t 1 line
do
    eval $(echo $line | awk -F, '{printf("reg=%s,var=%s",$1,$2)}')
    json_add_string "$reg" "$val"
done < $mb_ref_content_file

json_dump > $mcu_exchg_device
