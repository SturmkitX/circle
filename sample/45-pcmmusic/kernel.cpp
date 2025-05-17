//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2020  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include <circle/string.h>
#include <circle/debug.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// #include "fdlibm/fdlibm.h"
#include <math.h>
#include <circle/input/keymap.h>

#include <vc4/sound/vchiqsoundbasedevice.h>

#define PARTITION	"emmc1-1"
#define FILENAME	"circle.txt"

static const char FromKernel[] = "kernel";
static CSerialDevice* p_Serial;

static CKeyMap m_KeyMap;

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
// :	m_Screen (320, 200),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_USBHCI (&m_Interrupt, &m_Timer, TRUE),		// TRUE: enable plug-and-play
	m_pKeyboard (0),
	m_EMMC (&m_Interrupt, &m_Timer, &m_ActLED),
	m_VCHIQ (CMemorySystem::Get (), &m_Interrupt),
	m_pSound (0)
{
	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}
	
	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}
	
	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_USBHCI.Initialize ();
	}

	if (bOK)
	{
		bOK = m_EMMC.Initialize ();
	}

	if (bOK)
	{
		bOK = m_VCHIQ.Initialize ();
	}
	
	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	m_Logger.Write (FromKernel, LogNotice, "Please attach an USB keyboard, if not already done!");

	m_pSound = new CVCHIQSoundBaseDevice (&m_VCHIQ, SAMPLE_RATE, CHUNK_SIZE,
					(TVCHIQSoundDestination) m_Options.GetSoundOption ());

	// configure sound device
	if (!m_pSound->AllocateQueue (QUEUE_SIZE_MSECS))
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot allocate sound queue");
	}

	m_pSound->SetWriteFormat (FORMAT, WRITE_CHANNELS);

	// initially fill the whole queue with data
	unsigned nQueueSizeFrames = m_pSound->GetQueueSizeFrames ();

	// WriteSoundData (nQueueSizeFrames);

	// start sound device
	if (!m_pSound->Start ())
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot start sound device");
	}

	m_Logger.Write (FromKernel, LogNotice, "Should load the RAW PCM file now");

	m_Screen.Write ("\n", 1);

	InitSD();
	InitUSB();

	// read raw image content (640x480, RGBA)
	unsigned imgContentSize = 1228800;
    char* imgContent = (char*)malloc(imgContentSize);
    unsigned handle = m_FileSystem.FileOpen("alisa1.img");
    int imgBytes = m_FileSystem.FileRead(handle, imgContent, imgContentSize);
    m_FileSystem.FileClose(handle);

	// read raw PCM content (44100 Hz, stereo, 16 bit)
	unsigned rawContentSize = 67419836;
    char* rawContent = (char*)malloc(rawContentSize);
    handle = m_FileSystem.FileOpen("fanta.raw");
    int readBytes = m_FileSystem.FileRead(handle, rawContent, rawContentSize);
    m_FileSystem.FileClose(handle);

	CString str;
	str.Format("Read PCM bytes: %d", readBytes);
	m_Logger.Write (FromKernel, LogNotice, "Closed the raw PCM file");
	m_Logger.Write (FromKernel, LogNotice, str);

	unsigned *fbb = (unsigned*)imgContent;
	CBcmFrameBuffer *fb = m_Screen.GetFrameBuffer();
	for (int y=0; y < 480; y++) {
		for (int x=0; x < 640; x++) {
			unsigned int pixel = *fbb++;
			pixel = (pixel & 0xFF000000) |
					((pixel & 0xFF) << 16) |
					(pixel & 0x0000FF00) |
					((pixel >> 16) & 0xFF);
			fb->SetPixel(x, y, pixel);
		}
	}

	unsigned nFrames = rawContentSize / 2;
	unsigned offset = 0;

	while (nFrames > 0)
	{
		unsigned avail = nQueueSizeFrames - m_pSound->GetQueueFramesAvail();
		// str.Format("PCM Buffer available bytes: %u", avail);
		// m_Logger.Write (FromKernel, LogNotice, str);

		unsigned nWriteFrames = nFrames < avail ? nFrames : avail;
		unsigned nWriteBytes = nWriteFrames * WRITE_CHANNELS * TYPE_SIZE;

		int nResult = m_pSound->Write (rawContent + offset, nWriteBytes);
		offset += nResult;

		// str.Format("PCM write done: %u %u %d", nWriteBytes, offset, nResult);
		// m_Logger.Write (FromKernel, LogNotice, str);
		if (nResult != (int) nWriteBytes)
		{
			m_Logger.Write (FromKernel, LogError, "Sound data dropped");
		}

		nFrames -= nWriteFrames;

		m_Scheduler.Yield ();		// ensure the VCHIQ tasks can run
	}

	m_Logger.Write (FromKernel, LogNotice, "The following assertion will fail");
	assert (1 == 2);

	return ShutdownHalt;
}

void CKernel::InitSD() {
	// Mount file system
	CDevice *pPartition = m_DeviceNameService.GetDevice (PARTITION, TRUE);
	if (pPartition == 0)
	{
		m_Logger.Write (FromKernel, LogPanic, "Partition not found: %s", PARTITION);
	}

	if (!m_FileSystem.Mount (pPartition))
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot mount partition: %s", PARTITION);
	}

	// Show contents of root directory
	TDirentry Direntry;
	TFindCurrentEntry CurrentEntry;
	unsigned nEntry = m_FileSystem.RootFindFirst (&Direntry, &CurrentEntry);
	for (unsigned i = 0; nEntry != 0; i++)
	{
		if (!(Direntry.nAttributes & FS_ATTRIB_SYSTEM))
		{
			CString FileName;
			FileName.Format ("%-14s", Direntry.chTitle);

			m_Screen.Write ((const char *) FileName, FileName.GetLength ());

			if (i % 5 == 4)
			{
				m_Screen.Write ("\n", 1);
			}
		}

		nEntry = m_FileSystem.RootFindNext (&Direntry, &CurrentEntry);
	}
	m_Screen.Write ("\n", 1);
}

void CKernel::InitUSB() {
	// This must be called from TASK_LEVEL to update the tree of connected USB devices.
	boolean bUpdated = m_USBHCI.UpdatePlugAndPlay ();

	p_Serial = &m_Serial;

	if (   bUpdated
		&& m_pKeyboard == 0)
	{
		m_pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
		if (m_pKeyboard != 0)
		{
			m_pKeyboard->RegisterRemovedHandler (KeyboardRemovedHandler);

#if 0	// set to 0 to test raw mode
			m_pKeyboard->RegisterShutdownHandler (ShutdownHandler);
			m_pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
#else
			m_pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
#endif
			m_Logger.Write (FromKernel, LogNotice, "Keyboard initialized!");

		}
	}

	if (m_pKeyboard != 0)
	{
		// CUSBKeyboardDevice::UpdateLEDs() must not be called in interrupt context,
		// that's why this must be done here. This does nothing in raw mode.
		m_pKeyboard->UpdateLEDs ();
	}
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]) {
	CString str;
	str.Format("Got RAW Key status: %x %x %x %x %x %x\n", RawKeys[0], RawKeys[1], RawKeys[2], RawKeys[3], RawKeys[4], RawKeys[5]);
	p_Serial->Write(str, strlen(str));

}

void CKernel::KeyboardRemovedHandler (CDevice *pDevice, void *pContext) {
	CString str;
	str.Format("Removed USB keyboard event\n");
	p_Serial->Write(str, strlen(str));
}
