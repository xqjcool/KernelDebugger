#!/bin/sh
KVERSION=$(uname -r)
KMAJOR=$(echo $KVERSION | cut -d. -f1)
KMINOR=$(echo $KVERSION | cut -d. -f2)

kallsyms_lookup_name_addr=$(cat /proc/kallsyms | grep -w kallsyms_lookup_name | awk '{print $1}')

if [ "$KMAJOR" -gt 5 ] ;then
	insmod $@ lookup_func_addr=0x$kallsyms_lookup_name_addr
elif [ "$KMAJOR" -eq 5 ] && [ "$KMINOR" -ge 7 ] ;then
	insmod $@ lookup_func_addr=0x$kallsyms_lookup_name_addr
else
	insmod $@
fi
