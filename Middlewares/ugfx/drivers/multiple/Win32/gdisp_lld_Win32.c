/*
 * This file is subject to the terms of the GFX License. If a copy of
 * the license was not distributed with this file, you can obtain one at:
 *
 *              http://ugfx.org/license.html
 */

#include "gfx.h"

#if GFX_USE_GDISP

#define GDISP_DRIVER_VMT			GDISPVMT_Win32
#include "drivers/multiple/Win32/gdisp_lld_config.h"
#include "src/gdisp/driver.h"

#ifndef GDISP_SCREEN_WIDTH
	#define GDISP_SCREEN_WIDTH	640
#endif
#ifndef GDISP_SCREEN_HEIGHT
	#define GDISP_SCREEN_HEIGHT	480
#endif

// Setting this to TRUE delays updating the screen
// to the windows paint routine. Due to the
// drawing lock this does not add as much speed
// as might be expected but it is still faster in
// all tested circumstances and for all operations
// even draw_pixel().
// This is probably due to drawing operations being
// combined as the update regions are merged.
// The only time you might want to turn this off is
// if you are debugging drawing and want to see each
// pixel as it is set.
#define GDISP_WIN32_USE_INDIRECT_UPDATE		TRUE
//#define GDISP_WIN32_USE_INDIRECT_UPDATE		FALSE

// How far extra windows (multiple displays) should be offset from the first.
#define DISPLAY_X_OFFSET		50
#define DISPLAY_Y_OFFSET		50

#undef Red
#undef Green
#undef Blue

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <wingdi.h>
#include <assert.h>

#define GDISP_FLG_READY				(GDISP_FLG_DRIVER<<0)
#define GDISP_FLG_HASTOGGLE			(GDISP_FLG_DRIVER<<1)
#define GDISP_FLG_HASMOUSE			(GDISP_FLG_DRIVER<<2)
#if GDISP_HARDWARE_STREAM_WRITE || GDISP_HARDWARE_STREAM_READ
	#define GDISP_FLG_WSTREAM			(GDISP_FLG_DRIVER<<3)
	#define GDISP_FLG_WRAPPED			(GDISP_FLG_DRIVER<<4)
#endif

#if GINPUT_NEED_TOGGLE
	/* Include toggle support code */
	#include "src/ginput/driver_toggle.h"
#endif

#if GINPUT_NEED_MOUSE
	/* Include mouse support code */
	#include "src/ginput/driver_mouse.h"
#endif

static DWORD			winThreadId;
static volatile bool_t	QReady;
static HANDLE			drawMutex;
#if GINPUT_NEED_MOUSE
	static GDisplay *	mouseDisplay;
#endif

/*===========================================================================*/
/* Driver local routines    .                                                */
/*===========================================================================*/

#if GINPUT_NEED_TOGGLE
	#define WIN32_BUTTON_AREA		16
#else
	#define WIN32_BUTTON_AREA		0
#endif

#define APP_NAME "uGFX"

typedef struct winPriv {
	HWND			hwnd;
	HDC				dcBuffer;
	HBITMAP			dcBitmap;
	HBITMAP 		dcOldBitmap;
	#if GINPUT_NEED_MOUSE
		coord_t		mousex, mousey;
		uint16_t	mousebuttons;
	#endif
	#if GINPUT_NEED_TOGGLE
		uint8_t		toggles;
	#endif
	#if GDISP_HARDWARE_STREAM_WRITE || GDISP_HARDWARE_STREAM_READ
		coord_t		x0, y0, x1, y1;
		coord_t		x, y;
	#endif
} winPriv;


