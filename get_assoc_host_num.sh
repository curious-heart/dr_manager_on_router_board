#!/bin/sh

assoc_host_num=0
iw_devs=$(iwinfo | grep wlan | awk -F" " '{print $1}')
for iw_dev in $iw_devs
do
    n=$(iw $iw_dev station dump | grep Station | wc -l)
    assoc_host_num=$(($assoc_host_num + $n))
done

echo $assoc_host_num
