#include "PocketGnuboy.h"
#include "wce.h"
#include <ddraw.h>

extern "C" {

#include "defs.h"
#include "fb.h"
#include "lcd.h"

	void joy_init();
	void joy_close();
	void joy_poll();

	/*
	* process graphics
	*/

	int ddraw = 1;
	int screenmode = 0;
	int screenchanged = 0;

	unsigned char* screenvram;
	int screenpitchx;
	int screenpitchy;

	HDC screendc = 0;
	HBITMAP screenbmp = 0;
	byte* screenbuf = 0;

	IDirectDraw * pDD = NULL;
	IDirectDrawSurface * pDDSPrimary = NULL;
	DDSURFACEDESC ddsd;

	static int frameskip = -1;
	struct fb fb = {0};

	static int initok = 0;
	static int sleep_mod = 0;

	static int fps_reset = 1;
	static int fps_count = 0;
	static int fps_last_count = 0;
	static int fps_frame_count = 0;
	static int fps_time = 0;

#define MAX_FRAME_SKIP                  5
#define FRAME_PERIOD                    16742
	static int auto_skip = 0;
	static int auto_skip_count = 0;
	static _int64 auto_start_time = 0;
	static int reset_timer = 0;

	static int old_timer = 0;
	static _int64 start_time = 0;
	static double millisecs_per_tick;

	void timer_init()
	{
		_int64 ticks_per_sec;
		double secs_per_tick;

		if (!QueryPerformanceFrequency((LARGE_INTEGER*)&ticks_per_sec)) {
			// old machine, no performance counter available
			start_time = GetTickCount();
			old_timer = 1;
		}
		else {
			// newer machine, use performance counter
			QueryPerformanceCounter((LARGE_INTEGER *)&start_time);
			secs_per_tick = ((double)1.0) / ((double)ticks_per_sec);
			millisecs_per_tick = ((double)1000.0) / ((double)ticks_per_sec);
			old_timer = 0;
		}
	}

	DWORD timer_time_in_msec()
	{
		if (old_timer)
		{
			// fall back to GetTickCount();
			return (DWORD)(GetTickCount() - start_time);
		}
		else
		{
			// use performance counter
			_int64 temptime;
			QueryPerformanceCounter((LARGE_INTEGER*)&temptime);
			return (DWORD)(((float)(temptime - start_time)) * millisecs_per_tick);
		}
	}

	void vid_preinit()
	{
		/* do nothing; only needed on systems where we must drop perms */
	}

	void vid_create_screenbmp()
	{
		DWORD* quad;
		BITMAPINFOHEADER* pbi;

		HDC hdc = GetDC(g_hwndMain);
		screendc = CreateCompatibleDC(hdc);
		pbi = (BITMAPINFOHEADER*)new byte[sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD)];
		pbi = pbi;
		memset(pbi, 0, sizeof(BITMAPINFOHEADER) + 3 * sizeof(RGBQUAD));
		pbi->biSize = sizeof(BITMAPINFOHEADER);
		pbi->biWidth = SCREENBMP_WIDTH;
		pbi->biHeight = -SCREENBMP_HEIGHT;
		pbi->biPlanes = 1;
		pbi->biBitCount = 16;
		pbi->biCompression = BI_BITFIELDS;

		quad = (DWORD*)&((LPBITMAPINFO)pbi)->bmiColors[0];
		quad[0] = 0xF800; quad[1] = 0x07E0; quad[2] = 0x001F; // RGB565
		screenbmp = CreateDIBSection(NULL, (LPBITMAPINFO)pbi, DIB_RGB_COLORS, (void**)&screenbuf, NULL, 0);

		if (screenbuf) {
			memset(screenbuf, 0, 240 * 216 * 2);
			SelectObject(screendc, screenbmp);
		}
		ReleaseDC(g_hwndMain, hdc);
		delete pbi;
	}

	void vid_init()
	{
		if (initok) 
			return;

		joy_init();
		timer_init();
		vid_create_screenbmp();

		if (ddraw) {
			HRESULT hr;
			hr = DirectDrawCreate(NULL, &pDD, NULL);
			hr = pDD->SetCooperativeLevel(g_hwndMain, DDSCL_NORMAL);
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			hr = pDD->CreateSurface(&ddsd, &pDDSPrimary, NULL);
			if(hr != DD_OK)
				MessageBox(g_hwndMain, TEXT("Error"), TEXT("Error"), MB_OK);

			/*screenpitchx = ddsd.lXPitch >> 1;
			screenpitchy = ddsd.lPitch;

			fb.w = SCREENBMP_WIDTH;
			fb.h = SCREENBMP_HEIGHT;
			fb.pitch = screenpitchx;
			fb.ptr = NULL;*/
			screenpitchx = 1;
			screenpitchy = SCREENBMP_WIDTH * 2;

			fb.w = SCREENBMP_WIDTH;
			fb.h = SCREENBMP_HEIGHT;
			fb.pitch = SCREENBMP_WIDTH;
			fb.ptr = NULL;
		}
		else 
		{
			screenpitchx = 1;
			screenpitchy = SCREENBMP_WIDTH * 2;

			fb.w = SCREENBMP_WIDTH;
			fb.h = SCREENBMP_HEIGHT;
			fb.pitch = SCREENBMP_WIDTH;
			fb.ptr = NULL;
		}
		fb.pelsize = 2;
		fb.indexed = 0;
		fb.cc[0].r = 3;
		fb.cc[0].l = 11;
		fb.cc[1].r = 2;
		fb.cc[1].l = 5;
		fb.cc[2].r = 3;
		fb.cc[2].l = 0;
		fb.cc[3].r = 0;
		fb.cc[3].l = 0;
		fb.yuv = 0;
		fb.enabled = 1;
		fb.dirty = 1;

		initok = 1;
		reset_timer = 1;
		fps_reset = 1;
	}

	void vid_close()
	{
		if (screendc) {
			DeleteDC(screendc);
			screendc = 0;
		}
		if (screenbmp) {
			DeleteObject(screenbmp);
			screenbmp = 0;
			screenbuf = 0;
		}

		if (!initok) 
			return;

		joy_close();
		if (ddraw) {
			pDD->Release();
			pDD = NULL;
			pDDSPrimary->Release();
			pDDSPrimary = NULL;
		}
		initok = 0;
	}

	void ev_poll()
	{
		joy_poll();
	}
