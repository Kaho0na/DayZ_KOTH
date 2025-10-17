/**
 * KOTH_MissionServer.c (UNIFIED)
 *
 * King of the Hill by Kahoona
 * Unified mission server handling - startup and player connections
 * Place in: 5_Mission/KOTH_MissionServer.c
 */

modded class MissionServer
{
    // ═══════════════════════════════════════════════════════════════
    // SERVER INITIALIZATION
    // ═══════════════════════════════════════════════════════════════
    
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
        
        if (!settings.EnablePriorityZoneMovement)
        {
            Print("[KOTH] Priority zone movement is DISABLED in settings");
            return;
        }
        
        if (!KOTH_PriorityZoneManager.IsActive())
        {
            Print("[KOTH] WARNING: Priority zone manager not active, skipping configuration");
            return;
        }
        
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

    // ═══════════════════════════════════════════════════════════════
    // PLAYER CONNECTION HANDLING
    // ═══════════════════════════════════════════════════════════════
    
    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);

        if (!GetGame().IsDedicatedServer())
            return;

        if (!identity)
            return;

        string playerID = identity.GetId();
        string filePath = EXPANSION_KOTH_Players + playerID + ".json";

        if (!FileExist(EXPANSION_KOTH_Players))
            MakeDirectory(EXPANSION_KOTH_Players);

        KOTH_Players playerData;

        if (FileExist(filePath))
        {
            playerData = KOTH_Players.Load(playerID);
            Print("[DayZ_KOTH] Loaded existing player file for " + identity.GetName());
        }
        else
        {
            playerData = new KOTH_Players();
            playerData.m_FileName = playerID;
            playerData.PlayerID = playerID;
            playerData.PlayerName = identity.GetName();
            playerData.TotalMoneyinBank = 0;
            playerData.TotalExperienceEarned = 0;
            playerData.CurrentLevel = 1;
            playerData.TotalTimePlayed = 0;
            playerData.LongestKill = 0;
            playerData.LongestHeadshot = 0;
            playerData.HighestKillstreak = 0;
            playerData.TotalEnemiesKilled = 0;
            playerData.LastTeamSelection = "None";

            playerData.Save();

            Print("[DayZ_KOTH] Created new player file for " + identity.GetName() + " (" + playerID + ")");
        }

        KOTH_ZoneManager zoneManager;
        CF_Modules<KOTH_ZoneManager>.Get(zoneManager);

        if (!zoneManager)
        {
            Error("[KOTH] ERROR: Could not get KOTH_ZoneManager instance!");
            return;
        }

        if (!zoneManager.IsZoneActive())
        {
            Error("[KOTH] ERROR: No active zone loaded!");
            return;
        }

        vector spawnPos;

        if (playerData.LastTeamSelection == "East")
        {
            KOTH_PlayerLoadout.SetPlayerLoadout(player, "East");
            
            spawnPos = zoneManager.GetEastSpawnPosition();
            Print("[KOTH] " + playerData.PlayerName + " spawning at EAST base: " + spawnPos);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_SpawnUtils.SpawnPlayerAtPosition, 500, false, player, spawnPos);
        }
        else if (playerData.LastTeamSelection == "West")
        {
            KOTH_PlayerLoadout.SetPlayerLoadout(player, "West");
            
            spawnPos = zoneManager.GetWestSpawnPosition();
            Print("[KOTH] " + playerData.PlayerName + " spawning at WEST base: " + spawnPos);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_SpawnUtils.SpawnPlayerAtPosition, 500, false, player, spawnPos);
        }
        else
        {
            Print("[KOTH] " + playerData.PlayerName + " has no team — opening selection menu, skipping spawn.");
            player.SetPosition("0 10000 0");
            player.SetAllowDamage(false);
            
            KOTH_TeamSelectionModule teamModule;
            CF_Modules<KOTH_TeamSelectionModule>.Get(teamModule);
            Print("[DayZ_KOTH] Retrieved team module: " + teamModule);

            if (teamModule)
            {
                Print("[DayZ_KOTH] Starting team selection for player " + identity.GetName());
                teamModule.StartTeamSelection(player, identity);
            }
            else
            {
                Print("[DayZ_KOTH] ERROR: Could not get KOTH_TeamSelectionModule instance!");
            }

            return;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // UTILITY METHODS
    // ═══════════════════════════════════════════════════════════════
    
    void CheckPlayerArmbands()
    {
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity())
            {
                string uid = player.GetIdentity().GetId();
                KOTH_Players playerData = KOTH_Players.Load(uid);
                
                if (playerData && playerData.LastTeamSelection != "None")
                {
                    KOTH_PlayerLoadout.CheckAndRestoreArmband(player, playerData.LastTeamSelection);
                }
            }
        }
    }
}