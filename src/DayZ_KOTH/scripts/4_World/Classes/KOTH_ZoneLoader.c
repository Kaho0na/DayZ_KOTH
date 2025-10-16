/**
 * KOTH_ZoneLoader.c (UPDATED)
 *
 * Following Expansion pattern - now with dynamic priority zone
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
        
        // Create DYNAMIC priority zone if configured
        if (priRadius > 0)
        {
            CreateDynamicPriorityZone(aoCenter, aoRadius, priRadius);
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
    
    static void CreateDynamicPriorityZone(vector captureCenter, float captureRadius, float priorityRadius)
    {
        Print("[KOTH_ZoneLoader] Creating DYNAMIC priority zone system");
        Print("[KOTH_ZoneLoader] - Capture Center: " + captureCenter);
        Print("[KOTH_ZoneLoader] - Capture Radius: " + captureRadius + "m");
        Print("[KOTH_ZoneLoader] - Priority Radius: " + priorityRadius + "m");
        Print("[KOTH_ZoneLoader] - Trajectory Radius: " + (captureRadius / 2.0) + "m");
        
        // Initialize the dynamic priority zone manager
        KOTH_PriorityZoneManager.Initialize(captureCenter, captureRadius, priorityRadius);
        
        Print("[KOTH_ZoneLoader] Dynamic priority zone system initialized!");
        Print("[KOTH_ZoneLoader] Priority zone will move around the capture zone perimeter");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CLEANUP FOR ZONE ROTATION
    //! ═══════════════════════════════════════════════════════════════
    
    static void CleanupZones()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_ZoneLoader] Cleaning up zones for rotation");
        
        // Cleanup priority zone manager
        KOTH_PriorityZoneManager.Cleanup();
        
        // Note: Main zone cleanup handled by zone manager
    }
}