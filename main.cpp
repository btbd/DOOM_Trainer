#include "stdafx.h"

// #define DEBUG

HANDLE process = 0;
SINT base = 0, player_sig = 0, camera_sig = 0, god_sig = 0, fall_sig = 0;
SINT fall_offset = 0;

HWND hWnd = 0;
KEYBINDS keybinds = { 0 };

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpCmdLine, int nCmdShow) {
#ifdef DEBUG
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	keybinds = GetKeybinds();

	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
	wcex.lpszClassName = L"wnd";
	RegisterClassExW(&wcex);

	RECT rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
	hWnd = CreateWindowW(L"wnd", L"DOOM Trainer", WS_BORDER, rect.right - WIDTH, rect.bottom - HEIGHT, WIDTH, HEIGHT, 0, 0, hInstance, 0);
	if (hWnd == NULL) {
		return 0;
	}

	SetWindowLong(hWnd, GWL_STYLE, 0);
	SetWindowLong(hWnd, GWL_STYLE, WS_BORDER);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Listener, 0, 0, 0);

	SetTimer(hWnd, 0, 8, 0);

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void SetKeybinds(KEYBINDS *k) {
	char path[0xFF] = { 0 };
	GetTempPathA(sizeof(path), path);
	strcat(path, "doomtrainer.keybinds");
	FILE *file = fopen(path, "wb");
	if (file) {
		fwrite(k, sizeof(KEYBINDS), 1, file);
		fclose(file);
	}
}

KEYBINDS GetKeybinds() {
	KEYBINDS k = { 0 };
	struct stat st = { 0 };
	FILE *file = 0;
	char path[0xFF] = { 0 };

	GetTempPathA(sizeof(path), path);
	strcat(path, "doomtrainer.keybinds");
	stat(path, &st);

	if (st.st_size == sizeof(keybinds) && (file = fopen(path, "rb"))) {
		fread(&k, sizeof(k), 1, file);
		fclose(file);
	} else {
		k.forward = 0x57;
		k.backward = 0x53;
		k.left = 0x41;
		k.right = 0x44;
		k.up = 0x20;
		k.down = 0x10;
		k.increase = 0x45;
		k.decrease = 0x51;
		k.god = 0x31;
		k.fly = 0x32;
		k.timer = 0x33;
		k.save[0] = 0x34;
		k.load[0] = 0x35;
		SetKeybinds(&k);
	}

	keybinds = k;

	return k;
}

