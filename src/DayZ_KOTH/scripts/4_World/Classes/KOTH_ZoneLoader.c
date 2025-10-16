/**
 * KOTH_ZoneLoader.c
 *
 * Following Expansion pattern - no config registration needed!
 * Place in: 4_World/Classes/KOTH_ZoneLoader.c
 */

class KOTH_ZoneLoader
{
    static void CreateKOTHZones()
    {
        if (!GetGame().IsServer())
            return;
            
        Print("[KOTH_ZoneLoader] ═══════════════════════════════════════");
        Print("[KOTH_ZoneLoader] Creating KOTH zones (Expansion Method)");
        
        KOTH_ZoneManager zoneManager;
        CF_Modules<KOTH_ZoneManager>.Get(zoneManager);
        
        if (!zoneManager || !zoneManager.IsZoneActive())
        {
            Print("[KOTH_ZoneLoader] No active zone, using test position");
            CreateMainZone("6585 6 2439", 100);
            return;
        }
        
        KOTH_ZoneData activeZone = zoneManager.GetActiveZone();
        vector aoCenter = activeZone.GetAOZoneCenter();
        float aoRadius = activeZone.GetAOZoneRadius();
        float priRadius = activeZone.GetPriorityAORadius();
        
        Print("[KOTH_ZoneLoader] Zone data:");
        Print("[KOTH_ZoneLoader] - Center: " + aoCenter);
        Print("[KOTH_ZoneLoader] - Main Radius: " + aoRadius);
        Print("[KOTH_ZoneLoader] - Priority Radius: " + priRadius);
        
        // Create main zone
        CreateMainZone(aoCenter, aoRadius);
        
        // Create priority zone if configured
        if (priRadius > 0)
        {
            CreatePriorityZone(aoCenter, priRadius);
        }
        
        Print("[KOTH_ZoneLoader] ═══════════════════════════════════════");
    }
    
    static void CreateMainZone(vector pos, float radius)
    {
        Print("[KOTH_ZoneLoader] Creating main zone at " + pos);
        
        KOTH_Area area;
        if (Class.CastTo(area, GetGame().CreateObjectEx("KOTH_Area", pos, ECE_NONE)))
        {
            area.KOTH_Init(pos, radius);
            Print("[KOTH_ZoneLoader] Main zone created successfully!");
        }
        else
        {
            Error("[KOTH_ZoneLoader] Failed to create main zone!");
        }
    }
    
    static void CreatePriorityZone(vector pos, float radius)
    {
        Print("[KOTH_ZoneLoader] Creating priority zone at " + pos);
        
        KOTH_PriArea area;
        if (Class.CastTo(area, GetGame().CreateObjectEx("KOTH_PriArea", pos, ECE_NONE)))
        {
            area.KOTH_Init(pos, radius);
            Print("[KOTH_ZoneLoader] Priority zone created successfully!");
        }
        else
        {
            Error("[KOTH_ZoneLoader] Failed to create priority zone!");
        }
    }
}