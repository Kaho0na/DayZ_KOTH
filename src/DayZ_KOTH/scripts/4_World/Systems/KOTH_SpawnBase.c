// KOTH_SpawnBase.c — spawns East/West spawn compounds from in-code definition (no XML)

// Simple piece definition: class name (or p3d path), relative offsets, and yaw (deg)
class KOTHSpawnPiece
{
	string type;
	float dx;
	float dz;
	float dy;
	float yaw;

	void KOTHSpawnPiece(string t, float _dx, float _dz, float _dy, float _yaw)
	{
		type = t;
		dx = _dx;
		dz = _dz;
		dy = _dy;
		yaw = _yaw;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Your spawn compound (relative to anchor bldr_Platform1_Block at 0,0,0)
// Values are copied from your <group> listing.
// ─────────────────────────────────────────────────────────────────────────────
static ref array<ref KOTHSpawnPiece> KOTH_SPAWN_COMPOUND = {

	// Anchor (base pivot)
	new KOTHSpawnPiece("bldr_Platform1_Block", 0.00000, 0.00000, 0.00000, -54.0000),

	// Structure pieces (relative to anchor)
	new KOTHSpawnPiece("bldr_Platform1_Stairs_20",    9.64941,  -2.21118,  1.72863,  -54.0000),
	new KOTHSpawnPiece("bldr_Platform2_Stairs_30",   -7.45020,   5.36890,  0.86604,  126.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",           4.34424,  -2.12622,  3.95457,  126.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",          -5.33643,   0.58814,  3.95457,  126.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",          -2.17676,   4.96582,  3.95457,  -54.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",          -5.21143,  -2.11182,  3.95457,   36.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",           3.49951,   3.37842,  3.95457, -144.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",           5.23975,   2.10474,  3.95457, -144.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_Pole",        1.65723,   4.70557,  3.97192, -144.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",           0.34619,   5.63794,  3.95457,   36.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",          -2.05811,  -4.37134,  3.95457, -144.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",          -0.31787,  -5.64502,  3.95457, -144.0000),
	new KOTHSpawnPiece("bldr_Wall_Wood4_4",           2.09717,  -5.25562,  3.95457,  -53.9998),
	new KOTHSpawnPiece("bldr_Wall_Wood4_Pole",       -3.90039,  -3.04419,  3.97192, -144.0000),

	//new KOTHSpawnPiece("StaticObj_WhiteBoard",             3.72705,  -1.30811,  4.35944,  126.0000),
	//new KOTHSpawnPiece("bldr_Misc_Range_Roof",       -2.93896,  -2.21753,  4.34960,   36.0000),

	// NPCs removed - will be spawned separately with proper NPC system
	
	new KOTHSpawnPiece("bldr_misc_flagpole",         -0.55908,   5.8681,   6.66342,    0.0000),
	new KOTHSpawnPiece("bldr_prop_Flag_Bear",        -0.64502,   5.8510,   10.1842,    0.0000),

	//new KOTHSpawnPiece("ExpansionATM_1",        -2.30615,   3.86182,   3.38896,    -54)
};

// ─────────────────────────────────────────────────────────────────────────────
// Runtime spawner
// ─────────────────────────────────────────────────────────────────────────────
class KOTH_SpawnBase
{
	static ref array<Object> s_Spawned = new array<Object>();
	static ref array<ref ExpansionZone> s_SafeZones = new array<ref ExpansionZone>();
	static ref array<ref KOTH_VehicleSpawn> s_VehicleSpawners = new array<ref KOTH_VehicleSpawn>();
	static ref array<KOTH_NPCBase> s_SpawnedNPCs = new array<KOTH_NPCBase>();

	static void SpawnBaseAt(vector center, string side, float safeZoneRadius)
	{
		if (!GetGame().IsServer())
			return;

		Print("[KOTH_SpawnBase] ══════════════════════════════════════");
		Print("[KOTH_SpawnBase] Spawning base for: " + side);
		Print("[KOTH_SpawnBase] Center position: " + center.ToString());

		float baseYOffset = -2.0;
		vector platformPos;
		float platformYaw = -54.0;
		bool platformFound = false;

		foreach (KOTHSpawnPiece p: KOTH_SPAWN_COMPOUND)
		{
			string objType = p.type;

			if (p.type == "bldr_prop_Flag_Bear")
			{
				if (side == "West")
					objType = "bldr_prop_Flag_Wolf";
			}

			vector pos = Vector(p.dx + center[0], p.dy + baseYOffset + center[1], p.dz + center[2]);

			Object o = GetGame().CreateObjectEx(objType, pos, ECE_CREATEPHYSICS);
			if (!o)
			{
				Print("[KOTH] Failed to spawn piece: " + objType);
				continue;
			}

			o.SetOrientation(Vector(p.yaw, 0, 0));
			s_Spawned.Insert(o);

			if (p.type == "bldr_Platform1_Block")
			{
				platformPos = pos;
				platformYaw = p.yaw;
				platformFound = true;
				Print("[KOTH_SpawnBase] Platform found at: " + platformPos.ToString());
				Print("[KOTH_SpawnBase] Platform yaw: " + platformYaw.ToString());
			}
		}

		CreateSafeZone(center, safeZoneRadius);

		if (platformFound)
		{
			Print("[KOTH_SpawnBase] Spawning vehicle for side: " + side);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnFactionVehicle, 1000, false, center, side, platformYaw);
			
			Print("[KOTH_SpawnBase] Spawning NPCs for side: " + side);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SpawnBaseNPCs, 1500, false, center, side, baseYOffset);
		}
		else
		{
			Print("[KOTH_SpawnBase] ERROR: Platform not found! Cannot spawn vehicle or NPCs.");
		}

		Print("[KOTH_SpawnBase] ══════════════════════════════════════");
	}