static LRESULT myWindowProc(HWND hWnd,	UINT Msg, WPARAM wParam, LPARAM lParam)
{
	HDC				dc;
	PAINTSTRUCT		ps;
	GDisplay *		g;
	winPriv *		priv;
	#if GINPUT_NEED_TOGGLE
		HBRUSH		hbrOn, hbrOff;
		HPEN		pen;
		RECT		rect;
		HGDIOBJ		old;
		POINT 		p;
		coord_t		pos;
		uint8_t		bit;
	#endif

	switch (Msg) {
	case WM_CREATE:
		// Get our GDisplay structure and attach it to the window
		g = (GDisplay *)((LPCREATESTRUCT)lParam)->lpCreateParams;
		priv = (winPriv *)g->priv;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)g);

		// Fill in the private area
		priv->hwnd = hWnd;
		dc = GetDC(hWnd);
		priv->dcBitmap = CreateCompatibleBitmap(dc, g->g.Width, g->g.Height);
		priv->dcBuffer = CreateCompatibleDC(dc);
		ReleaseDC(hWnd, dc);
		priv->dcOldBitmap = SelectObject(priv->dcBuffer, priv->dcBitmap);

		// Mark the window as ready to go
		g->flags |= GDISP_FLG_READY;
		break;

	#if GINPUT_NEED_MOUSE || GINPUT_NEED_TOGGLE
		case WM_LBUTTONDOWN:
			// Get our GDisplay structure
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;

			// Handle mouse down on the window
			#if GINPUT_NEED_MOUSE
				if ((coord_t)HIWORD(lParam) < GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASMOUSE)) {
					priv->mousebuttons |= GINPUT_MOUSE_BTN_LEFT;
					goto mousemove;
				}
			#endif

			// Handle mouse down on the toggle area
			#if GINPUT_NEED_TOGGLE
				if ((coord_t)HIWORD(lParam) >= GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASTOGGLE)) {
					bit = 1 << ((coord_t)LOWORD(lParam)*8/g->g.Width);
					priv->toggles ^= bit;
					rect.left = 0;
					rect.right = GDISP_SCREEN_WIDTH;
					rect.top = GDISP_SCREEN_HEIGHT;
					rect.bottom = GDISP_SCREEN_HEIGHT + WIN32_BUTTON_AREA;
					InvalidateRect(hWnd, &rect, FALSE);
					UpdateWindow(hWnd);
					#if GINPUT_TOGGLE_POLL_PERIOD == TIME_INFINITE
						ginputToggleWakeup();
					#endif
				}
			#endif
			break;

		case WM_LBUTTONUP:
			// Get our GDisplay structure
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;

			// Handle mouse up on the toggle area
			#if GINPUT_NEED_TOGGLE
				if ((g->flags & GDISP_FLG_HASTOGGLE)) {
					if ((priv->toggles & 0x0F)) {
						priv->toggles &= ~0x0F;
						rect.left = 0;
						rect.right = GDISP_SCREEN_WIDTH;
						rect.top = GDISP_SCREEN_HEIGHT;
						rect.bottom = GDISP_SCREEN_HEIGHT + WIN32_BUTTON_AREA;
						InvalidateRect(hWnd, &rect, FALSE);
						UpdateWindow(hWnd);
						#if GINPUT_TOGGLE_POLL_PERIOD == TIME_INFINITE
							ginputToggleWakeup();
						#endif
					}
				}
			#endif

			// Handle mouse up on the window
			#if GINPUT_NEED_MOUSE
				if ((coord_t)HIWORD(lParam) < GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASMOUSE)) {
					priv->mousebuttons &= ~GINPUT_MOUSE_BTN_LEFT;
					goto mousemove;
				}
			#endif
			break;
	#endif

	#if GINPUT_NEED_MOUSE
		case WM_MBUTTONDOWN:
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;
			if ((coord_t)HIWORD(lParam) < GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASMOUSE)) {
				priv->mousebuttons |= GINPUT_MOUSE_BTN_MIDDLE;
				goto mousemove;
			}
			break;
		case WM_MBUTTONUP:
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;
			if ((coord_t)HIWORD(lParam) < GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASMOUSE)) {
				priv->mousebuttons &= ~GINPUT_MOUSE_BTN_MIDDLE;
				goto mousemove;
			}
			break;
		case WM_RBUTTONDOWN:
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;
			if ((coord_t)HIWORD(lParam) < GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASMOUSE)) {
				priv->mousebuttons |= GINPUT_MOUSE_BTN_RIGHT;
				goto mousemove;
			}
			break;
		case WM_RBUTTONUP:
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;
			if ((coord_t)HIWORD(lParam) < GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASMOUSE)) {
				priv->mousebuttons &= ~GINPUT_MOUSE_BTN_RIGHT;
				goto mousemove;
			}
			break;
		case WM_MOUSEMOVE:
			g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			priv = (winPriv *)g->priv;
			if ((coord_t)HIWORD(lParam) >= GDISP_SCREEN_HEIGHT || !(g->flags & GDISP_FLG_HASMOUSE))
				break;
		mousemove:
			priv->mousex = (coord_t)LOWORD(lParam);
			priv->mousey = (coord_t)HIWORD(lParam);
			#if GINPUT_MOUSE_POLL_PERIOD == TIME_INFINITE
				ginputMouseWakeup();
			#endif
			break;
	#endif

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYUP:
		break;
	case WM_CHAR:
	case WM_DEADCHAR:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
		break;

	case WM_ERASEBKGND:
		// Pretend we have erased the background.
		// We know we don't really need to do this as we
		// redraw the entire surface in the WM_PAINT handler.
		return TRUE;

	case WM_PAINT:
		// Get our GDisplay structure
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		priv = (winPriv *)g->priv;

		// Paint the main window area
		WaitForSingleObject(drawMutex, INFINITE);
		dc = BeginPaint(hWnd, &ps);
		BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			(ps.rcPaint.bottom > GDISP_SCREEN_HEIGHT ? GDISP_SCREEN_HEIGHT : ps.rcPaint.bottom) - ps.rcPaint.top,
			priv->dcBuffer, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

		// Paint the toggle area
		#if GINPUT_NEED_TOGGLE
			if (ps.rcPaint.bottom >= GDISP_SCREEN_HEIGHT && (g->flags & GDISP_FLG_HASTOGGLE)) {
				pen = CreatePen(PS_SOLID, 1, gdispColor2Native(Black));
				hbrOn = CreateSolidBrush(gdispColor2Native(Blue));
				hbrOff = CreateSolidBrush(gdispColor2Native(Gray));
				old = SelectObject(dc, pen);
				MoveToEx(dc, 0, GDISP_SCREEN_HEIGHT, &p);
				LineTo(dc, GDISP_SCREEN_WIDTH, GDISP_SCREEN_HEIGHT);
				for(pos = 0, bit=1; pos < wWidth; pos=rect.right, bit <<= 1) {
					rect.left = pos;
					rect.right = pos + GDISP_SCREEN_WIDTH/8;
					rect.top = GDISP_SCREEN_HEIGHT;
					rect.bottom = GDISP_SCREEN_HEIGHT + WIN32_BUTTON_AREA;
					FillRect(dc, &rect, (priv->toggles & bit) ? hbrOn : hbrOff);
					if (pos > 0) {
						MoveToEx(dc, rect.left, rect.top, &p);
						LineTo(dc, rect.left, rect.bottom);
					}
				}
				DeleteObject(hbrOn);
				DeleteObject(hbrOff);
				SelectObject(dc, old);
			}
		#endif
		EndPaint(hWnd, &ps);
		ReleaseMutex(drawMutex);
		break;

	case WM_DESTROY:
		// Get our GDisplay structure
		g = (GDisplay *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		priv = (winPriv *)g->priv;

		// Restore the window and free our bitmaps
		SelectObject(priv->dcBuffer, priv->dcOldBitmap);
		DeleteDC(priv->dcBuffer);
		DeleteObject(priv->dcBitmap);

		// Cleanup the private area
		gfxFree(priv);

		// Quit the application
		PostQuitMessage(0);

		// Actually the above doesn't work (who knows why)
		ExitProcess(0);
		break;

	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	return 0;
}

