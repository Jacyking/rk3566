#!/bin/sh
dirname=`dirname $0`

if [ -f "$dirname/s7server2" ]; then
        sudo mv $dirname/s7server2 $dirname/s7server
fi

if [ ! -x "$dirname/s7server" ]; then
        sudo chmod +x $dirname/s7server
fi

if [ 0 -eq `ps -elf | grep -w s7server | grep -v grep | wc -l` ]; then
        sudo $dirname/s7server /dev/ttyS9 > $dirname/output.log 2>&1
fi
