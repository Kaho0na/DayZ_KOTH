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
		// Example: loads from $profile/Expansion/KOTH_Zones/Chernogorsk.json
		KOTH_ZoneData zone = KOTH_ZoneData.Load("Chernogorsk.json");
		if (!zone)
		{
			Print("[KOTH] ERROR: Could not load zone 'Chernogorsk.json'. No bases spawned.");
			return;
		}

		// Spawn ONLY the East & West spawn buildings (fixed facing, no players, no safezones)
		KOTH_SpawnBasesForZone(zone);

		Print("[KOTH] Spawned East/West bases for zone: " + zone.GetZoneName());
	}
}