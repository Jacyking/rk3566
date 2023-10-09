#!/bin/sh
appname=app
appname2=app2
dirname=`dirname $0`

if [ -f "$dirname/$appname2" ]; then
        sudo mv $dirname/$appname2 $dirname/$appname
        killall -9 $appname
fi

running=`ps -elf | grep -w s7server | grep -v grep | wc -l`
if [ 0 -eq $running ]; then
        sudo nohup $dirname/s7server /dev/ttyS9 > $dirname/output.log 2>&1 &
fi

running=`ps -elf | grep -w xcompmgr | grep -v grep | wc -l`
if [ 0 -eq $running ]; then
	hsetroot -solid "#000000"
	xcompmgr &
fi

running=`ps -elf | grep -w openbox | grep -v grep | wc -l`
if [ 0 -eq $running ]; then
        openbox -nocursor &
fi

running=`ps -elf | grep -w $appname | grep -v grep | wc -l`
if [ 1 -eq $running ]; then
        return
fi

bright=/sys/class/backlight/backlight4/brightness
if [ ! -O $bright ]; then
        sudo chown ${USER} $bright
fi

if [ ! -x "$dirname/$appname" ]; then
        sudo chmod +x $dirname/$appname
fi

if [ `ulimit -c` != 'unlimited' ]; then
        ulimit -c unlimited
fi

cd $dirname

sudo QT_IM_MODULE=tgtsml ./$appname "$@"
