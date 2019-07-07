#include "KeInterface.h"
#include "Process.h"

#define EntityList_Offset 0x1F9AE68
#define EntityCount_Offset 0xC00B508
#define LocalPlayer_Offset 0x234DEE8
#define PlayerNameList_Offset 0xC1BBA40
#define BulletSpeed_Offset 0x1BB4
#define TeamId_Offset 0x3E4

DWORD ProcessId = NULL;
DWORD64 BaseAddress = NULL;
DWORD64 EntityList = NULL;
KeInterface Driver("\\\\.\\*******");

INT GetEntityCount()
{
	return Driver.ReadVirtualMemory<INT>(ProcessId, BaseAddress + EntityCount_Offset);
}

DWORD64 GetEntityById(int Entity, DWORD64 EntityList)
{
	return Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityList + (Entity << 5));
}

VOID EnableHighlight(DWORD64 Entity, FLOAT r, FLOAT g, FLOAT b)
{
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x380, 1);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x2F0, 1);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B0, r);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B4, g);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B8, b);
	for (INT offset = 0x2B0; offset <= 0x2C8; offset += 0x4)
		Driver.WriteVirtualMemory(ProcessId, Entity + offset, FLT_MAX);

	Driver.WriteVirtualMemory(ProcessId, Entity + 0x2DC, FLT_MAX);
}

BOOL IsSameTeam(DWORD64 LocalPlayer, DWORD64 Entity)
{
	DWORD myTeamId = Driver.ReadVirtualMemory<DWORD>(ProcessId, LocalPlayer + TeamId_Offset);
	DWORD entityTeamId = Driver.ReadVirtualMemory<DWORD>(ProcessId, Entity + TeamId_Offset);
	if (myTeamId == entityTeamId)
		return TRUE;

	return FALSE;
}

DWORD64 GetLocalPlayer()
{
	return Driver.ReadVirtualMemory<DWORD64>(ProcessId, BaseAddress + LocalPlayer_Offset);
}

VOID OnAttach()
{
	DWORD64 LocalPlayer = NULL;
	for (;;)
	{
		LocalPlayer = GetLocalPlayer();
		if (!LocalPlayer)
			continue;

		for (INT i = 0; i < GetEntityCount(); i++)
		{
			DWORD64 Entity = GetEntityById(i, EntityList);
			if (Entity == NULL)
				continue;

			DWORD64 EntityHandle = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Entity + 0x500);
			DWORD64 EntityName = Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityHandle);
			if (EntityName == 0x0000726579616c70
				&& LocalPlayer != Entity
				&& !IsSameTeam(LocalPlayer, Entity))
			{
				EnableHighlight(Entity, 0.0f, 0.0f, 125.f);
			}
		}
		Sleep(100);
	}
}

VOID Load()
{
	if (Driver.IsInvalidDriver())
	{
		system("***********************");
		Driver.ConnectDriver();
	}

	printf("Waiting for ApexLegends..\n");
	while ((ProcessId = GetPId("r5apex.exe")) == NULL)
	{
		Sleep(100);
	}
	printf("ProcessId:%d\n", ProcessId);

	while ((BaseAddress = reinterpret_cast<DWORD64>(Driver.GetClientModule(ProcessId))) == NULL)
	{
		Sleep(100);
	}
	printf("ModuleBase:0x%p\n", BaseAddress);

	EntityList = BaseAddress + EntityList_Offset;
	while (!Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityList))
	{
		Sleep(100);
	}

	OnAttach();
}

int main()
{
	Load();
	system("pause");

	return 0;
}
