#
# Makefile
#

USE_VCHIQ_SOUND = 1

CIRCLEHOME = ../..

OBJS	= main.o kernel.o doom.o PureDOOM/src/DOOM/am_map.o \
		PureDOOM/src/DOOM/DOOM.o \
		PureDOOM/src/DOOM/doomdef.o \
		PureDOOM/src/DOOM/doomstat.o \
		PureDOOM/src/DOOM/dstrings.o \
		PureDOOM/src/DOOM/d_items.o \
		PureDOOM/src/DOOM/d_main.o \
		PureDOOM/src/DOOM/d_net.o \
		PureDOOM/src/DOOM/f_finale.o \
		PureDOOM/src/DOOM/f_wipe.o \
		PureDOOM/src/DOOM/g_game.o \
		PureDOOM/src/DOOM/hu_lib.o \
		PureDOOM/src/DOOM/hu_stuff.o \
		PureDOOM/src/DOOM/info.o \
		PureDOOM/src/DOOM/i_net.o \
		PureDOOM/src/DOOM/i_sound.o \
		PureDOOM/src/DOOM/i_system.o \
		PureDOOM/src/DOOM/i_video.o \
		PureDOOM/src/DOOM/m_argv.o \
		PureDOOM/src/DOOM/m_bbox.o \
		PureDOOM/src/DOOM/m_cheat.o \
		PureDOOM/src/DOOM/m_fixed.o \
		PureDOOM/src/DOOM/m_menu.o \
		PureDOOM/src/DOOM/m_misc.o \
		PureDOOM/src/DOOM/m_random.o \
		PureDOOM/src/DOOM/m_swap.o \
		PureDOOM/src/DOOM/p_ceilng.o \
		PureDOOM/src/DOOM/p_doors.o \
		PureDOOM/src/DOOM/p_enemy.o \
		PureDOOM/src/DOOM/p_floor.o \
		PureDOOM/src/DOOM/p_inter.o \
		PureDOOM/src/DOOM/p_lights.o \
		PureDOOM/src/DOOM/p_map.o \
		PureDOOM/src/DOOM/p_maputl.o \
		PureDOOM/src/DOOM/p_mobj.o \
		PureDOOM/src/DOOM/p_plats.o \
		PureDOOM/src/DOOM/p_pspr.o \
		PureDOOM/src/DOOM/p_saveg.o \
		PureDOOM/src/DOOM/p_setup.o \
		PureDOOM/src/DOOM/p_sight.o \
		PureDOOM/src/DOOM/p_spec.o \
		PureDOOM/src/DOOM/p_switch.o \
		PureDOOM/src/DOOM/p_telept.o \
		PureDOOM/src/DOOM/p_tick.o \
		PureDOOM/src/DOOM/p_user.o \
		PureDOOM/src/DOOM/r_bsp.o \
		PureDOOM/src/DOOM/r_data.o \
		PureDOOM/src/DOOM/r_draw.o \
		PureDOOM/src/DOOM/r_main.o \
		PureDOOM/src/DOOM/r_plane.o \
		PureDOOM/src/DOOM/r_segs.o \
		PureDOOM/src/DOOM/r_sky.o \
		PureDOOM/src/DOOM/r_things.o \
		PureDOOM/src/DOOM/sounds.o \
		PureDOOM/src/DOOM/st_lib.o \
		PureDOOM/src/DOOM/st_stuff.o \
		PureDOOM/src/DOOM/s_sound.o \
		PureDOOM/src/DOOM/tables.o \
		PureDOOM/src/DOOM/v_video.o \
		PureDOOM/src/DOOM/wi_stuff.o \
		PureDOOM/src/DOOM/w_wad.o \
		PureDOOM/src/DOOM/z_zone.o

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
