#include <Windows.h>

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

class KeInterface
{
private:
	HANDLE hDriver;
	LPCSTR RegistryPath;
public:
	KeInterface(LPCSTR RegistryPath);
	void ConnectDriver();
	BOOL IsInvalidDriver();
	PVOID GetClientModule(DWORD processId);
	BOOL ClearUnloadedDriver();

	template <typename T>
	T ReadVirtualMemory(ULONG ProcessId, DWORD64 ReadAddress)
	{
		if (IsInvalidDriver())
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
		if (IsInvalidDriver())
			return FALSE;

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
};
