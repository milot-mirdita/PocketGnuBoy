#include "PocketGnuboy.h"
#include "MainWnd.h"
#include "Registory.h"
#include "wce.h"

#include <regext.h>
#include <snapi.h>


BOOL    g_fVPadPress = FALSE;

int MainWnd_MessageLoop()
{
        MSG msg;
        for (;;) {
                if (get_romfile() && emu_get_run()) {
                        emu_do_frame();
                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                                if (msg.message == WM_QUIT)
                                        return msg.wParam;
                                if (!TranslateAccelerator(msg.hwnd, g_hAccelTable, &msg)) {
                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                }
                        }
                }
                else {
                        if (GetMessage(&msg, NULL, 0, 0)) {
                                if (!TranslateAccelerator(msg.hwnd, g_hAccelTable, &msg)) {
                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                }
                        }
                        else
                                return msg.wParam;
                }
        }
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        switch (message) {
                case WM_CREATE:
                        return HANDLE_WM_CREATE(hWnd, wParam, lParam, MainWnd_OnCreate);
                case WM_ACTIVATE:
                        HANDLE_WM_ACTIVATE(hWnd, wParam, lParam, MainWnd_OnActivate);
                        break;
                case WM_COMMAND:
                        HANDLE_WM_COMMAND(hWnd, wParam, lParam, MainWnd_OnCommand);
                        break;
                case WM_PAINT:
                        HANDLE_WM_PAINT(hWnd, wParam, lParam, MainWnd_OnPaint);
                        break;
                case WM_CLOSE:
                        HANDLE_WM_CLOSE(hWnd, wParam, lParam, MainWnd_OnClose);
                        break;
                case WM_DESTROY:
                        HANDLE_WM_DESTROY(hWnd, wParam, lParam, MainWnd_OnDestroy);
                        break;
                case WM_LBUTTONDOWN:
                        HANDLE_WM_LBUTTONDOWN(hWnd, wParam, lParam, MainWnd_OnLButtonDown);
                        break;
                case WM_LBUTTONUP:
                        HANDLE_WM_LBUTTONUP(hWnd, wParam, lParam, MainWnd_OnLButtonUp);
                        break;
                case WM_MOUSEMOVE:
                        HANDLE_WM_MOUSEMOVE(hWnd, wParam, lParam, MainWnd_OnMouseMove);
                        break;
                case WM_INITMENUPOPUP:
                        HANDLE_WM_INITMENUPOPUP(hWnd, wParam, lParam, MainWnd_OnInitMenuPopup);
                        break;
                case WM_ENTERMENULOOP:
                        MainWnd_OnEnterMenuLoop(hWnd);
                        break;
                case WM_SETTINGCHANGE:
                        SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
                break;
                case WM_UPDATEFPS:
                        MainWnd_OnUpdateFPS(hWnd, wParam, lParam);
                        break;
                default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}
HREGNOTIFY g_hNotify = NULL;

// Callback function that is called when the bit indicating a call is being made changes
void PhoneCallIncomingCallback(HREGNOTIFY hNotify, DWORD dwUserData, const PBYTE pData, const UINT cbData)
{
	DWORD PhoneCallIncomingStatus = *((DWORD *)pData);

	// only look at the value of the bit we are interested in
	if (PhoneCallIncomingStatus & SN_PHONEINCOMINGCALL_BITMASK)
	{
		emu_pause();
	}
}

HRESULT RegisterPhoneIncomingCallback()
{
  HRESULT hr;
  NOTIFICATIONCONDITION nc;

  // Register the phone call calling notification.
  // By setting dwMask = SN_PHONECALLCALLING_BITMASK, we will get a notification only when there is a 
  // change in the bit indicating whether the phone is currently attempting to connect an outgoing call.

  nc.ctComparisonType = REG_CT_ANYCHANGE;
  nc.dwMask           = SN_PHONEINCOMINGCALL_BITMASK;
  nc.TargetValue.dw   = 0;

  hr = RegistryNotifyCallback(
		SN_PHONEINCOMINGCALL_ROOT,     // registry root to monitor
		SN_PHONEINCOMINGCALL_PATH,     // registry path to monitor
		SN_PHONEINCOMINGCALL_VALUE,    // registry value to monitor
		PhoneCallIncomingCallback,     // callback to be called when bit changes
                0,       
                &nc,
                &g_hNotify
                );

  return hr;
}

