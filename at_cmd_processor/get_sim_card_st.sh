#!/bin/sh

. set_at_proc_parms.sh

test_flag=$1

cmd_name="+CIMI"
cmd_str="AT$cmd_name"

if [ ! -e $at_init_flag ]; then
    init_at_st.sh
    if [ "$?" != "0" ]; then
        #init at fails. maybe because modem device has not started up. just exit and app will call this script in next cycle.
        exit 1
    fi
fi

if [ -c $at_port_dev ] && ([ "x$test_flag" = "x" ] || [ ! -e $at_res_file ]); then #$test flag is empty
    #send AT cmd
    echo -e "$cmd_str\r\n" > $at_port_dev

    #save the AT cmd response into $at_res_file.
    rm -rf $at_res_file
    while read -t 1 line
    do
        echo "$line" >> $at_res_file
    done < $at_port_dev
fi

imsi=""
if [ -e $at_res_file ]; then
    while read line
    do
        imsi=$(expr match "$line" '\([0-9]*\)')
        if [ ! "x$imsi" = "x" ]; then
            break
        fi
    done < $at_res_file
fi

echo imsi:$imsi
