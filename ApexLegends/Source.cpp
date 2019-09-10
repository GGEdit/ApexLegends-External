#include "Apex.h"

VOID OnAttach(DWORD ProcessId, DWORD64 BaseAddress)
{
	DWORD64 EntityList = BaseAddress + ENTITY_LIST_OFFSET;
	Entity entity = Entity::CreateInstance(ProcessId, BaseAddress);
	Weapon weapon = Weapon::CreateInstance(ProcessId, EntityList);
	Glow glow = Glow::CreateInstance(ProcessId, EntityList);
	DWORD64 LocalPlayer = NULL;

	while (!Driver.ReadVirtualMemory<DWORD64>(ProcessId, EntityList))
	{
		Sleep(100);
	}
	for (;;)
	{
		LocalPlayer = entity.GetLocalPlayer();
		if (!LocalPlayer)
			continue;

		weapon.NoRecoil(LocalPlayer);
		//weapon.BulletSpeed(LocalPlayer);

		DWORD64 Next = EntityList;
		int emergencyExit = 0;
		while (Next != NULL && emergencyExit < 10000)
		{
			emergencyExit++;
			DWORD64 Entity = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Next);
			Next = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Next + 0x18);
			if (!Entity)
				continue;

			DWORD64 EntityHandle = Driver.ReadVirtualMemory<DWORD64>(ProcessId, Entity + ENTITY_HANDLE_OFFSET);
			string EntityName = Driver.ReadVirtualMemoryString(ProcessId, EntityHandle);
			if (EntityName.find("prop_sur") != string::npos)
			{
				int lootId = entity.GetLootID(Entity);
				if (lootId == LOOT_LIST::R301
					|| lootId == LOOT_LIST::R99
					|| lootId == LOOT_LIST::WINGMAN)
				{
					//Weapon
					glow.EnableItemHighlight(Entity, 255.0f, 255.0f, 255.0f);
				}
				else if (lootId == LOOT_LIST::LIGHT_ROUNDS)
				{
					//AMMO
					glow.EnableItemHighlight(Entity, 0.0f, 255.0f, 0.0f);
				}
				else if (lootId == LOOT_LIST::BACKPACK_LV2
					|| lootId == LOOT_LIST::ARMOR_LV2
					|| lootId == LOOT_LIST::HEAVY_MAG_LV2
					|| lootId == LOOT_LIST::LIGHT_MAG_LV2)
				{
					//LV2 Item
					glow.EnableItemHighlight(Entity, 0.0f, 75.0f, 187.0f);
				}
				else if (lootId == LOOT_LIST::ARMOR_LV3
					|| lootId == LOOT_LIST::BACKPACK_LV3
					|| lootId == LOOT_LIST::HEAVY_MAG_LV3
					|| lootId == LOOT_LIST::LIGHT_MAG_LV3
					|| lootId == LOOT_LIST::BARREL_LV3
					|| lootId == LOOT_LIST::STOCK_LV3
					|| lootId == LOOT_LIST::HAMMERPOINT
					|| lootId == LOOT_LIST::DISRUPTOR)
				{
					//LV3 Item
					glow.EnableItemHighlight(Entity, 226.0f, 175.0f, 255.0f);
				}
				else if (lootId == LOOT_LIST::ARMOR_LV4 ||
					lootId == LOOT_LIST::BACKPACK_LV4 ||
					lootId == LOOT_LIST::HELMET_LV4 ||
					lootId == LOOT_LIST::KNOCKDOWN_LV4 ||
					lootId == LOOT_LIST::BARREL_LV4)
				{
					//LV4 Item
					glow.EnableItemHighlight(Entity, 255.0f, 175.0f, 64.0f);
				}
			}
			if (EntityName == "player"
				&& LocalPlayer != Entity
				&& !entity.IsSameTeam(LocalPlayer, Entity))
			{
				glow.EnableHighlight(Entity, 125.0f, 0.0f, 0.0f);
			}
		}
	}
}

int main()
{
	DWORD ProcessId = NULL;
	DWORD64 BaseAddress = NULL;
	Driver = KeInterface::CreateInstance("\\\\.\\******");
	if (Driver.IsInvalidDriver())
	{
		system("***");
		Driver.ConnectDriver();
	}
	while ((ProcessId = GetPId("***.exe")) == NULL)
	{
		Sleep(100);
	}
	printf("Found GameProcess..!\n");

	while ((BaseAddress = reinterpret_cast<DWORD64>(Driver.GetClientModule(ProcessId))) == NULL)
	{
		Sleep(100);
	}
	printf("ModuleBase:%p\n", BaseAddress);
	OnAttach(ProcessId, BaseAddress);

	return 0;
}