	static void SpawnBaseNPCs(vector baseCenter, string side, float baseYOffset)
	{
		if (!GetGame().IsServer())
			return;
		
		Print("[KOTH_SpawnBase] Spawning NPCs at base: " + side);
		
		string loadout;
		if (side == "East")
			loadout = "EastLoadout";
		else if (side == "West")
			loadout = "WestLoadout";
		else
			loadout = "WestLoadout";
		
		Print("[KOTH_SpawnBase] Using loadout: " + loadout);
		
		KOTH_NPCBase shopNPC = SpawnNPC("KOTH_NPCMirek", baseCenter, -3.75195, -1.15649, 3.38896, 36.0, baseYOffset, loadout);
		if (shopNPC)
		{
			s_SpawnedNPCs.Insert(shopNPC);
			Print("[KOTH_SpawnBase] Shop NPC (Mirek) spawned successfully");
		}
		
		KOTH_NPCBase vehiclesNPC = SpawnNPC("KOTH_NPCBoris", baseCenter, -1.83447, -2.88159, 3.38896, 36.0, baseYOffset, loadout);
		if (vehiclesNPC)
		{
			s_SpawnedNPCs.Insert(vehiclesNPC);
			Print("[KOTH_SpawnBase] Vehicles NPC (Boris) spawned successfully");
		}
		
		KOTH_NPCBase clothesNPC = SpawnNPC("KOTH_NPCDenis", baseCenter, 1.00293, -4.79736, 3.38896, 0.0, baseYOffset, loadout);
		if (clothesNPC)
		{
			s_SpawnedNPCs.Insert(clothesNPC);
			Print("[KOTH_SpawnBase] Clothes NPC (Denis) spawned successfully");
		}
		
		Print("[KOTH_SpawnBase] All NPCs spawned. Total: " + s_SpawnedNPCs.Count());
	}

	static KOTH_NPCBase SpawnNPC(string npcClassName, vector baseCenter, float dx, float dz, float dy, float yaw, float baseYOffset, string loadoutClass)
	{
		vector npcPos = Vector(dx + baseCenter[0], dy + baseYOffset + baseCenter[1], dz + baseCenter[2]);
		vector npcOri = Vector(yaw, 0, 0);
		
		Print("[KOTH_SpawnBase] Spawning NPC: " + npcClassName + " at " + npcPos.ToString());
		
		EntityAI obj = EntityAI.Cast(ExpansionGame.CreateObjectSafe(npcClassName, npcPos));
		if (!obj)
		{
			Print("[KOTH_SpawnBase] ERROR: Failed to create NPC object: " + npcClassName);
			return null;
		}
		
		KOTH_NPCBase npc = KOTH_NPCBase.Cast(obj);
		if (!npc)
		{
			Print("[KOTH_SpawnBase] ERROR: Failed to cast to KOTH_NPCBase: " + npcClassName);
			obj.DeleteSafe();
			return null;
		}
		
		npc.SetPosition(npcPos);
		npc.SetOrientation(npcOri);
		
		ExpansionHumanLoadout.Apply(npc, loadoutClass, true);
		npc.Update();
		
		s_Spawned.Insert(npc);
		
		Print("[KOTH_SpawnBase] NPC spawned successfully: " + npcClassName);
		return npc;
	}

