#!/bin/sh

. /usr/share/libubox/jshn.sh
. dr_manager_configuration.sh

get_wifi_d_info_script="/usr/bin/get_wifi_dynamic_info.sh"

mcu_exchg_json_val_data="info"

ssid_infos=$($get_wifi_d_info_script)
client_ssid=$(echo $ssid_infos | awk -F, '{print $5}')
master_ssid1=$(echo $ssid_infos | awk -F, '{print $6}')
master_ssid2=$(echo $ssid_infos | awk -F, '{print $7}')

hwaddr_title="HWaddr "
mac_addr_str=$(ifconfig wlan0 | grep -o "$hwaddr_title[a-f0-9A-F]\\([a-f0-9A-F]\\:[a-f0-9A-F]\\)\\{5\\}[a-f0-9A-F]" | sed "s/$hwaddr_title//")

if [ "$mac_addr_str" == "" ]; then
    echo 0
    exit
fi

version_file="/tmp/.dr_manager_version"
version_str=$(cat $version_file)

mac_addr_key="mac_address"
version_key="version"
client_ssid_key="client_ssid"
master_ssid1_key="master_ssid1"
master_ssid2_key="master_ssid2"

#echo "$mac_addr_key:" "$mac_addr_str"
#echo "$version_key:" "$version_str"
#echo "$client_ssid_key:" "$client_ssid"
#echo "$master_ssid1_key:" "$master_ssid1"
#echo "$master_ssid2_key:" "$master_ssid2"

json_init
json_add_string "$mcu_exchg_json_key_type" "$mcu_exchg_json_val_data"
json_add_string "$mac_addr_key" "$mac_addr_str"
json_add_string "$version_key" "$version_str"
#json_add_string "$client_ssid_key" "$client_ssid"
#json_add_string "$master_ssid1_key" "$master_ssid1"
#json_add_string "$master_ssid2_key" "$master_ssid2"
json_dump > $mcu_exchg_device
echo 1
