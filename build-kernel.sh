#/bin/sh 

echo "Preparing to build VaeVictus"
echo " a kernel for the SPH-L300 goghvmu"
echo " Virgin Mobile Galaxy Victory"
echo " "
echo "Setting environment variables..."
echo " "
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=~/android/arm-eabi-4.6/bin/arm-eabi-

echo " "
echo "Running make clean and make mrproper..."
echo " "
make clean
make mrproper

echo " "
echo "Setting configuration files..."
echo " "
make VaeVictus_defconfig
make oldconfig

echo " "
echo "Repairing filesystem..."
echo " "
sudo chown -R jerryscript:jerryscript ~/android/source/Kernel
sudo chmod a+x -R ~/anrdoid/source/Kernel

echo " "
echo "Beginning actual build..."
echo " "
make ARCH=arm SUBARCH=arm CROSS_COMPILE=~/android/arm-eabi-4.6/bin/arm-eabi- -j4

echo " "
echo "Build complete."
echo " "
echo "If there were no errors, grab /Kernel/arch/arm/boot/zImage"
echo " "


