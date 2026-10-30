# SUBARCH tells the usermode build what the underlying arch is.  That is set
# first, and if a usermode build is happening, the "ARCH=um" on the command
# line overrides the setting of ARCH below.  If a native build is happening,
# then ARCH is assigned, getting whatever value it gets normally, and
# SUBARCH is subsequently ignored.
# eg:
# x86(make modules or make)
# arm(make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- modules)
SUBARCH := arm64
CURARCH ?= $(SUBARCH)
ARCH ?= $(SUBARCH)
CROSS_COMPILE := ../../../../../prebuilts/gcc/linux-x86/aarch64/gcc-arm-10.3-2021.07-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
CONFIG_MODULE_SIG=n
EXTRA_CFLAGS = -Wall -g -I$(CURDIR) -I$(subst fuxi-linux-release-package/module_fuxi/src,common,$(PWD)) -I$(PWD)
EXTRA_CFLAGS += -DFXGMAC_DEBUG
KSRC = ../../../../../kernel-5.10
KDST = ../../../../../kernel/drivers/net/ethernet/motorcomm/
KFILE = yt6801.ko

yt6801-objs :=  fuxi-gmac-common.o fuxi-gmac-desc.o fuxi-gmac-ethtool.o fuxi-gmac-hw.o fuxi-gmac-net.o fuxi-gmac-pci.o fuxi-gmac-phy.o fuxi-efuse.o fuxi-dbg.o fuxi-gmac-debugfs.o
obj-m += yt6801.o

modules:
	make -C $(KSRC) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules -j24
clean:
	make -C $(KSRC) M=$(PWD) clean
