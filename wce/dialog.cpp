#include "PocketGnuboy.h"
#include "Dialog.h"
#include "wce.h"

#define REG_KEY_GNUBOYROM       _T("GnuboyROM")
#define REG_KEY_GB                      _T(".gb")
#define REG_KEY_GBC                     _T(".gbc")
#define IsDlgButtonChecked(hwnd, id) SendMessage(GetDlgItem(hwnd, id), BM_GETCHECK, 0, 0)
#define CheckDlgButton(hwnd, id, check) SendMessage(GetDlgItem(hwnd, id), BM_SETCHECK, check, 0)

void AssociateGBExtension()
{
        TCHAR sz[MAX_PATH];
        TCHAR szModule[MAX_PATH];
        TCHAR szType[MAX_LOADSTRING];
        int nIconID = IDI_MAIN;

        GetModuleFileName(NULL, szModule, MAX_PATH);
        LoadString(g_hInst, IDS_ROM_TYPENAME, szType, MAX_LOADSTRING);

        HKEY hKey, hKeySub;
        DWORD dwDisposition;
        if (RegCreateKeyEx(HKEY_CLASSES_ROOT, REG_KEY_GNUBOYROM, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {

                // type
                RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szType, sizeof(TCHAR) * (_tcslen(szType) + 1));

                // DefaultIcon
                if (RegCreateKeyEx(hKey, _T("DefaultIcon"), 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeySub, &dwDisposition) == ERROR_SUCCESS) {
                        wsprintf(sz, _T("%s,-%d"), szModule, nIconID);
                        RegSetValueEx(hKeySub, NULL, 0, REG_SZ, (LPBYTE)sz, sizeof(TCHAR) * (_tcslen(sz) + 1));
                        RegCloseKey(hKeySub);
                }

                // Shell-Open-Command
                if (RegCreateKeyEx(hKey, _T("Shell\\Open\\Command"), 0, NULL,
                        REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeySub, &dwDisposition) == ERROR_SUCCESS) {
                        wsprintf(sz, _T("\"%s\" \"%%1\""), szModule);
                        RegSetValueEx(hKeySub, NULL, 0, REG_SZ, (LPBYTE)sz, sizeof(TCHAR) * (_tcslen(sz) + 1));
                        RegCloseKey(hKeySub);
                }

                RegCloseKey(hKey);

                _tcscpy(sz, REG_KEY_GNUBOYROM);

                // .gb
                if (RegCreateKeyEx(HKEY_CLASSES_ROOT, REG_KEY_GB, 0, 0,
                                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
                        RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)sz, sizeof(TCHAR) * (_tcslen(sz) + 1));
                        RegCloseKey(hKey);
                }
                // .gbc
                if (RegCreateKeyEx(HKEY_CLASSES_ROOT, REG_KEY_GBC, 0, 0,
                                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, &dwDisposition) == ERROR_SUCCESS) {
                        RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)sz, sizeof(TCHAR) * (_tcslen(sz) + 1));
                        RegCloseKey(hKey);
                }
        }
}

void UndoAssociateGBExtension()
{
        RegDeleteKey(HKEY_CLASSES_ROOT, REG_KEY_GB);
        RegDeleteKey(HKEY_CLASSES_ROOT, REG_KEY_GBC);
        RegDeleteKey(HKEY_CLASSES_ROOT, REG_KEY_GNUBOYROM);
}

int press_key;
BOOL CALLBACK CTR_KeyPressDlg(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        switch (uMsg) {
                case WM_INITDIALOG:
                        press_key = 0;
                        return TRUE;
                case WM_COMMAND:
                        if (LOWORD(wParam) == IDCANCEL) {
                                press_key = 0;
                                EndDialog(hwndDlg, IDCANCEL);
                        }
                        return TRUE;
                case WM_KEYDOWN:
                /*{
                        if (wParam == 0x5B) {
                                for (int i = 0xC1; i < 0xC6; i++) {
                                        if (GetAsyncKeyState(i)) {
                                                press_key = i;
                                                EndDialog(hwndDlg, IDOK);
                                                return TRUE;
                                        }
                                }
                        }
                        // for jornada 56x
                        if (wParam == 0x84) {
                                for (int i = 0x25; i < 0x29; i++) {
                                        if (GetAsyncKeyState(i)) {
                                                press_key = i;
                                                EndDialog(hwndDlg, IDOK);
                                                return TRUE;
                                        }
                                }
                        }
                        return FALSE;
                }
                case WM_KEYUP:*/
					press_key = wParam;
                    EndDialog(hwndDlg, IDOK);
                    return TRUE;
                default: return FALSE;
        }
}

int CTR_StartKeyDialog(HWND hwndParent)
{
        AllKeys(true);
        DialogBox(g_hInst, (LPCTSTR)IDD_KEYPRESS, hwndParent, CTR_KeyPressDlg);
        AllKeys(false);
        return press_key;
}

void CTR_InitDialog(HWND hwndDlg, int keys[])
{
        TCHAR sz[32];

        SHINITDLGINFO shidi;
        shidi.dwMask = SHIDIM_FLAGS;
        shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
        shidi.hDlg = hwndDlg;
        SHInitDialog(&shidi);

        for (int i = 0; i < JOY_MAX; i++) {
                keys[i] = joy_get_key(i);

                wsprintf(sz, _T("0x%X"), keys[i]);
                SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_KEY_A + i), sz);
        }
}

void CTR_OnOK(HWND hwndDlg, int keys[])
{
        for (int i = 0; i < JOY_MAX; i++)
                joy_set_key(i, keys[i]);

        EndDialog(hwndDlg, IDOK);
}

