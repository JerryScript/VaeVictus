#!/bin/bash
./mkbootimg \
--kernel zImage \
--ramdisk initramfs.cpio.gz \
--ramdiskaddr 0x01300000 \
--base 0x80200000 \
--cmdline 'console=null androidboot.hardware=qcom user_debug=31' \
-o new_boot.img
