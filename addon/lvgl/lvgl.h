//
// lvgl.h
//
// C++ wrapper for LVGL with mouse and touch screen support
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2019-2024  R. Stange <rsta2@o2online.de>
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
#ifndef _lvgl_lvgl_h
#define _lvgl_lvgl_h

#include <lvgl/lvgl/lvgl.h>
#include <circle/screen.h>
#include <circle/display.h>
#include <circle/interrupt.h>
#include <circle/input/mouse.h>
#include <circle/input/touchscreen.h>
#include <circle/dmachannel.h>
#include <circle/types.h>
#include <assert.h>

class CLVGL
{
public:
	CLVGL (CScreenDevice *pScreen);
	CLVGL (CDisplay *pDisplay);
	~CLVGL (void);

	boolean Initialize (void);

	void Update (boolean bPlugAndPlayUpdated = FALSE);

private:
	static void DisplayFlush (lv_display_t *pDisplay, const lv_area_t *pArea, u8 *pBuffer);
	static void DisplayFlushComplete (void *pParam);

	static void PointerRead (lv_indev_t *pIndev, lv_indev_data_t *pData);
	static void MouseEventHandler (TMouseEvent Event, unsigned nButtons,
				       unsigned nPosX, unsigned nPosY, int nWheelMove);
	static void TouchScreenEventHandler (TTouchScreenEvent Event, unsigned nID,
					     unsigned nPosX, unsigned nPosY);

	static void LogPrint (lv_log_level_t LogLevel, const char *pMessage);

	static void MouseRemovedHandler (CDevice *pDevice, void *pContext);

	static void PeriodicTickHandler (void);

	void SetupCursor (lv_indev_t *pIndev);

private:
	u16 *m_pBuffer1;
	u16 *m_pBuffer2;

	CScreenDevice *m_pScreen;
	CDisplay *m_pDisplay;
	unsigned m_nLastUpdate;

	CMouseDevice * volatile m_pMouseDevice;
	CTouchScreenDevice *m_pTouchScreen;
	unsigned m_nLastTouchUpdate;
	lv_indev_data_t m_PointerData;

	lv_indev_t *m_pIndev;
	lv_img_dsc_t *m_pCursorDesc;

	static CLVGL *s_pThis;
};

#endif