static DWORD WINAPI WindowThread(void *param) {
	(void)param;
	MSG msg;

	// Establish this thread as a message queue thread
	winThreadId = GetCurrentThreadId();
	PeekMessage(&msg, 0, WM_USER, WM_USER, PM_NOREMOVE);
	QReady = TRUE;

	// Create the window class
	{
		WNDCLASS		wc;
		ATOM			winClass;

		wc.style           = CS_HREDRAW | CS_VREDRAW; // | CS_OWNDC;
		wc.lpfnWndProc     = (WNDPROC)myWindowProc;
		wc.cbClsExtra      = 0;
		wc.cbWndExtra      = 0;
		wc.hInstance       = GetModuleHandle(0);
		wc.hIcon           = LoadIcon(0, IDI_APPLICATION);
		wc.hCursor         = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground   = GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName    = 0;
		wc.lpszClassName   = APP_NAME;
		winClass = RegisterClass(&wc);
		assert(winClass != 0);
	}

	do {
		// This is a high priority task - make sure other tasks get a go.
		Sleep(1);

		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			// Is this our special thread message to create a new window?
			if (!msg.hwnd && msg.message == WM_USER) {
				RECT		rect;
				GDisplay	*g;

				g = (GDisplay *)msg.lParam;

				// Set the window rectangle
				rect.top = 0; rect.bottom = g->g.Height;
				rect.left = 0; rect.right = g->g.Width;
				#if GINPUT_NEED_TOGGLE
					if ((g->flags & GDISP_FLG_HASTOGGLE))
						rect.bottom += WIN32_BUTTON_AREA;
				#endif
				AdjustWindowRect(&rect, WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU, 0);

				// Create the window
				msg.hwnd = CreateWindow(APP_NAME, "", WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_BORDER, msg.wParam*DISPLAY_X_OFFSET, msg.wParam*DISPLAY_Y_OFFSET,
								rect.right-rect.left, rect.bottom-rect.top, 0, 0,
								GetModuleHandle(0), g);
				assert(msg.hwnd != 0);

			// Or just a normal window message
			} else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	} while (msg.message != WM_QUIT);
	ExitProcess(0);
	return msg.wParam;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

LLDSPEC bool_t gdisp_lld_init(GDisplay *g) {
	winPriv	*	priv;
	char		buf[132];

	// Initialise the window thread and the window class (if it hasn't been done already)
	if (!QReady) {
		HANDLE			hth;

		// Create the draw mutex
		drawMutex = CreateMutex(0, FALSE, 0);

		// Create the thread
		if (!(hth = CreateThread(0, 0, WindowThread, 0, CREATE_SUSPENDED, 0)))
			return FALSE;
		SetThreadPriority(hth, THREAD_PRIORITY_ABOVE_NORMAL);
		ResumeThread(hth);
		CloseHandle(hth);

		// Wait for our thread to be ready
		while (!QReady)
			Sleep(1);
	}

	// Initialise the GDISP structure
	g->g.Orientation = GDISP_ROTATE_0;
	g->g.Powermode = powerOn;
	g->g.Backlight = 100;
	g->g.Contrast = 50;
	g->g.Width = GDISP_SCREEN_WIDTH;
	g->g.Height = GDISP_SCREEN_HEIGHT;

	// Turn on toggles for the first GINPUT_TOGGLE_CONFIG_ENTRIES windows
	#if GINPUT_NEED_TOGGLE
		if (g->controllerdisplay < GINPUT_TOGGLE_CONFIG_ENTRIES)
			g->flags |= GDISP_FLG_HASTOGGLE;
	#endif

	// Only turn on mouse on the first window for now
	#if GINPUT_NEED_MOUSE
		if (!g->controllerdisplay) {
			mouseDisplay = g;
			g->flags |= GDISP_FLG_HASMOUSE;
		}
	#endif

	// Create a private area for this window
	priv = gfxAlloc(sizeof(winPriv));
	assert(priv != 0);
	memset(priv, 0, sizeof(winPriv));
	g->priv = priv;
	#if GDISP_HARDWARE_STREAM_WRITE || GDISP_HARDWARE_STREAM_READ
		// Initialise with an invalid window
		g->flags &= ~GDISP_FLG_WSTREAM;
	#endif
	g->board = 0;			// no board interface for this controller

	// Create the window in the message thread
	PostThreadMessage(winThreadId, WM_USER, (WPARAM)g->controllerdisplay, (LPARAM)g);

	// Wait for the window creation to complete (for safety)
	while(!(((volatile GDisplay *)g)->flags & GDISP_FLG_READY))
		Sleep(1);

	sprintf(buf, APP_NAME " - %u", g->systemdisplay+1);
	SetWindowText(priv->hwnd, buf);
	ShowWindow(priv->hwnd, SW_SHOW);
	UpdateWindow(priv->hwnd);

	return TRUE;
}

#if GDISP_HARDWARE_FLUSH
	LLDSPEC void gdisp_lld_flush(GDisplay *g) {
		winPriv	*	priv;

		priv = g->priv;
		UpdateWindow(priv->hwnd);
	}
#endif

#if GDISP_HARDWARE_STREAM_WRITE || GDISP_HARDWARE_STREAM_READ
	void BAD_PARAMETER(const char *msg) {
		fprintf(stderr, "%s\n", msg);
	}
#endif

#if GDISP_HARDWARE_STREAM_WRITE
	LLDSPEC	void gdisp_lld_write_start(GDisplay *g) {
		winPriv	*	priv;

		if (g->flags & GDISP_FLG_WSTREAM)
			BAD_PARAMETER("write_start: already in streaming mode");
		if (g->p.cx <= 0 || g->p.cy <= 0 || g->p.x < 0 || g->p.y < 0 || g->p.x+g->p.cx > g->g.Width || g->p.y+g->p.cy > g->g.Height)
			BAD_PARAMETER("write_start: bad window parameter");

		priv = g->priv;
		priv->x0 = g->p.x;	priv->x1 = g->p.x + g->p.cx - 1;
		priv->y0 = g->p.y;	priv->y1 = g->p.y + g->p.cy - 1;
		#if GDISP_HARDWARE_STREAM_POS
			priv->x = g->p.x-1;			// Make sure these values are invalid (for testing)
			priv->y = g->p.y-1;
		#else
			priv->x = g->p.x;
			priv->y = g->p.y;
		#endif
		g->flags |= GDISP_FLG_WSTREAM;
		g->flags &= ~GDISP_FLG_WRAPPED;
	}
	LLDSPEC	void gdisp_lld_write_color(GDisplay *g) {
		winPriv	*	priv;
		int			x, y;
		COLORREF	color;

		priv = g->priv;
		color = gdispColor2Native(g->p.color);

		if (!(g->flags & GDISP_FLG_WSTREAM))
			BAD_PARAMETER("write_color: not in streaming mode");
		if (priv->x < priv->x0 || priv->x > priv->x1 || priv->y < priv->y0 || priv->y > priv->y1)
			BAD_PARAMETER("write_color: cursor outside streaming area");
		if (g->flags & GDISP_FLG_WRAPPED) {
			BAD_PARAMETER("write_color: Warning - Area wrapped.");
			g->flags &= ~GDISP_FLG_WRAPPED;
		}

		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				x = priv->x;
				y = priv->y;
				break;
			case GDISP_ROTATE_90:
				x = priv->y;
				y = g->g.Width - 1 - priv->x;
				break;
			case GDISP_ROTATE_180:
				x = g->g.Width - 1 - priv->x;
				y = g->g.Height - 1 - priv->y;
				break;
			case GDISP_ROTATE_270:
				x = g->g.Height - 1 - priv->y;
				y = priv->x;
				break;
			}
		#else
			x = priv->x;
			y = priv->y;
		#endif

		// Draw the pixel on the screen and in the buffer.
		WaitForSingleObject(drawMutex, INFINITE);
		SetPixel(priv->dcBuffer, x, y, color);
		#if GDISP_WIN32_USE_INDIRECT_UPDATE
			ReleaseMutex(drawMutex);
			{
				RECT	r;
				r.left = x; r.right = x+1;
				r.top = y; r.bottom = y+1;
				InvalidateRect(priv->hwnd, &r, FALSE);
			}
		#else
			{
				HDC		dc;
				dc = GetDC(priv->hwnd);
				SetPixel(dc, x, y, color);
				ReleaseDC(priv->hwnd, dc);
				ReleaseMutex(drawMutex);
			}
		#endif

		// Update the cursor
		if (++priv->x > priv->x1) {
			priv->x = priv->x0;
			if (++priv->y > priv->y1) {
				g->flags |= GDISP_FLG_WRAPPED;
				priv->y = priv->y0;
			}
		}
	}
	LLDSPEC	void gdisp_lld_write_stop(GDisplay *g) {
		if (!(g->flags & GDISP_FLG_WSTREAM))
			BAD_PARAMETER("write_stop: not in streaming mode");
		g->flags &= ~GDISP_FLG_WSTREAM;
	}
	#if GDISP_HARDWARE_STREAM_POS
		LLDSPEC void gdisp_lld_write_pos(GDisplay *g) {
			winPriv	*	priv;

			priv = g->priv;

			if (!(g->flags & GDISP_FLG_WSTREAM))
				BAD_PARAMETER("write_pos: not in streaming mode");
			if (g->p.x < priv->x0 || g->p.x > priv->x1 || g->p.y < priv->y0 || g->p.y > priv->y1)
				BAD_PARAMETER("write_color: new cursor outside streaming area");
			priv->x = g->p.x;
			priv->y = g->p.y;
		}
	#endif
