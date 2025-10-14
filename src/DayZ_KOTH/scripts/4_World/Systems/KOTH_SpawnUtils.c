/**
 * KOTH_SpawnUtils.c
 *
 * Utility methods for safe player spawning in King of the Hill.
 * Used by both MissionServer and KOTH_TeamSelectionModule.
 *
 * Author: Kahoona
 * Credit: DayZ Expansion Mod Team
 */

class KOTH_SpawnUtils
{
	//! Safely spawns player near target position with ground correction and offset
	static void SpawnPlayerAtPosition(PlayerBase player, vector spawnPos)
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
};
