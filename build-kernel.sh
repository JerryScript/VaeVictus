#/bin/sh 

#
export toolchains="git clone git://github.com/rbheromax/toolchains"
export set="export CROSS_COMPILE=toolchains/arm-eabi-linaro-4.6.2/bin/arm-eabi-"
J=$(cat /proc/cpuinfo | grep "^processor" | wc -l)
clear
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
clear
echo " "
echo "Beginning actual build..."
echo " "
while true; do
	read -p "Do you have a toolchain?" yn
	case $yn in
		[Yy]* ) echo ""; break;;
		[Nn]* ) $toolchains; $set; break;;
		*) echo "Gimme an answer dammit!!!"
	esac
done

make ARCH=arm -j"$J"

echo " "
if [ -f arch/arm/boot/zImage ]
then
echo "Build complete."
echo " "
echo "grab /Kernel/arch/arm/boot/zImage"
else echo "There was an error, try again."
echo " "
fi

build/script.sh
