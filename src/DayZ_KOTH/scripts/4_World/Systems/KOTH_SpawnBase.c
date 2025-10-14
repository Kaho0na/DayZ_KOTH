

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

	new KOTHSpawnPiece("StaticObj_WhiteBoard",             3.72705,  -1.30811,  4.35944,  126.0000),
	new KOTHSpawnPiece("bldr_Misc_Range_Roof",       -2.93896,  -2.21753,  4.34960,   36.0000),

	new KOTHSpawnPiece("SurvivorM_Boris",            -3.75195,  -1.15649,  3.38896,   36.0000),
	new KOTHSpawnPiece("SurvivorM_Mirek",            -1.83447,  -2.88159,  3.38896,   36.0000),
	new KOTHSpawnPiece("SurvivorM_Denis",             1.00293,  -4.79736,  3.38896,    0.0000),

	new KOTHSpawnPiece("bldr_misc_flagpole",         -0.55908,   5.8681,   6.66342,    0.0000),
	new KOTHSpawnPiece("bldr_prop_Flag_Bear",        -0.64502,   5.8510,   10.1842,    0.0000)
};

// ─────────────────────────────────────────────────────────────────────────────
// Runtime spawner
// ─────────────────────────────────────────────────────────────────────────────
class KOTH_SpawnBase
{
	static ref array<Object> s_Spawned = new array<Object>();

	static void SpawnBaseAt(vector center, string side)
	{
		if (!GetGame().IsServer())
			return;

		float baseYOffset = -2.0;  // drop entire base to terrain

		foreach (KOTHSpawnPiece p: KOTH_SPAWN_COMPOUND)
		{
			string objType = p.type;

			// ─── Faction flag logic ───────────────────────────────
			if (p.type == "bldr_prop_Flag_Bear")
			{
				if (side == "West")
					objType = "bldr_prop_Flag_Wolf";  // swap Bear → Wolf for Blue side
			}

			vector pos = Vector(center[0] + p.dx, center[1] + p.dy + baseYOffset, center[2] + p.dz);

			Object o = GetGame().CreateObjectEx(objType, pos, ECE_CREATEPHYSICS);
			if (!o)
			{
				Print("[KOTH] Failed to spawn piece: " + objType);
				continue;
			}

			o.SetOrientation(Vector(p.yaw, 0, 0));
			s_Spawned.Insert(o);
		}
	}

	static void SpawnBases(vector eastCenter, vector westCenter)
	{
		SpawnBaseAt(eastCenter, "East");
		SpawnBaseAt(westCenter, "West");
	}

	static void DespawnAll()
	{
		if (!GetGame().IsServer())
			return;

		for (int i = s_Spawned.Count() - 1; i >= 0; i--)
		{
			Object o = s_Spawned[i];
			if (o)
				GetGame().ObjectDelete(o);
			s_Spawned.Remove(i);
		}
	}
}


// Convenience wrapper for your zone object (uses your getters)
static void KOTH_SpawnBasesForZone(KOTH_ZoneData zone)
{
	if (!GetGame().IsServer()) return;

	vector east = zone.GetEastSpawnBuilding();
	vector west = zone.GetWestSpawnBuilding();

	KOTH_SpawnBase.SpawnBases(east, west);
}

// Optional cleanup, if needed during iteration
static void KOTH_DespawnBases()
{
	KOTH_SpawnBase.DespawnAll();
}
