Verizon Wireless Samsung Galaxy Stellar Kernel SCH-I200 jaspervzw

Built from Samsung Galaxy Victory MC1 JB update using the Linaro 4.6.2 toolchain.

To build:
make clean
make jasper_defconfig
make ARCH=arm CROSS_COMPILE=/ <path-to-toolchain-folder> /bin/arm-eabi-

After builid completes, kernel will be located at arch/arm/boot/zImage
