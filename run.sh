#!/bin/sh
appname=app
appname2=app2
dirname=`dirname $0`

if [ -f "$dirname/$appname2" ]; then
        sudo mv $dirname/$appname2 $dirname/$appname
        killall -9 $appname
fi

if [ 0 -eq `ps -elf | grep -w xcompmgr | grep -v grep | wc -l` ]; then
        hsetroot -solid "#000000"
        xcompmgr &
fi

if [ 0 -eq `ps -elf | grep -w openbox | grep -v grep | wc -l` ]; then
        openbox &
fi

if [ 1 -eq `ps -elf | grep -w $appname | grep -v grep | wc -l` ]; then
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
