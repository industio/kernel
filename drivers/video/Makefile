# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_VGASTATE)            += vgastate.o
obj-$(CONFIG_HDMI)                += hdmi.o

obj-$(CONFIG_VT)		  += console/
obj-$(CONFIG_FB_STI)		  += console/
obj-$(CONFIG_LOGO)		  += logo/
obj-y				  += backlight/
obj-y				  += rockchip/

obj-y				  += fbdev/

obj-$(CONFIG_VIDEOMODE_HELPERS) += display_timing.o videomode.o
ifeq ($(CONFIG_OF),y)
obj-$(CONFIG_VIDEOMODE_HELPERS) += of_display_timing.o of_videomode.o
endif

# obj-y				  += LT8912B/
obj-y				  += LT8912B_Linux_Driver_v1.0/
