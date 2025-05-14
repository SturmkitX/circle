#ifndef _MYOS_DOOM_H_
#define _MYOS_DOOM_H_

#include <circle/types.h>
#include <circle/serial.h>
#include <circle/fs/fat/fatfs.h>
#include <circle/bcmframebuffer.h>

class CDoom {
public:
    CDoom(CSerialDevice*, CFATFileSystem*, CBcmFrameBuffer*);
    ~CDoom();

    boolean InitDoom();
    void Update();
};

#endif
