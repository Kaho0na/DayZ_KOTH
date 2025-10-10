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
		string filePath = EXPANSION_DAYZ_KOTH_PLAYERS + playerID + ".json";

		// Ensure the KOTH player folder exists
		if (!FileExist(EXPANSION_DAYZ_KOTH_PLAYERS))
			MakeDirectory(EXPANSION_DAYZ_KOTH_PLAYERS);

		DayZ_KOTH_Players playerData;

		// If file exists, load it; if not, create it
		if (FileExist(filePath))
		{
			playerData = DayZ_KOTH_Players.Load(playerID);
			Print("[DayZ_KOTH] Loaded existing player file for " + identity.GetName());
		}
		else
		{
			playerData = new DayZ_KOTH_Players();
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
		Dayz_KOTH_Zones zone = Dayz_KOTH_Zones.Load("Chernogorsk");

		vector spawnPos;

		if (playerData.LastTeamSelection == "East")
		{
			spawnPos = Vector(zone.EastSpawnBuilding[0], zone.EastSpawnBuilding[1], zone.EastSpawnBuilding[2]);
			Print("[KOTH] " + playerData.PlayerName + " spawning at EAST base: " + spawnPos);
		}
		else if (playerData.LastTeamSelection == "West")
		{
			spawnPos = Vector(zone.WestSpawnBuilding[0], zone.WestSpawnBuilding[1], zone.WestSpawnBuilding[2]);
			Print("[KOTH] " + playerData.PlayerName + " spawning at WEST base: " + spawnPos);
		}
		else
		{
			Print("[KOTH] " + playerData.PlayerName + " has no valid team; spawning randomly.");
			return;
		}

		// Wait a short delay to ensure player entity is ready
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnPlayerAtPosition, 500, false, player, spawnPos);
	}

// Handles safe player teleportation with ground detection and random offset
void SpawnPlayerAtPosition(PlayerBase player, vector spawnPos)
{
	if (!player)
		return;

	//! ────────────── RANDOM OFFSET ──────────────
	float offsetX = Math.RandomFloatInclusive(-1.5, 1.5);
	float offsetZ = Math.RandomFloatInclusive(-1.5, 1.5);

	spawnPos[0] = spawnPos[0] + offsetX;
	spawnPos[2] = spawnPos[2] + offsetZ;

	//! ────────────── GROUND / PLATFORM CORRECTION ──────────────
	vector rayStart = spawnPos + "0 50 0";   // 50m above
	vector rayEnd   = spawnPos + "0 -50 0";  // 50m below
	vector hitPos;
	vector hitNormal;
	int hitComp;

	bool hit = DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComp, null, null, player, false, false, ObjIntersectView, 0.5);

	if (hit)
	{
		// Lift slightly to prevent clipping into surface
		hitPos[1] = hitPos[1] + 0.15;
		player.SetPosition(hitPos);
		Print("[KOTH] Safe-spawned player at: " + hitPos + " (offset " + offsetX + ", " + offsetZ + ")");
	}
	else
	{
		// Fallback offset if no raycast hit
		spawnPos[1] = spawnPos[1] + 1.0;
		player.SetPosition(spawnPos);
		Print("[KOTH] Fallback spawn at: " + spawnPos + " (offset " + offsetX + ", " + offsetZ + ")");
	}

	player.MessageStatus("[KOTH] You have spawned safely at your team's base!");
}


}