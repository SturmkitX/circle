//
// st7789display.cpp
//
// Based on:
//	https://github.com/pimoroni/st7789-python/blob/master/library/ST7789/__init__.py
//	Copyright (c) 2014 Adafruit Industries
//	Author: Tony DiCola
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include <display/st7789display.h>
#include <circle/timer.h>
#include <assert.h>

#define ST7789_NOP	0x00
#define ST7789_SWRESET	0x01
#define ST7789_RDDID	0x04
#define ST7789_RDDST	0x09

#define ST7789_SLPIN	0x10
#define ST7789_SLPOUT	0x11
#define ST7789_PTLON	0x12
#define ST7789_NORON	0x13

#define ST7789_INVOFF	0x20
#define ST7789_INVON	0x21
#define ST7789_DISPOFF	0x28
#define ST7789_DISPON	0x29

#define ST7789_CASET	0x2A
#define ST7789_RASET	0x2B
#define ST7789_RAMWR	0x2C
#define ST7789_RAMRD	0x2E

#define ST7789_PTLAR	0x30
#define ST7789_MADCTL	0x36
#define ST7789_COLMOD	0x3A

#define RAMCTRL		0xB0
#define ST7789_FRMCTR1	0xB1
#define ST7789_FRMCTR2	0xB2
#define ST7789_FRMCTR3	0xB3
#define ST7789_INVCTR	0xB4
#define ST7789_DISSET5	0xB6

#define ST7789_GCTRL	0xB7
#define ST7789_GTADJ	0xB8
#define ST7789_VCOMS	0xBB

#define ST7789_LCMCTRL	0xC0
#define ST7789_IDSET	0xC1
#define ST7789_VDVVRHEN	0xC2
#define ST7789_VRHS	0xC3
#define ST7789_VDVS	0xC4
#define ST7789_VMCTR1	0xC5
#define ST7789_FRCTRL2	0xC6
#define ST7789_CABCCTRL	0xC7

#define ST7789_PWCTRL1	0xD0

#define ST7789_RDID1	0xDA
#define ST7789_RDID2	0xDB
#define ST7789_RDID3	0xDC
#define ST7789_RDID4	0xDD

#define ST7789_GMCTRP1	0xE0
#define ST7789_GMCTRN1	0xE1

#define ST7789_PWCTR6	0xFC

CST7789Display::CST7789Display (CSPIMaster *pSPIMaster,
				unsigned nDCPin, unsigned nResetPin, unsigned nBackLightPin,
				unsigned nWidth, unsigned nHeight,
				unsigned CPOL, unsigned CPHA, unsigned nClockSpeed,
				unsigned nChipSelect, boolean bSwapColorBytes)
:	CDisplay (bSwapColorBytes ? RGB565_BE : RGB565),
	m_pSPIMaster (pSPIMaster),
	m_nResetPin (nResetPin),
	m_nBackLightPin (nBackLightPin),
	m_nWidth (nWidth),
	m_nHeight (nHeight),
	m_CPOL (CPOL),
	m_CPHA (CPHA),
	m_nClockSpeed (nClockSpeed),
	m_nChipSelect (nChipSelect),
	m_bSwapColorBytes (bSwapColorBytes),
	m_DCPin (nDCPin, GPIOModeOutput)
{
	assert (nDCPin != None);

	if (m_nBackLightPin != None)
	{
		m_BackLightPin.AssignPin (m_nBackLightPin);
		m_BackLightPin.SetMode (GPIOModeOutput, FALSE);
	}

	if (m_nResetPin != None)
	{
		m_ResetPin.AssignPin (m_nResetPin);
		m_ResetPin.SetMode (GPIOModeOutput, FALSE);
	}
		
	m_nRotation = 0;

	m_pBuffer = new u16[m_nWidth * m_nHeight];
	assert (m_pBuffer != 0);
}

CST7789Display::~CST7789Display (void)
{
	delete [] m_pBuffer;
}