	static void SpawnFactionVehicle(vector baseCenter, string side, float baseYaw)
	{
		if (!GetGame().IsServer())
		{
			Print("[KOTH_SpawnBase] ERROR: SpawnFactionVehicle called on client");
			return;
		}

		string vehicleType;

		if (side == "East")
		{
			vehicleType = "Hatchback_02";
		}
		else if (side == "West")
		{
			vehicleType = "Hatchback_02_Blue";
		}
		else
		{
			Print("[KOTH_SpawnBase] ERROR: Invalid side: " + side);
			return;
		}

		Print("[KOTH_SpawnBase] Vehicle type determined: " + vehicleType);

		float offsetX = -10;
		float offsetZ = 10;
		
		float vehicleX = baseCenter[0] + offsetX;
		float vehicleZ = baseCenter[2] + offsetZ;
		float vehicleY = GetGame().SurfaceY(vehicleX, vehicleZ);
		vector vehiclePos = Vector(vehicleX, vehicleY, vehicleZ);
		
		Print("[KOTH_SpawnBase] Vehicle offset - X: " + offsetX.ToString() + " Z: " + offsetZ.ToString());
		Print("[KOTH_SpawnBase] Vehicle position calculated: " + vehiclePos.ToString());

		vector vehicleOrientation = Vector(baseYaw, 0, 0);
		Print("[KOTH_SpawnBase] Vehicle orientation: " + vehicleOrientation.ToString());

		Print("[KOTH_SpawnBase] Creating KOTH_VehicleSpawn instance...");
		KOTH_VehicleSpawn spawner = new KOTH_VehicleSpawn(vehicleType, vehiclePos, vehicleOrientation);

		Print("[KOTH_SpawnBase] Calling SpawnVehicle()...");
		Car spawnedCar = spawner.SpawnVehicle();

		if (spawnedCar)
		{
			Print("[KOTH_SpawnBase] Vehicle spawned successfully!");
			s_VehicleSpawners.Insert(spawner);
		}
		else
		{
			Print("[KOTH_SpawnBase] ERROR: Vehicle spawn returned null");
		}
	}

	static void CreateSafeZone(vector center, float radius)
	{
		if (!GetGame().IsServer())
			return;

		ExpansionZone zone = new ExpansionZoneCircle(ExpansionZoneType.SAFE, center, radius);
		s_SafeZones.Insert(zone);

		Print("[KOTH] Safe zone created at: " + center.ToString() + " with radius: " + radius.ToString());
	}

	static void SpawnBases(vector eastCenter, vector westCenter, float eastRadius, float westRadius)
	{
		SpawnBaseAt(eastCenter, "East", eastRadius);
		SpawnBaseAt(westCenter, "West", westRadius);
	}

	static void DespawnAll()
	{
		if (!GetGame().IsServer())
			return;

		Print("[KOTH_SpawnBase] Despawning all bases, vehicles, and NPCs...");

		for (int n = s_SpawnedNPCs.Count() - 1; n >= 0; n--)
		{
			KOTH_NPCBase npc = s_SpawnedNPCs[n];
			if (npc)
			{
				Print("[KOTH_SpawnBase] Deleting NPC: " + npc.GetType());
				GetGame().ObjectDelete(npc);
			}
			s_SpawnedNPCs.Remove(n);
		}

		for (int k = s_VehicleSpawners.Count() - 1; k >= 0; k--)
		{
			KOTH_VehicleSpawn vs = s_VehicleSpawners[k];
			if (vs)
			{
				vs.StopRespawnTimer();
				vs.CleanupVehicle();
				delete vs;
			}
			s_VehicleSpawners.Remove(k);
		}

		for (int i = s_Spawned.Count() - 1; i >= 0; i--)
		{
			Object o = s_Spawned[i];
			if (o)
			{
				EntityAI entity = EntityAI.Cast(o);
				if (entity)
					GetGame().ObjectDelete(entity);
				else
					GetGame().ObjectDelete(o);
			}
			s_Spawned.Remove(i);
		}

		for (int j = s_SafeZones.Count() - 1; j >= 0; j--)
		{
			ExpansionZone z = s_SafeZones[j];
			if (z)
				delete z;
			s_SafeZones.Remove(j);
		}

		Print("[KOTH_SpawnBase] All bases, vehicles, and NPCs despawned");
	}

	static array<ref KOTH_VehicleSpawn> GetVehicleSpawners()
	{
		return s_VehicleSpawners;
	}
}

static void KOTH_SpawnBasesForZone(KOTH_ZoneData zone)
{
	if (!GetGame().IsServer()) return;

	vector east = zone.GetEastSpawnBuilding();
	vector west = zone.GetWestSpawnBuilding();
	float eastRadius = zone.GetEastSafeZoneRadius();
	float westRadius = zone.GetWestSafeZoneRadius();

	KOTH_SpawnBase.SpawnBases(east, west, eastRadius, westRadius);
}

static void KOTH_DespawnBases()
{
	KOTH_SpawnBase.DespawnAll();
}