BOOL CALLBACK ControllersDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
        static int keys[JOY_MAX];
        switch (message) {
                case WM_INITDIALOG:
                {
                        CTR_InitDialog(hwndDlg, keys);
                        break;
                }
                case WM_COMMAND:
                {
                        WORD wID = LOWORD(wParam);
                        if (wID == IDOK) {
                                CTR_OnOK(hwndDlg, keys);
                                return TRUE;
                        }
                        else if (wID == IDCANCEL) {
                                EndDialog(hwndDlg, LOWORD(wParam));
                                return TRUE;
                        }
                        else if (wID >= IDC_A && wID <= IDC_ESCAPE) {
                                int n = CTR_StartKeyDialog(hwndDlg);
                                if (n) {
                                        TCHAR sz[32];
                                        int nIndex = wID - IDC_A;
                                        wsprintf(sz, _T("0x%X"), n);
                                        SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_KEY_A + nIndex), sz);
                                        keys[nIndex] = n;
                                }
                                return TRUE;
                        }
                        else if (wID >= IDC_DEL_A && wID <= IDC_DEL_ESCAPE) {
                                TCHAR sz[32];
                                int nIndex = wID - IDC_DEL_A;
                                wsprintf(sz, _T("0x%X"), 0);
                                SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_KEY_A + nIndex), sz);
                                keys[nIndex] = 0;
                                return TRUE;
                        }
                        break;
                }
        }
        return FALSE;
}

void ShowControllersDlg(HWND hwnd)
{
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONTROLLERS), hwnd, ControllersDlgProc);
}

///////////////////////////////////////////////////////
void PRF_InitDialog(HWND hwndDlg)
{
        char dir[MAX_PATH];
        TCHAR sz[MAX_PATH];

        SHINITDLGINFO shidi;
        shidi.dwMask = SHIDIM_FLAGS;
        shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIPDOWN | SHIDIF_SIZEDLGFULLSCREEN;
        shidi.hDlg = hwndDlg;
        SHInitDialog(&shidi);

        get_savedir(dir);
        MultiByteToWideChar(CP_ACP, 0, dir, -1, sz, MAX_PATH);

        if (!_tcslen(sz))
                CheckRadioButton(hwndDlg, IDC_RADIO_SRAM_ROMDIRECTORY,
                                        IDC_RADIO_SRAM_DIRECTORY, IDC_RADIO_SRAM_ROMDIRECTORY);
        else {
                CheckRadioButton(hwndDlg, IDC_RADIO_SRAM_ROMDIRECTORY,
                                        IDC_RADIO_SRAM_DIRECTORY, IDC_RADIO_SRAM_DIRECTORY);
                SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_SRAM), sz);
        }

        if (g_fThrottling)
                CheckDlgButton(hwndDlg, IDC_CHECK_THROTTLING, BST_CHECKED);
        if (lcd_get_contrast())
                CheckDlgButton(hwndDlg, IDC_CHECK_CONTRAST, BST_CHECKED);
        if (g_fShowFPS)
                CheckDlgButton(hwndDlg, IDC_CHECK_SHOWFPS, BST_CHECKED);
        if (g_fDrawScreen)
                CheckDlgButton(hwndDlg, IDC_CHECK_DRAWSCREEN, BST_CHECKED);
        if (vid_get_ddraw())
                CheckDlgButton(hwndDlg, IDC_CHECK_DDRAW, BST_CHECKED);
        if (g_fGSGetFile)
                CheckDlgButton(hwndDlg, IDC_CHECK_GSGETFILE, BST_CHECKED);
}

void PRF_OnOK(HWND hwndDlg)
{
        char dir[MAX_PATH];
        TCHAR sz[MAX_PATH] = {0};

        if (IsDlgButtonChecked(hwndDlg, IDC_RADIO_SRAM_DIRECTORY))
                GetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_SRAM), sz, MAX_PATH);

        WideCharToMultiByte(CP_ACP, NULL, sz, -1, dir, MAX_PATH, NULL, NULL);
        set_savedir(dir);

        g_fThrottling = IsDlgButtonChecked(hwndDlg, IDC_CHECK_THROTTLING);
        lcd_set_contrast(IsDlgButtonChecked(hwndDlg, IDC_CHECK_CONTRAST));
        g_fShowFPS = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHOWFPS);
        g_fDrawScreen = IsDlgButtonChecked(hwndDlg, IDC_CHECK_DRAWSCREEN);
        vid_set_ddraw(IsDlgButtonChecked(hwndDlg, IDC_CHECK_DDRAW));
        g_fGSGetFile = IsDlgButtonChecked(hwndDlg, IDC_CHECK_GSGETFILE);

        EndDialog(hwndDlg, IDOK);
}

BOOL CALLBACK PreferencesDlgProc(HWND hwndDlg, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
        switch (message){
                case WM_INITDIALOG:
                {

                        PRF_InitDialog(hwndDlg);
                        return TRUE;
                }
                case WM_COMMAND:
                        switch (LOWORD(wParam)) {
                                case IDOK:
                                        PRF_OnOK(hwndDlg);
                                        return TRUE;
                                case IDCANCEL:
                                        EndDialog(hwndDlg, LOWORD(wParam));
                                        return TRUE;
                                case IDC_ASSOCIATE:
                                        AssociateGBExtension();
                                        return TRUE;
                                case IDC_UNDO:
                                        UndoAssociateGBExtension();
                                        return TRUE;
                        }
                        return FALSE;
        }
        return FALSE;
}

void ShowPreferencesDlg(HWND hwnd)
{
        DialogBox(g_hInst, MAKEINTRESOURCE(IDD_PREFERENCES), hwnd, PreferencesDlgProc);
}
