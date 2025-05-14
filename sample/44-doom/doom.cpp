#define DOOM_IMPLEMENTATION
#include "PureDOOM/src/DOOM/DOOM.h"
#include "doom.h"

#include <stdlib.h>
#include <string.h>
#include <circle/timer.h>


CSerialDevice *p_Serial;
CFATFileSystem *p_FileSystem;
CBcmFrameBuffer *p_FrameBuffer;

char *doomContent;
int doomSeek = 0;
int doomContentSize = 4196020;  // doom wad size

u8 keyCodesLast[128];
u8 keyCodesCurr[128];

void doom_print_fn_impl(const char* str) {
    p_Serial->Write(str, strlen(str));
}

void* doom_malloc_fn_impl(int size) {
    return malloc(size);
}

void doom_free_fn_impl(void* ptr) {
    free(ptr);
}

void* doom_open_fn_impl(const char* filename, const char* mode) {
    CString str;
    str.Format("DOOM now opening file %s with mode %s\n", filename, mode);
    p_Serial->Write(str, strlen(str));

    // paths are prefixed with "./" (when env is empty, at least)
    unsigned int handle = p_FileSystem->FileOpen(filename + 2);   // Open only supports READ mode; WRITE replaces old files

    str.Format("FileOpen returned handle: %u\n", handle);
    p_Serial->Write(str, strlen(str));
    return (void*)(unsigned long long)handle;
}

void doom_close_fn_impl(void* handle) {
    unsigned nHandle = (unsigned int)(unsigned long long)handle;
    CString str;
    str.Format("DOOM now closing handle %u\n", nHandle);
    p_Serial->Write(str, strlen(str));

    unsigned int status = p_FileSystem->FileClose(nHandle);

    str.Format("FileClose returned status: %u\n", status);
    p_Serial->Write(str, strlen(str));
}

// int doom_read_fn_impl(void* handle, void *buf, int count) {
//     unsigned nHandle = (unsigned int)(unsigned long long)handle;
//     CString str;
//     str.Format("DOOM now reading file handle %u\n", nHandle);
//     p_Serial->Write(str, strlen(str));

//     unsigned read = p_FileSystem->FileRead(nHandle, buf, count);

//     str.Format("Read %u bytes\n", read);
//     p_Serial->Write(str, strlen(str));

//     str.Format("DOOM Position after read: %d\n", p_FileSystem->FileTell(nHandle));
//     p_Serial->Write(str, strlen(str));

//     char* cBuf = (char*)buf;
//     str.Format("DOOM First 5 bytes read: %x %x %x %x %x\n", cBuf[0], cBuf[1], cBuf[2], cBuf[3], cBuf[4]);
//     p_Serial->Write(str, strlen(str));
//     return (int)read;
// }

// we simulate with the in-memory wad
int doom_read_fn_impl(void* handle, void *buf, int count) {
    unsigned nHandle = (unsigned int)(unsigned long long)handle;

    CString str;
    str.Format("DOOM now reading file handle %u\n", nHandle);
    //p_Serial->Write(str, strlen(str));

    memcpy(buf, doomContent + doomSeek, count);
    doomSeek += count;

    str.Format("Read %u bytes\n", count);
    //p_Serial->Write(str, strlen(str));

    str.Format("DOOM Position after read: %d\n", doomSeek);
    //p_Serial->Write(str, strlen(str));

    char* cBuf = (char*)buf;
    str.Format("DOOM First 5 bytes read: %x %x %x %x %x\n", cBuf[0], cBuf[1], cBuf[2], cBuf[3], cBuf[4]);
    //p_Serial->Write(str, strlen(str));

    return count;
}

int doom_write_fn_impl(void* handle, const void *buf, int count) {
    unsigned nHandle = (unsigned int)(unsigned long long)handle;
    CString str;
    str.Format("DOOM now writing to file handle %u\n", nHandle);
    p_Serial->Write(str, strlen(str));

    unsigned written = p_FileSystem->FileWrite(nHandle, buf, count);

    str.Format("Wrote %u bytes\n", written);
    p_Serial->Write(str, strlen(str));
    return (int)written;
}

