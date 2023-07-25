#!/bin/sh

. set_at_proc_parms.sh

cmd_name="+QENG"
cmd_parm="\"servingcell\""
cmd_str="AT$cmd_name=$cmd_parm"

ORI_CELL_MODE_LTE="LTE"
ORI_CELL_MODE_NR5GSA="NR5G-SA"
ORI_CELL_MODE_NR5GNSA="NR5G-NSA"
ORI_CELL_MODE_WCDMA="WCDMA"

#send AT cmd
echo -e "$cmd_str\r\n" > $at_port_dev

#save the AT cmd response into $at_res_file.
rm -rf $at_res_file
while read -t 1 line
do
    echo "$line" >> $at_res_file
done < $at_port_dev

#parse the $at_res_file.
#ouput is as the following: mode,state,signal_bars
#5G/4G/3G/NOSRV,SEARCH/LIMSRV/NOCONN/CONNECT, 5/4/3/2/1/0
cell_mode="NOSRV"
cell_state="SEARCH"
signal_bars=0

ori_cell_mode=$cell_mode
rsrp=-156
rsrq=-43
sinr=-100
rssi=-100
rscp=-100
while read line
do
    echo $line
    echo $line | awk -F: '{print $1" "$2}' > $at_proc_tmp_file
    read tk0 tk1n < $at_proc_tmp_file
    if [ "$tk0" = "$cmd_name" ]; then
        echo $tk1n | awk -F, '{print $1" "$2" "$3}' > $at_proc_tmp_file
        read tk1 tk2 tk3 < $at_proc_tmp_file
        if [ "x$tk3" = "x" ]; then
            cell_state=$tk2
            continue
        elif [ "$tk1" = "$cmd_parm" ]; then
            cell_state=$tk2
            ori_cell_mode=$tk3
        else
            ori_cell_mode=$tk1
        fi
        case "$ori_cell_mode" in
            $ORI_CELL_MODE_LTE)
                if [ "$tk1" = "$cmd_parm" ]; then
                    echo $tk1n | awk -F, '{print $14" "$15" "$16" "$17}' > $at_proc_tmp_file
                else
                    echo $tk1n | awk -F, '{print $12" "$13" "$14" "$15}' > $at_proc_tmp_file
                fi
                read rsrp rsrq rssi sinr < $at_proc_tmp_file
                ;;
            $ORI_CELL_MODE_NR5GSA)
                echo $tk1n | awk -F, '{print $13" "$14" "$15}' > $at_proc_tmp_file
                read rsrp rsrq sinr < $at_proc_tmp_file
                ;;
            $ORI_CELL_MODE_NR5GNSA)
                echo $tk1n | awk -F, '{print $5" "$6" "$7}' > $at_proc_tmp_file
                read rsrp rsrq sinr < $at_proc_tmp_file
                echo;;
            $ORI_CELL_MODE_WCDMA)
                echo $tk1n | awk -F, '{print $10}' > $at_proc_tmp_file
                read rscp < $at_proc_tmp_file
                echo;;
            *)
                ;;
        esac

    fi
    #tk0=token before ":", then tk1 to tkn is the token seperated by ","
    #if $line begins with "$cmd_name:"
    #    if $tk1 is equal to $cmd_parm
    #        cell_state=$tk2
    #        if number of tokens > 3
    #            $cell_mode=map $tk3
    #            $signal_bars=map ...
    #    else
    #        $cell_mode=map $tk1
    #            
done < $at_res_file

echo ori_cell_mode:$ori_cell_mode
echo cell_state:$cell_state
echo rsrp:$rsrp
echo rsrq:$rsrq
echo sinr:$sinr
echo rssi:$rssi
echo rscp:$rscp

echo "---------------------"
echo "$cell_mode,$cell_state,$signal"
