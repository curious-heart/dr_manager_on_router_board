#!/bin/sh

. set_at_proc_parms.sh

rm -rf $at_init_flag

#disable echo
if [ -c $at_port_dev ]; then #$test flag is empty
    echo -e "ATE0\r\n" > $at_port_dev
    #change the URC port to avoid URC command overwrite at_res_file.
    echo -e "AT+QURCCFG=\"urcport\",\"uart1\"\r\n" > $at_port_dev

    touch $at_init_flag
    exit 0
else
    exit 1
fi
