/**
 * KOTH_NPCBase.c
 *
 * Base class for all KOTH NPCs - inherits from ExpansionNPCBase
 * Config will make these inherit from survivor models
 */

class KOTH_NPCBase: ExpansionNPCBase
{
	void KOTH_NPCBase()
	{
		Print("[KOTH_NPCBase] Constructor called");
	}

	void ~KOTH_NPCBase()
	{
	}
};

class KOTH_NPCMirek: KOTH_NPCBase
{
	void KOTH_NPCMirek()
	{
		Print("[KOTH_NPCMirek] Shop NPC constructed");
	}
};

class KOTH_NPCDenis: KOTH_NPCBase
{
	void KOTH_NPCDenis()
	{
		Print("[KOTH_NPCDenis] Clothes NPC constructed");
	}
};

class KOTH_NPCBoris: KOTH_NPCBase
{
	void KOTH_NPCBoris()
	{
		Print("[KOTH_NPCBoris] Vehicles NPC constructed");
	}
};

class KOTH_NPCCyril: KOTH_NPCBase
{
	void KOTH_NPCCyril()
	{
		Print("[KOTH_NPCCyril] Perks NPC constructed");
	}
};