#!/bin/bash
echo "making working dir"
cd build
mkdir work
cd work
echo "copying initramfs"
cp ../initramfs/* . -r
echo "compressing"
find . | cpio -o > ../initramfs.cpio | gzip ../initramfs.cpio
rm * -rf
cp ../initramfs.cpio.gz .
echo "compiling boot.img"
./victory-boot.sh
mv new_boot.img boot.img
cp boot.img ../../
