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

        // Use the same raycast as original but with adjusted height
        bool hit = DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComp, null, null, player, false, false, ObjIntersectView, 0.5);

        if (hit)
        {
            // Reduced lift to prevent floating - adjust this value as needed
            // Try values between 0.01 and 0.1 to find the sweet spot
            hitPos[1] = hitPos[1] + 0.05; // Reduced from 0.15 to 0.05
            player.SetPosition(hitPos);
            Print("[KOTH] Safe-spawned player at: " + hitPos + " (offset " + offsetX + ", " + offsetZ + ")");
        }
        else
        {
            // Fallback offset if no raycast hit - also reduced
            spawnPos[1] = spawnPos[1] + 0.1; // Reduced from 1.0 to 0.1
            player.SetPosition(spawnPos);
            Print("[KOTH] Fallback spawn at: " + spawnPos + " (offset " + offsetX + ", " + offsetZ + ")");
        }

        player.MessageStatus("[KOTH] You have spawned safely at your team's base!");
    }
};