#endif

#if GDISP_HARDWARE_STREAM_READ
	LLDSPEC	void gdisp_lld_read_start(GDisplay *g) {
		winPriv	*	priv;

		if (g->flags & GDISP_FLG_WSTREAM)
			BAD_PARAMETER("read_start: already in streaming mode");
		if (g->p.cx <= 0 || g->p.cy <= 0 || g->p.x < 0 || g->p.y < 0 || g->p.x+g->p.cx > g->g.Width || g->p.y+g->p.cy > g->g.Height)
			BAD_PARAMETER("read_start: bad window parameter");

		priv = g->priv;
		priv->x0 = g->p.x;	priv->x1 = g->p.x + g->p.cx - 1;
		priv->y0 = g->p.y;	priv->y1 = g->p.y + g->p.cy - 1;
		priv->x = g->p.x;
		priv->y = g->p.y;
		g->flags |= GDISP_FLG_WSTREAM;
		g->flags &= ~GDISP_FLG_WRAPPED;
	}
	LLDSPEC	color_t gdisp_lld_read_color(GDisplay *g) {
		winPriv	*	priv;
		COLORREF	color;

		priv = g->priv;

		if (!(g->flags & GDISP_FLG_WSTREAM))
			BAD_PARAMETER("read_color: not in streaming mode");
		if (priv->x < priv->x0 || priv->x > priv->x1 || priv->y < priv->y0 || priv->y > priv->y1)
			BAD_PARAMETER("read_color: cursor outside streaming area");
		if (g->flags & GDISP_FLG_WRAPPED) {
			BAD_PARAMETER("read_color: Warning - Area wrapped.");
			g->flags &= ~GDISP_FLG_WRAPPED;
		}

		WaitForSingleObject(drawMutex, INFINITE);
		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				color = GetPixel(priv->dcBuffer, priv->x, priv->y);
				break;
			case GDISP_ROTATE_90:
				color = GetPixel(priv->dcBuffer, priv->y, g->g.Width - 1 - priv->x);
				break;
			case GDISP_ROTATE_180:
				color = GetPixel(priv->dcBuffer, g->g.Width - 1 - priv->x, g->g.Height - 1 - priv->y);
				break;
			case GDISP_ROTATE_270:
				color = GetPixel(priv->dcBuffer, g->g.Height - 1 - priv->y, priv->x);
				break;
			}
		#else
			color = GetPixel(priv->dcBuffer, priv->x, priv->y);
		#endif
		ReleaseMutex(drawMutex);

		// Update the cursor
		if (++priv->x > priv->x1) {
			priv->x = priv->x0;
			if (++priv->y > priv->y1) {
				g->flags |= GDISP_FLG_WRAPPED;
				priv->y = priv->y0;
			}
		}

		return gdispNative2Color(color);
	}
	LLDSPEC	void gdisp_lld_read_stop(GDisplay *g) {
		if (!(g->flags & GDISP_FLG_WSTREAM))
			BAD_PARAMETER("write_stop: not in streaming mode");
		g->flags &= ~GDISP_FLG_WSTREAM;
	}
