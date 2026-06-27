#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL StartMCPPipeServer(HWND hWnd, HINSTANCE hInst);
void StopMCPPipeServer();
void StartAutoPlay(int maxSteps, int speedMs);

#ifdef __cplusplus
}
#endif
