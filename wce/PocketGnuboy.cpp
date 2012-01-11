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

#include "PocketGnuboy.h"
#include "MainWnd.h"
#include "wce.h"

// Global Variables:
HINSTANCE                       g_hInst;
HWND                            g_hwndMain;
HWND                            g_hwndCB;
HACCEL                          g_hAccelTable;
SHACTIVATEINFO          s_sai;

HDC                                     g_hdcVPad = NULL;
HBITMAP                         g_hbmpVPad = NULL;
BOOL                            g_fVPad = FALSE;
BOOL                            g_fDrawScreen = TRUE;
BOOL                            g_fGSGetFile = TRUE;
BOOL                            g_fThrottling = TRUE;
BOOL                            g_fShowFPS = FALSE;
TCHAR                           g_szFPS[64] = {0};
SIZE                            g_sizeFPS = {0};
TCHAR                           g_szLastDir[MAX_PATH] = {0};

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE, LPTSTR);
BOOL InitInstance(HINSTANCE, LPTSTR, int);
LRESULT CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(     HINSTANCE hInstance,
                                        HINSTANCE hPrevInstance,
                                        LPTSTR    lpCmdLine,
                                        int       nCmdShow)
{
#if defined(_WIN32_WCE) && 0
        LANGID lid = GetSystemDefaultLangID();
        if (PRIMARYLANGID(lid) == LANG_JAPANESE) {
                MessageBox(NULL,
                        _T("Sorry!\nJapanese WindowsCE is not supported."), NULL, MB_OK | MB_ICONSTOP);
                return FALSE;
        }
#endif

        if (!InitInstance(hInstance, lpCmdLine, nCmdShow))
                return FALSE;

        g_hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_POCKETGNUBOY);
        return MainWnd_MessageLoop();
}

ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
        WNDCLASS        wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = szWindowClass;

        return RegisterClass(&wc);
}

BOOL InitInstance(HINSTANCE hInstance, LPTSTR pszCmdline, int nCmdShow)
{
        TCHAR szTitle[MAX_LOADSTRING];
        TCHAR szWindowClass[MAX_LOADSTRING];

        g_hInst = hInstance;
        LoadString(hInstance, IDC_POCKETGNUBOY, szWindowClass, MAX_LOADSTRING);
        LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

        g_hwndMain = FindWindow(szWindowClass, szTitle);
        if (g_hwndMain) {
                SetForegroundWindow((HWND)(((DWORD)g_hwndMain) | 0x01));
                return 0;
        }

        MyRegisterClass(hInstance, szWindowClass);

        g_hwndMain = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
        if (!g_hwndMain)
                return FALSE;

        //When the main window is created using CW_USEDEFAULT the height of the menubar (if one
        // is created is not taken into account). So we resize the window after creating it
        // if a menubar is present
        {
                RECT rc;
                GetWindowRect(g_hwndMain, &rc);
                rc.bottom -= MENU_HEIGHT;
                if (g_hwndCB)
                        MoveWindow(g_hwndMain, rc.left, rc.top, rc.right, rc.bottom, FALSE);
        }

        if (LoadCmdLineROM(pszCmdline))
                Resume();

        ShowWindow(g_hwndMain, nCmdShow);
        UpdateWindow(g_hwndMain);

        return TRUE;
}