#endif

#if GDISP_HARDWARE_DRAWPIXEL
	LLDSPEC void gdisp_lld_draw_pixel(GDisplay *g) {
		winPriv	*	priv;
		int			x, y;
		COLORREF	color;
	
		priv = g->priv;
		color = gdispColor2Native(g->p.color);
	
		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				x = g->p.x;
				y = g->p.y;
				break;
			case GDISP_ROTATE_90:
				x = g->p.y;
				y = g->g.Width - 1 - g->p.x;
				break;
			case GDISP_ROTATE_180:
				x = g->g.Width - 1 - g->p.x;
				y = g->g.Height - 1 - g->p.y;
				break;
			case GDISP_ROTATE_270:
				x = g->g.Height - 1 - g->p.y;
				y = g->p.x;
				break;
			}
		#else
			x = g->p.x;
			y = g->p.y;
		#endif

		// Draw the pixel on the screen and in the buffer.
		WaitForSingleObject(drawMutex, INFINITE);
		SetPixel(priv->dcBuffer, x, y, color);
		#if GDISP_WIN32_USE_INDIRECT_UPDATE
			ReleaseMutex(drawMutex);
			{
				RECT	r;
				r.left = x; r.right = x+1;
				r.top = y; r.bottom = y+1;
				InvalidateRect(priv->hwnd, &r, FALSE);
			}
		#else
			{
				HDC		dc;
				dc = GetDC(priv->hwnd);
				SetPixel(dc, x, y, color);
				ReleaseDC(priv->hwnd, dc);
				ReleaseMutex(drawMutex);
			}
		#endif
	}
#endif

/* ---- Optional Routines ---- */

