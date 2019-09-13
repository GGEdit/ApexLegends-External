#include "Apex.h"

VOID OnAttach(DWORD ProcessId, DWORD64 BaseAddress)
{
	Ents ents = Ents::CreateInstance(Driver, ProcessId, BaseAddress);
	Weapon weapon = Weapon::CreateInstance(Driver, ProcessId);
	Glow glow = Glow::CreateInstance(Driver, ProcessId);
	Aimbot aimbot = Aimbot::CreateInstance(Driver, ProcessId, BaseAddress);
	vector<Entity> entity;
	LocalPlayer localPlayer;
	for (;;)
	{
		if (!ents.IsEntityList())
			continue;

		ents.Update(&entity, &localPlayer);
		weapon.NoRecoil(localPlayer);
		for (int i = 0; i < entity.size(); i++)
		{
			if (entity[i].IsPlayer && !entity[i].IsTeam) {
				glow.EnableHighlight(entity[i].Address, 125.0f, 0.0f, 0.0f);
			}
			else if (!entity[i].IsPlayer)
			{
				INT lootId = entity[i].LootId;
				if (lootId == LOOT_LIST::R301
					|| lootId == LOOT_LIST::R99
					|| lootId == LOOT_LIST::WINGMAN)
				{
					//Weapon
					glow.EnableItemHighlight(entity[i].Address, 255.0f, 255.0f, 255.0f);
				}
				else if (lootId == LOOT_LIST::LIGHT_ROUNDS)
				{
					//AMMO
					glow.EnableItemHighlight(entity[i].Address, 0.0f, 255.0f, 0.0f);
				}
				else if (lootId == LOOT_LIST::BACKPACK_LV2
					|| lootId == LOOT_LIST::ARMOR_LV2
					|| lootId == LOOT_LIST::HEAVY_MAG_LV2
					|| lootId == LOOT_LIST::LIGHT_MAG_LV2)
				{
					//LV2 Item
					glow.EnableItemHighlight(entity[i].Address, 0.0f, 75.0f, 187.0f);
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
					glow.EnableItemHighlight(entity[i].Address, 226.0f, 175.0f, 255.0f);
				}
				else if (lootId == LOOT_LIST::ARMOR_LV4 ||
					lootId == LOOT_LIST::BACKPACK_LV4 ||
					lootId == LOOT_LIST::HELMET_LV4 ||
					lootId == LOOT_LIST::KNOCKDOWN_LV4 ||
					lootId == LOOT_LIST::BARREL_LV4)
				{
					//LV4 Item
					glow.EnableItemHighlight(entity[i].Address, 255.0f, 175.0f, 64.0f);
				}
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
