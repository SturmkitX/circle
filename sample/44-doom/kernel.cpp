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

// #include "fdlibm/fdlibm.h"
#include <math.h>
#include <circle/input/keymap.h>

#include "doom.h"

#include <vc4/sound/vchiqsoundbasedevice.h>

#define PARTITION	"emmc1-1"
#define FILENAME	"circle.txt"

static const char FromKernel[] = "kernel";
static CSerialDevice* p_Serial;
static CDoom *p_Doom;

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
	// unsigned nQueueSizeFrames = m_pSound->GetQueueSizeFrames ();

	// WriteSoundData (nQueueSizeFrames);

	// start sound device
	if (!m_pSound->Start ())
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot start sound device");
	}

	m_Logger.Write (FromKernel, LogNotice, "Playing DOOM Sounds");

	// show the character set on screen
	// for (char chChar = ' '; chChar <= '~'; chChar++)
	// {
	// 	if (chChar % 8 == 0)
	// 	{
	// 		m_Screen.Write ("\n", 1);
	// 	}

	// 	CString Message;
	// 	Message.Format ("%02X: \'\u001b[7m%c\u001b[0m\' ", (unsigned) chChar, chChar);
		
	// 	m_Screen.Write ((const char *) Message, Message.GetLength ());
	// }
	m_Screen.Write ("\n", 1);

	InitSD();
	InitUSB();

#ifndef NDEBUG
	// some debugging features
	// m_Logger.Write (FromKernel, LogDebug, "Dumping the start of the ATAGS");
	// debug_hexdump ((void *) 0x100, 128, FromKernel);

	// CString Message2;
	// Message2.Format ("Testing simple float print: %f\n", 5.2f);
	// m_Screen.Write ((const char *) Message2, Message2.GetLength ());

	// float xx = (float)pow(2, 3);
	// CString Message3;
	// Message3.Format ("Testing pow(2,3): %f\n", sqrt(16));
	// m_Screen.Write ((const char *) Message3, Message3.GetLength ());

	boolean resize_status = m_Screen.Resize(320, 200);

	CDoom doom(&m_Serial, &m_FileSystem, m_Screen.GetFrameBuffer(), m_pSound, &m_Scheduler);
	p_Doom = &doom;
	
	boolean res = doom.InitDoom();

	// must loop, otherwise m_Logger will overwrite the screen at the next call
	doom.Update();

	m_Logger.Write (FromKernel, LogNotice, "The following assertion will fail");
	assert (1 == 2);
#endif

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

	// Create file and write to it
	unsigned hFile = m_FileSystem.FileCreate (FILENAME);
	if (hFile == 0)
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot create file: %s", FILENAME);
	}

	for (unsigned nLine = 1; nLine <= 5; nLine++)
	{
		CString Msg;
		Msg.Format ("Hello File! (Line %u)\n", nLine);

		if (m_FileSystem.FileWrite (hFile, (const char *) Msg, Msg.GetLength ()) != Msg.GetLength ())
		{
			m_Logger.Write (FromKernel, LogError, "Write error");
			break;
		}
	}

	if (!m_FileSystem.FileClose (hFile))
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot close file");
	}

	// Reopen file, read it and display its contents
	hFile = m_FileSystem.FileOpen (FILENAME);
	if (hFile == 0)
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot open file: %s", FILENAME);
	}

	char Buffer[100];
	unsigned nResult;
	while ((nResult = m_FileSystem.FileRead (hFile, Buffer, sizeof Buffer)) > 0)
	{
		if (nResult == FS_ERROR)
		{
			m_Logger.Write (FromKernel, LogError, "Read error");
			break;
		}

		m_Screen.Write (Buffer, nResult);
	}
	
	if (!m_FileSystem.FileClose (hFile))
	{
		m_Logger.Write (FromKernel, LogPanic, "Cannot close file");
	}
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
			m_Logger.Write (FromKernel, LogNotice, "Just type something!");

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
	// p_Serial->Write(str, strlen(str));

	p_Doom->InterpretKeyboard(ucModifiers, RawKeys, &m_KeyMap);
}

void CKernel::KeyboardRemovedHandler (CDevice *pDevice, void *pContext) {
	CString str;
	str.Format("Removed USB keyboard event\n");
	// p_Serial->Write(str, strlen(str));
}
