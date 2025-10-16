/**
 * KOTH_MissionStart.c (UPDATED)
 *
 * King of the Hill by Kahoona
 * Mission startup with unified zone system and dynamic priority zone
 */

modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();

        if (!GetGame().IsServer()) 
            return;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeKOTHZoneSystem, 1500, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_ZoneLoader.CreateKOTHZones, 5000, false);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ConfigurePriorityZone, 6000, false);
    }

    void InitializeKOTHZoneSystem()
    {
        KOTH_ZoneManager zoneManager;
        CF_Modules<KOTH_ZoneManager>.Get(zoneManager);

        if (!zoneManager)
        {
            Error("[KOTH] ERROR: Could not get KOTH_ZoneManager instance!");
            return;
        }

        if (!zoneManager.LoadFirstAvailableZone())
        {
            Error("[KOTH] ERROR: Failed to load initial zone. No bases spawned.");
            return;
        }

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
    
    void ConfigurePriorityZone()
    {
        if (!GetGame().IsServer())
            return;
        
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        
        if (!settings)
        {
            Print("[KOTH] WARNING: Could not get KOTH settings, using default priority zone config");
            return;
        }
        
        // Check if priority zone movement is enabled
        if (!settings.EnablePriorityZoneMovement)
        {
            Print("[KOTH] Priority zone movement is DISABLED in settings");
            return;
        }
        
        // Check if priority zone manager is active
        if (!KOTH_PriorityZoneManager.IsActive())
        {
            Print("[KOTH] WARNING: Priority zone manager not active, skipping configuration");
            return;
        }
        
        // Configure priority zone based on settings
        Print("[KOTH] ═══════════════════════════════════════════════════");
        Print("[KOTH] Configuring Dynamic Priority Zone");
        Print("[KOTH] - Movement Enabled: " + settings.EnablePriorityZoneMovement);
        Print("[KOTH] - Movement Interval: " + settings.PriorityZoneMovementInterval + " seconds");
        Print("[KOTH] - Angle Increment: " + settings.PriorityZoneAngleIncrement + "°");
        
        string direction;
        if (settings.PriorityZoneClockwise)
            direction = "Clockwise";
        else
            direction = "Counter-clockwise";
        Print("[KOTH] - Direction: " + direction);
        
        Print("[KOTH] - Bonus Multiplier: " + settings.PriorityZoneBonusMultiplier + "x");
        
        KOTH_PriorityZoneManager.SetMovementInterval(settings.PriorityZoneMovementInterval);
        KOTH_PriorityZoneManager.SetAngleIncrement(settings.PriorityZoneAngleIncrement);
        KOTH_PriorityZoneManager.SetDirection(settings.PriorityZoneClockwise);
        
        Print("[KOTH] Priority zone configuration complete");
        Print("[KOTH] ═══════════════════════════════════════════════════");
    }
}