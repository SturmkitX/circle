#ifndef _MYOS_DOOM_H_
#define _MYOS_DOOM_H_

#include <circle/types.h>
#include <circle/serial.h>
#include <circle/fs/fat/fatfs.h>
#include <circle/bcmframebuffer.h>
#include <circle/input/keymap.h>
#include <circle/sound/soundbasedevice.h>
#include <circle/sched/scheduler.h>

#include "PureDOOM/src/DOOM/DOOM.h"

#define SAMPLE_RATE 11025
#define CHUNK_SIZE	1024	// 512 frames, 16 bits, 2 channels
#define WRITE_FORMAT	1		// 0: 8-bit unsigned, 1: 16-bit signed, 2: 24-bit signed
#define WRITE_CHANNELS	2		// 1: Mono, 2: Stereo
#define VOLUME		0.5		// [0.0, 1.0]
#define QUEUE_SIZE_MSECS 200		// size of the sound queue in milliseconds duration

#if WRITE_FORMAT == 0
	#define FORMAT		SoundFormatUnsigned8
	#define TYPE		u8
	#define TYPE_SIZE	sizeof (u8)
	#define FACTOR		((1 << 7)-1)
	#define NULL_LEVEL	(1 << 7)
#elif WRITE_FORMAT == 1
	#define FORMAT		SoundFormatSigned16
	#define TYPE		s16
	#define TYPE_SIZE	sizeof (s16)
	#define FACTOR		((1 << 15)-1)
	#define NULL_LEVEL	0
#elif WRITE_FORMAT == 2
	#define FORMAT		SoundFormatSigned24
	#define TYPE		s32
	#define TYPE_SIZE	(sizeof (u8)*3)
	#define FACTOR		((1 << 23)-1)
	#define NULL_LEVEL	0
#endif

class CDoom {
public:
    CDoom(CSerialDevice*, CFATFileSystem*, CBcmFrameBuffer*, CSoundBaseDevice*, CScheduler*);
    ~CDoom();

    boolean InitDoom();
    void Update();
    static void InterpretKeyboard(unsigned char, const unsigned char[], CKeyMap *);

private:
    void WriteSoundData (short*, unsigned);
};

#endif
