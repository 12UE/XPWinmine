#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

BOOL StartMCPPipeServer(HWND hWnd, HINSTANCE hInst);
void StopMCPPipeServer();

#ifdef __cplusplus
}
#endif
