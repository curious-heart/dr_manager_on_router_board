#!/bin/sh

. set_at_proc_parms.sh

test_flag=$1

cmd_name="+QENG"
cmd_parm="\"servingcell\""
cmd_str="AT$cmd_name=$cmd_parm"

NOSRV_MODE_STR="NOSRV"
ORI_CELL_MODE_LTE="LTE"
ORI_CELL_MODE_NR5GSA="NR5G-SA"
ORI_CELL_MODE_NR5GNSA="NR5G-NSA"
ORI_CELL_MODE_WCDMA="WCDMA"

if [ "x$test_flag" = "x" ] || [ ! -e $at_res_file ]; then #$test flag is empty, that is, NOT test mode.
    #send AT cmd
    echo -e "$cmd_str\r\n" > $at_port_dev

    #save the AT cmd response into $at_res_file.
    rm -rf $at_res_file
    while read -t 1 line
    do
        echo "$line" >> $at_res_file
    done < $at_port_dev
fi

#parse the $at_res_file.
#ouput is as the following: mode,state,signal_bars
#5G/4G/3G/NOSRV,SEARCH/LIMSRV/NOCONN/CONNECT, 5/4/3/2/1/0
cell_mode=$NOSRV_MODE_STR
cell_state="SEARCH"
signal_bars=0

ori_cell_mode=$cell_mode
rsrp=-200 #-156
rsrq=-200 #-43
rscp=-200 #-113
ecno=-200 #-25
sinr=-200 #-100
rssi=-200 #-100

cell_mode_map()
{
    case "$1" in
        "$ORI_CELL_MODE_LTE")
            mapped_v="4G"
            ;;
        "$ORI_CELL_MODE_NR5GSA")
            mapped_v="5G"
            ;;
        "$ORI_CELL_MODE_NR5GNSA")
            mapped_v="5G"
            ;;
        "$ORI_CELL_MODE_WCDMA")
            mapped_v="3G"
            ;;
        *)
            mapped_v=$NOSRV_MODE_STR
            ;;
    esac

    cell_mode=$mapped_v
}

signal_bars_map()
{
    #parameter list: ori_mode rsrp rsrq rscp ecno rssi sinr
    l_mode=$1
    l_rsrp=$2
    l_rsrq=$3
    l_rscp=$4
    l_ecno=$5
    l_rssi=$6
    l_sinr=$7

    l_bars=0
    case "$l_mode" in
        "$ORI_CELL_MODE_LTE")
            #now just use l_rsrp
            if [ $l_rsrp -lt -140 ]; then
                l_bars=0
            elif [ -140 -le $l_rsrp ] && [ $l_rsrp -lt -120 ]; then
                l_bars=1
            elif [ -120 -le $l_rsrp ] && [ $l_rsrp -lt -100 ]; then
                l_bars=2
            elif [ -100 -le $l_rsrp ] && [ $l_rsrp -lt -80 ]; then
                l_bars=3
            elif [ -80 -le $l_rsrp ] && [ $l_rsrp -lt -60 ]; then
                l_bars=4
            else
                l_bars=5
            fi
            ;;
        "$ORI_CELL_MODE_NR5GSA")
            #now just use l_rsrp
            if [ $l_rsrp -lt -156 ]; then
                l_bars=0
            elif [ -156 -le $l_rsrp ] && [ $l_rsrp -lt -131 ]; then
                l_bars=1
            elif [ -131 -le $l_rsrp ] && [ $l_rsrp -lt -106 ]; then
                l_bars=2
            elif [ -106 -le $l_rsrp ] && [ $l_rsrp -lt -81 ]; then
                l_bars=3
            elif [ -81 -le $l_rsrp ] && [ $l_rsrp -lt -56 ]; then
                l_bars=4
            else
                l_bars=5
            fi
            ;;
        "$ORI_CELL_MODE_NR5GNSA")
            #now just use l_rsrp
            if [ $l_rsrp -lt -156 ]; then
                l_bars=0
            elif [ -156 -le $l_rsrp ] && [ $l_rsrp -lt -131 ]; then
                l_bars=1
            elif [ -131 -le $l_rsrp ] && [ $l_rsrp -lt -106 ]; then
                l_bars=2
            elif [ -106 -le $l_rsrp ] && [ $l_rsrp -lt -81 ]; then
                l_bars=3
            elif [ -81 -le $l_rsrp ] && [ $l_rsrp -lt -56 ]; then
                l_bars=4
            else
                l_bars=5
            fi
            ;;
        "$ORI_CELL_MODE_WCDMA")
            #now just use rscp
            if [ $l_rscp -lt -112 ]; then
                l_bars=0
            elif [ -112 -le $l_rscp ] && [ $l_rscp -lt -90 ]; then
                l_bars=1
            elif [ -90 -le $l_rscp ] && [ $l_rscp -lt -68 ]; then
                l_bars=2
            elif [ -68 -le $l_rscp ] && [ $l_rscp -lt -46 ]; then
                l_bars=3
            elif [ -46 -le $l_rscp ] && [ $l_rscp -lt -24 ]; then
                l_bars=4
            else
                l_bars=5
            fi
            ;;
        *)
            ;;
    esac
    signal_bars=$l_bars
}

while read line
do
    echo $line | awk -F: '{print $1" "$2}' > $at_proc_tmp_file
    read tk0 tk1n < $at_proc_tmp_file
    if [ "$tk0" = "$cmd_name" ]; then
        echo $tk1n | awk -F, '{print $1" "$2" "$3}' > $at_proc_tmp_file
        read tk1 tk2 tk3 < $at_proc_tmp_file
        if [ "x$tk3" = "x" ]; then
            cell_state=${tk2//\"/}
            continue
        elif [ "$tk1" = "$cmd_parm" ]; then
            cell_state=${tk2//\"/}
            ori_cell_mode=${tk3//\"/}
        else
            ori_cell_mode=${tk1//\"/}
        fi
        case "$ori_cell_mode" in
            "$ORI_CELL_MODE_LTE")
                if [ "$tk1" = "$cmd_parm" ]; then
                    echo $tk1n | awk -F, '{print $14" "$15" "$16" "$17}' > $at_proc_tmp_file
                else
                    echo $tk1n | awk -F, '{print $12" "$13" "$14" "$15}' > $at_proc_tmp_file
                fi
                read rsrp rsrq rssi sinr < $at_proc_tmp_file
                ;;
            "$ORI_CELL_MODE_NR5GSA")
                echo $tk1n | awk -F, '{print $13" "$14" "$15}' > $at_proc_tmp_file
                read rsrp rsrq sinr < $at_proc_tmp_file
                ;;
            "$ORI_CELL_MODE_NR5GNSA")
                echo $tk1n | awk -F, '{print $5" "$6" "$7}' > $at_proc_tmp_file
                read rsrp rsrq sinr < $at_proc_tmp_file
                ;;
            "$ORI_CELL_MODE_WCDMA")
                echo $tk1n | awk -F, '{print $10" "$11}' > $at_proc_tmp_file
                read rscp ecno < $at_proc_tmp_file
                ;;
            *)
                ;;
        esac

    fi
done < $at_res_file

cell_mode_map $ori_cell_mode

signal_bars_map $ori_cell_mode $rsrp $rsrq $rscp $ecno $rssi $sinr

echo cell_mode:$cell_mode
echo cell_state:$cell_state
echo signal_bars:$signal_bars
echo rsrp:$rsrp
echo rsrq:$rsrq
echo rscp:$rscp
echo ecno:$ecno
echo sinr:$sinr
echo rssi:$rssi
echo ori_cell_mode:$ori_cell_mode
