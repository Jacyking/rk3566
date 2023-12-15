#!/bin/sh
dd if=/dev/zero of=linuxroot1.img bs=1M count=6800
mkfs.ext4 /home/jacyking/rk3566/linuxroot1.img
mkdir temp1 && sudo mount /home/jacyking/rk3566/linuxroot1.img /home/jacyking/rk3566/temp1
sudo rm /home/jacyking/rk3566/rootfs-1024x600/opt/tjx/* -r
sudo cp /home/jacyking/rk3566/miniwave/* /home/jacyking/rk3566/rootfs-1024x600/opt/tjx/ -r
sudo cp -rfp /home/jacyking/rk3566/rootfs-1024x600/*  /home/jacyking/rk3566/temp1/
sudo umount /home/jacyking/rk3566/temp1/
e2fsck -p -f /home/jacyking/rk3566/linuxroot1.img
resize2fs  -M /home/jacyking/rk3566/linuxroot1.img
mv /home/jacyking/rk3566/linuxroot1.img /home/jacyking/rk3566/linux4.19-1024x600/rootfs
rm /home/jacyking/rk3566/temp1 -r
echo "done"
cd /home/jacyking/rk3566/linux4.19-1024x600
./build.sh