BOOL MainWnd_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
        emu_close();
        LoadRegistory();

        g_hwndCB = CreateRpCommandBar(hwnd);
        if (!g_hwndCB)
                return FALSE;

        HDC hDC = GetDC(hwnd);
        g_hdcVPad = CreateCompatibleDC(hDC);
        ReleaseDC(hwnd, hDC);

        if (!g_hdcVPad)
                return FALSE;

        g_hbmpVPad = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_VPAD));
        SelectObject(g_hdcVPad, g_hbmpVPad);

        if (!g_hbmpVPad)
                return FALSE;

        UpdateToolbar();

		RegisterPhoneIncomingCallback();

        return TRUE;
}

void MainWnd_OnPaint(HWND hwnd)
{
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hwnd, &ps);

        if (get_romfile()) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                if (emu_get_run()) {
                        if (g_fVPad) {
                                BitBlt(hDC, VPAD_LEFT, VPAD_TOP, VPAD_WIDTH, VPAD_HEIGHT, g_hdcVPad, 0, 0, SRCCOPY);

                                if (g_fShowFPS && _tcslen(g_szFPS)) {
                                        rc.left = rc.right - g_sizeFPS.cx;
                                        rc.top = rc.bottom - VPAD_HEIGHT;
                                        rc.bottom = rc.top + g_sizeFPS.cy;

                                        int oldmode = SetBkMode(hDC, TRANSPARENT);
                                        COLORREF oldbk = SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
                                        DrawText(hDC, g_szFPS, -1, &rc, DT_BOTTOM | DT_RIGHT | DT_NOPREFIX | DT_SINGLELINE);
                                        SetTextColor(hDC, oldbk);
                                        SetBkMode(hDC, oldmode);
                                }
                        }
                        else if (g_fShowFPS && _tcslen(g_szFPS)) {
                                rc.bottom -= MENU_HEIGHT;
                                rc.left = rc.right - g_sizeFPS.cx;
                                rc.top = rc.bottom - g_sizeFPS.cy;

                                int oldmode = SetBkMode(hDC, TRANSPARENT);
                                COLORREF oldbk = SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
                                DrawText(hDC, g_szFPS, -1, &rc, DT_BOTTOM | DT_RIGHT | DT_NOPREFIX | DT_SINGLELINE);
                                SetTextColor(hDC, oldbk);
                                SetBkMode(hDC, oldmode);
                        }
                }
                else {
                        rc.bottom -= MENU_HEIGHT;

                        if (g_fDrawScreen && vid_get_screen_dc()) {

                                if (GetSystemMetrics(SM_CXSCREEN) != 240 || GetSystemMetrics(SM_CYSCREEN) != 320) {
                                        if (vid_get_screenmode())
                                                BitBlt(hDC, ((rc.right - rc.left) - SCREENBMP_WIDTH) / 2,
                                                                        ((rc.bottom - rc.top) - SCREENBMP_HEIGHT) / 2,
                                                                        SCREENBMP_WIDTH, SCREENBMP_HEIGHT, vid_get_screen_dc(), 0, 0, SRCCOPY);
                                        else
                                                BitBlt(hDC, ((rc.right - rc.left) - 160) / 2,
                                                                        ((rc.bottom - rc.top) - 144) / 2,
                                                                        160, 144, vid_get_screen_dc(), 40, 36, SRCCOPY);
                                }
                                else {
                                        int top = g_fVPad ? 0 : 52 - MENU_HEIGHT;
                                        BitBlt(hDC, 0, top, SCREENBMP_WIDTH, SCREENBMP_HEIGHT, vid_get_screen_dc(), 0, 0, SRCCOPY);
                                }
                        }

                        TCHAR szMessage[MAX_LOADSTRING];
                        LoadString(g_hInst, IDS_MSG_PAUSE, szMessage, MAX_LOADSTRING);
                        //int oldmode = SetBkMode(hDC, TRANSPARENT);
                        COLORREF oldTx = SetTextColor(hDC, RGB(0xff, 0xff, 0xff));
                        COLORREF oldBk = SetBkColor(hDC, RGB(0, 0, 0));

						DrawText(hDC, szMessage, -1, &rc, DT_BOTTOM | DT_CENTER | DT_NOPREFIX | DT_SINGLELINE);

                        SetBkColor(hDC, oldBk);
                        SetTextColor(hDC, oldTx);
                        //SetBkMode(hDC, oldmode);
                }
        }

        EndPaint(hwnd, &ps);
}

void MainWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
        switch (id) {
        case IDM_HELP_ABOUT:
                DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hwnd, (DLGPROC)AboutDlgProc);
                break;
        case IDOK:
                SendMessage(hwnd, WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, 0), (LPARAM)hwnd);
                SendMessage(hwnd, WM_CLOSE, 0, 0);
                break;
        case IDM_FILE_OPEN:
                OnFileOpen(hwnd);
                break;
        case IDM_FILE_CLOSE:
                OnFileClose(hwnd);
                break;
        case IDM_FILE_RESUME:
                OnFileResume(hwnd);
                break;
        case IDM_FILE_RESET:
                OnFileReset(hwnd);
                break;
        case IDM_FILE_STATE0:
        case IDM_FILE_STATE1:
        case IDM_FILE_STATE2:
        case IDM_FILE_STATE3:
        case IDM_FILE_STATE4:
        case IDM_FILE_STATE5:
        case IDM_FILE_STATE6:
        case IDM_FILE_STATE7:
        case IDM_FILE_STATE8:
        case IDM_FILE_STATE9:
                OnFileState(hwnd, id - IDM_FILE_STATE0);
                break;
        case IDM_FILE_STATE_LOAD:
                OnFileStateLoad(hwnd);
                break;
        case IDM_FILE_STATE_SAVE:
                OnFileStateSave(hwnd);
                break;
        case IDM_FILE_EXIT:
                OnFileExit(hwnd);
                break;
        case IDM_CTRL_SCREEN_160_144:
        case IDM_CTRL_SCREEN_240_216:
        case IDM_CTRL_SCREEN_240_216H:
        case IDM_CTRL_SCREEN_240_216A:
                OnCtrlScreen(hwnd, id - IDM_CTRL_SCREEN_160_144);
                break;
        case IDM_CTRL_FRAMESKIP_AUTO:
        case IDM_CTRL_FRAMESKIP0:
        case IDM_CTRL_FRAMESKIP1:
        case IDM_CTRL_FRAMESKIP2:
        case IDM_CTRL_FRAMESKIP3:
        case IDM_CTRL_FRAMESKIP4:
        case IDM_CTRL_FRAMESKIP5:
                OnCtrlFrameSkip(hwnd, id - IDM_CTRL_FRAMESKIP0);
                break;
        case IDM_CTRL_AUDIO_ENABLE:
                OnCtrlAudioEnable(hwnd);
                break;
        case IDM_CTRL_AUDIO_11K:
        case IDM_CTRL_AUDIO_22K:
        case IDM_CTRL_AUDIO_44K:
                OnCtrlAudioSampleRate(hwnd, id - IDM_CTRL_AUDIO_11K);
                break;
        case IDM_CTRL_AUDIO_MONO:
        case IDM_CTRL_AUDIO_STEREO:
                OnCtrlAudioStereo(hwnd, id == IDM_CTRL_AUDIO_STEREO);
                break;
        case IDM_CTRL_AUDIO_8BITS:
        case IDM_CTRL_AUDIO_16BITS:
                OnCtrlAudioBits(hwnd, id == IDM_CTRL_AUDIO_16BITS);
                break;
        case IDM_CTRL_CONTROLLERS:
                OnCtrlControllers(hwnd);
                break;
        case IDM_CTRL_PREFERENCES:
                OnCtrlPreferences(hwnd);
                break;
        case IDM_CTRL_TURBOA:
                OnCtrlTurboA(hwnd);
                break;
        case IDM_CTRL_TURBOB:
                OnCtrlTurboB(hwnd);
                break;
        case IDM_CTRL_VPAD:
                OnCtrlVPad(hwnd);
                break;
        case IDM_CTRL_AUDIO_BUF4:
        case IDM_CTRL_AUDIO_BUF8:
        case IDM_CTRL_AUDIO_BUF12:
        case IDM_CTRL_AUDIO_BUF16:
        case IDM_CTRL_AUDIO_BUF24:
        case IDM_CTRL_AUDIO_BUF32:
                OnCtrlAudioBuffers(hwnd, id - IDM_CTRL_AUDIO_BUF4);
                break;
        }
}