// int doom_seek_fn_impl(void* handle, int offset, doom_seek_t origin) {
//     unsigned nHandle = (unsigned int)(unsigned long long)handle;
//     CString str;
//     str.Format("DOOM called seek handle %u offset %d origin %d\n", nHandle, offset, origin);
//     p_Serial->Write(str, strlen(str));

//     int seekStatus = p_FileSystem->FileSeek(nHandle, offset, origin);

//     str.Format("DOOM Position after seek: %d\n", p_FileSystem->FileTell(nHandle));
//     p_Serial->Write(str, strlen(str));

//     return seekStatus;
// }

int doom_seek_fn_impl(void* handle, int offset, doom_seek_t origin) {
    unsigned nHandle = (unsigned int)(unsigned long long)handle;
    CString str;
    str.Format("DOOM called seek handle %u offset %d origin %d\n", nHandle, offset, origin);
    //p_Serial->Write(str, strlen(str));

    switch(origin) {
        case 0: doomSeek = offset; break;
        case 1: doomSeek += offset; break;
        default: doomSeek = doomContentSize + offset;
    }

    str.Format("DOOM Position after seek: %d\n", doomSeek);
    //p_Serial->Write(str, strlen(str));

    return 0;
}

// int doom_tell_fn_impl(void* handle) {
//     unsigned nHandle = (unsigned int)(unsigned long long)handle;
//     CString str;
//     str.Format("DOOM called tell handle %u\n", nHandle);
//     p_Serial->Write(str, strlen(str));

//     return (p_FileSystem->FileTell((unsigned long long)handle));
// }

int doom_tell_fn_impl(void* handle) {
    unsigned nHandle = (unsigned int)(unsigned long long)handle;
    CString str;
    str.Format("DOOM called tell handle %u\n", nHandle);
    p_Serial->Write(str, strlen(str));

    return doomSeek;
}

// int doom_eof_fn_impl(void* handle) {
//     unsigned nHandle = (unsigned int)(unsigned long long)handle;
//     CString str;
//     str.Format("DOOM called eof handle %u\n", nHandle);
//     p_Serial->Write(str, strlen(str));
//     return (p_FileSystem->FileEOF((unsigned long long)handle));
// }

int doom_eof_fn_impl(void* handle) {
    unsigned nHandle = (unsigned int)(unsigned long long)handle;
    CString str;
    str.Format("DOOM called eof handle %u\n", nHandle);
    p_Serial->Write(str, strlen(str));
    return (doomSeek >= doomContentSize);
}

void doom_gettime_fn_impl(int* sec, int* usec) {
    u64 usecs = CTimer::GetClockTicks64();

    *sec = usecs / 1000000;
    *usec = usecs % 1000000;
}

void doom_exit_fn_impl(int code) {
    CString str;
    str.Format("Exit now with exit code %d\n", code);
    p_Serial->Write(str, strlen(str));
}

char* doom_getenv_fn_impl(const char* var) {
    CString str;
    str.Format("Retrieving ENV value for %s\n", var);
    p_Serial->Write(str, strlen(str));
    return NULL;
}


CDoom::CDoom(CSerialDevice* serial, CFATFileSystem* fatfs, CBcmFrameBuffer* fb) {
    p_Serial = serial;
    p_FileSystem = fatfs;
    p_FrameBuffer = fb;
}

CDoom::~CDoom() {

}

boolean CDoom::InitDoom() {
    doom_set_print(doom_print_fn_impl);
    doom_set_malloc(doom_malloc_fn_impl, doom_free_fn_impl);
    doom_set_file_io(doom_open_fn_impl,
                      doom_close_fn_impl,
                      doom_read_fn_impl,
                      doom_write_fn_impl,
                      doom_seek_fn_impl,
                      doom_tell_fn_impl,
                      doom_eof_fn_impl);
    doom_set_gettime(doom_gettime_fn_impl);
    doom_set_exit(doom_exit_fn_impl);
    doom_set_getenv(doom_getenv_fn_impl);

    // Change default bindings to modern mapping
    doom_set_default_int("key_up",          DOOM_KEY_W);
    doom_set_default_int("key_down",        DOOM_KEY_S);
    doom_set_default_int("key_left",  DOOM_KEY_A);
    doom_set_default_int("key_right", DOOM_KEY_D);
    doom_set_default_int("key_use",         DOOM_KEY_E);
    doom_set_default_int("key_fire",         DOOM_KEY_R);
    doom_set_default_int("mouse_move",      0); // Mouse will not move forward

    char* argv[] = {
        "doom.exe",
        "doom1.wad"
    };
    int argc = 2;

    // read DOOM wad content
    doomContent = (char*)malloc(doomContentSize);
    unsigned handle = p_FileSystem->FileOpen("doom1.wad");
    int readBytes = p_FileSystem->FileRead(handle, doomContent, doomContentSize);
    p_FileSystem->FileClose(handle);

    CString str;
    str.Format("Read %d bytes from DOOM wad: ", readBytes);
    p_Serial->Write(str, strlen(str));


    doom_init(argc, argv, 0);

    str.Format("Finished DOOM Load\n");
    p_Serial->Write(str, strlen(str));

    return 1;
}

