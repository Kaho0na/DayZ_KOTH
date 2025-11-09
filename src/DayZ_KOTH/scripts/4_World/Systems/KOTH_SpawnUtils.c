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
    static void SpawnPlayerAtPosition(PlayerBase player, vector spawnPos)
    {
        if (!player)
            return;

        float offsetX = Math.RandomFloatInclusive(-2.0, 2.0);
        float offsetZ = Math.RandomFloatInclusive(-2.0, 2.0);

        // Use the SAME platform height calculation as NPCs
        float baseYOffset = -2.0;  // Match KOTH_SpawnBase baseYOffset
        float platformSurfaceY = 3.38896;  // Match NPC dy value (platform top surface)
        
        // Calculate final position using platform-relative coordinates
        vector finalPos = Vector(spawnPos[0] + offsetX, spawnPos[1] + platformSurfaceY + baseYOffset, spawnPos[2] + offsetZ);
        
        player.SetPosition(finalPos);
        
        Print("[KOTH] Player spawned at: " + finalPos.ToString() + " (offset X:" + offsetX + " Z:" + offsetZ + ")");    }
}