void MainWnd_OnClose(HWND hwnd)
{
        emu_close();
        SaveRegistory();

        DeleteDC(g_hdcVPad);
        DeleteObject(g_hbmpVPad);

		if (g_hNotify)
				RegistryCloseNotification(g_hNotify);

        DestroyWindow(hwnd);
}

void MainWnd_OnDestroy(HWND hwnd)
{
        CommandBar_Destroy(g_hwndCB);
        PostQuitMessage(0);
}

void MainWnd_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
        if (get_romfile()) {
                if (!MainWnd_UpdateVPadState(x, y)) {
                        if (emu_get_run())
                                Pause(hwnd);
                        else
                                Resume(hwnd);
                }
        }
}

void MainWnd_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
        if (get_romfile()) {
                if (g_fVPad)
                        joy_set_vpadstate(VPAD_STATE_NONE);
        }
        g_fVPadPress = FALSE;
}

void MainWnd_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
        if (get_romfile()) {
                if (g_fVPadPress)
                        MainWnd_UpdateVPadState(x, y);
        }
}

void MainWnd_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu)
{
        int nCheck;
        UINT uEnable;

        if (get_romfile() && emu_get_run())
                Pause(hwnd);

        uEnable = get_romfile() ? MF_BYCOMMAND | MF_ENABLED : MF_BYCOMMAND | MF_GRAYED;
        EnableMenuItem(hMenu, IDM_FILE_CLOSE, uEnable);
        EnableMenuItem(hMenu, IDM_FILE_RESUME, uEnable);
        EnableMenuItem(hMenu, IDM_FILE_RESET, uEnable);
        EnableMenuItem(hMenu, IDM_FILE_STATE_LOAD, uEnable);
        EnableMenuItem(hMenu, IDM_FILE_STATE_SAVE, uEnable);

        CheckMenuRadioItem(hMenu, IDM_FILE_STATE0, IDM_FILE_STATE9,
                                           IDM_FILE_STATE0 + get_saveslot(), MF_BYCOMMAND);

        CheckMenuRadioItem(hMenu, IDM_CTRL_SCREEN_160_144, IDM_CTRL_SCREEN_240_216A,
                                           IDM_CTRL_SCREEN_160_144 + vid_get_screenmode(), MF_BYCOMMAND);

        CheckMenuRadioItem(hMenu, IDM_CTRL_FRAMESKIP_AUTO, IDM_CTRL_FRAMESKIP5,
                                           IDM_CTRL_FRAMESKIP0 + vid_get_frameskip(), MF_BYCOMMAND);

        CheckMenuItem(hMenu, IDM_CTRL_AUDIO_ENABLE, pcm_get_enable() ?
                                                MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);

        switch (pcm_get_samplerate()) {
        case 22050:
                nCheck = IDM_CTRL_AUDIO_22K; break;
        case 44100:
                nCheck = IDM_CTRL_AUDIO_44K; break;
        default:
                nCheck = IDM_CTRL_AUDIO_11K; break;
        }
        CheckMenuRadioItem(hMenu, IDM_CTRL_AUDIO_11K, IDM_CTRL_AUDIO_44K, nCheck, MF_BYCOMMAND);
        CheckMenuRadioItem(hMenu, IDM_CTRL_AUDIO_MONO, IDM_CTRL_AUDIO_STEREO,
                                pcm_get_stereo() ? IDM_CTRL_AUDIO_STEREO : IDM_CTRL_AUDIO_MONO, MF_BYCOMMAND);
        CheckMenuRadioItem(hMenu, IDM_CTRL_AUDIO_8BITS, IDM_CTRL_AUDIO_16BITS,
                                pcm_get_16bits() ? IDM_CTRL_AUDIO_16BITS : IDM_CTRL_AUDIO_8BITS, MF_BYCOMMAND);

        CheckMenuItem(hMenu, IDM_CTRL_TURBOA, joy_get_turbo_a() ?
                                                MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_CTRL_TURBOB, joy_get_turbo_b() ?
                                                MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_CTRL_VPAD, g_fVPad ?
                                                MF_BYCOMMAND | MF_CHECKED : MF_BYCOMMAND | MF_UNCHECKED);

        switch (pcm_get_buffer_count()) {
        case 4:
                nCheck = IDM_CTRL_AUDIO_BUF4; break;
        case 8:
                nCheck = IDM_CTRL_AUDIO_BUF8; break;
        case 16:
                nCheck = IDM_CTRL_AUDIO_BUF16; break;
        case 24:
                nCheck = IDM_CTRL_AUDIO_BUF24; break;
        case 32:
                nCheck = IDM_CTRL_AUDIO_BUF32; break;
        default:
        nCheck = IDM_CTRL_AUDIO_BUF12; break;
        }
        CheckMenuRadioItem(hMenu, IDM_CTRL_AUDIO_BUF4, IDM_CTRL_AUDIO_BUF32, nCheck, MF_BYCOMMAND);
}