boolean CST7789Display::Initialize (void)
{
	assert (m_pSPIMaster != 0);

	if (m_nBackLightPin != None)
	{
		m_BackLightPin.Write (LOW);
		CTimer::SimpleMsDelay (100);
		m_BackLightPin.Write (HIGH);
	}

	if (m_nResetPin != None)
	{
		m_ResetPin.Write (HIGH);
		CTimer::SimpleMsDelay (50);
		m_ResetPin.Write (LOW);
		CTimer::SimpleMsDelay (50);
		m_ResetPin.Write (HIGH);
		CTimer::SimpleMsDelay (50);
	}

	Command (ST7789_SWRESET);	// Software reset
	CTimer::SimpleMsDelay (150);

	Command (ST7789_MADCTL);
	Data (0x70);

	Command (ST7789_FRMCTR2);	// Frame rate ctrl - idle mode
	Data (0x0C);
	Data (0x0C);
	Data (0x00);
	Data (0x33);
	Data (0x33);

	Command (ST7789_COLMOD);
	Data (0x05);

	Command (RAMCTRL);		// Set Endian Mode
	Data (0x00);
	Data (m_bSwapColorBytes ? 0xF0 : 0xF8);

	Command (ST7789_GCTRL);
	Data (0x14);

	Command (ST7789_VCOMS);
	Data (0x37);

	Command (ST7789_LCMCTRL);	// Power control
	Data (0x2C);

	Command (ST7789_VDVVRHEN);	// Power control
	Data (0x01);

	Command (ST7789_VRHS);		// Power control
	Data (0x12);

	Command (ST7789_VDVS);		// Power control
	Data (0x20);

	Command (ST7789_PWCTRL1);
	Data (0xA4);
	Data (0xA1);

	Command (ST7789_FRCTRL2);
	Data (0x0F);

	Command (ST7789_GMCTRP1);	// Set Gamma
	Data (0xD0);
	Data (0x04);
	Data (0x0D);
	Data (0x11);
	Data (0x13);
	Data (0x2B);
	Data (0x3F);
	Data (0x54);
	Data (0x4C);
	Data (0x18);
	Data (0x0D);
	Data (0x0B);
	Data (0x1F);
	Data (0x23);

	Command (ST7789_GMCTRN1);	// Set Gamma
	Data (0xD0);
	Data (0x04);
	Data (0x0C);
	Data (0x11);
	Data (0x13);
	Data (0x2C);
	Data (0x3F);
	Data (0x44);
	Data (0x51);
	Data (0x2F);
	Data (0x1F);
	Data (0x1F);
	Data (0x20);
	Data (0x23);

	Command (ST7789_INVON);		// Invert display

	Command (ST7789_SLPOUT);

	On ();

	Clear ();

	return TRUE;
}

void CST7789Display::SetRotation (unsigned nRot)
{
	if (nRot == 0 || nRot == 90 || nRot == 180 || nRot == 270)
	{
		m_nRotation = nRot;
	}
}

void CST7789Display::On (void)
{
	Command (ST7789_DISPON);
	CTimer::SimpleMsDelay (100);
}

void CST7789Display::Off (void)
{
	Command (ST7789_DISPOFF);
}

void CST7789Display::Clear (TST7789Color Color)
{
	assert (m_nWidth > 0);
	assert (m_nHeight > 0);

	SetWindow (0, 0, m_nWidth-1, m_nHeight-1);

	TST7789Color Buffer[m_nWidth];
	for (unsigned x = 0; x < m_nWidth; x++)
	{
		Buffer[x] = Color;
	}

	for (unsigned y = 0; y < m_nHeight; y++)
	{
		SendData (Buffer, sizeof Buffer);
	}
}

unsigned CST7789Display::RotX (unsigned x, unsigned y)
{
	//   0 -> [x,y]
	//  90 -> [y, MAX_Y-x]
	// 180 -> [MAX_X-x, MAX_Y-y]
	// 270 -> [MAX_X-y, x]
	switch (m_nRotation)
	{
		case 90:
			return y;
			break;

		case 180:
			return m_nWidth - 1 - x;
			break;

		case 270:
			return m_nWidth - 1 - y;
			break;

		default: // 0 or other
			return x;
			break;
	}
}

