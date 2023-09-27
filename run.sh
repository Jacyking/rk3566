#!/bin/sh
appname=app
appname2=app2
dirname=`dirname $0`

if [ -f "$dirname/$appname2" ]; then
        mv $dirname/$appname2 $dirname/$appname
        killall -9 $appname
fi

running=`ps -elf | grep -w s7server | grep -v grep | wc -l`
if [ 0 -eq $running ]; then
        sudo nohup $dirname/s7server /dev/ttyS9 > $dirname/output.log 2>&1 &
fi

running=`ps -elf | grep -w $appname | grep -v grep | wc -l`
if [ 1 -eq $running ]; then
        return
fi

if [ ! -x "$dirname/$appname" ]; then
        chmod +x $dirname/$appname
fi

ulimit -c unlimited

cd $dirname

xcompmgr &

openbox &

QT_IM_MODULE=tgtsml ./$appname "$@"
