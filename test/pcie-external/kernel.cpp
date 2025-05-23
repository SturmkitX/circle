//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2025  R. Stange <rsta2@o2online.de>
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
#include <circle/memorymap.h>

#define PCIE_SLOT		0
#define PCIE_FUNC		0

// See: https://admin.pci-ids.ucw.cz/read/PD/
#define PCIE_CLASS_CODE		0x010802	// NVM Express

LOGMODULE ("kernel");

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_PCIeExternal (PCIE_BUS_EXTERNAL, &m_Interrupt)
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
		bOK = m_PCIeExternal.Initialize ();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	LOGNOTE ( "Compile time: " __DATE__ " " __TIME__);

	if (!m_PCIeExternal.EnableDevice (PCIE_CLASS_CODE, PCIE_SLOT, PCIE_FUNC))
	{
		LOGPANIC ( "Cannot enable device");
	}

	for (unsigned nOffset = 0; nOffset < 0x40; nOffset += 4)
	{
		u32 *pMem = (u32 *) (MEM_PCIE_EXT_RANGE_START + nOffset);

		LOGNOTE ( "%04X: %08X", nOffset, *pMem);
	}

	return ShutdownHalt;
}
