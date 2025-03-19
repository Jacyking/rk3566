#!/bin/sh
dirname=`dirname $0`
dd if=/dev/zero of=$dirname/linuxroot1.img bs=1M count=6800
mkfs.ext4 $dirname/linuxroot1.img
mkdir $dirname/temp1 && sudo mount $dirname/linuxroot1.img $dirname/temp1
sudo cp -rfp $dirname/rootfs/*  $dirname/temp1/
sudo umount $dirname/temp1/
e2fsck -p -f $dirname/linuxroot1.img
resize2fs  -M $dirname/linuxroot1.img
rm temp1 -r
mv $dirname/linuxroot1.img $dirname/linux4.19/rootfs
echo "done"
$dirname/linux4.19/build.sh
