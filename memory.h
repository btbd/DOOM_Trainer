#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <tlhelp32.h>
#include <Winternl.h>

#pragma comment(lib, "Ntdll.lib")

typedef LONG NTSTATUS;
typedef DWORD KPRIORITY;
typedef WORD UWORD;

typedef struct _CLIENT_ID {
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS                ExitStatus;
	PVOID                   TebBaseAddress;
	CLIENT_ID               ClientId;
	KAFFINITY               AffinityMask;
	KPRIORITY               Priority;
	KPRIORITY               BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

#ifdef _WIN64
#define SINT unsigned long long

#define CONTEXT32 WOW64_CONTEXT
#define LDT_ENTRY32 WOW64_LDT_ENTRY
#define GetThreadContext32 Wow64GetThreadContext
#define GetThreadSelectorEntry32 Wow64GetThreadSelectorEntry

#else
#define SINT unsigned int

#define CONTEXT32 CONTEXT
#define LDT_ENTRY32 LDT_ENTRY
#define GetThreadContext32 GetThreadContext
#define GetThreadSelectorEntry32 GetThreadSelectorEntry

#endif

typedef struct {
	float x, y, z;
} VECTOR;

enum {
	ARGUMENT_BYTE = 0,
	ARGUMENT_SHORT,
	ARGUMENT_INT,
	ARGUMENT_FLOAT,
	ARGUMENT_LONGLONG,
	ARGUMENT_DOUBLE
};

typedef struct {
	byte type;
	unsigned long long value;
} ARGUMENT;

typedef struct {
	void *buffer;
	unsigned char element_size;
	unsigned int length, allocated;
} ARRAY;

ARRAY ArrayNew(unsigned char element_size);
void *ArrayGet(ARRAY *array, DWORD index);
void *ArraySet(ARRAY *array, DWORD index, void *element);
void *ArrayPush(ARRAY *array, void *element);
void ArrayPop(ARRAY *array, void *out);
void ArrayMerge(ARRAY *dest, ARRAY *array);
void ArrayFree(ARRAY *array);

char *WCharToChar(char *dest, wchar_t *src);
wchar_t *CharToWChar(wchar_t *dest, char *src);

PROCESSENTRY32 GetProcessInfoById(DWORD pid);
PROCESSENTRY32 GetProcessInfoByName(wchar_t *exe_name);
MODULEENTRY32 GetModuleInfoByName(int process_id, wchar_t *module_name);
THREADENTRY32 GetThreadInfoById(int thread_id);
void SuspendProcess(int process_id);
void ResumeProcess(int process_id);
ULARGE_INTEGER GetThreadCreationTime(HANDLE thread);
THREADENTRY32 GetThreadInfoByNumber(int process_id, int number);
void *GetThreadStackTop(int thread_id);
void *GetThreadStack(int thread_id);
DWORD GetProcessThreadCount(int process_id);
void *GetPointer(HANDLE process, unsigned int offset_count, ...);

DWORD ReadBuffer(HANDLE process, void *address, void *buffer, DWORD size);
byte ReadByte(HANDLE process, void *address);
short ReadShort(HANDLE process, void *address);
int ReadInt(HANDLE process, void *address);
long ReadLong(HANDLE process, void *address);
float ReadFloat(HANDLE process, void *address);
long long ReadLongLong(HANDLE process, void *address);
double ReadDouble(HANDLE process, void *address);
VECTOR ReadVector(HANDLE process, void *address);

bool WriteBuffer(HANDLE process, void *address, void *buffer, DWORD size);
bool WriteByte(HANDLE process, void *address, byte value);
bool WriteShort(HANDLE process, void *address, short value);
bool WriteInt(HANDLE process, void *address, int value);
bool WriteLong(HANDLE process, void *address, long value);
bool WriteFloat(HANDLE process, void *address, float value);
bool WriteLongLong(HANDLE process, void *address, long long value);
bool WriteDouble(HANDLE process, void *address, double value);
bool WriteVector(HANDLE process, void *address, VECTOR *value);

ARGUMENT ArgumentByte(byte value);
ARGUMENT ArgumentShort(short value);
ARGUMENT ArgumentInt(int value);
ARGUMENT ArgumentFloat(float value);
ARGUMENT ArgumentLongLong(long long value);
ARGUMENT ArgumentDouble(double value);
DWORD GetPushSize32(ARGUMENT *argument);

bool CallCDECL(HANDLE process, void *function, DWORD argument_count, ...);
bool CallSTDCALL(HANDLE process, void *function, DWORD argument_count, ...);

bool MaskCompare(char *s1, char *s2, char *mask);
void *FindLocalPattern(void *base, unsigned int length, char *pattern, char *mask);
void *FindPattern(HANDLE process, void *base, unsigned int length, char *pattern, char *mask);
