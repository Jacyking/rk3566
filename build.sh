#!/bin/sh
dd if=/dev/zero of=linuxroot1.img bs=1M count=6800
mkfs.ext4 linuxroot1.img
mkdir temp1 && sudo mount linuxroot1.img ./temp1
sudo cp -rfp rootfs/*  temp1/
sudo umount temp1/
e2fsck -p -f linuxroot1.img
resize2fs  -M linuxroot1.img
rm temp1 -r
