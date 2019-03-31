#include "KeInterface.h"

#define EntityList_Offset 0x1F96D88
#define EntityCount_Offset 0xC006788
#define LocalPlayerId_Offset 0x1747EFC
#define LocalPlayer_Offset 0x2349E08
#define PlayerNameList_Offset 0xC1B6CC0
#define BulletSpeed_Offset 0x1BB4

DWORD ProcessId;
DWORD64 BaseAddress;
KeInterface Driver("\\\\.\\FuckEAC");

INT GetEntityCount()
{
	return Driver.ReadVirtualMemory<INT>(ProcessId, BaseAddress + EntityCount_Offset);
}

DWORD64 GetEntityById(int Entity, DWORD64 EntityList)
{
	return Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityList + (Entity << 5));
}

VOID EnableHighlight(DWORD64 Entity, float r, float g, float b)
{
	//Enable Glow
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x380, 1);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x2F0, 1);

	//Setting Color
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B0, r);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B4, g);
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x1B8, b);

	//Glow Time Max
	for (int offset = 0x2B0; offset <= 0x2C8; offset += 0x4)
		Driver.WriteVirtualMemory(ProcessId, Entity + offset, FLT_MAX);

	//Distance Max
	Driver.WriteVirtualMemory(ProcessId, Entity + 0x2DC, FLT_MAX);
}

BOOL IsSameTeam(DWORD64 LocalPlayer, DWORD64 Entity)
{
	DWORD myTeamId = Driver.ReadVirtualMemory<DWORD>(ProcessId, LocalPlayer + 0x3E4);
	DWORD entityTeamNum = Driver.ReadVirtualMemory<DWORD>(ProcessId, Entity + 0x3E4);
	if (myTeamId == entityTeamNum)
		return TRUE;

	return FALSE;
}

DWORD64 GetLocalPlayer()
{
	return Driver.ReadVirtualMemory<DWORD64>(ProcessId, BaseAddress + LocalPlayer_Offset);
}

DWORD64 GetLocalPlayerByEntityId(DWORD ProcessId, DWORD64 BaseAddress, DWORD64 EntityList)
{
	int LocalPlayerId = Driver.ReadVirtualMemory<int>(ProcessId, BaseAddress + LocalPlayerId_Offset);
	int EntityCount = GetEntityCount();
	printf("%d\n", EntityCount);
	for (int i = 0; i < EntityCount; i++)
	{
		DWORD64 Entity = GetEntityById(EntityList, i);
		if (!Entity)
			continue;

		DWORD64 EntityId = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Entity + 0x8);
		DWORD64 EntityHandle = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Entity + 0x500);
		DWORD64 EntityName = Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityHandle);
		char *Identifier = reinterpret_cast<char*>(EntityName);
		const char* add = reinterpret_cast<const char*>(&Identifier);
		if (!strcmp(add, "player") && LocalPlayerId == EntityId)
			return Entity;
	}

	return NULL;
}

VOID OnAttach()
{
	DWORD64 BaseEntity = NULL;
	DWORD64 EntityList = BaseAddress + EntityList_Offset;
	DWORD64 LocalPlayer = NULL;

	while (!BaseEntity)
	{
		BaseEntity = Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityList);
	}
	printf("BaseEntity取得:%p\n", BaseEntity);

	while (!LocalPlayer)
	{
		LocalPlayer = GetLocalPlayer();
	}
	printf("LocalPlayer取得:%p\n", LocalPlayer);

	for (;;)
	{
		for (int i = 0; i < 100; i++)
		{
			DWORD64 Entity = GetEntityById(i, EntityList);
			if (Entity == NULL)
				continue;

			DWORD64 EntityHandle = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Entity + 0x500);
			DWORD64 EntityName = Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityHandle);
			if (EntityName == 0x0000726579616c70)
				EnableHighlight(Entity, 120.0f, 0.f, 0.f);
		}
		Sleep(100);
	}
}

VOID Load()
{
	if (Driver.hDriver == INVALID_HANDLE_VALUE)
	{
		printf("ドライバの初期化に失敗\n");
		return;
	}

	ProcessId = GetPId("r5apex.exe");
	BaseAddress = reinterpret_cast<DWORD64>(Driver.GetClientModule(ProcessId));
	if (!ProcessId || !BaseAddress)
	{
		printf("ゲームプロセスへアタッチ失敗\n");
		return;
	}

	printf("ゲームプロセスID取得: %d\n", ProcessId);
	printf("プロセスハンドル取得: 0x%p\n", BaseAddress);
	OnAttach();
}

int main()
{
	Load();
	system("pause");

	return 0;
}