unsigned CST7789Display::RotY (unsigned x, unsigned y)
{
	switch (m_nRotation)
	{
		case 90:
			return m_nHeight - 1 - x;
			break;

		case 180:
			return m_nHeight - 1 - y;
			break;

		case 270:
			return x;
			break;

		default: // 0 or other
			return y;
			break;
	}
}

void CST7789Display::SetPixel (unsigned nPosX, unsigned nPosY, TST7789Color Color)
{
	unsigned xc = RotX(nPosX, nPosY);
	unsigned yc = RotY(nPosX, nPosY);
	SetWindow (xc, yc, xc, yc);

	SendData (&Color, sizeof Color);
}

void CST7789Display::DrawText (unsigned nPosX, unsigned nPosY, const char *pString,
			       TST7789Color Color, TST7789Color BgColor,
			       bool bDoubleWidth, bool bDoubleHeight, const TFont &rFont)
{
	assert (pString != 0);

	CCharGenerator CharGen (rFont, CCharGenerator::MakeFlags (bDoubleWidth, bDoubleHeight));
	unsigned nCharWidth = CharGen.GetCharWidth ();
	unsigned nCharHeight = CharGen.GetCharHeight ();

	TST7789Color Buffer[nCharHeight * nCharWidth];

	char chChar;
	while ((chChar = *pString++) != '\0')
	{
		for (unsigned y = 0; y < nCharHeight; y++)
		{
			CCharGenerator::TPixelLine Line = CharGen.GetPixelLine (chChar, y);

			for (unsigned x = 0; x < nCharWidth; x++)
			{
				TST7789Color pix = CharGen.GetPixel (x, Line) ? Color : BgColor;

				// Rotation determines order of bytes in the buffer
				switch (m_nRotation)
				{
					case 90:
						// (Last C - C)*rows + R
						Buffer[(nCharWidth-1-x)*nCharHeight + y] = pix;
						break;
					case 180:
						// (Last R - R)*cols + (Last C - C)
						Buffer[(nCharHeight-1-y)*nCharWidth + (nCharWidth-1-x)] = pix;
						break;
					case 270:
						// C*rows + (Last R - R)
						Buffer[x*nCharHeight + nCharHeight - 1 - y] = pix;
						break;
					default:
						// R*cols + C
						Buffer[y*nCharWidth + x] = pix;
						break;
				}
			}
		}
		
		// Need to set different corners of the window depending on
		// the rotation to avoid have negative/inside out windows.
		unsigned x1=nPosX;
		unsigned x2=nPosX+nCharWidth-1;
		unsigned y1=nPosY;
		unsigned y2=nPosY+nCharHeight-1;
		switch (m_nRotation)
		{
			case 90:
				SetWindow (RotX(x2,y1), RotY(x2,y1), RotX(x1,y2), RotY(x1,y2));
				break;
			case 180:
				SetWindow (RotX(x2,y2), RotY(x2,y2), RotX(x1,y1), RotY(x1,y1));
				break;
			case 270:
				SetWindow (RotX(x1,y2), RotY(x1,y2), RotX(x2,y1), RotY(x2,y1));
				break;
			default:
				SetWindow (x1, y1, x2, y2);
				break;
		}

		SendData (Buffer, sizeof Buffer);

		nPosX += nCharWidth;
	}
}

void CST7789Display::SetPixel (unsigned nPosX, unsigned nPosY, TRawColor nColor)
{
	SetPixel (nPosX, nPosY, (TST7789Color) nColor);
}

