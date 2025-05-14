//
/// \file screen.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2024  R. Stange <rsta2@o2online.de>
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
#ifndef _circle_screen_h
#define _circle_screen_h

#include <circle/device.h>
#include <circle/bcmframebuffer.h>
#include <circle/terminal.h>
#include <circle/chargenerator.h>
#include <circle/sysconfig.h>
#include <circle/types.h>

#ifndef DEPTH
#define DEPTH	32		// can be: 8, 16 or 32
#endif

// really ((green) & 0x3F) << 5, but to have a 0-31 range for all colors
#define COLOR16(red, green, blue)	  (((red) & 0x1F) << 11 \
					| ((green) & 0x1F) << 6 \
					| ((blue) & 0x1F))

#define COLOR32(red, green, blue, alpha)  (((blue) & 0xFF)       \
					| ((green) & 0xFF) << 8  \
					| ((red) & 0xFF)   << 16 \
					| ((alpha) & 0xFF) << 24)

#define BLACK_COLOR	0

#define NORMAL_COLOR	BRIGHT_WHITE_COLOR
#define HIGH_COLOR	BRIGHT_RED_COLOR
#define HALF_COLOR	BLUE_COLOR

#if DEPTH == 8
	typedef u8 TScreenColor;

	#define RED_COLOR16			COLOR16 (170 >> 3, 0, 0)
	#define GREEN_COLOR16			COLOR16 (0, 170 >> 3, 0)
	#define YELLOW_COLOR16			COLOR16 (170 >> 3, 85 >> 3, 0)
	#define BLUE_COLOR16			COLOR16 (0, 0, 170 >> 3)
	#define MAGENTA_COLOR16			COLOR16 (170 >> 3, 0, 170 >> 3)
	#define CYAN_COLOR16			COLOR16 (0, 170 >> 3, 170 >> 3)
	#define WHITE_COLOR16			COLOR16 (170 >> 3, 170 >> 3, 170 >> 3)

	#define BRIGHT_BLACK_COLOR16		COLOR16 (85 >> 3, 85 >> 3, 85 >> 3)
	#define BRIGHT_RED_COLOR16		COLOR16 (255 >> 3, 85 >> 3, 85 >> 3)
	#define BRIGHT_GREEN_COLOR16		COLOR16 (85 >> 3, 255 >> 3, 85 >> 3)
	#define BRIGHT_YELLOW_COLOR16		COLOR16 (255 >> 3, 255 >> 3, 85 >> 3)
	#define BRIGHT_BLUE_COLOR16		COLOR16 (85 >> 3, 85 >> 3, 255 >> 3)
	#define BRIGHT_MAGENTA_COLOR16		COLOR16 (255 >> 3, 85 >> 3, 255 >> 3)
	#define BRIGHT_CYAN_COLOR16		COLOR16 (85 >> 3, 255 >> 3, 255 >> 3)
	#define BRIGHT_WHITE_COLOR16		COLOR16 (255 >> 3, 255 >> 3, 255 >> 3)

	#define RED_COLOR			1
	#define GREEN_COLOR			2
	#define YELLOW_COLOR			3
	#define BLUE_COLOR			4
	#define MAGENTA_COLOR			5
	#define CYAN_COLOR			6
	#define WHITE_COLOR			7

	#define BRIGHT_BLACK_COLOR		8
	#define BRIGHT_RED_COLOR		9
	#define BRIGHT_GREEN_COLOR		10
	#define BRIGHT_YELLOW_COLOR		11
	#define BRIGHT_BLUE_COLOR		12
	#define BRIGHT_MAGENTA_COLOR		13
	#define BRIGHT_CYAN_COLOR		14
	#define BRIGHT_WHITE_COLOR		15

#elif DEPTH == 16
	typedef u16 TScreenColor;

	#define RED_COLOR			COLOR16 (170 >> 3, 0, 0)
	#define GREEN_COLOR			COLOR16 (0, 170 >> 3, 0)
	#define YELLOW_COLOR			COLOR16 (170 >> 3, 85 >> 3, 0)
	#define BLUE_COLOR			COLOR16 (0, 0, 170 >> 3)
	#define MAGENTA_COLOR			COLOR16 (170 >> 3, 0, 170 >> 3)
	#define CYAN_COLOR			COLOR16 (0, 170 >> 3, 170 >> 3)
	#define WHITE_COLOR			COLOR16 (170 >> 3, 170 >> 3, 170 >> 3)

	#define BRIGHT_BLACK_COLOR		COLOR16 (85 >> 3, 85 >> 3, 85 >> 3)
	#define BRIGHT_RED_COLOR		COLOR16 (255 >> 3, 85 >> 3, 85 >> 3)
	#define BRIGHT_GREEN_COLOR		COLOR16 (85 >> 3, 255 >> 3, 85 >> 3)
	#define BRIGHT_YELLOW_COLOR		COLOR16 (255 >> 3, 255 >> 3, 85 >> 3)
	#define BRIGHT_BLUE_COLOR		COLOR16 (85 >> 3, 85 >> 3, 255 >> 3)
	#define BRIGHT_MAGENTA_COLOR		COLOR16 (255 >> 3, 85 >> 3, 255 >> 3)
	#define BRIGHT_CYAN_COLOR		COLOR16 (85 >> 3, 255 >> 3, 255 >> 3)
	#define BRIGHT_WHITE_COLOR		COLOR16 (255 >> 3, 255 >> 3, 255 >> 3)