#if 0
	void vid_settitle(char *title)
	{
	}

	void vid_setpal(int i, int r, int g, int b)
	{
	}
#endif

	void vid_begin()
	{
		static int skip = 0;

		if (frameskip < 0) {
			if (pcm_get_enable()) {
				skip = (skip + 1) % (auto_skip > 0 ? auto_skip + 1 : 1);
				fb.enabled = skip == 0;
			}
			else fb.enabled = auto_skip == 0;
		}
		else {
			skip = (skip + 1) % (frameskip > 0 ? frameskip + 1 : 1);
			fb.enabled = skip == 0;
		}

		if (!fb.enabled)
			return;

		if (!initok) {
			fb.enabled = 0;
			return;
		}

		fps_frame_count++;

		if (ddraw) {
			HRESULT hr = pDDSPrimary->Lock(NULL, &ddsd, DDLOCK_WAITNOTBUSY, NULL);
			if(hr != DD_OK)
				return;
			screenvram = (unsigned char *) ddsd.lpSurface;
			if (!screenvram) {
				fb.enabled = 0;
				return;
			}

			fb.ptr = screenvram;
			if (screenmode) {
				if (g_fVPad)
					screenvram += 26 * ddsd.lPitch;
				else
					screenvram += 52 * ddsd.lPitch;
				fb.pitch = fb.w * 4;
			}
			else {
				screenvram += 40 * ddsd.lXPitch;
				if (g_fVPad)
					screenvram += 62 * ddsd.lPitch;
				else
					screenvram += 88 * ddsd.lPitch;
				fb.pitch = fb.w * 2;
			}
			hr = pDDSPrimary->Unlock(NULL);
			if(hr != DD_OK)
				return;
		}
		else {
			if (!screenbuf) {
				fb.enabled = 0;
				return;
			}
			screenvram = screenbuf;
			fb.ptr = screenvram;
			if (!screenmode) {
				screenvram += 40 * 2;
				screenvram += SCREENBMP_WIDTH * 36 * 2;
			}
		}
	}

	void vid_end()
	{
		if (!initok || !fb.enabled) 
			return;

		if (ddraw)
			pDDSPrimary->Unlock(NULL);
		else {
			HDC hdc = GetDC(g_hwndMain);
			if (GetSystemMetrics(SM_CXSCREEN) != 240 || GetSystemMetrics(SM_CYSCREEN) != 320) {
				// Landscape
				RECT rc;
				GetClientRect(g_hwndMain, &rc);
				rc.bottom -= MENU_HEIGHT;
				if (screenmode)
					BitBlt(hdc, ((rc.right - rc.left) - SCREENBMP_WIDTH) / 2,
					((rc.bottom - rc.top) - SCREENBMP_HEIGHT) / 2,
					SCREENBMP_WIDTH, SCREENBMP_HEIGHT, vid_get_screen_dc(), 0, 0, SRCCOPY);
				else
					BitBlt(hdc, ((rc.right - rc.left) - 160) / 2,
					((rc.bottom - rc.top) - 144) / 2,
					160, 144, vid_get_screen_dc(), 40, 36, SRCCOPY);
			}
			else {
				// Portrait
				int top = g_fVPad ? 0 : 52 - MENU_HEIGHT;
				HDC hdc = GetDC(g_hwndMain);
				if (screenmode)
					BitBlt(hdc, 0, top, SCREENBMP_WIDTH, SCREENBMP_HEIGHT, vid_get_screen_dc(), 0, 0, SRCCOPY);
				else
					BitBlt(hdc, 40, top + 36, 160, 144, vid_get_screen_dc(), 40, 36, SRCCOPY);
			}
			ReleaseDC(g_hwndMain, hdc);
		}
	}

	void vid_suspend()
	{
		int top;
		word *src, *dst, *vram;

		if (!initok) 
			return;

		if (ddraw) {
			HRESULT hr;
			hr = pDDSPrimary->Lock(NULL, &ddsd, DDLOCK_WAITNOTBUSY, NULL);
			vram = (word *) ddsd.lpSurface;
			if (vram && screenbuf) {
				top = g_fVPad ? 26 : 52;
				for (int y = 0; y < 216; y++) {
					src = vram + (top + y) * (ddsd.lPitch >> 1);
					dst = (word*)(screenbuf + 240 * y * 2);
					for (int x = 0; x < 240; x++) {
						*dst = *src;
						dst++; src += (ddsd.lXPitch >> 1);
					}
				}
			}
			pDDSPrimary->Unlock(NULL);
		}
		else if (!g_fDrawScreen && screenbuf) {
			memset(screenbuf, 0, SCREENBMP_WIDTH * SCREENBMP_HEIGHT * 2);
			InvalidateRect(g_hwndMain, NULL, TRUE);
			UpdateWindow(g_hwndMain);
		}
	}

	void vid_resume()
	{
		if (!initok) {
			vid_init();
			return;
		}

		if (screenchanged) {
			if (screenbuf) memset(screenbuf, 0, SCREENBMP_WIDTH * SCREENBMP_HEIGHT * 2);
			screenchanged = 0;

			InvalidateRect(g_hwndMain, NULL, TRUE);
			UpdateWindow(g_hwndMain);
		}

		reset_timer = 1;
		fps_reset = 1;
	}

	int vid_get_thread_time()
	{
		FILETIME ftCreate, ftExit, ftKernel, ftUser;
		GetThreadTimes(GetCurrentThread(), &ftCreate, &ftExit, &ftKernel, &ftUser);
		return (int)((((LARGE_INTEGER*)&ftKernel)->QuadPart / 10000) + (((LARGE_INTEGER*)&ftUser)->QuadPart) / 10000);
	}

	void vid_start_frame()
	{
		if (!initok) return;

		fps_count++;
		if (g_fShowFPS) {
			if (fps_reset) {
				fps_reset = 0;
				fps_count = 0;
				fps_frame_count = 0;
				fps_time = timer_time_in_msec();
			}
			else if (fps_count > 120) {
				int cur = timer_time_in_msec();
				int fps = (int)((double)fps_frame_count / (cur - fps_time) * 100000);

				fps_count = 0;
				fps_frame_count = 0;
				fps_time = cur;

				PostMessage(g_hwndMain, WM_UPDATEFPS, fps, 0);
			}
		}

		if (pcm_get_enable()) {
			if (frameskip < 0) {
				if (g_fThrottling) {
					if (reset_timer) {
						reset_timer = 0;
						auto_skip = 0;
						auto_start_time = timer_time_in_msec() * 1000;
						fb.frames = 0;
					}
					else if (fb.frames > auto_skip) {
#if 1
						_int64 cur = timer_time_in_msec() * 1000;
						_int64 buffering = pcm_get_buffering_time();
						_int64 wait = pcm_get_last_wait_time();
						_int64 time_per_frame = (cur - auto_start_time - wait) / fb.frames;

						if (buffering < time_per_frame * 2)
							auto_skip = min(auto_skip + 1, MAX_FRAME_SKIP);
						else if (buffering > pcm_get_buffer_count() * time_per_frame / 2)
							auto_skip = max(auto_skip - 1, 0);

						auto_start_time = cur;
#else
						_int64 buffering = pcm_get_buffering_time();

						if (buffering < FRAME_PERIOD * 2)
							auto_skip = min(auto_skip + 1, MAX_FRAME_SKIP);
						else if (buffering > pcm_get_buffer_count() * FRAME_PERIOD / 2)
							auto_skip = max(auto_skip - 1, 0);
#endif
						fb.frames = 0;
					}
				}
				else auto_skip = 0;
			}
		}
		else {
			if (reset_timer) {
				fb.frames = 0;
				reset_timer = 0;
				auto_skip_count = 0;
				auto_start_time = timer_time_in_msec() * 1000;
			}
			else if (fb.frames) {
				_int64 cur = timer_time_in_msec() * 1000;
				if (cur - auto_start_time < (_int64)FRAME_PERIOD * fb.frames) {
					auto_skip = 0;
					auto_skip_count = 0;
				}
				else auto_skip = 1;
				if (auto_skip && ++auto_skip_count > MAX_FRAME_SKIP)
					reset_timer = 1;

				if (g_fThrottling) {
					if (cur - auto_start_time < (_int64)FRAME_PERIOD * fb.frames) {
						UINT sleep = (UINT)((_int64)FRAME_PERIOD * fb.frames - (cur - auto_start_time)) / 1000;
						Sleep(sleep);
					}
				}
			}
		}
	}

	void vid_end_frame()
	{
	}

	int vid_get_frameskip()
	{
		return frameskip;
	}

	void vid_set_frameskip(int skip)
	{
		frameskip = skip;
	}

	int vid_get_screenmode()
	{
		return screenmode;
	}

	void vid_set_screenmode(int mode)
	{
		screenmode = mode;
		screenchanged = 1;
	}

	int vid_get_ddraw()
	{
		return ddraw;
	}

	void vid_set_ddraw(int mode)
	{
		vid_close();
		ddraw = mode;
	}

	HDC vid_get_screen_dc()
	{
		return screendc;
	}

} // extern "C"
