#!/bin/sh

. set_at_proc_parms.sh

test_flag=$1

cmd_name="+CIMI"
cmd_str="AT$cmd_name"

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

imsi=""
while read line
do
    imsi=$(expr match "$line" '\([0-9]*\)')
    if [ ! "x$imsi" = "x" ]; then
        break
    fi
done < $at_res_file

echo imsi:$imsi
