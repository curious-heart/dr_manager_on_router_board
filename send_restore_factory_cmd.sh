#!/bin/sh

. /usr/share/libubox/jshn.sh
. dr_manager_configuration.sh


mcu_exchg_json_val_cmd="cmd"
cmd_key="command"
cmd_val="factory reset"

json_init
json_add_string "$mcu_exchg_json_key_type" "$mcu_exchg_json_val_cmd"
json_add_string "$cmd_key" "$cmd_val"
json_dump > $mcu_exchg_device
