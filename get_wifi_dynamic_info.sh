#!/bin/sh

#Note: read trim the leading space by default, so the ^[[:space:]] must be used carefully.
#
wifi_info_tmp_file=/tmp/.wifi_info
wlan_line_pattern="^wlan.*ESSID:"
Signal_line_pattern="^[[:space:]]\+Signal:"
Mode_line_pattern="^[[:space:]]\+Mode:"
drew_lines_pattern="$wlan_line_pattern""\|""$Signal_line_pattern""\|""$Mode_line_pattern"

iwinfo | grep "$drew_lines_pattern" > $wifi_info_tmp_file

ESSID_MARK="ESSID: "
CLIENT_MARK="Mode: Client"
MASTER_MARK="Mode: Master"
client_ssid=""
master_ssid1=""
master_ssid2=""
curr_ssid=""
master_idx=1
is_client=0
client_signal=0
signal_got=0
while read -t 1 line
do
    if [ "$signal_got" == "0" ]; then
        t_line_1=$(echo "$line" | grep "^[[:space:]]*Mode:[[:space:]]\+Client[[:space:]]\+Channel:[[:space:]]*[0-9]\{1,\}")
        if [ "$t_line_1" != "" ]; then
            is_client=1
        elif [ "$is_client" == "1" ]; then
            t_line_2=$(echo "$line" | grep "^[[:space:]]*Signal:[[:space:]]\+-\{0,1\}[0-9]\{1,\}")
            if [ "$t_line_2" != "" ]; then
                client_signal=$(echo $line | awk -F" " '{print $2}')
                #break
                signal_got=1
            fi
        fi
    fi

    local_str=$(echo "$line" | grep -o "$ESSID_MARK.*")
    if [ "$local_str" != "" ]; then
        curr_ssid=$(echo "$local_str" | sed "s/$ESSID_MARK//")
        curr_ssid=${curr_ssid#\"}
        curr_ssid=${curr_ssid%\"}
        continue
    fi
    local_str=$(echo "$line" | grep -o "$CLIENT_MARK.*")
    if [ "$local_str" != "" ]; then
        client_ssid=$curr_ssid
        continue
    fi
    local_str=$(echo "$line" | grep -o "$MASTER_MARK.*")
    if [ "$local_str" != "" ]; then
        if [ "$master_idx" == "1" ]; then
            master_ssid1=$curr_ssid
        else
            master_ssid2=$curr_ssid
        fi
        master_idx=$((master_idx+1))
        continue
    fi
done < $wifi_info_tmp_file

assoc_host_num=0
iw_devs=$(grep "$wlan_line_pattern" $wifi_info_tmp_file | awk -F" " '{print $1}')
for iw_dev in $iw_devs
do
    n=$(iw $iw_dev station dump | grep Station | wc -l)
    assoc_host_num=$(($assoc_host_num + $n))
done
assoc_host_num=$(($assoc_host_num - $is_client))

client_signal_bars=0
if [ $client_signal -lt -90 ]; then
    client_signal_bars=0
elif [ -90 -le $client_signal ] && [ $client_signal -lt -78 ]; then
    client_signal_bars=1
elif [ -78 -le $client_signal ] && [ $client_signal -lt -67 ]; then
    client_signal_bars=2
elif [ -67 -le $client_signal ] && [ $client_signal -lt -55 ]; then
    client_signal_bars=3
else
    client_signal_bars=4
fi

echo $assoc_host_num,$is_client,$client_signal,$client_signal_bars,$client_ssid,$master_ssid1,$master_ssid2
