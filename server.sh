#!/bin/sh
dirname=`dirname $0`
if [ 0 -eq `ps -elf | grep -w s7server | grep -v grep | wc -l` ]; then
        sudo nohup $dirname/s7server /dev/ttyS9 > $dirname/output.log 2>&1 &
fi
