#include "PocketGnuboy.h"
#include "wce.h"

#define REG_KEY_GNUBOY                          _T("Software\\PocketGnuboy")
#define REG_NAME_SAVEDIR                        _T("SaveDir")
#define REG_NAME_VIDEO_FRAMESKIP        _T("VideoFrameSkip")
#define REG_NAME_VIDEO_SCREENMODE       _T("VideoScreenMode")
#define REG_NAME_AUDIO_ENABLE           _T("AudioEnable")
#define REG_NAME_AUDIO_STEREO           _T("AudioStereo")
#define REG_NAME_AUDIO_SAMPLERATE       _T("AudioSampleRate")
#define REG_NAME_AUDIO_16BITS           _T("Audio16Bits")
#define REG_NAME_AUDIO_BUFFERS          _T("AudioBuffers")
#define REG_NAME_VPAD                           _T("VPad")
#define REG_NAME_THROTTLING                     _T("SpeedThrottling")
#define REG_NAME_LCD_CONTRAST           _T("Contrast")
#define REG_NAME_SHOWFPS                        _T("ShowFPS")
#define REG_NAME_DRAWSCREEN                     _T("DrawScreen")
#define REG_NAME_DDRAW                           _T("DDRAW")
#define REG_NAME_GSGETFILE                      _T("gsGetFile.dll")
#define REG_NAME_LASTDIR                        _T("LastDir")
#define REG_NAME_JOY_KEY                        _T("JoyKey%d")

void LoadRegistory()
{
        HKEY hKey;
        DWORD dwValue, dwSize, dwType;
        TCHAR sz[MAX_PATH];
        char szDir[MAX_PATH];

        if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY_GNUBOY, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
                return;

        dwSize = sizeof(char) * MAX_PATH;
        if (RegQueryValueEx(hKey, REG_NAME_SAVEDIR, 0, &dwType, (LPBYTE)szDir, &dwSize) == ERROR_SUCCESS)
                set_savedir(szDir);

        dwSize = sizeof(dwValue);
        if (RegQueryValueEx(hKey, REG_NAME_VIDEO_FRAMESKIP, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                vid_set_frameskip(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_VIDEO_SCREENMODE, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                vid_set_screenmode(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_AUDIO_ENABLE, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                pcm_set_enable(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_AUDIO_STEREO, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                pcm_set_stereo(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_AUDIO_SAMPLERATE, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                pcm_set_samplerate(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_AUDIO_16BITS, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                pcm_set_16bits(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_AUDIO_BUFFERS, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                pcm_set_buffer_count(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_VPAD, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                g_fVPad = dwValue;

        if (RegQueryValueEx(hKey, REG_NAME_THROTTLING, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                g_fThrottling = dwValue;

        if (RegQueryValueEx(hKey, REG_NAME_LCD_CONTRAST, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                lcd_set_contrast(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_SHOWFPS, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                g_fShowFPS = dwValue;

        if (RegQueryValueEx(hKey, REG_NAME_DRAWSCREEN, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                g_fDrawScreen = dwValue;

        if (RegQueryValueEx(hKey, REG_NAME_DDRAW, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                vid_set_ddraw(dwValue);

        if (RegQueryValueEx(hKey, REG_NAME_GSGETFILE, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                g_fGSGetFile = dwValue;

        for (int i = 0; i < JOY_MAX; i++) {
                wsprintf(sz, REG_NAME_JOY_KEY, i);
                if (RegQueryValueEx(hKey, sz, 0, &dwType, (LPBYTE)&dwValue, &dwSize) == ERROR_SUCCESS)
                        joy_set_key(i, dwValue);
        }

        dwSize = sizeof(TCHAR) * MAX_PATH;
        if (RegQueryValueEx(hKey, REG_NAME_LASTDIR, 0, &dwType, (LPBYTE)sz, &dwSize) == ERROR_SUCCESS)
                _tcscpy(g_szLastDir, sz);

        RegCloseKey(hKey);
}

void SaveRegistory()
{
        HKEY hKey;
        DWORD dwDisposition, dwValue;
        TCHAR sz[MAX_PATH];
        char szDir[MAX_PATH];

        if (RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY_GNUBOY, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition) != ERROR_SUCCESS)
                return;

        get_savedir(szDir);
        RegSetValueEx(hKey, REG_NAME_SAVEDIR, 0, REG_BINARY, (LPBYTE)szDir, sizeof(char) * (strlen(szDir) + 1));
        dwValue = vid_get_frameskip();
        RegSetValueEx(hKey, REG_NAME_VIDEO_FRAMESKIP, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = vid_get_screenmode();
        RegSetValueEx(hKey, REG_NAME_VIDEO_SCREENMODE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = pcm_get_enable();
        RegSetValueEx(hKey, REG_NAME_AUDIO_ENABLE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = pcm_get_stereo();
        RegSetValueEx(hKey, REG_NAME_AUDIO_STEREO, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = pcm_get_samplerate();
        RegSetValueEx(hKey, REG_NAME_AUDIO_SAMPLERATE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = pcm_get_16bits();
        RegSetValueEx(hKey, REG_NAME_AUDIO_16BITS, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = pcm_get_buffer_count();
        RegSetValueEx(hKey, REG_NAME_AUDIO_BUFFERS, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = g_fVPad;
        RegSetValueEx(hKey, REG_NAME_VPAD, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = g_fThrottling;
        RegSetValueEx(hKey, REG_NAME_THROTTLING, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = lcd_get_contrast();
        RegSetValueEx(hKey, REG_NAME_LCD_CONTRAST, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = g_fShowFPS;
        RegSetValueEx(hKey, REG_NAME_SHOWFPS, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = g_fDrawScreen;
        RegSetValueEx(hKey, REG_NAME_DRAWSCREEN, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = vid_get_ddraw();
        RegSetValueEx(hKey, REG_NAME_DDRAW, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        dwValue = g_fGSGetFile;
        RegSetValueEx(hKey, REG_NAME_GSGETFILE, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));

        for (int i = 0; i < JOY_MAX; i++) {
                wsprintf(sz, REG_NAME_JOY_KEY, i);
                dwValue = joy_get_key(i);
                RegSetValueEx(hKey, sz, 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
        }

        RegSetValueEx(hKey, REG_NAME_LASTDIR, 0, REG_SZ, (LPBYTE)g_szLastDir, sizeof(TCHAR) * (_tcslen(g_szLastDir) + 1));

        RegCloseKey(hKey);
}
