#!/bin/sh

. /usr/share/libubox/jshn.sh

external_dev="/dev/ttyS2"
get_wifi_d_info_script="/usr/bin/get_wifi_dynamic_info.sh"

ssid_infos=$($get_wifi_d_info_script)
client_ssid=$(echo $ssid_infos | awk -F, '{print $5}')
master_ssid1=$(echo $ssid_infos | awk -F, '{print $6}')
master_ssid2=$(echo $ssid_infos | awk -F, '{print $7}')

hwaddr_title="HWaddr "
mac_addr_str=$(ifconfig wlan0 | grep -o "$hwaddr_title.*" | sed "s/$hwaddr_title//")

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
json_add_string "$mac_addr_key" "$mac_addr_str"
json_add_string "$version_key" "$version_str"
json_add_string "$client_ssid_key" "$client_ssid"
json_add_string "$master_ssid1_key" "$master_ssid1"
json_add_string "$master_ssid2_key" "$master_ssid2"
json_dump > $external_dev
