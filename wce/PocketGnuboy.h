/*
 *  PocketGnuboy - GameBoy emulator for PocketPC
 *  Copyright (C) 2003  Y.Nagamidori
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef __POCKETGNUBOY_H__
#define __POCKETGNUBOY_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#define WIN32_LEAN_AND_MEAN
#define MAX_LOADSTRING 100
#define MENU_HEIGHT 26
#define VPAD_LEFT       0
#define VPAD_TOP        216
#define VPAD_WIDTH      240
#define VPAD_HEIGHT     80
#define WM_UPDATEFPS (WM_USER + 1000)

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <aygshell.h>
#include <sipapi.h>

extern HINSTANCE g_hInst;
extern HWND g_hwndMain;
extern HWND g_hwndCB;
extern HACCEL g_hAccelTable;
extern SHACTIVATEINFO s_sai;
extern HDC g_hdcVPad;
extern HBITMAP g_hbmpVPad;
extern BOOL g_fVPad;
extern BOOL g_fDrawScreen;
extern BOOL g_fGSGetFile;
extern BOOL g_fThrottling;
extern BOOL g_fShowFPS;
extern TCHAR g_szFPS[64];
extern SIZE g_sizeFPS;
extern TCHAR g_szLastDir[MAX_PATH];

HWND CreateRpCommandBar(HWND);
void UpdateToolbar();
LRESULT CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL LoadCmdLineROM(LPTSTR szFile);
BOOL LoadROM(LPTSTR szFile);
void Pause(HWND hwnd = g_hwndMain);
void Resume(HWND hwnd = g_hwndMain);

#endif // __POCKETGNUBOY_H__