INT_PTR CALLBACK KeybindsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static char text[0xFF] = { 0 };
	static DWORD active_id = 0;

	switch (uMsg) {
		case WM_INITDIALOG:
			active_id = 0;
			SetTextFromKey(IDC_FORWARD, keybinds.forward);
			SetTextFromKey(IDC_BACKWARD, keybinds.backward);
			SetTextFromKey(IDC_LEFT, keybinds.left);
			SetTextFromKey(IDC_RIGHT, keybinds.right);
			SetTextFromKey(IDC_UP, keybinds.up);
			SetTextFromKey(IDC_DOWN, keybinds.down);
			SetTextFromKey(IDC_INCREASE, keybinds.increase);
			SetTextFromKey(IDC_DECREASE, keybinds.decrease);
			SetTextFromKey(IDC_GOD, keybinds.god);
			SetTextFromKey(IDC_FLY, keybinds.fly);
			SetTextFromKey(IDC_TIMER, keybinds.timer);
			SetTextFromKey(IDC_SAVE0, keybinds.save[0]);
			SetTextFromKey(IDC_LOAD0, keybinds.load[0]);
			SetTextFromKey(IDC_SAVE1, keybinds.save[1]);
			SetTextFromKey(IDC_LOAD1, keybinds.load[1]);
			SetTextFromKey(IDC_SAVE2, keybinds.save[2]);
			SetTextFromKey(IDC_LOAD2, keybinds.load[2]);
			SetTextFromKey(IDC_SAVE3, keybinds.save[3]);
			SetTextFromKey(IDC_LOAD3, keybinds.load[3]);
			SetTextFromKey(IDC_SAVE4, keybinds.save[4]);
			SetTextFromKey(IDC_LOAD4, keybinds.load[4]);
			SetTimer(hDlg, 0, 17, NULL);
			return (INT_PTR)TRUE;
		case WM_TIMER: {
			if (active_id) {
				short key = 0;
				for (short i = 4; i <= 0xFF; ++i) {
					if (GetAsyncKeyState(i) < 0) {
						key = i;
						break;
					}
				}

				if (key) {
					switch (active_id) {
						case IDC_FORWARD:
							keybinds.forward = key;
							break;
						case IDC_BACKWARD:
							keybinds.backward = key;
							break;
						case IDC_LEFT:
							keybinds.left = key;
							break;
						case IDC_RIGHT:
							keybinds.right = key;
							break;
						case IDC_UP:
							keybinds.up = key;
							break;
						case IDC_DOWN:
							keybinds.down = key;
							break;
						case IDC_INCREASE:
							keybinds.increase = key;
							break;
						case IDC_DECREASE:
							keybinds.decrease = key;
							break;
						case IDC_GOD:
							keybinds.god = key;
							break;
						case IDC_FLY:
							keybinds.fly = key;
							break;
						case IDC_TIMER:
							keybinds.timer = key;
							break;
						case IDC_SAVE0:
							keybinds.save[0] = key;
							break;
						case IDC_LOAD0:
							keybinds.load[0] = key;
							break;
						case IDC_SAVE1:
							keybinds.save[1] = key;
							break;
						case IDC_LOAD1:
							keybinds.load[1] = key;
							break;
						case IDC_SAVE2:
							keybinds.save[2] = key;
							break;
						case IDC_LOAD2:
							keybinds.load[2] = key;
							break;
						case IDC_SAVE3:
							keybinds.save[3] = key;
							break;
						case IDC_LOAD3:
							keybinds.load[3] = key;
							break;
						case IDC_SAVE4:
							keybinds.save[4] = key;
							break;
						case IDC_LOAD4:
							keybinds.load[4] = key;
							break;
					}

					SetKeybinds(&keybinds);
					SetTextFromKey(active_id, key);

					active_id = 0;
				}

				break;
			}

			break;
		}
		case WM_COMMAND: {
			DWORD id = LOWORD(wParam);
			switch (id) {
				case IDC_FORWARD: case IDC_BACKWARD: case IDC_LEFT: case IDC_RIGHT: case IDC_UP: case IDC_DOWN: case IDC_INCREASE: case IDC_DECREASE: case IDC_GOD: case IDC_FLY: case IDC_TIMER: case IDC_SAVE0: case IDC_LOAD0: case IDC_SAVE1: case IDC_LOAD1:case IDC_SAVE2: case IDC_LOAD2: case IDC_SAVE3: case IDC_LOAD3: case IDC_SAVE4: case IDC_LOAD4: {
					active_id = id;
					break;
				}
				case IDC_FORWARD_CLEAR:
					keybinds.forward = 0;
					SetTextFromKey(IDC_FORWARD, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_BACKWARD_CLEAR:
					keybinds.backward = 0;
					SetTextFromKey(IDC_BACKWARD, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_LEFT_CLEAR:
					keybinds.left = 0;
					SetTextFromKey(IDC_LEFT, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_RIGHT_CLEAR:
					keybinds.right = 0;
					SetTextFromKey(IDC_RIGHT, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_UP_CLEAR:
					keybinds.up = 0;
					SetTextFromKey(IDC_UP, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_DOWN_CLEAR:
					keybinds.down = 0;
					SetTextFromKey(IDC_DOWN, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_INCREASE_CLEAR:
					keybinds.increase = 0;
					SetTextFromKey(IDC_INCREASE, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_DECREASE_CLEAR:
					keybinds.decrease = 0;
					SetTextFromKey(IDC_DECREASE, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_GOD_CLEAR:
					keybinds.god = 0;
					SetTextFromKey(IDC_GOD, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_FLY_CLEAR:
					keybinds.fly = 0;
					SetTextFromKey(IDC_FLY, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_TIMER_CLEAR:
					keybinds.timer = 0;
					SetTextFromKey(IDC_TIMER, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_SAVE0_CLEAR:
					keybinds.save[0] = 0;
					SetTextFromKey(IDC_SAVE0, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_LOAD0_CLEAR:
					keybinds.load[0] = 0;
					SetTextFromKey(IDC_LOAD0, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_SAVE1_CLEAR:
					keybinds.save[1] = 0;
					SetTextFromKey(IDC_SAVE1, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_LOAD1_CLEAR:
					keybinds.load[1] = 0;
					SetTextFromKey(IDC_LOAD1, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_SAVE2_CLEAR:
					keybinds.save[2] = 0;
					SetTextFromKey(IDC_SAVE2, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_LOAD2_CLEAR:
					keybinds.load[2] = 0;
					SetTextFromKey(IDC_LOAD2, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_SAVE3_CLEAR:
					keybinds.save[3] = 0;
					SetTextFromKey(IDC_SAVE3, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_LOAD3_CLEAR:
					keybinds.load[3] = 0;
					SetTextFromKey(IDC_LOAD3, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_SAVE4_CLEAR:
					keybinds.save[4] = 0;
					SetTextFromKey(IDC_SAVE4, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
				case IDC_LOAD4_CLEAR:
					keybinds.load[4] = 0;
					SetTextFromKey(IDC_LOAD4, 0);
					active_id = 0;
					SetKeybinds(&keybinds);
					break;
			}

			break;
		}
		case WM_CLOSE:
			active_id = 0;
			EndDialog(hDlg, 0);
			SetKeybinds(&keybinds);
			return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int drag_x = 0, drag_y = 0;

	switch (uMsg) {
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case 0:
					ShowWindow(CreateDialog(GetModuleHandle(0), MAKEINTRESOURCE(IDD_KEYBINDS), hWnd, KeybindsProc), SW_SHOW);
					break;
				case 1:
					goto safe_exit;
					break;
			}

			break;
		}
		case WM_RBUTTONDOWN: {
			HMENU menu = CreatePopupMenu();
			InsertMenu(menu, 0, MF_BYPOSITION | MF_STRING, 0, L"Keybinds");
			InsertMenu(menu, 1, MF_BYPOSITION | MF_STRING, 1, L"Exit");
			SetForegroundWindow(hWnd);

			POINT p;
			GetCursorPos(&p);

			TrackPopupMenu(menu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, 0, hWnd, NULL);

			break;
		}
		case WM_LBUTTONDOWN: {
			SetCapture(hWnd);

			POINT p;
			GetCursorPos(&p);
			RECT r;
			GetWindowRect(hWnd, &r);

			drag_x = p.x - r.left;
			drag_y = p.y - r.top;

			break;
		}
		case WM_LBUTTONUP:
			ReleaseCapture();
			break;
		case WM_MOUSEMOVE: {
			if (wParam == MK_LBUTTON) {
				POINT p;
				GetCursorPos(&p);

				MoveWindow(hWnd, p.x - drag_x, p.y - drag_y, WIDTH, HEIGHT, false);
			}

			break;
		}
		case WM_TIMER:
			Update();
			break;
		case WM_DESTROY:
			safe_exit:
			WriteInt(process, (void *)(GetPointer(process, 5, god_sig, (SINT)0x0, (SINT)0x18, (SINT)0x228, (SINT)0x98)), 0);
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void Update() {
	static SINT save_poffsets[]     = { 0, 4, 8, 12, 16, 20 };
	static SINT save_coffsets[]     = { 0, 4 };

	static bool  fly                 = false;
	static bool  fly_press           = false;
	static float fly_position[3]     = { 0 };
	static float fly_velocity[3]     = { 0 };
	static float fly_multiplier		 = 1;

	static bool  god                 = false;
	static bool  god_press	         = false;

	static float timer_position[3]   = { 0 };
	static DWORD timer_start         = 0;
	static DWORD timer_press         = 0;
	static float timer               = 0;

	float position[3] = { 0 };
	float velocity[3] = { 0 };
	float rotation[2] = { 0 };

	if (process) {
		SINT player_base = (SINT)GetPointer(process, 4, player_sig, (SINT)0x8, (SINT)0x808, (SINT)0x1AB8);
		SINT camera_base = player_base + 1416; // (SINT)GetPointer(process, 3, player_sig, (SINT)0x8, (SINT)0x12850);
		bool active = GetForegroundWindow() == FindWindowA("Zion", 0);

		if (active) {
			if (GetAsyncKeyState(keybinds.god) < 0) {
				if (!god_press) {
					god = !god;
					if (!god) {
						WriteInt(process, (void *)(GetPointer(process, 5, god_sig, (SINT)0x0, (SINT)0x18, (SINT)0x228, (SINT)0x98)), 0);
					}
				}

				god_press = true;
			} else {
				god_press = false;
			}

			if (GetAsyncKeyState(keybinds.fly) < 0) {
				if (!fly_press) {
					fly = !fly;

					if (fly) {
						fly_multiplier = 1;
						memset(fly_velocity, 0, sizeof(fly_velocity));
						ReadBuffer(process, (void *)player_base, fly_position, sizeof(fly_position));
					} else {
						float angle = (float)atan2(fly_velocity[1], fly_velocity[0]);
						float speed = (float)sqrt(fly_velocity[0] * fly_velocity[0] + fly_velocity[1] * fly_velocity[1]) * 60.0f;
						WriteFloat(process, (void *)(player_base + 12), (float)cos(angle) * speed);
						WriteFloat(process, (void *)(player_base + 16), (float)sin(angle) * speed);
						WriteFloat(process, (void *)(player_base + 20), fly_velocity[2] * 60.0f);
					}
				}

				fly_press = true;
			} else {
				fly_press = false;
			}

			if (GetAsyncKeyState(keybinds.timer) < 0) {
				if (++timer_press > 50) {
					timer_position[0] = 0;
					timer = 0;
				} else {
					ReadBuffer(process, (void *)player_base, timer_position, sizeof(timer_position));
				}
			} else {
				timer_press = 0;
			}

			for (int s = 0; s < sizeof(keybinds.save) / sizeof(keybinds.save[0]); ++s) {
				if (keybinds.save[s] && GetAsyncKeyState(keybinds.save[s]) < 0) {
					SAVE save = { 0 };
					save.fz = ReadFloat(process, GetPointer(process, 4, player_sig, 0x8, 0x800, fall_offset));
					ReadBuffer(process, (void *)player_base, save.player, sizeof(save.player));
					if (fly) {
						float angle = (float)atan2(fly_velocity[1], fly_velocity[0]);
						float speed = (float)sqrt(fly_velocity[0] * fly_velocity[0] + fly_velocity[1] * fly_velocity[1]) * 60.0f;
						*(float *)&save.player[12] = (float)cos(angle) * speed;
						*(float *)&save.player[16] = (float)sin(angle) * speed;
						*(float *)&save.player[20] = fly_velocity[2] * 60.0f;
					}
					ReadBuffer(process, (void *)camera_base, save.camera, sizeof(save.camera));

					char path[0xFF] = { 0 };
					GetTempPathA(sizeof(path), path);
					sprintf(path + strlen(path), "doomtrainer_save_%d", s);

					FILE *file = fopen(path, "wb");
					if (file) {
						fwrite(&save, sizeof(save), 1, file);
						fclose(file);
					}
				}

				if (keybinds.load[s] && GetAsyncKeyState(keybinds.load[s]) < 0) {
					char path[0xFF] = { 0 };
					GetTempPathA(sizeof(path), path);
					sprintf(path + strlen(path), "doomtrainer_save_%d", s);

					FILE *file = fopen(path, "rb");
					struct stat st = { 0 };
					stat(path, &st);
					if (file && st.st_size == sizeof(SAVE)) {
						SAVE save = { 0 };
						fread(&save, sizeof(save), 1, file);

						SINT camera = (SINT)ReadLongLong(process, (void *)camera_sig);
						
						byte fall_og[8] = { 0 };
						ReadBuffer(process, (void *)fall_sig, fall_og, 8);
						WriteBuffer(process, (void *)fall_sig, "\x90\x90\x90\x90\x90\x90\x90\x90", 8);
						WriteFloat(process, GetPointer(process, 4, player_sig, 0x8, 0x800, fall_offset), save.fz);
						WriteBuffer(process, (void *)player_base, save.player, sizeof(save.player));
						WriteFloat(process, (void *)(camera + 0x14), ReadFloat(process, (void *)(camera + 0x14)) - (ReadFloat(process, (void *)(camera_base + 4)) - *(float *)&save.camera[4]));
						WriteFloat(process, (void *)(camera + 0x10), ReadFloat(process, (void *)(camera + 0x10)) - (ReadFloat(process, (void *)camera_base) - *(float *)&save.camera));
						ReadBuffer(process, (void *)player_base, fly_position, sizeof(fly_position));
						WriteBuffer(process, (void *)fall_sig, fall_og, 8);
						Sleep(17);
						
						if (timer_position[0] != 0) {
							timer_start = timeGetTime();
						}

						fclose(file);
					}
				}
			}
		}

		if (god || fly) {
			WriteInt(process, GetPointer(process, 5, god_sig, (SINT)0x0, (SINT)0x18, (SINT)0x228, (SINT)0x98), 2);
		}

		if (fly) {
			float r = ReadFloat(process, (void *)(camera_base + 4)) * (PI / 180);
			
			float vx = 0, vy = 0, vz = 0;
			if (active) {
				vx = (float)(GetAsyncKeyState(keybinds.forward) < 0) - (GetAsyncKeyState(keybinds.backward) < 0);
				vy = (float)(GetAsyncKeyState(keybinds.left) < 0) - (GetAsyncKeyState(keybinds.right) < 0);
				vz = (float)(GetAsyncKeyState(keybinds.up) < 0) - (GetAsyncKeyState(keybinds.down) < 0);

				if (GetAsyncKeyState(keybinds.increase) < 0) {
					fly_multiplier += 0.05f;
				} else if (GetAsyncKeyState(keybinds.decrease) < 0) {
					fly_multiplier -= 0.05f;
					if (fly_multiplier < 0.1) {
						fly_multiplier = 0.1f;
					}
				}
			}
			float v = (float)sqrt(vx * vx + vy * vy + vz * vz);
			if (v != 0) {
				vx /= v;
				vy /= v;
				vz /= v;
			}

			fly_velocity[0] += (float)(cos(r) * vx - sin(r) * vy) * fly_multiplier;
			fly_velocity[1] += (float)(sin(r) * vx + cos(r) * vy) * fly_multiplier;
			fly_velocity[2] += vz * fly_multiplier;

			for (DWORD i = 0; i < 3; ++i) fly_position[i] += fly_velocity[i];

			WriteBuffer(process, (void *)player_base, fly_position, sizeof(fly_position));
			WriteBuffer(process, (void *)(player_base + 12), "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 12);

			for (DWORD i = 0; i < 3; ++i) {
				fly_velocity[i] *= 0.90f;
				velocity[i] = fly_velocity[i] * 60.0f;
			}
		} else {
			ReadBuffer(process, (void *)(player_base + 12), velocity, sizeof(velocity));
		}

		ReadBuffer(process, (void *)player_base, position, sizeof(position));
		ReadBuffer(process, (void *)camera_base, rotation, sizeof(rotation));

		if (timer_position[0] != 0 && timer_start != 0) {
			timer = (float)(timeGetTime() - timer_start) / 1000.0f;

			float dx = timer_position[0] - position[0];
			float dy = timer_position[1] - position[1];
			float dz = timer_position[2] - position[2];
			if (sqrt(dx * dx + dy * dy + dz * dz) < 100) {
				timer_start = 0;
			}
		}
	}

	static char buffer[0xFFF];
	static HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));

	HDC hdcOld = GetDC(hWnd);
	HDC hdc = CreateCompatibleDC(hdcOld);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdcOld, WIDTH, HEIGHT);
	HGDIOBJ hOld = SelectObject(hdc, hbmMem);
	RECT rect = { 0, 0, WIDTH, HEIGHT };
	FillRect(hdc, &rect, brush);
	SetBkColor(hdc, RGB(255, 2555, 255));
	SetTextColor(hdc, RGB(0, 0, 0));
	HFONT font = CreateFontA(26, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
	SelectObject(hdc, font);

	sprintf(buffer,
		"%.2f\n"
		"%.2f\n"
		"%.2f\n\n"
		"%.2f\n\n"
		"%.2f\n"
		"%.2f\n\n"
		"%.2f\n",
		position[0], position[1], position[2], (float)sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]), rotation[1], rotation[0], timer);

	rect.right -= 5;
	DrawTextA(hdc, buffer, -1, &rect, DT_RIGHT | DT_WORDBREAK);

	rect.left += 5;
	strcpy(buffer, "x\ny\nz\n\nv\n\nrx\nry\n\nt");
	DrawTextA(hdc, buffer, -1, &rect, DT_LEFT | DT_WORDBREAK);

	rect.top += HEIGHT - 26 - 5;
	rect.left += 40;
	rect.right -= 40;

	if (god) SetTextColor(hdc, RGB(255, 0, 0));
	DrawTextA(hdc, "god", -1, &rect, DT_LEFT);
	SetTextColor(hdc, RGB(0, 0, 0));
	if (fly) SetTextColor(hdc, RGB(255, 0, 0));
	DrawTextA(hdc, "fly", -1, &rect, DT_RIGHT);

	BitBlt(hdcOld, 0, 0, WIDTH, HEIGHT, hdc, 0, 0, SRCCOPY);
	SelectObject(hdc, hOld);
	DeleteObject(hbmMem);
	DeleteDC(hdc);
	DeleteObject(font);
	ReleaseDC(hWnd, hdcOld);
}

void Listener() {
	DWORD pid = 0;
	MODULEENTRY32 module = { 0 };

	for (;;) {
		DWORD t = GetProcessInfoByName(L"doomx64.exe").th32ProcessID;
		if (!t) t = GetProcessInfoByName(L"doomx64vk.exe").th32ProcessID;

		if (t) {
			if (pid != t) {
				pid = t;
				if (process) CloseHandle(process);
				process = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

				module = GetModuleInfoByName(GetProcessId(process), L"doomx64.exe");
				if (!module.modBaseSize) module = module = GetModuleInfoByName(GetProcessId(process), L"doomx64vk.exe");
				base = (SINT)module.modBaseAddr;

				player_sig = (SINT)FindPattern(process, module.modBaseAddr, module.modBaseSize, "\x48\x03\x0D\x00\x00\x00\x00\x39\x01", "xxx????xx");
				player_sig += 3;
				player_sig += ReadInt(process, (void *)player_sig) + 4;

				god_sig = (SINT)FindPattern(process, module.modBaseAddr, module.modBaseSize, "\x48\x8D\x87\xE8\x03\x00\x00\x75\x04\x48\x8D\x47\x20", "xxxxxxxxxxxxx");
				god_sig += 16;
				god_sig += ReadInt(process, (void *)god_sig) + 4;

				fall_sig = (SINT)FindPattern(process, module.modBaseAddr, module.modBaseSize, "\x40\x32\xFF\x44\x88\x6C\x24\x61", "xxxxxxxx");
				fall_sig = (SINT)FindPattern(process, (void *)fall_sig, module.modBaseSize - (DWORD)(fall_sig - (SINT)module.modBaseAddr), "\xF3\x0F\x11\x00\x00\x00\x00\x00\xF3\x0F\x11", "xxx?????xxx");
				fall_sig += 16;

				fall_offset = ReadInt(process, (void *)(fall_sig + 4));

				SINT src = (SINT)FindPattern(process, module.modBaseAddr, module.modBaseSize, "\x49\xB9\x00\x00\x00\x00\x00\x00\x00\x00\x49\x89\x09", "xx????????xxx");
				if (src) {
					camera_sig = ReadLongLong(process, (void *)(src + 2));
				} else {
					src = (SINT)FindPattern(process, module.modBaseAddr, module.modBaseSize, "\x8B\xD5\x48\x8B\xCB\xE8\x00\x00\x00\x00\xEB\x00\x48\x8B\x83", "xxxxxx????x?xxx") + 143;
					camera_sig = (SINT)VirtualAllocEx(process, 0, 8, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
					SINT dest = (SINT)VirtualAllocEx(process, 0, 1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

					byte original[13] = { 0 };
					ReadBuffer(process, (void *)src, original, 13);

					WriteBuffer(process, (void *)dest, "\x49\xB9\x00\x00\x00\x00\x00\x00\x00\x00\x49\x89\x09", 13);
					WriteLongLong(process, (void *)(dest + 2), camera_sig);
					WriteBuffer(process, (void *)(dest + 13), original, 13);
					WriteBuffer(process, (void *)(dest + 13 + 13), "\x49\xB9\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\xE1", 13);
					WriteLongLong(process, (void *)(dest + 13 + 13 + 2), src + 13);

					memcpy(original, "\x49\xB9\x00\x00\x00\x00\x00\x00\x00\x00\x41\xFF\xE1", 13);
					*(SINT *)(&original[2]) = dest;
					WriteBuffer(process, (void *)src, original, 13);
				}
			}
		} else {
			pid = 0;
			base = 0;
			process = 0;
		}

		Sleep(2000);
	}
}