#if GDISP_HARDWARE_FILLS
	LLDSPEC void gdisp_lld_fill_area(GDisplay *g) {
		winPriv	*	priv;
		RECT		rect;
		HBRUSH		hbr;
		COLORREF	color;

		priv = g->priv;
		color = gdispColor2Native(g->p.color);
		hbr = CreateSolidBrush(color);

		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				rect.top = g->p.y;
				rect.bottom = rect.top + g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left + g->p.cx;
				break;
			case GDISP_ROTATE_90:
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom - g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left + g->p.cy;
				break;
			case GDISP_ROTATE_180:
				rect.bottom = g->g.Height - g->p.y;
				rect.top = rect.bottom - g->p.cy;
				rect.right = g->g.Width - g->p.x;
				rect.left = rect.right - g->p.cx;
				break;
			case GDISP_ROTATE_270:
				rect.top = g->p.x;
				rect.bottom = rect.top + g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right - g->p.cy;
				break;
			}
		#else
			rect.top = g->p.y;
			rect.bottom = rect.top + g->p.cy;
			rect.left = g->p.x;
			rect.right = rect.left + g->p.cx;
		#endif


		WaitForSingleObject(drawMutex, INFINITE);
		FillRect(priv->dcBuffer, &rect, hbr);
		#if GDISP_WIN32_USE_INDIRECT_UPDATE
			ReleaseMutex(drawMutex);
			InvalidateRect(priv->hwnd, &rect, FALSE);
		#else
			{
				HDC		dc;
				dc = GetDC(priv->hwnd);
				FillRect(dc, &rect, hbr);
				ReleaseDC(priv->hwnd, dc);
				ReleaseMutex(drawMutex);
			}
		#endif

		DeleteObject(hbr);
	}
#endif

#if GDISP_HARDWARE_BITFILLS && GDISP_NEED_CONTROL
	static pixel_t *rotateimg(GDisplay *g, const pixel_t *buffer) {
		pixel_t	*dstbuf;
		pixel_t	*dst;
		const pixel_t	*src;
		size_t	sz;
		coord_t	i, j;

		// Allocate the destination buffer
		sz = (size_t)g->p.cx * (size_t)g->p.cy;
		if (!(dstbuf = (pixel_t *)malloc(sz * sizeof(pixel_t))))
			return 0;
		
		// Copy the bits we need
		switch(g->g.Orientation) {
		case GDISP_ROTATE_0:
		default:
			return 0;					// not handled as it doesn't need to be.
		case GDISP_ROTATE_90:
			for(src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx) {
				dst = dstbuf+sz-g->p.cy+j;
				for(i = 0; i < g->p.cx; i++, dst -= g->p.cy)
					*dst = *src++;
			}
			break;
		case GDISP_ROTATE_180:
			for(dst = dstbuf+sz, src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx)
				for(i = 0; i < g->p.cx; i++)
					*--dst = *src++;
			break;
		case GDISP_ROTATE_270:
			for(src = buffer+g->p.x1, j = 0; j < g->p.cy; j++, src += g->p.x2 - g->p.cx) {
				dst = dstbuf+g->p.cy-j-1;
				for(i = 0; i < g->p.cx; i++, dst += g->p.cy)
					*dst = *src++;
			}
			break;
		}
		return dstbuf;
	}
#endif
	
