modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		if (!GetGame().IsServer()) return;

		// Give the world a moment to finish loading terrain/CE before we place objects.
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnKOTHBasesOnce, 1500, false);
	}

	// Runs once on mission start
	void SpawnKOTHBasesOnce()
	{
		// ────────────────────────────────────────────────────────────
		// Get the Zone Manager and load first available zone
		// ────────────────────────────────────────────────────────────
		KOTH_ZoneManager zoneManager;
		CF_Modules<KOTH_ZoneManager>.Get(zoneManager);

		if (!zoneManager)
		{
			Error("[KOTH] ERROR: Could not get KOTH_ZoneManager instance!");
			return;
		}

		if (!zoneManager.LoadFirstAvailableZone())
		{
			Error("[KOTH] ERROR: Failed to load initial zone. No bases spawned.");
			return;
		}

		KOTH_Zones activeZone = zoneManager.GetActiveZone();
		if (!activeZone)
		{
			Error("[KOTH] ERROR: Active zone is NULL after loading!");
			return;
		}

		// ────────────────────────────────────────────────────────────
		// Spawn ONLY the East & West spawn buildings
		// ────────────────────────────────────────────────────────────
		vector eastSpawn = Vector(activeZone.EastSpawnBuilding[0], activeZone.EastSpawnBuilding[1], activeZone.EastSpawnBuilding[2]);
		vector westSpawn = Vector(activeZone.WestSpawnBuilding[0], activeZone.WestSpawnBuilding[1], activeZone.WestSpawnBuilding[2]);

		KOTH_SpawnBase.SpawnBases(eastSpawn, westSpawn);

		Print("[KOTH] ═══════════════════════════════════════════════════");
		Print("[KOTH] Active Zone: " + zoneManager.GetActiveZoneName());
		Print("[KOTH] Zone Display Name: " + activeZone.ZoneName);
		Print("[KOTH] East/West bases spawned successfully");
		Print("[KOTH] ═══════════════════════════════════════════════════");
	}
}