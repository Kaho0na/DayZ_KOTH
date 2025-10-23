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

        vector finalPos = Vector(spawnPos[0] + offsetX, spawnPos[1], spawnPos[2] + offsetZ);

        vector rayStart = Vector(finalPos[0], finalPos[1] + 10, finalPos[2]);
        vector rayEnd = Vector(finalPos[0], finalPos[1] - 10, finalPos[2]);
        vector hitPos;
        vector hitNormal;
        int hitComp;

        bool hit = DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComp, null, null, player, false, false, ObjIntersectView, 0.5);

        if (hit)
        {
            hitPos[1] = hitPos[1] + 0.1;
            player.SetPosition(hitPos);
            Print("[KOTH] Player spawned at: " + hitPos + " (offset X:" + offsetX + " Z:" + offsetZ + ")");
        }
        else
        {
            finalPos[1] = finalPos[1] + 0.1;
            player.SetPosition(finalPos);
            Print("[KOTH] Player spawned (no raycast) at: " + finalPos + " (offset X:" + offsetX + " Z:" + offsetZ + ")");
        }

        player.MessageStatus("[KOTH] You have spawned safely at your team's base!");
    }
};