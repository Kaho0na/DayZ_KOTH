/**
 * KOTH_MissionStart.c
 *
 * King of the Hill by Kahoona
 * Mission startup with unified zone system
 */

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        if (!GetGame().IsServer()) 
            return;

        // Give the world a moment to finish loading terrain/CE before we place objects
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeKOTHZoneSystem, 1500, false);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_ZoneLoader.CreateKOTHZones, 5000, false);

    }

    void InitializeKOTHZoneSystem()
    {
        // ────────────────────────────────────────────────────────────
        // Get the Zone Manager
        // ────────────────────────────────────────────────────────────
        KOTH_ZoneManager zoneManager;
        CF_Modules<KOTH_ZoneManager>.Get(zoneManager);

        if (!zoneManager)
        {
            Error("[KOTH] ERROR: Could not get KOTH_ZoneManager instance!");
            return;
        }

        // ────────────────────────────────────────────────────────────
        // Load first zone
        // ────────────────────────────────────────────────────────────
        if (!zoneManager.LoadFirstAvailableZone())
        {
            Error("[KOTH] ERROR: Failed to load initial zone. No bases spawned.");
            return;
        }

        // ────────────────────────────────────────────────────────────
        // Get settings and configure auto-rotation if enabled
        // ────────────────────────────────────────────────────────────
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        
        if (settings && settings.EnableZoneRotation)
        {
            KOTHZoneSelectionMode mode = KOTHZoneSelectionMode.SEQUENTIAL;
            
            if (settings.ZoneSelectionMode == 1)
                mode = KOTHZoneSelectionMode.RANDOM;
            else if (settings.ZoneSelectionMode == 2)
                mode = KOTHZoneSelectionMode.VOTE;
            
            zoneManager.EnableAutoRotation(settings.ZoneRotationInterval, mode);
        }

        // ────────────────────────────────────────────────────────────
        // Log startup information
        // ────────────────────────────────────────────────────────────
        KOTH_ZoneData activeZone = zoneManager.GetActiveZone();
        
        Print("[KOTH] ═══════════════════════════════════════════════════");
        Print("[KOTH] Zone System Initialized");
        Print("[KOTH] Active Zone: " + zoneManager.GetActiveZoneName());
        Print("[KOTH] Zone Display Name: " + activeZone.GetZoneName());
        Print("[KOTH] Available Zones: " + zoneManager.GetAvailableZones().Count());
        
        if (settings)
        {
            Print("[KOTH] Auto-Rotation: " + settings.EnableZoneRotation);
            if (settings.EnableZoneRotation)
            {
                string modeStr = "Sequential";
                if (settings.ZoneSelectionMode == 1) modeStr = "Random";
                else if (settings.ZoneSelectionMode == 2) modeStr = "Vote";
                
                Print("[KOTH] Rotation Mode: " + modeStr);
                Print("[KOTH] Rotation Interval: " + settings.ZoneRotationInterval + " seconds");
            }
        }
        
        Print("[KOTH] East/West bases spawned successfully");
        Print("[KOTH] ═══════════════════════════════════════════════════");
    }
}