HWND CreateRpCommandBar(HWND hwnd)
{
        SHMENUBARINFO mbi;
        OSVERSIONINFO ovi = {0};

        ovi.dwOSVersionInfoSize = sizeof(ovi);
        GetVersionEx(&ovi);

        memset(&mbi, 0, sizeof(SHMENUBARINFO));
        mbi.cbSize     = sizeof(SHMENUBARINFO);
        mbi.hwndParent = hwnd;
        mbi.nToolBarId = IDM_MENU;
        mbi.hInstRes   = g_hInst;

        if (ovi.dwMajorVersion < 5) {
                mbi.nBmpId     = IDR_TOOLBAR;
                mbi.cBmpImages = 2;
        }

        if (!SHCreateMenuBar(&mbi))
                return NULL;

        TBBUTTON tbb[] = {
                {0, IDM_FILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
                {1, IDM_CTRL_AUDIO_ENABLE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
                {0, 0, 0, TBSTYLE_SEP, 0, 0},
                {3, IDM_FILE_STATE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
                {4, IDM_FILE_STATE_LOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0}
        };

        if (ovi.dwMajorVersion < 5)
                CommandBar_AddButtons(mbi.hwndMB, sizeof(tbb) / sizeof(TBBUTTON), tbb);

        return mbi.hwndMB;
}

void UpdateToolbar()
{
        SendMessage(g_hwndCB, TB_CHANGEBITMAP, IDM_CTRL_AUDIO_ENABLE, MAKELONG(pcm_get_enable() ? 1 : 2, 0));
        SendMessage(g_hwndCB, TB_CHECKBUTTON, IDM_CTRL_AUDIO_ENABLE, MAKELONG(!pcm_get_enable(), 0));

        int enable = get_romfile() ? 1 : 0;
        SendMessage(g_hwndCB, TB_ENABLEBUTTON, IDM_FILE_STATE_LOAD, MAKELONG(enable, 0));
        SendMessage(g_hwndCB, TB_ENABLEBUTTON, IDM_FILE_STATE_SAVE, MAKELONG(enable, 0));
}

// Mesage handler for the About box.
LRESULT CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
        SHINITDLGINFO shidi;

        switch (message)
        {
                case WM_INITDIALOG:
                {
                        // Create a Done button and size it.
                        shidi.dwMask = SHIDIM_FLAGS;
                        shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
                        shidi.hDlg = hDlg;
                        SHInitDialog(&shidi);

                        SetDlgItemText(hDlg, IDC_STATIC_DATE, _T(__DATE__));
                        return TRUE;
                }
                case WM_COMMAND:
                        if (LOWORD(wParam) == IDOK) {
                                EndDialog(hDlg, LOWORD(wParam));
                                return TRUE;
                        }
                        break;
        }
    return FALSE;
}

BOOL LoadCmdLineROM(LPTSTR szFile)
{
        if (!_tcslen(szFile)) return FALSE;

        // are there quotes?
        if (szFile[0] == _T('"')) {
                szFile++;
                if (szFile[_tcslen(szFile) - 1] == _T('"')) {
                        szFile[_tcslen(szFile) - 1] = _T('\0');
                }
        }

        if (!_tcslen(szFile)) return FALSE;

        return LoadROM(szFile);
}

BOOL LoadROM(LPTSTR szFile)
{
        HCURSOR hCur;
        char name[MAX_PATH] = {0};

        WideCharToMultiByte(CP_ACP, NULL, szFile, -1, name, MAX_PATH, NULL, NULL);

        emu_close();

        hCur = SetCursor(LoadCursor(NULL, IDC_WAIT));

        if (!loader_init(name)) {
                SetCursor(hCur);
                return FALSE;
        }
        emu_init();

        SetCursor(hCur);
        return TRUE;
}

void HideSipButton(BOOL bHide)
{
    DWORD dwState;
    dwState = bHide ? SHFS_HIDESIPBUTTON : SHFS_SHOWSIPBUTTON;
	SetForegroundWindow(g_hwndMain);
	SHFullScreen(g_hwndMain, dwState);            
}

void Pause(HWND hwnd)
{
        if (get_romfile()) {
                emu_pause();
                ShowWindow(g_hwndCB, SW_SHOW);
				HideSipButton(false);
                g_szFPS[0] = NULL;

                /*RECT rc;
                GetClientRect(hwnd, &rc);
                if (g_fVPad)
                        rc.top = SCREENBMP_HEIGHT;
                else
                        rc.top = SCREENBMP_HEIGHT + 26;
                InvalidateRect(hwnd, &rc, TRUE);
                UpdateWindow(hwnd);*/

                InvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
        }
}

void Resume(HWND hwnd)
{
        if (get_romfile()) {
                emu_resume();
                if (g_fVPad)
                        ShowWindow(g_hwndCB, SW_HIDE);
				HideSipButton(true);
                /*RECT rc;
                GetClientRect(hwnd, &rc);
                if (g_fVPad)
                        rc.top = SCREENBMP_HEIGHT;
                else
                        rc.top = SCREENBMP_HEIGHT + 26;
                InvalidateRect(hwnd, &rc, TRUE);
                UpdateWindow(hwnd);*/

                InvalidateRect(hwnd, NULL, TRUE);
                UpdateWindow(hwnd);
        }
}
