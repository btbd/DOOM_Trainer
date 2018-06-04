#include "stdafx.h"

ARRAY ArrayNew(unsigned char element_size) {
	ARRAY array;

	array.element_size = element_size;
	array.allocated = 0xFF;
	array.buffer = malloc(element_size * 0xFF);
	array.length = 0;

	return array;
}

void *ArrayGet(ARRAY *array, DWORD index) {
	return (void *)((SINT)array->buffer + (index * array->element_size));
}

void *ArraySet(ARRAY *array, DWORD index, void *element) {
	return memcpy((void *)((SINT)array->buffer + (index * array->element_size)), element, array->element_size);
}

void *ArrayPush(ARRAY *array, void *element) {
	if (array->length >= array->allocated) {
		array->allocated *= 2;
		array->buffer = realloc(array->buffer, array->allocated * array->element_size);
	}

	return memcpy((void *)((SINT)array->buffer + (array->length++ * array->element_size)), element, array->element_size);
}

void ArrayPop(ARRAY *array, void *out) {
	if (out) {
		memcpy(out, ArrayGet(array, --array->length), array->element_size);
	} else {
		--array->length;
	}
}

void ArrayMerge(ARRAY *dest, ARRAY *array) {
	dest->allocated += array->allocated;
	dest->buffer = realloc(dest->buffer, dest->allocated * array->element_size);

	memcpy((void *)((SINT)dest->buffer + (dest->length * dest->element_size)), array->buffer, array->length * dest->element_size);

	dest->length += array->length;
}

void ArrayFree(ARRAY *array) {
	if (array->buffer) {
		free(array->buffer);
		array->buffer = 0;
	}
}

char *WCharToChar(char *dest, wchar_t *src) {
	sprintf(dest, "%ws", src);
	return dest;
}

wchar_t *CharToWChar(wchar_t *dest, char *src) {
	int length = (int)strlen(src);
	dest[MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, length, dest, length)] = 0;
	return dest;
}

