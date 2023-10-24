#!/bin/sh

#currently only mac addr last 6 digit is returned.
mac_tail6=$(ifconfig wlan0 | grep "HWaddr " | awk -F: '{print $5$6$7}')
mac_tail6=${mac_tail6:0:6}
mac_tail6=${mac_tail6/://}

echo $mac_tail6