void CST7789Display::SetArea (const TArea &rArea, const void *pPixels,
			      TAreaCompletionRoutine *pRoutine, void *pParam)
{
	int nWidth = rArea.x2 - rArea.x1 + 1;
	int nHeight = rArea.y2 - rArea.y1 + 1;

	if (m_nRotation == 0)
	{
		SetWindow (rArea.x1, rArea.y1, rArea.x2, rArea.y2);
	}
	else
	{
		const u16 *pFrom = (const u16 *) pPixels;
		u16 *pTo = m_pBuffer;

		switch (m_nRotation)
		{
		case 90:
			SetWindow (m_nWidth-rArea.y2-1, rArea.x1,
				   m_nWidth-rArea.y1-1, rArea.x2);
			for (int x = 0; x < nWidth; x++)
			{
				for (int y = nHeight-1; y >= 0; y--)
				{
					*pTo++ = pFrom[x + y * nWidth];
				}
			}
			break;

		case 180:
			SetWindow (m_nWidth-rArea.x2-1, m_nHeight-rArea.y2-1,
				   m_nWidth-rArea.x1-1, m_nHeight-rArea.y1-1);
			for (int y = nHeight-1; y >= 0; y--)
			{
				for (int x = nWidth-1; x >= 0; x--)
				{
					*pTo++ = pFrom[x + y * nWidth];
				}
			}
			break;

		case 270:
			SetWindow (rArea.y1, m_nHeight-rArea.x2-1,
				   rArea.y2, m_nHeight-rArea.x1-1);
			for (int x = nWidth-1; x >= 0; x--)
			{
				for (int y = 0; y < nHeight; y++)
				{
					*pTo++ = pFrom[x + y * nWidth];
				}
			}
			break;

		default:
			assert (0);
			break;
		}

		pPixels = m_pBuffer;
	}

	size_t ulSize = nWidth * nHeight * sizeof (u16);
	while (ulSize)
	{
		// The BCM2835 SPI master has a transfer size limit.
		// TODO: Request this parameter from the SPI master driver.
		const size_t MaxTransferSize = 0xFFFC;

		size_t ulBlockSize = ulSize >= MaxTransferSize ? MaxTransferSize : ulSize;

		SendData (pPixels, ulBlockSize);

		pPixels = (const void *) ((uintptr) pPixels + ulBlockSize);

		ulSize -= ulBlockSize;
	}

	if (pRoutine)
	{
		(*pRoutine) (pParam);
	}
}

void CST7789Display::SetWindow (unsigned x0, unsigned y0, unsigned x1, unsigned y1)
{
	assert (x0 <= x1);
	assert (y0 <= y1);
	assert (x1 < m_nWidth);
	assert (y1 < m_nHeight);

	Command (ST7789_CASET);
	Data (x0 >> 8);
	Data (x0 & 0xFF);
	Data (x1 >> 8);
	Data (x1 & 0xFF);

	Command (ST7789_RASET);
	Data (y0 >> 8);
	Data (y0 & 0xFF);
	Data (y1 >> 8);
	Data (y1 & 0xFF);

	Command (ST7789_RAMWR);
}

void CST7789Display::SendByte (u8 uchByte, boolean bIsData)
{
	assert (m_pSPIMaster != 0);

	m_DCPin.Write (bIsData ? HIGH : LOW);

	m_pSPIMaster->SetClock (m_nClockSpeed);
	m_pSPIMaster->SetMode (m_CPOL, m_CPHA);

#ifndef NDEBUG
	int nResult =
#endif
		m_pSPIMaster->Write (m_nChipSelect, &uchByte, sizeof uchByte);
	assert (nResult == (int) sizeof uchByte);
}

void CST7789Display::SendData (const void *pData, size_t nLength)
{
	assert (pData != 0);
	assert (nLength > 0);
	assert (m_pSPIMaster != 0);

	m_DCPin.Write (HIGH);

	m_pSPIMaster->SetClock (m_nClockSpeed);
	m_pSPIMaster->SetMode (m_CPOL, m_CPHA);

#ifndef NDEBUG
	int nResult =
#endif
		m_pSPIMaster->Write (m_nChipSelect, pData, nLength);
	assert (nResult == (int) nLength);
}
