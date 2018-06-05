#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <TlHelp32.h>
#include <math.h>
#include <sys/stat.h>
#include <Dbghelp.h>

#include "resource.h"
#include "memory.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Dbghelp.lib")

#define WIDTH 185
#define HEIGHT 300
#define PI 3.141592653589793f
#define SetTextFromKey(id, key) if (key == 0) { strcpy(text, "None"); } else { GetKeyNameTextA(MapVirtualKey(key, MAPVK_VK_TO_VSC) << 16, text, sizeof(text)); } SetDlgItemTextA(hDlg, id, text);

typedef struct {
	short forward, backward, left, right, up, down, increase, decrease, god, fly, timer;
	short save[5];
	short load[5];
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