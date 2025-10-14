modded class MissionServer
{
	override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
	{
		super.InvokeOnConnect(player, identity);

		if (!GetGame().IsDedicatedServer())
			return;

		if (!identity)
			return;

		string playerID = identity.GetId(); // Unique persistent ID (same used by Expansion ATM)
		string filePath = EXPANSION_KOTH_Players + playerID + ".json";

		// Ensure the KOTH player folder exists
		if (!FileExist(EXPANSION_KOTH_Players))
			MakeDirectory(EXPANSION_KOTH_Players);

		KOTH_Players playerData;

		// If file exists, load it; if not, create it
		if (FileExist(filePath))
		{
			playerData = KOTH_Players.Load(playerID);
			Print("[DayZ_KOTH] Loaded existing player file for " + identity.GetName());
		}
		else
		{
			playerData = new KOTH_Players();
			playerData.m_FileName = playerID;
			playerData.PlayerID = playerID;
			playerData.PlayerName = identity.GetName();
			playerData.TotalMoneyinBank = 0;
			playerData.TotalExperienceEarned = 0;
			playerData.CurrentLevel = 1;
			playerData.TotalTimePlayed = 0;
			playerData.LongestKill = 0;
			playerData.LongestHeadshot = 0;
			playerData.HighestKillstreak = 0;
			playerData.TotalEnemiesKilled = 0;
			playerData.LastTeamSelection = "East";

			playerData.Save();

			Print("[DayZ_KOTH] Created new player file for " + identity.GetName() + " (" + playerID + ")");
		}

		
		//! ────────────────────────────────────────────────
		//!  KOTH TEAM SPAWNING LOGIC
		//! ────────────────────────────────────────────────

		// Load KOTH zone (currently hardcoded for testing)
		KOTH_Zones zone = KOTH_Zones.Load("Chernogorsk");

		vector spawnPos;

		if (playerData.LastTeamSelection == "East")
		{
			spawnPos = Vector(zone.EastSpawnBuilding[0], zone.EastSpawnBuilding[1], zone.EastSpawnBuilding[2]);
			Print("[KOTH] " + playerData.PlayerName + " spawning at EAST base: " + spawnPos);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_SpawnUtils.SpawnPlayerAtPosition, 500, false, player, spawnPos);

		}
		else if (playerData.LastTeamSelection == "West")
		{
			spawnPos = Vector(zone.WestSpawnBuilding[0], zone.WestSpawnBuilding[1], zone.WestSpawnBuilding[2]);
			Print("[KOTH] " + playerData.PlayerName + " spawning at WEST base: " + spawnPos);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_SpawnUtils.SpawnPlayerAtPosition, 500, false, player, spawnPos);

		}
		else
		{
			Print("[KOTH] " + playerData.PlayerName + " has no team — opening selection menu, skipping spawn.");
			player.SetPosition("0 10000 0"); // move temporarily in the sky
			player.SetAllowDamage(false);    // prevent falling damage
			KOTH_TeamSelectionModule teamModule;
			CF_Modules<KOTH_TeamSelectionModule>.Get(teamModule);
			Print("[DayZ_KOTH] Retrieved team module: " + teamModule);

			if (teamModule)
			{
				Print("[DayZ_KOTH] Starting team selection for player " + identity.GetName());
				teamModule.StartTeamSelection(player, identity);
			}
			else
			{
				Print("[DayZ_KOTH] ERROR: Could not get KOTH_TeamSelectionModule instance!");
			}

			return;
		}


	}

}