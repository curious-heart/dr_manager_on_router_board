#!/bin/sh

. /usr/share/libubox/jshn.sh

external_dev="/dev/ttyS2"
mb_ref_content_file="/tmp/.dr_mb_reg_content"

type_key="json_type"
type_key_val="register"

json_init

json_add_string "$type_key" "$type_key_val"

while read -t 1 line
do
    reg=$(echo $line | awk -F, '{print $1}')
    val=$(echo $line | awk -F, '{print $2}')
    json_add_string "$reg" "$val"
done < $mb_ref_content_file

json_dump > $external_dev