void MainWnd_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized)
{
        if (state == WA_INACTIVE) {
                if (get_romfile() && emu_get_run())
                        Pause(hwnd);
        }
}

void MainWnd_OnEnterMenuLoop(HWND hwnd)
{
        if (get_romfile() && emu_get_run())
                Pause(hwnd);
}

BOOL MainWnd_UpdateVPadState(int x, int y)
{
#define VPAD_CURSOR_LEFT        17
#define VPAD_CURSOR_TOP         5
#define VPAD_CURSOR_WIDTH       26
#define VPAD_CURSOR_HEIGHT      26
#define VPAD_A_LEFT                     189
#define VPAD_A_TOP                      17
#define VPAD_A_WIDTH            36
#define VPAD_A_HEIGHT           36
#define VPAD_B_LEFT                     137
#define VPAD_B_TOP                      34
#define VPAD_B_WIDTH            36
#define VPAD_B_HEIGHT           36
#define VPAD_AB_LEFT            171
#define VPAD_AB_TOP                     26
#define VPAD_AB_WIDTH           18
#define VPAD_AB_HEIGHT          36
        if (!g_fVPad || !emu_get_run())
                return FALSE;

        if (x < VPAD_LEFT || x > VPAD_LEFT + VPAD_WIDTH ||
                y < ( VPAD_TOP - 5 ) || y > VPAD_TOP + VPAD_HEIGHT)
                return FALSE;

        x -= VPAD_LEFT;
        y -= VPAD_TOP;

        if ((x >= VPAD_CURSOR_LEFT && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH) &&
                (y >= VPAD_CURSOR_TOP && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT)) {
                joy_set_vpadstate(VPAD_STATE_UPLEFT);
        }
        else if ((x >= VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 2) &&
                (y >= VPAD_CURSOR_TOP && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT)) {
                joy_set_vpadstate(VPAD_STATE_UP);
        }
        else if ((x >= VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 2 && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 3) &&
                (y >= VPAD_CURSOR_TOP && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT)) {
                joy_set_vpadstate(VPAD_STATE_UPRIGHT);
        }
        else if ((x >= VPAD_CURSOR_LEFT && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH) &&
                (y >= VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 2)) {
                joy_set_vpadstate(VPAD_STATE_LEFT);
        }
        else if ((x >= VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 2) &&
                (y >= VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 2)) {
                joy_set_vpadstate(VPAD_STATE_CENTER);
        }
        else if ((x >= VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 2 && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 3) &&
                (y >= VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 2)) {
                joy_set_vpadstate(VPAD_STATE_RIGHT);
        }
        else if ((x >= VPAD_CURSOR_LEFT && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH) &&
                (y >= VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 2 && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 3)) {
                joy_set_vpadstate(VPAD_STATE_DOWNLEFT);
        }
        else if ((x >= VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 2) &&
                (y >= VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 2 && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 3)) {
                joy_set_vpadstate(VPAD_STATE_DOWN);
        }
        else if ((x >= VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 2 && x < VPAD_CURSOR_LEFT + VPAD_CURSOR_WIDTH * 3) &&
                (y >= VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 2 && y < VPAD_CURSOR_TOP + VPAD_CURSOR_HEIGHT * 3)) {
                joy_set_vpadstate(VPAD_STATE_DOWNRIGHT);
        }
        else if ((x >= VPAD_A_LEFT && x < VPAD_A_LEFT + VPAD_A_WIDTH) &&
                (y >= VPAD_A_TOP && y < VPAD_A_TOP + VPAD_A_HEIGHT)) {
                joy_set_vpadstate(VPAD_STATE_A);
        }
        else if ((x >= VPAD_B_LEFT && x < VPAD_B_LEFT + VPAD_B_WIDTH) &&
                (y >= VPAD_B_TOP && y < VPAD_B_TOP + VPAD_B_HEIGHT)) {
                joy_set_vpadstate(VPAD_STATE_B);
        }
        else if ((x >= VPAD_AB_LEFT && x < VPAD_AB_LEFT + VPAD_AB_WIDTH) &&
                (y >= VPAD_AB_TOP && y < VPAD_AB_TOP + VPAD_AB_HEIGHT)) {
                joy_set_vpadstate(VPAD_STATE_AB);
        }
        else
                joy_set_vpadstate(VPAD_STATE_NONE);

        g_fVPadPress = TRUE;
        return TRUE;
}

void MainWnd_OnUpdateFPS(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
        SIZE old = g_sizeFPS;

        HDC hDC = GetDC(hwnd);
        wsprintf(g_szFPS, _T("%d.%02d FPS"), (int)wParam / 100, (int)wParam % 100);
        GetTextExtentPoint32(hDC, g_szFPS, _tcslen(g_szFPS), &g_sizeFPS);
        ReleaseDC(hwnd, hDC);

        RECT rc;
        GetClientRect(hwnd, &rc);
        if (g_fVPad) {
                rc.left = rc.right - max(g_sizeFPS.cx, old.cx);
                rc.top = rc.bottom - VPAD_HEIGHT;
                rc.bottom = rc.top + max(g_sizeFPS.cy, old.cy);
                InvalidateRect(hwnd, &rc, FALSE);
        }
        else {
                rc.bottom -= MENU_HEIGHT;
                rc.left = rc.right - max(g_sizeFPS.cx, old.cx);
                rc.top = rc.bottom - max(g_sizeFPS.cy, old.cy);
                InvalidateRect(hwnd, &rc, TRUE);
        }
        UpdateWindow(hwnd);
}