PROCESSENTRY32 GetProcessInfoById(DWORD pid) {
	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry)) {
		do {
			if (entry.th32ProcessID == pid) {
				CloseHandle(snapshot);
				return entry;
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return{ 0 };
}

PROCESSENTRY32 GetProcessInfoByName(wchar_t *exe_name) {
	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry)) {
		do {
			if (_wcsicmp(entry.szExeFile, exe_name) == 0) {
				CloseHandle(snapshot);
				return entry;
			}
		} while (Process32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return{ 0 };
}

MODULEENTRY32 GetModuleInfoByName(int process_id, wchar_t *module_name) {
	MODULEENTRY32 entry = { 0 };
	entry.dwSize = sizeof(MODULEENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

	if (Module32First(snapshot, &entry)) {
		do {
			if (_wcsicmp(entry.szModule, module_name) == 0) {
				CloseHandle(snapshot);
				return entry;
			}
		} while (Module32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return{ 0 };
}

THREADENTRY32 GetThreadInfoById(int thread_id) {
	THREADENTRY32 entry = { 0 };
	entry.dwSize = sizeof(THREADENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);

	if (Thread32First(snapshot, &entry)) {
		do {
			if (entry.th32ThreadID == thread_id) {
				CloseHandle(snapshot);
				return entry;
			}
		} while (Thread32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return{ 0 };
}

void SuspendProcess(int process_id) {
	THREADENTRY32 entry = { 0 };
	entry.dwSize = sizeof(THREADENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);

	if (Thread32First(snapshot, &entry)) {
		do {
			if (entry.th32OwnerProcessID == process_id) {
				HANDLE thread = OpenThread(THREAD_ALL_ACCESS, 0, entry.th32ThreadID);
				SuspendThread(thread);
				CloseHandle(thread);
			}
		} while (Thread32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
}

void ResumeProcess(int process_id) {
	THREADENTRY32 entry = { 0 };
	entry.dwSize = sizeof(THREADENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);

	if (Thread32First(snapshot, &entry)) {
		do {
			if (entry.th32OwnerProcessID == process_id) {
				HANDLE thread = OpenThread(THREAD_ALL_ACCESS, 0, entry.th32ThreadID);
				ResumeThread(thread);
				CloseHandle(thread);
			}
		} while (Thread32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
}

ULARGE_INTEGER GetThreadCreationTime(HANDLE thread) {
	FILETIME filetime, idle;

	GetThreadTimes(thread, &filetime, &idle, &idle, &idle);

	ULARGE_INTEGER time;
	time.LowPart = filetime.dwLowDateTime;
	time.HighPart = filetime.dwHighDateTime;

	return time;
}

int _thread_compare(const void *a, const void *b) {
	HANDLE thread_a = *(HANDLE *)a;
	HANDLE thread_b = *(HANDLE *)b;

	long long diff = GetThreadCreationTime(thread_a).QuadPart - GetThreadCreationTime(thread_b).QuadPart;

	return diff == 0 ? 0 : diff < 0 ? -1 : 1;
}

THREADENTRY32 GetThreadInfoByNumber(int process_id, int number) {
	ARRAY threads = ArrayNew(sizeof(HANDLE));

	THREADENTRY32 entry = { 0 };
	entry.dwSize = sizeof(THREADENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, process_id);

	if (Thread32First(snapshot, &entry)) {
		do {
			if (entry.th32OwnerProcessID == process_id) {
				HANDLE thread = OpenThread(THREAD_ALL_ACCESS, 0, entry.th32ThreadID);
				ArrayPush(&threads, &thread);
			}
		} while (Thread32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);

	qsort(threads.buffer, threads.length, threads.element_size, _thread_compare);

	THREADENTRY32 thread_info = GetThreadInfoById(GetThreadId(*(HANDLE *)ArrayGet(&threads, number)));

	for (DWORD i = 0; i < threads.length; i++) {
		CloseHandle(*(HANDLE *)ArrayGet(&threads, i));
	}

	ArrayFree(&threads);

	return thread_info;
}

void *GetThreadStackTop(int thread_id) {
	HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, 0, thread_id);
	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, GetThreadInfoById(thread_id).th32OwnerProcessID);

	if (process && thread) {
		SuspendThread(thread);

		BOOL x32;
		IsWow64Process(process, &x32);

		if (x32) {
			CONTEXT32 context = { 0 };
			context.ContextFlags = CONTEXT_SEGMENTS;

			if (GetThreadContext32(thread, &context)) {
				LDT_ENTRY32 entry = { 0 };
				if (GetThreadSelectorEntry32(thread, context.SegFs, &entry)) {
					DWORD stack = entry.BaseLow + (entry.HighWord.Bytes.BaseMid << 16) + (entry.HighWord.Bytes.BaseHi << 24);

					ReadProcessMemory(process, (void *)(stack + sizeof(stack)), &stack, sizeof(stack), NULL);

					ResumeThread(thread);
					CloseHandle(thread);
					CloseHandle(process);

					return (void *)((SINT)stack - sizeof(stack));
				}
			}
		} else {
			THREAD_BASIC_INFORMATION tbi = { 0 };
			if (!NtQueryInformationThread(thread, (THREADINFOCLASS)0, &tbi, sizeof(tbi), 0)) {
				SINT stack = 0;
				if (ReadProcessMemory(process, (void *)((SINT)tbi.TebBaseAddress + 8), &stack, sizeof(stack), 0)) {
					ResumeThread(thread);
					CloseHandle(thread);
					CloseHandle(process);

					return (void *)(stack - sizeof(stack));
				}
			}
		}

		ResumeThread(thread);
	}

	CloseHandle(thread);
	CloseHandle(process);
	return NULL;
}

void *GetThreadStack(int thread_id) {
	SINT top = (SINT)GetThreadStackTop(thread_id);
	if (!top) {
		return NULL;
	}

	DWORD process_id = GetThreadInfoById(thread_id).th32OwnerProcessID;
	MODULEENTRY32 kernel32 = GetModuleInfoByName(process_id, L"kernel32.dll");
	if (!(SINT)kernel32.hModule) {
		return NULL;
	}

	SINT min = (SINT)kernel32.modBaseAddr;
	SINT max = min + kernel32.modBaseSize;

	HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, process_id);
	if (process) {
		BOOL x32 = true;
		IsWow64Process(process, &x32);

		if (x32) {
			DWORD stack = 0;
			for (;; top -= sizeof(stack), stack = 0) {
				if (!ReadProcessMemory(process, (void *)top, &stack, sizeof(stack), NULL)) {
					break;
				}

				if (stack >= min && stack < max) {
					CloseHandle(process);
					return (void *)top;
				}
			}
		} else {
			SINT stack = 0;
			for (;; top -= sizeof(stack), stack = 0) {
				if (!ReadProcessMemory(process, (void *)top, &stack, sizeof(stack), NULL)) {
					break;
				}

				if (stack >= min && stack < max) {
					CloseHandle(process);
					return (void *)top;
				}
			}
		}
	}

	CloseHandle(process);
	return NULL;
}

DWORD GetProcessThreadCount(int process_id) {
	THREADENTRY32 entry = { 0 };
	entry.dwSize = sizeof(THREADENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);

	DWORD count = 0;
	if (Thread32First(snapshot, &entry)) {
		do {
			if (entry.th32OwnerProcessID == process_id) {
				++count;
			}
		} while (Thread32Next(snapshot, &entry));
	}

	CloseHandle(snapshot);
	return count;
}

void *GetPointer(HANDLE process, unsigned int offset_count, ...) {
	BOOL x32 = false;
	IsWow64Process(process, &x32);

	if (x32) {
		va_list offsets;
		va_start(offsets, offset_count);

		DWORD base = 0;
		for (unsigned int i = 0; i < offset_count - 1; i++) {
			ReadProcessMemory(process, (void *)((SINT)(base + va_arg(offsets, DWORD))), &base, sizeof(base), NULL);
		}

		base += va_arg(offsets, DWORD);
		va_end(offsets);

		return (void *)((SINT)base);
	} else {
		va_list offsets;
		va_start(offsets, offset_count);

		SINT base = 0;
		for (unsigned int i = 0; i < offset_count - 1; i++) {
			ReadProcessMemory(process, (void *)(base + va_arg(offsets, SINT)), &base, sizeof(base), NULL);
		}

		base += va_arg(offsets, SINT);
		va_end(offsets);

		return (void *)((SINT)base);
	}
}

DWORD ReadBuffer(HANDLE process, void *address, void *buffer, DWORD size) {
	ReadProcessMemory(process, address, buffer, size, (SIZE_T *)&size);
	return size;
}

byte ReadByte(HANDLE process, void *address) {
	byte b = 0;
	ReadProcessMemory(process, address, &b, sizeof(b), 0);
	return b;
}

short ReadShort(HANDLE process, void *address) {
	short s = 0;
	ReadProcessMemory(process, address, &s, sizeof(s), 0);
	return s;
}

int ReadInt(HANDLE process, void *address) {
	int i = 0;
	ReadProcessMemory(process, address, &i, sizeof(i), 0);
	return i;
}

long ReadLong(HANDLE process, void *address) {
	long l = 0;
	ReadProcessMemory(process, address, &l, sizeof(l), 0);
	return l;
}

float ReadFloat(HANDLE process, void *address) {
	float f = 0;
	ReadProcessMemory(process, address, &f, sizeof(f), 0);
	return f;
}

long long ReadLongLong(HANDLE process, void *address) {
	long long l = 0;
	ReadProcessMemory(process, address, &l, sizeof(l), 0);
	return l;
}

double ReadDouble(HANDLE process, void *address) {
	double d = 0;
	ReadProcessMemory(process, address, &d, sizeof(d), 0);
	return d;
}

VECTOR ReadVector(HANDLE process, void *address) {
	VECTOR v = { 0 };
	ReadProcessMemory(process, address, &v, sizeof(v), 0);
	return v;
}

bool WriteBuffer(HANDLE process, void *address, void *buffer, DWORD size) {
	return WriteProcessMemory(process, address, buffer, size, 0) != 0;
}

bool WriteByte(HANDLE process, void *address, byte value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteShort(HANDLE process, void *address, short value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteInt(HANDLE process, void *address, int value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteLong(HANDLE process, void *address, long value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteFloat(HANDLE process, void *address, float value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteLongLong(HANDLE process, void *address, long long value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteDouble(HANDLE process, void *address, double value) {
	return WriteProcessMemory(process, address, &value, sizeof(value), 0) != 0;
}

bool WriteVector(HANDLE process, void *address, VECTOR *value) {
	return WriteProcessMemory(process, address, value, sizeof(value), 0) != 0;
}

ARGUMENT ArgumentByte(byte value) {
	ARGUMENT a = { 0 };
	a.type = ARGUMENT_BYTE;
	*(byte *)&a.value = value;
	return a;
}

ARGUMENT ArgumentShort(short value) {
	ARGUMENT a = { 0 };
	a.type = ARGUMENT_SHORT;
	*(short *)&a.value = value;
	return a;
}

ARGUMENT ArgumentInt(int value) {
	ARGUMENT a = { 0 };
	a.type = ARGUMENT_INT;
	*(short *)&a.value = value;
	return a;
}

ARGUMENT ArgumentFloat(float value) {
	ARGUMENT a = { 0 };
	a.type = ARGUMENT_FLOAT;
	*(float *)&a.value = value;
	return a;
}

ARGUMENT ArgumentLongLong(long long value) {
	ARGUMENT a = { 0 };
	a.type = ARGUMENT_LONGLONG;
	a.value = value;
	return a;
}

ARGUMENT ArgumentDouble(double value) {
	ARGUMENT a = { 0 };
	a.type = ARGUMENT_DOUBLE;
	*(double *)&a.value = value;
	return a;
}

DWORD GetPushSize32(ARGUMENT *argument) {
	switch (argument->type) {
		case ARGUMENT_BYTE:
			return 2;
		case ARGUMENT_SHORT: case ARGUMENT_INT: case ARGUMENT_FLOAT:
			return 5;
		case ARGUMENT_LONGLONG: case ARGUMENT_DOUBLE:
			return 10;
	}

	return 0;
}

bool CallCDECL(HANDLE process, void *function, DWORD argument_count, ...) {
	BOOL x32;
	IsWow64Process(process, &x32);

	if (x32) {
		byte bytes[10] = { 0 };
		va_list va_args;
		ARGUMENT *args;
		LPVOID base;
		DWORD size = 6 + argument_count;
		SINT addr;

		args = (ARGUMENT *)malloc(argument_count * sizeof(ARGUMENT));
		va_start(va_args, argument_count);
		for (DWORD i = 0; i < argument_count; ++i) {
			args[i] = va_arg(va_args, ARGUMENT);
			size += GetPushSize32(&args[i]);
			if (args[i].type == ARGUMENT_LONGLONG || args[i].type == ARGUMENT_DOUBLE) {
				++size;
			}
		}
		va_end(va_args);

		base = VirtualAllocEx(process, 0, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!base) {
			free(args);
			return false;
		}
		addr = (SINT)base;

		for (int i = (int)argument_count - 1; i > -1; --i) {
			DWORD push_size = GetPushSize32(&args[i]);

			switch (args[i].type) {
				case ARGUMENT_BYTE:
					bytes[0] = 0x6A;
					bytes[1] = *(byte *)&args[i].value;
					break;
				case ARGUMENT_SHORT:
					bytes[0] = 0x68;
					*(DWORD *)&bytes[1] = *(short *)&args[i].value;
					break;
				case ARGUMENT_INT: case ARGUMENT_FLOAT:
					bytes[0] = 0x68;
					*(DWORD *)&bytes[1] = *(DWORD *)&args[i].value;
					break;
				case ARGUMENT_LONGLONG: case ARGUMENT_DOUBLE:
					bytes[0] = 0x68;
					*(DWORD *)&bytes[1] = *(DWORD *)((SINT)&args[i].value + 4);
					bytes[5] = 0x68;
					*(DWORD *)&bytes[6] = *(DWORD *)&args[i].value;
					++argument_count;
					break;
			}

			WriteProcessMemory(process, (LPVOID)addr, bytes, push_size, 0);
			addr += push_size;
		}

		bytes[0] = 0xE8;
		*(int *)&bytes[1] = (int)((SINT)function - (SINT)addr - 5);
		WriteProcessMemory(process, (LPVOID)addr, bytes, 5, 0);
		addr += 5;

		for (DWORD i = 0; i < argument_count; ++i) {
			bytes[0] = 0x58;
			WriteProcessMemory(process, (LPVOID)addr++, bytes, 1, 0);
		}

		bytes[0] = 0xC3;
		WriteProcessMemory(process, (LPVOID)addr, bytes, 1, 0);

		WaitForSingleObject(CreateRemoteThread(process, 0, 0, (LPTHREAD_START_ROUTINE)base, 0, 0, 0), INFINITE);

		VirtualFreeEx(process, base, 0, MEM_RELEASE);
		free(args);

		return true;
	}

	return false;
}

bool CallSTDCALL(HANDLE process, void *function, DWORD argument_count, ...) {
	BOOL x32;
	IsWow64Process(process, &x32);

	if (x32) {
		byte bytes[10] = { 0 };
		va_list va_args;
		ARGUMENT *args;
		LPVOID base;
		DWORD size = 6;
		SINT addr;

		args = (ARGUMENT *)malloc(argument_count * sizeof(ARGUMENT));
		va_start(va_args, argument_count);
		for (DWORD i = 0; i < argument_count; ++i) {
			args[i] = va_arg(va_args, ARGUMENT);
			size += GetPushSize32(&args[i]);
		}
		va_end(va_args);

		base = VirtualAllocEx(process, 0, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!base) {
			free(args);
			return false;
		}
		addr = (SINT)base;

		for (int i = (int)argument_count - 1; i > -1; --i) {
			DWORD push_size = GetPushSize32(&args[i]);

			switch (args[i].type) {
				case ARGUMENT_BYTE:
					bytes[0] = 0x6A;
					bytes[1] = *(byte *)&args[i].value;
					break;
				case ARGUMENT_SHORT:
					bytes[0] = 0x68;
					*(DWORD *)&bytes[1] = *(short *)&args[i].value;
					break;
				case ARGUMENT_INT: case ARGUMENT_FLOAT:
					bytes[0] = 0x68;
					*(DWORD *)&bytes[1] = *(DWORD *)&args[i].value;
					break;
				case ARGUMENT_LONGLONG: case ARGUMENT_DOUBLE:
					bytes[0] = 0x68;
					*(DWORD *)&bytes[1] = *(DWORD *)((SINT)&args[i].value + 4);
					bytes[5] = 0x68;
					*(DWORD *)&bytes[6] = *(DWORD *)&args[i].value;
					++argument_count;
					break;
			}

			WriteProcessMemory(process, (LPVOID)addr, bytes, push_size, 0);
			addr += push_size;
		}

		bytes[0] = 0xE8;
		*(int *)&bytes[1] = (int)((SINT)function - (SINT)addr - 5);
		WriteProcessMemory(process, (LPVOID)addr, bytes, 5, 0);
		addr += 5;

		bytes[0] = 0xC3;
		WriteProcessMemory(process, (LPVOID)addr, bytes, 1, 0);

		WaitForSingleObject(CreateRemoteThread(process, 0, 0, (LPTHREAD_START_ROUTINE)base, 0, 0, 0), INFINITE);

		VirtualFreeEx(process, base, 0, MEM_RELEASE);
		free(args);

		return true;
	}

	return false;
}

bool MaskCompare(char *s1, char *s2, char *mask) {
	for (; *mask; ++mask, ++s1, ++s2) {
		if (*mask == 'x' && *s1 != *s2) {
			return false;
		}
	}

	return true;
}

void *FindLocalPattern(void *base, unsigned int length, char *pattern, char *mask) {
	for (SINT l = (SINT)base + length - strlen(mask) + 1; (SINT)base < l; base = (void *)((SINT)base + 1)) {
		if (MaskCompare((char *)base, pattern, mask)) {
			return base;
		}
	}

	return 0;
}

void *FindPattern(HANDLE process, void *base, unsigned int length, char *pattern, char *mask) {
	char *buffer = (char *)malloc(length);
	if (!buffer) return 0;

	SINT addr = 0;
	if (ReadProcessMemory(process, base, buffer, length, 0)) {
		addr = (SINT)FindLocalPattern(buffer, length, pattern, mask);
		if (addr) {
			addr += (SINT)base - (SINT)buffer;
		}
	}

	free(buffer);
	return (void *)addr;
}