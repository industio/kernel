
# 1. Be sure to change KERN_DIR when using a different board kernel
# 2. The kernel in KERN_DIR must be configured and compiled in advance, and the following environment variables must be set in order to compile the kernel:
# 2.1 ARCH,          For example: export ARCH=arm64
# 2.2 CROSS_COMPILE, For example: export CROSS_COMPILE=aarch64-linux-gnu-
# 2.3 PATH,          For example: export PATH=$PATH:/home/book/100ask_roc-rk3399-pc/ToolChain-6.3.1/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin 
# Note: The above three environment variables may not be the same for different development boards and different compilers.
#       For details, see the Advanced user manual of each development board


KERN_DIR = /home/ysy/workspace/imx6ull_pro/Imx6ull_source_code/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
	


lt8912b-y := src/LontiumDrv.o    src/LT8912B_Main.o  src/LT8912B.o  src/OcmI2cMaster.o \



obj-m += lt8912b.o

