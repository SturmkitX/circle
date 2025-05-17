#
# Makefile
#

USE_VCHIQ_SOUND = 1

CIRCLEHOME = ../..

OBJS	= main.o kernel.o

LIBS	= $(CIRCLEHOME)/lib/libcircle.a \
		$(CIRCLEHOME)/lib/fs/fat/libfatfs.a \
	  	$(CIRCLEHOME)/lib/fs/libfs.a \
		$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
		$(CIRCLEHOME)/lib/input/libinput.a \
		$(CIRCLEHOME)/lib/usb/libusb.a \
		$(CIRCLEHOME)/lib/sched/libsched.a \
	   $(CIRCLEHOME)/lib/sound/libsound.a

ifeq ($(strip $(USE_VCHIQ_SOUND)),1)
LIBS	+= $(CIRCLEHOME)/addon/vc4/sound/libvchiqsound.a \
	  $(CIRCLEHOME)/addon/vc4/vchiq/libvchiq.a \
	  $(CIRCLEHOME)/addon/linux/liblinuxemu.a

CFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND)
endif

include ../Rules.mk

-include $(DEPS)
