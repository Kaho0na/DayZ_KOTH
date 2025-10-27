class CfgPatches
{
	class DayZ_KOTH_Dta_Core
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Characters",
			"DZ_Scripts",
			"DZ_AI",
			"DayZExpansion_Dta_Core"
		};
	};
};

class CfgVehicleClasses
{
	class KOTH_Shop_NPC
	{
		displayName="KOTH Shop NPCs";
	};
	class KOTH_Clothes_NPC
	{
		displayName="KOTH Clothes NPCs";
	};
	class KOTH_Vehicles_NPC
	{
		displayName="KOTH Vehicles NPCs";
	};
	class KOTH_Perks_NPC
	{
		displayName="KOTH Perks NPCs";
	};
};

class CfgVehicles
{
	class KOTH_NPCBase;
	class SurvivorM_Mirek;
	class SurvivorM_Denis;
	class SurvivorM_Boris;
	class SurvivorM_Cyril;

	class KOTH_NPCMirek: SurvivorM_Mirek
	{
		scope=2;
		displayName="Mirek (Shop Trader)";
		vehicleClass="KOTH_Shop_NPC";
	};
	class KOTH_NPCDenis: SurvivorM_Denis
	{
		scope=2;
		displayName="Denis (Clothes Trader)";
		vehicleClass="KOTH_Clothes_NPC";
	};
	class KOTH_NPCBoris: SurvivorM_Boris
	{
		scope=2;
		displayName="Boris (Vehicles Trader)";
		vehicleClass="KOTH_Vehicles_NPC";
	};
	class KOTH_NPCCyril: SurvivorM_Cyril
	{
		scope=2;
		displayName="Cyril (Perks Trader)";
		vehicleClass="KOTH_Perks_NPC";
	};
};