#!/bin/sh

. set_at_proc_parms.sh

#disable echo
echo -e "ATE0\r\n" > $at_port_dev
#change the URC port to avoid URC command overwrite at_res_file.
echo -e "AT+QURCCFG=\"urcport\",\"uart1\"\r\n" > $at_port_dev
