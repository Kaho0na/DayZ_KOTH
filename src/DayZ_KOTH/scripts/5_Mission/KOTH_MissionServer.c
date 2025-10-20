/**
 * KOTH_MissionServer.c (PHASE 3 - REWARD INTEGRATION)
 *
 * King of the Hill by Kahoona
 * Integrated reward system with initial stats sync
 * Place in: 5_Mission/KOTH_MissionServer.c
 */

modded class MissionServer
{
    //! ═══════════════════════════════════════════════════════════════
    //! SERVER INITIALIZATION
    //! ═══════════════════════════════════════════════════════════════

    override void OnInit()
    {
        super.OnInit();

        if (!GetGame().IsServer()) 
            return;

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeKOTHSystem, 2000, false);
        
        //GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_AIDebugHelper.DebugAllEntitiesInGame, 10000, false);
        //GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_AIDebugHelper.TestAIInZone, 15000, true);

    
    }


    void InitializeKOTHSystem()
    {
        Print("[KOTH] ═══════════════════════════════════════════════════");
        Print("[KOTH] Initializing King of the Hill System");
        Print("[KOTH] ═══════════════════════════════════════════════════");
        
        KOTH_ZoneManager zoneManager;
        CF_Modules<KOTH_ZoneManager>.Get(zoneManager);

        if (!zoneManager)
        {
            Error("[KOTH] ERROR: Could not get KOTH_ZoneManager instance!");
            return;
        }

        if (!zoneManager.LoadFirstAvailableZone())
        {
            Error("[KOTH] ERROR: Failed to load initial zone!");
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
        Print("[KOTH] System Initialized Successfully");
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
            
            Print("[KOTH] Priority Zone Movement: " + settings.EnablePriorityZoneMovement);
            if (settings.EnablePriorityZoneMovement)
            {
                Print("[KOTH] - Interval: " + settings.PriorityZoneMovementInterval + "s");
                Print("[KOTH] - Bonus: " + settings.PriorityZoneBonusMultiplier + "x");
            }
        }
        
        Print("[KOTH] ═══════════════════════════════════════════════════");
    }

    //! ═══════════════════════════════════════════════════════════════
    //! PLAYER CONNECTION HANDLING
    //! ═══════════════════════════════════════════════════════════════
    
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
            
            // PHASE 3: Sync initial stats to client
            SyncInitialStatsToClient(player, identity, playerData);
        }
        else if (playerData.LastTeamSelection == "West")
        {
            KOTH_PlayerLoadout.SetPlayerLoadout(player, "West");
            
            spawnPos = zoneManager.GetWestSpawnPosition();
            Print("[KOTH] " + playerData.PlayerName + " spawning at WEST base: " + spawnPos);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_SpawnUtils.SpawnPlayerAtPosition, 500, false, player, spawnPos);
            
            // PHASE 3: Sync initial stats to client
            SyncInitialStatsToClient(player, identity, playerData);
        }
        else
        {
            Print("[KOTH] " + playerData.PlayerName + " has no team — opening selection menu.");
            player.SetPosition("0 10000 0");
            player.SetAllowDamage(false);
            
            KOTH_TeamSelectionModule teamModule;
            CF_Modules<KOTH_TeamSelectionModule>.Get(teamModule);

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
    
    //! ═══════════════════════════════════════════════════════════════
    //! PHASE 3: INITIAL STATS SYNC
    //! ═══════════════════════════════════════════════════════════════
    
    void SyncInitialStatsToClient(PlayerBase player, PlayerIdentity identity, KOTH_Players data)
    {
        if (!player || !identity || !data)
            return;
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (rewardManager)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(rewardManager.SyncPlayerStatsToClient, 2000, false, identity, data);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(rewardManager.SyncPlayerStatsToClient, 3000, false, identity, data);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(rewardManager.SyncPlayerStatsToClient, 4000, false, identity, data);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(rewardManager.SyncPlayerStatsToClient, 5000, false, identity, data);
            Print("[KOTH] Scheduled initial stats sync for " + identity.GetName());
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! UTILITY METHODS
    //! ═══════════════════════════════════════════════════════════════
    
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