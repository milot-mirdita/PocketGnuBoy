#ifndef __MAINWND_H__
#define __MAINWND_H__

int MainWnd_MessageLoop();
LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL MainWnd_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void MainWnd_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
void MainWnd_OnPaint(HWND hwnd);
void MainWnd_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void MainWnd_OnClose(HWND hwnd);
void MainWnd_OnDestroy(HWND hwnd);
void MainWnd_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void MainWnd_OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void MainWnd_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void MainWnd_OnInitMenuPopup(HWND hwnd, HMENU hMenu, UINT item, BOOL fSystemMenu);
void MainWnd_OnActivate(HWND hwnd, UINT state, HWND hwndActDeact, BOOL fMinimized);
void MainWnd_OnEnterMenuLoop(HWND hwnd);
BOOL MainWnd_UpdateVPadState(int x, int y);
void MainWnd_OnUpdateFPS(HWND hwnd, WPARAM wParam, LPARAM lParam);

void OnFileOpen(HWND hwnd);
void OnFileClose(HWND hwnd);
void OnFileResume(HWND hwnd);
void OnFileReset(HWND hwnd);
void OnFileState(HWND hwnd, int nSlot);
void OnFileStateLoad(HWND hwnd);
void OnFileStateSave(HWND hwnd);
void OnFileExit(HWND hwnd);

void OnCtrlScreen(HWND hwnd, int nScreen);
void OnCtrlFrameSkip(HWND hwnd, int nSkip);
void OnCtrlFrameSkipAuto(HWND hwnd);
void OnCtrlAudioEnable(HWND hwnd);
void OnCtrlAudioSampleRate(HWND hwnd, int nRate);
void OnCtrlAudioStereo(HWND hwnd, BOOL fStereo);
void OnCtrlAudioBits(HWND hwnd, BOOL f16bits);
void OnCtrlControllers(HWND hwnd);
void OnCtrlPreferences(HWND hwnd);
void OnCtrlTurboA(HWND hwnd);
void OnCtrlTurboB(HWND hwnd);
void OnCtrlVPad(HWND hwnd);
void OnCtrlAudioBuffers(HWND hwnd, int n);

#endif // __MAINWND_H__
