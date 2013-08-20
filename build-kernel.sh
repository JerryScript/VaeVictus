#/bin/sh 

echo "Preparing to build VaeVictus"
echo " a kernel for the SPH-L300 goghvmu"
echo " Virgin Mobile Galaxy Victory"
echo " "

echo " "
echo "Running make mrproper..."
echo " "
make mrproper

echo " "
echo "Setting configuration files..."
echo " "
make VaeVictus_defconfig
make oldconfig

echo " "
echo "Repairing filesystem..."
echo " "
export user=$(whoami)
sudo chown -R $user:$user *
sudo chmod a+x -R *

echo " "
echo "Beginning actual build..."
echo " "
make ARCH=arm CROSS_COMPILE=~/Android/toolchains/arm-eabi-linaro-4.6.2/bin/arm-eabi- -j20

echo " "
if [ -f arch/arm/boot/zImage ]
then
echo "Build complete."
echo " "
echo "grab /Kernel/arch/arm/boot/zImage"
else echo "There was an error, try again."
echo " "
fi
