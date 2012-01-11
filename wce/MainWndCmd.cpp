#include "PocketGnuboy.h"
#include "MainWnd.h"
#include "Dialog.h"
#include "wce.h"

void OnFileOpen(HWND hwnd)
{
        OPENFILENAME ofn = {0};
        TCHAR szFile[MAX_PATH] = {0};

        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrTitle = 0;
        ofn.lpstrFilter =
				_T("GB Roms (*.gb;*.gbc)\0*.gb;*.gbc\0All Files (*.*)\0*.*\0");
                //_T("ROM Files (*.gb;*.gbc;*.zip;*.gz)\0*.gb;*.gbc;*.zip;*.gz\0GB Roms (*.gb;*.gbc)\0*.gb;*.gbc\0ZIP Files (*.zip)\0*.zip\0GZ Files (*.gz)\0*.gz\0All Files (*.*)\0*.*\0");
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrInitialDir = g_szLastDir;

        BOOL fRet;
        BOOL (*gsGetOpenFileName)(OPENFILENAME*) = NULL;
        HINSTANCE hInst = LoadLibrary(_T("gsgetfile.dll"));
        if (hInst) (FARPROC&)gsGetOpenFileName = GetProcAddress(hInst, _T("gsGetOpenFileName"));

        if (g_fGSGetFile && gsGetOpenFileName)
                fRet = gsGetOpenFileName(&ofn);
        else
                fRet = GetOpenFileName(&ofn);

        if (fRet) {
                if (LoadROM(szFile)) {
                        OnFileResume(hwnd);

                        LPTSTR p = _tcsrchr(szFile, _T('\\'));
                        if (p) *p = NULL;
                        _tcscpy(g_szLastDir, szFile);
                }
        }
        if (hInst) FreeLibrary(hInst);
        UpdateToolbar();
}

void OnFileClose(HWND hwnd)
{
        emu_close();
        UpdateToolbar();

        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
}

void OnFileResume(HWND hwnd)
{
        Resume();
}

void OnFileReset(HWND hwnd)
{
        if (get_romfile()) {
                emu_reset();
                OnFileResume(hwnd);
        }
}

void OnFileState(HWND hwnd, int nSlot)
{
        set_saveslot(nSlot);
}

void OnFileStateLoad(HWND hwnd)
{
        int result = MessageBox(hwnd, TEXT("Are you sure?"), TEXT("Confirm"), MB_OKCANCEL);
        if (result == IDOK && get_romfile()) {
                state_load(get_saveslot());
                OnFileResume(hwnd);
        }
}

void OnFileStateSave(HWND hwnd)
{
		int result = MessageBox(hwnd, TEXT("Are you sure?"), TEXT("Confirm"), MB_OKCANCEL);
        if (result == IDOK && get_romfile()) {
                state_save(get_saveslot());
                OnFileResume(hwnd);
        }
}

void OnFileExit(HWND hwnd)
{
        PostMessage(hwnd, WM_CLOSE, 0, 0);
}

void OnCtrlScreen(HWND hwnd, int nScreen)
{
        vid_set_screenmode(nScreen);
}

void OnCtrlFrameSkip(HWND hwnd, int nSkip)
{
        vid_set_frameskip(nSkip);
}

void OnCtrlAudioEnable(HWND hwnd)
{
        pcm_set_enable(!pcm_get_enable());
        UpdateToolbar();
}

void OnCtrlAudioSampleRate(HWND hwnd, int nRate)
{
        switch (nRate) {
        case 1:
                pcm_set_samplerate(22050);
                break;
        case 2:
                pcm_set_samplerate(44100);
                break;
        default:
                pcm_set_samplerate(11025);
                break;
        }
}

void OnCtrlAudioStereo(HWND hwnd, BOOL fStereo)
{
        pcm_set_stereo(fStereo);
}

void OnCtrlAudioBits(HWND hwnd, BOOL f16bits)
{
        pcm_set_16bits(f16bits);
}

void OnCtrlControllers(HWND hwnd)
{
        ShowControllersDlg(hwnd);
}

void OnCtrlPreferences(HWND hwnd)
{
        ShowPreferencesDlg(hwnd);
}

void OnCtrlTurboA(HWND hwnd)
{
        joy_set_turbo_a(!joy_get_turbo_a());
}

void OnCtrlTurboB(HWND hwnd)
{
        joy_set_turbo_b(!joy_get_turbo_b());
}

void OnCtrlVPad(HWND hwnd)
{
        g_fVPad = !g_fVPad;
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
}

void OnCtrlAudioBuffers(HWND hwnd, int n)
{
        const int buffers[] = {4, 8, 12, 16, 24, 32};
        pcm_set_buffer_count(buffers[n]);
}
