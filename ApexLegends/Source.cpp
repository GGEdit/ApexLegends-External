#include "KeInterface.h"

#define EntityList_Offset 0x1F96D88
DWORD ProcessId;
DWORD64 BaseAddress;
KeInterface Driver("\\\\.\\FuckEAC");

DWORD64 GetEntityById(int Entity, DWORD64 Base)
{
	DWORD64 EntityList = Base + EntityList_Offset;
	DWORD64 BaseEntity = Driver.ReadVirtualMemory(ProcessId, EntityList, sizeof(DWORD64));
	if (!BaseEntity)
		return NULL;

	return Driver.ReadVirtualMemory(ProcessId, EntityList + (Entity << 5), sizeof(DWORD64));
}

VOID EnableHighlight(DWORD64 Entity, float r, float g, float b)
{
	//Enable Glow
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x380, 1, sizeof(bool));
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x2F0, 1, sizeof(int));

	//Setting Color
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B0, r, sizeof(float));
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B4, g, sizeof(float));
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B8, b, sizeof(float));

	//Glow Time Max
	for (int offset = 0x2B0; offset <= 0x2C8; offset += 0x4)
		Driver.WriteVirtualMemory(ProcessId, Entity + offset, FLT_MAX, sizeof(float));

	//Distance Max
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x2DC, FLT_MAX, sizeof(float));
}

VOID Load()
{
	while (1) 
	{

		for (int i = 0; i < 100; i++)
		{
			DWORD64 Entity = GetEntityById(i, BaseAddress);
			if (Entity == 0)
				continue;

			DWORD64 EntityHandle = Driver.ReadVirtualMemory(ProcessId, Entity + 0x500, sizeof(DWORD64));
			DWORD64 EntityName = Driver.ReadVirtualMemory(ProcessId, EntityHandle, sizeof(DWORD64));
			char *Identifier = reinterpret_cast<char*>(EntityName);
			const char* add = reinterpret_cast<const char*>(&Identifier);
			std::string str = add;
			if (str.find("player") != str.npos)
				EnableHighlight(Entity, 120.0f, 0.f, 0.f);
		}
		Sleep(100);
	}
}

BOOL Init()
{
	if (Driver.hDriver == INVALID_HANDLE_VALUE)
		return FALSE;

	ProcessId = GetPId("r5apex.exe");
	BaseAddress = reinterpret_cast<DWORD64>(Driver.GetClientModule(ProcessId));
	if (!ProcessId || !BaseAddress)
		return FALSE;

	return TRUE;
}

int main()
{
	if (Init())
		Load();

	system("pause");

	return 0;
}