void CDoom::Update() {
    u64 start_time, curr_time;

    // render the game at 25fps
    unsigned int delayTime = 1000000 / 25;
    start_time = CTimer::GetClockTicks64();
    CString str;

    while (1) {
        curr_time = CTimer::GetClockTicks64();
        doom_update();
        // char kbuf = 0;
        // char lastPressed = 0;
        // int serialRead = 0;

        // serialRead = p_Serial->Read(&kbuf, 1);
        // if (serialRead > 0) {
        //     if (kbuf != lastPressed) {
        //         doom_key_down((doom_key_t)kbuf);
        //         lastPressed = kbuf;
        //     }
        // } else if (lastPressed != 0) {
        //     doom_key_up((doom_key_t)lastPressed);
        //     lastPressed = 0;
        // }

        if ((curr_time - start_time) % delayTime == 0) {
            const u8* framebuffer = doom_get_framebuffer(4 /* RGBA */);
            
            // str.Format("Before SetArea\n");
            // p_Serial->Write(str, strlen(str));

            // p_FrameBuffer->SetArea(area, framebuffer);
            unsigned int* fbp = (unsigned int*)framebuffer;
            for (int y=0; y < 200; y++) {
                for (int x=0; x < 320; x++) {
                    p_FrameBuffer->SetPixel((x << 1), (y << 1), *fbp);
                    p_FrameBuffer->SetPixel((x << 1) | 1, (y << 1), *fbp);
                    p_FrameBuffer->SetPixel((x << 1), (y << 1) | 1, *fbp);
                    p_FrameBuffer->SetPixel((x << 1) | 1, (y << 1) | 1, *fbp);
                    fbp++;
                }
            }
            // p_FrameBuffer->SetPixel(0, 0, *(unsigned int*)framebuffer);

            // str.Format("After SetArea\n");
            // p_Serial->Write(str, strlen(str));
            
            // str.Format("Finished displaying the framebuffer\n");
            // p_Serial->Write(str, strlen(str));
        }
    }
}

void CDoom::InterpretKeyboard(unsigned char ucModifiers, const unsigned char RawKeys[6], CKeyMap *keyMap) {
    // modifiers are not taken into consideration
    memset(keyCodesCurr, 0, 128);

    for (int i=0; i < 6; i++) {
        keyCodesCurr[RawKeys[i]] = 1;
    }

    CString str;
    
    for (int i=0; i < 128; i++) {
        if (keyCodesCurr[i] == 1 && keyCodesLast[i] == 0) {
            // key down
            u16 code = keyMap->Translate(i, 0);

            if (code == 260) {
                // ENTER
                doom_key_down(DOOM_KEY_ENTER);
            } else {
                doom_key_down((doom_key_t)code);
            }
            
            str.Format("Key down: %u\n", code);
            p_Serial->Write(str, strlen(str));
        } else if (keyCodesCurr[i] == 0 && keyCodesLast[i] == 1) {
            u16 code = keyMap->Translate(i, 0);
            if (code == 260) {
                // ENTER
                doom_key_up(DOOM_KEY_ENTER);
            } else {
                doom_key_up((doom_key_t)code);
            }
            str.Format("Key up: %u\n", code);
            p_Serial->Write(str, strlen(str));
        }

        keyCodesLast[i] = keyCodesCurr[i];
    }
}