#if GDISP_HARDWARE_BITFILLS
	#if COLOR_SYSTEM != GDISP_COLORSYSTEM_TRUECOLOR || COLOR_TYPE_BITS <= 8
		#error "GDISP Win32: This driver's bitblit currently only supports true-color with bit depths > 8 bits."
	#endif

	LLDSPEC void gdisp_lld_blit_area(GDisplay *g) {
		winPriv	*		priv;
		pixel_t	*		buffer;
		RECT			rect;
		BITMAPV4HEADER	bmpInfo;

		// Make everything relative to the start of the line
		priv = g->priv;
		buffer = g->p.ptr;
		buffer += g->p.x2*g->p.y1;
		
		memset(&bmpInfo, 0, sizeof(bmpInfo));
		bmpInfo.bV4Size = sizeof(bmpInfo);
		bmpInfo.bV4Planes = 1;
		bmpInfo.bV4BitCount = COLOR_TYPE_BITS;
		bmpInfo.bV4AlphaMask = 0;
		bmpInfo.bV4RedMask		= RGB2COLOR(255,0,0);
		bmpInfo.bV4GreenMask	= RGB2COLOR(0,255,0);
		bmpInfo.bV4BlueMask		= RGB2COLOR(0,0,255);
		bmpInfo.bV4V4Compression = BI_BITFIELDS;
		bmpInfo.bV4XPelsPerMeter = 3078;
		bmpInfo.bV4YPelsPerMeter = 3078;
		bmpInfo.bV4ClrUsed = 0;
		bmpInfo.bV4ClrImportant = 0;
		bmpInfo.bV4CSType = 0; //LCS_sRGB;

		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				bmpInfo.bV4SizeImage = (g->p.cy*g->p.x2) * sizeof(pixel_t);
				bmpInfo.bV4Width = g->p.x2;
				bmpInfo.bV4Height = -g->p.cy; /* top-down image */
				rect.top = g->p.y;
				rect.bottom = rect.top+g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left+g->p.cx;
				break;
			case GDISP_ROTATE_90:
				if (!(buffer = rotateimg(g, buffer))) return;
				bmpInfo.bV4SizeImage = (g->p.cy*g->p.cx) * sizeof(pixel_t);
				bmpInfo.bV4Width = g->p.cy;
				bmpInfo.bV4Height = -g->p.cx; /* top-down image */
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom-g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left+g->p.cy;
				break;
			case GDISP_ROTATE_180:
				if (!(buffer = rotateimg(g, buffer))) return;
				bmpInfo.bV4SizeImage = (g->p.cy*g->p.cx) * sizeof(pixel_t);
				bmpInfo.bV4Width = g->p.cx;
				bmpInfo.bV4Height = -g->p.cy; /* top-down image */
				rect.bottom = g->g.Height-1 - g->p.y;
				rect.top = rect.bottom-g->p.cy;
				rect.right = g->g.Width - g->p.x;
				rect.left = rect.right-g->p.cx;
				break;
			case GDISP_ROTATE_270:
				if (!(buffer = rotateimg(g, buffer))) return;
				bmpInfo.bV4SizeImage = (g->p.cy*g->p.cx) * sizeof(pixel_t);
				bmpInfo.bV4Width = g->p.cy;
				bmpInfo.bV4Height = -g->p.cx; /* top-down image */
				rect.top = g->p.x;
				rect.bottom = rect.top+g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right-g->p.cy;
				break;
			}
		#else
			bmpInfo.bV4SizeImage = (g->p.cy*g->p.x2) * sizeof(pixel_t);
			bmpInfo.bV4Width = g->p.x2;
			bmpInfo.bV4Height = -g->p.cy; /* top-down image */
			rect.top = g->p.y;
			rect.bottom = rect.top+g->p.cy;
			rect.left = g->p.x;
			rect.right = rect.left+g->p.cx;
		#endif

		WaitForSingleObject(drawMutex, INFINITE);
		SetDIBitsToDevice(priv->dcBuffer, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0, 0, 0, rect.bottom-rect.top, buffer, (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS);
		#if GDISP_WIN32_USE_INDIRECT_UPDATE
			ReleaseMutex(drawMutex);
			InvalidateRect(priv->hwnd, &rect, FALSE);
		#else
			{
				HDC		dc;
				dc = GetDC(priv->hwnd);
				SetDIBitsToDevice(dc, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, 0, 0, 0, rect.bottom-rect.top, buffer, (BITMAPINFO*)&bmpInfo, DIB_RGB_COLORS);
				ReleaseDC(priv->hwnd, dc);
				ReleaseMutex(drawMutex);
			}
		#endif

		#if GDISP_NEED_CONTROL
			if (buffer != (pixel_t *)g->p.ptr)
				free(buffer);
		#endif
	}
#endif

#if GDISP_HARDWARE_PIXELREAD
	LLDSPEC	color_t gdisp_lld_get_pixel_color(GDisplay *g) {
		winPriv	*	priv;
		COLORREF	color;

		priv = g->priv;

		WaitForSingleObject(drawMutex, INFINITE);
		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				color = GetPixel(priv->dcBuffer, g->p.x, g->p.y);
				break;
			case GDISP_ROTATE_90:
				color = GetPixel(priv->dcBuffer, g->p.y, g->g.Width - 1 - g->p.x);
				break;
			case GDISP_ROTATE_180:
				color = GetPixel(priv->dcBuffer, g->g.Width - 1 - g->p.x, g->g.Height - 1 - g->p.y);
				break;
			case GDISP_ROTATE_270:
				color = GetPixel(priv->dcBuffer, g->g.Height - 1 - g->p.y, g->p.x);
				break;
			}
		#else
			color = GetPixel(priv->dcBuffer, g->p.x, g->p.y);
		#endif
		ReleaseMutex(drawMutex);
		
		return gdispNative2Color(color);
	}
#endif

