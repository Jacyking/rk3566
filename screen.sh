#!/bin/sh
if [ 1 -eq `xrandr | grep connected | wc -l` ]; then
	return
fi
path=/etc/profile.d/res.sh
if [ ! -f $path ]; then
	touch $path
fi
if [ -O $path ]; then
	device='HDMI-1'
	info='"800x480_60.00"'
	result=`cvt 800 480 | awk -v FS='Modeline' '{print $2}'`
	echo '#!/bin/sh' > $path
	echo xrandr --newmode $result >> $path
	echo xrandr --addmode $device $info >> $path
	echo xrandr --output $device --mode $info >> $path
fi
