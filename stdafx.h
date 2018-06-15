#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <math.h>
#include <sys/stat.h>
#include <Dbghelp.h>
#include <Commctrl.h>

#include "resource.h"
#include "memory.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Comctl32.lib")

#define WIDTH 185
#define HEIGHT 300
#define PI 3.141592653589793f

#define ClearKeybind(k, i) Keybind((short *)&k, 0, 0, 0); SetTextFromKeybind(hDlg, i, k); active_id = 0; memset(&keybind, 0, sizeof(keybind)); SetKeybinds(&keybinds);

typedef struct {
	short forward[3], backward[3], left[3], right[3], up[3], down[3], increase[3], decrease[3], god[3], ammo[3], fly[3], timer[3], time_increase[3], time_decrease[3], time_reset[3];
	short save[5][3];
	short load[5][3];
} KEYBINDS;

typedef struct {
	char player[24];
	float fz;
	char camera[8];
} SAVE;

void Update();
void Listener();
void SetKeybinds(KEYBINDS *k);
KEYBINDS GetKeybinds();
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);