#elif DEPTH == 32
	typedef u32 TScreenColor;

	#define RED_COLOR			COLOR32 (170, 0, 0, 255)
	#define GREEN_COLOR			COLOR32 (0, 170, 0, 255)
	#define YELLOW_COLOR			COLOR32 (170, 85, 0, 255)
	#define BLUE_COLOR			COLOR32 (0, 0, 170, 255)
	#define MAGENTA_COLOR			COLOR32 (170, 0, 170, 255)
	#define CYAN_COLOR			COLOR32 (0, 170, 170, 255)
	#define WHITE_COLOR			COLOR32 (170, 170, 170, 255)

	#define BRIGHT_BLACK_COLOR		COLOR32 (85, 85, 85, 255)
	#define BRIGHT_RED_COLOR		COLOR32 (255, 85, 85, 255)
	#define BRIGHT_GREEN_COLOR		COLOR32 (85, 255, 85, 255)
	#define BRIGHT_YELLOW_COLOR		COLOR32 (255, 255, 85, 255)
	#define BRIGHT_BLUE_COLOR		COLOR32 (85, 85, 255, 255)
	#define BRIGHT_MAGENTA_COLOR		COLOR32 (255, 85, 255, 255)
	#define BRIGHT_CYAN_COLOR		COLOR32 (85, 255, 255, 255)
	#define BRIGHT_WHITE_COLOR		COLOR32 (255, 255, 255, 255)
#else
	#error DEPTH must be 8, 16 or 32
#endif

/// \note For compatibility and performance this class works with raw colors.

class CScreenDevice : public CDevice	/// Writing characters to screen
{
public:
	/// \param nWidth   Screen width in pixels (0 for default resolution)
	/// \param nHeight  Screen height in pixels (0 for default resolution)
	/// \param rFont    Font to be used
	/// \param FontFlags Font flags
	/// \param nDisplay Zero-based display number (for Raspberry Pi 4)
	CScreenDevice (unsigned nWidth, unsigned nHeight, const TFont &rFont = DEFAULT_FONT,
		       CCharGenerator::TFontFlags FontFlags = CCharGenerator::FontFlagsNone,
		       unsigned nDisplay = 0);

	~CScreenDevice (void);

	/// \return Operation successful?
	boolean Initialize (void);

	/// \param nWidth  New screen width in pixels
	/// \param nHeight New screen height in pixels
	/// \return Operation successful?
	/// \note When FALSE is returned, the width and/or height are not supported.\n
	///	  The object is in an uninitialized state then and must not be used,\n
	///	  but Resize() can be called again with other parameters.
	boolean Resize (unsigned nWidth, unsigned nHeight);

	/// \return Screen width in pixels
	unsigned GetWidth (void) const;
	/// \return Screen height in pixels
	unsigned GetHeight (void) const;

	/// \return Screen width in characters
	unsigned GetColumns (void) const;
	/// \return Screen height in characters
	unsigned GetRows (void) const;

#ifndef SCREEN_HEADLESS
	/// \return Pointer to frame buffer object
	CBcmFrameBuffer *GetFrameBuffer (void);
#endif

	/// \brief Write characters to screen
	/// \note Supports several escape sequences (see: doc/screen.txt).
	/// \param pBuffer Pointer to the characters to be written
	/// \param nCount  Number of characters to be written
	/// \return Number of written characters
	int Write (const void *pBuffer, size_t nCount);

	/// \brief Set a pixel to a specific color
	/// \param nPosX X-Position of the pixel (based on 0)
	/// \param nPosY Y-Position of the pixel (based on 0)
	/// \param Color The color to be set (value depends on screen DEPTH)
	void SetPixel (unsigned nPosX, unsigned nPosY, TScreenColor Color);
	/// \brief Get the color value of a pixel
	/// \param nPosX X-Position of the pixel (based on 0)
	/// \param nPosY Y-Position of the pixel (based on 0)
	/// \return The requested color value (depends on screen DEPTH)
	TScreenColor GetPixel (unsigned nPosX, unsigned nPosY);

	/// \brief Displays rotating symbols in the upper right corner of the screen
	/// \param nIndex Index of the rotor to be displayed (0..3)
	/// \param nCount Phase (angle) of the current rotor symbol (0..3)
	void Rotor (unsigned nIndex, unsigned nCount);

	/// \brief Implements a block cursor instead of the default underline
	void SetCursorBlock(boolean bCursorBlock);

	/// \brief Update the display from internal display buffer
	/// \param nMillis Update only, when N ms were passed since previous update (0 to disable)
	/// \note Once this method has been called, the display is only updated,
	///	  when this method is called and only in the given time interval.
	void Update (unsigned nMillis = 0);

private:
	static const unsigned Rotors = 4;

private:
	unsigned	 m_nInitWidth;
	unsigned	 m_nInitHeight;
	unsigned	 m_nDisplay;
#ifndef SCREEN_HEADLESS
	const TFont	&m_rFont;
	CCharGenerator::TFontFlags m_FontFlags;
	CBcmFrameBuffer	*m_pFrameBuffer;
	CTerminalDevice	*m_pTerminal;
#else
	unsigned	 m_nWidth;
	unsigned	 m_nHeight;
	unsigned	 m_nUsedHeight;
	CCharGenerator	 m_CharGen;
#endif
};

#endif
