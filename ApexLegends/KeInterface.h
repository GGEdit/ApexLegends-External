#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
using namespace std;

#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_WRITE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_GET_MODULE_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0703, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_CLEAR_UNLOADEDDRIVER_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0704, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _KERNEL_READ_REQUEST
{
	ULONG ProcessId;
	ULONG AgentId;
	ULONGLONG Address;
	PVOID Buffer;
	SIZE_T Size;
} KERNEL_READ_REQUEST, *PKERNEL_READ_REQUEST;

typedef struct _KERNEL_WRITE_REQUEST
{
	ULONG ProcessId;
	ULONG AgentId;
	ULONGLONG Address;
	PVOID Buffer;
	SIZE_T Size;
} KERNEL_WRITE_REQUEST, *PKERNEL_WRITE_REQUEST;

typedef struct _KERNEL_GET_MODULE_REQUEST
{
	ULONG ProcessId;
	PVOID Address;
} KERNEL_GET_MODULE_REQUEST, *PKERNEL_GET_MODULE_REQUEST;

typedef struct _Vector3
{
	float x;
	float y;
	float z;
} Vector3;

typedef struct _RGB
{
	float x;
	float y;
	float z;
} RGB;

DWORD GetPId(string processName)
{
	PROCESSENTRY32 processInfo;
	HANDLE processSnapshot;
	processInfo.dwSize = sizeof(processInfo);

	processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processSnapshot);
	return 0;
}

class KeInterface
{
public:
	HANDLE hDriver;
	KeInterface(LPCSTR RegistryPath)
	{
		hDriver = CreateFileA(RegistryPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	}

	template <typename T>
	T ReadVirtualMemory(ULONG ProcessId, DWORD64 ReadAddress)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return FALSE;

		KERNEL_READ_REQUEST ReadRequest;
		ReadRequest.ProcessId = ProcessId;
		ReadRequest.AgentId = GetCurrentProcessId();
		ReadRequest.Address = ReadAddress;
		ReadRequest.Size = sizeof(T);

		if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadRequest, sizeof(ReadRequest), &ReadRequest, sizeof(ReadRequest), 0, 0))
			return reinterpret_cast<T>(ReadRequest.Buffer);

		return FALSE;
	}

	template <typename T>
	BOOL WriteVirtualMemory(ULONG ProcessId, DWORD64 WriteAddress, T WriteValue)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;

		KERNEL_WRITE_REQUEST  WriteRequest;
		WriteRequest.ProcessId = ProcessId;
		WriteRequest.AgentId = GetCurrentProcessId();
		WriteRequest.Address = WriteAddress;
		WriteRequest.Buffer = reinterpret_cast<PVOID>(&WriteValue);
		WriteRequest.Size = sizeof(WriteValue);

		if (DeviceIoControl(hDriver, IO_WRITE_REQUEST, &WriteRequest, sizeof(WriteRequest), 0, 0, 0, 0))
			return TRUE;

		return FALSE;
	}

	PVOID GetClientModule(DWORD processId)
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return false;

		KERNEL_GET_MODULE_REQUEST moduleRequest;
		moduleRequest.ProcessId = processId;

		if (DeviceIoControl(hDriver, IO_GET_MODULE_REQUEST, &moduleRequest, sizeof(moduleRequest), &moduleRequest, sizeof(moduleRequest), 0, 0))
			return moduleRequest.Address;

		return false;
	}

	BOOL ClearUnloadedDriver()
	{
		if (hDriver == INVALID_HANDLE_VALUE)
			return FALSE;

		if (DeviceIoControl(hDriver, IO_CLEAR_UNLOADEDDRIVER_REQUEST, NULL, NULL, NULL, NULL, NULL, NULL))
			return TRUE;

		return FALSE;
	}
};