#if GDISP_NEED_SCROLL && GDISP_HARDWARE_SCROLL
	LLDSPEC void gdisp_lld_vertical_scroll(GDisplay *g) {
		winPriv	*	priv;
		RECT		rect;
		coord_t		lines;
		
		priv = g->priv;

		#if GDISP_NEED_CONTROL
			switch(g->g.Orientation) {
			case GDISP_ROTATE_0:
			default:
				rect.top = g->p.y;
				rect.bottom = rect.top+g->p.cy;
				rect.left = g->p.x;
				rect.right = rect.left+g->p.cx;
				lines = -g->p.y1;
				goto vertical_scroll;
			case GDISP_ROTATE_90:
				rect.bottom = g->g.Width - g->p.x;
				rect.top = rect.bottom-g->p.cx;
				rect.left = g->p.y;
				rect.right = rect.left+g->p.cy;
				lines = -g->p.y1;
				goto horizontal_scroll;
			case GDISP_ROTATE_180:
				rect.bottom = g->g.Height - g->p.y;
				rect.top = rect.bottom-g->p.cy;
				rect.right = g->g.Width - g->p.x;
				rect.left = rect.right-g->p.cx;
				lines = g->p.y1;
			vertical_scroll:
				if (lines > 0) {
					rect.bottom -= lines;
				} else {
					rect.top -= lines;
				}
				if (g->p.cy >= lines && g->p.cy >= -lines) {
					WaitForSingleObject(drawMutex, INFINITE);
					ScrollDC(priv->dcBuffer, 0, lines, &rect, 0, 0, 0);
					#if GDISP_WIN32_USE_INDIRECT_UPDATE
						ReleaseMutex(drawMutex);
						InvalidateRect(priv->hwnd, &rect, FALSE);
					#else
						{
							HDC		dc;
							dc = GetDC(priv->hwnd);
							ScrollDC(dc, 0, lines, &rect, 0, 0, 0);
							ReleaseDC(priv->hwnd, dc);
							ReleaseMutex(drawMutex);
						}
					#endif
				}
				break;
			case GDISP_ROTATE_270:
				rect.top = g->p.x;
				rect.bottom = rect.top+g->p.cx;
				rect.right = g->g.Height - g->p.y;
				rect.left = rect.right-g->p.cy;
				lines = g->p.y1;
			horizontal_scroll:
				if (lines > 0) {
					rect.right -= lines;
				} else {
					rect.left -= lines;
				}
				if (g->p.cy >= lines && g->p.cy >= -lines) {
					WaitForSingleObject(drawMutex, INFINITE);
					ScrollDC(priv->dcBuffer, lines, 0, &rect, 0, 0, 0);
					#if GDISP_WIN32_USE_INDIRECT_UPDATE
						ReleaseMutex(drawMutex);
						InvalidateRect(priv->hwnd, &rect, FALSE);
					#else
						{
							HDC		dc;
							dc = GetDC(priv->hwnd);
							ScrollDC(dc, lines, 0, &rect, 0, 0, 0);
							ReleaseDC(priv->hwnd, dc);
							ReleaseMutex(drawMutex);
						}
					#endif
				}
				break;
			}
		#else
			rect.top = g->p.y;
			rect.bottom = rect.top+g->p.cy;
			rect.left = g->p.x;
			rect.right = rect.left+g->p.cx;
			lines = -g->p.y1;
			if (lines > 0) {
				rect.bottom -= lines;
			} else {
				rect.top -= lines;
			}
			if (g->p.cy >= lines && g->p.cy >= -lines) {
				WaitForSingleObject(drawMutex, INFINITE);
				ScrollDC(priv->dcBuffer, 0, lines, &rect, 0, 0, 0);
				#if GDISP_WIN32_USE_INDIRECT_UPDATE
					ReleaseMutex(drawMutex);
					InvalidateRect(priv->hwnd, &rect, FALSE);
				#else
					{
						HDC		dc;
						dc = GetDC(priv->hwnd);
						ScrollDC(dc, 0, lines, &rect, 0, 0, 0);
						ReleaseDC(priv->hwnd, dc);
						ReleaseMutex(drawMutex);
					}
				#endif
			}
		#endif
	}
#endif

#if GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL
	LLDSPEC void gdisp_lld_control(GDisplay *g) {
		switch(g->p.x) {
		case GDISP_CONTROL_ORIENTATION:
			if (g->g.Orientation == (orientation_t)g->p.ptr)
				return;
			switch((orientation_t)g->p.ptr) {
				case GDISP_ROTATE_0:
				case GDISP_ROTATE_180:
					g->g.Width = GDISP_SCREEN_WIDTH;
					g->g.Height = GDISP_SCREEN_HEIGHT;
					break;
				case GDISP_ROTATE_90:
				case GDISP_ROTATE_270:
					g->g.Height = GDISP_SCREEN_WIDTH;
					g->g.Width = GDISP_SCREEN_HEIGHT;
					break;
				default:
					return;
			}
			g->g.Orientation = (orientation_t)g->p.ptr;
			return;
/*
		case GDISP_CONTROL_POWER:
		case GDISP_CONTROL_BACKLIGHT:
		case GDISP_CONTROL_CONTRAST:
*/
		}
	}
#endif

#if GINPUT_NEED_MOUSE
	void ginput_lld_mouse_init(void) {}
	void ginput_lld_mouse_get_reading(MouseReading *pt) {
		GDisplay *	g;
		winPriv	*	priv;

		g = mouseDisplay;
		priv = g->priv;

		pt->x = priv->mousex;
		pt->y = priv->mousey > g->g.Height ? g->g.Height : priv->mousey;
		pt->z = (priv->mousebuttons & GINPUT_MOUSE_BTN_LEFT) ? 100 : 0;
		pt->buttons = priv->mousebuttons;
	}
#endif /* GINPUT_NEED_MOUSE */

#if GINPUT_NEED_TOGGLE
	#if GINPUT_TOGGLE_CONFIG_ENTRIES > GDISP_DRIVER_COUNT_WIN32
		#error "GDISP Win32: GINPUT_TOGGLE_CONFIG_ENTRIES must not be greater than GDISP_DRIVER_COUNT_WIN32"
	#endif

	GToggleConfig GInputToggleConfigTable[GINPUT_TOGGLE_CONFIG_ENTRIES];

	void ginput_lld_toggle_init(const GToggleConfig *ptc) {
		// Save the associated window struct
		ptc->id = &GDISP_WIN32[ptc - GInputToggleConfigTable];

		// We have 8 buttons per window.
		ptc->mask = 0xFF;

		// No inverse or special mode
		ptc->invert = 0x00;
		ptc->mode = 0;
	}
	unsigned ginput_lld_toggle_getbits(const GToggleConfig *ptc) {
		return ((GDisplay *)(ptc->id))->priv->toggles;
	}
#endif /* GINPUT_NEED_TOGGLE */

#endif /* GFX_USE_GDISP */
