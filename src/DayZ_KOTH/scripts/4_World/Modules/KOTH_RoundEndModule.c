/**
 * KOTH_RoundEndModule.c (SERVER CALCULATES TOP PERFORMERS)
 *
 * King of the Hill by Kahoona
 * Server calculates and sends top performers to clients
 *
 * Place in: 4_World/Modules/KOTH_RoundEndModule.c
 */

[CF_RegisterModule(KOTH_RoundEndModule)]
class KOTH_RoundEndModule: CF_ModuleWorld
{
    private static ref KOTH_RoundEndModule s_Instance;
    
    static ref ScriptInvoker SI_UpdateVoteCounts = new ScriptInvoker();
    
    protected ref ScriptInvoker m_RoundEndMenuInvoker;
    
    private ref KOTH_GameMode m_GameMode;
    private ref KOTH_RoundStatsTracker m_StatsTracker;
    private ref KOTH_PlayerRewardManager m_RewardManager;
    private ref KOTH_ZoneManager m_ZoneManager;
    
    private ref map<string, int> m_ZoneVotes;
    private ref array<string> m_AvailableZones;
    
    private float m_EndScreenDisplaySeconds;
    private float m_VoteTimeSeconds;
    private bool m_VotingEnabled;
    
    private bool m_RoundEndInProgress;
    private ref KOTH_HUDDataSync m_HUDSync;

    private KOTH_Settings m_Settings;
    
    void KOTH_RoundEndModule()
    {
        s_Instance = this;
        m_ZoneVotes = new map<string, int>();
        m_AvailableZones = new array<string>();
        m_RoundEndMenuInvoker = new ScriptInvoker();
        m_RoundEndInProgress = false;
        
        if (!SI_UpdateVoteCounts)
            SI_UpdateVoteCounts = new ScriptInvoker();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        
        Expansion_RegisterClientRPC("RPC_ShowRoundEndScreen");
        Expansion_RegisterClientRPC("RPC_UpdateVoteCounts");
        Expansion_RegisterServerRPC("RPC_PlayerVote");
        
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeModule, 3000, false);
        }
        
        Print("[KOTH_RoundEndModule] Initialized");
    }
    
    void InitializeModule()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] ============================================");
        Print("[KOTH_RoundEndModule] InitializeModule CALLED");
        Print("[KOTH_RoundEndModule] ============================================");
        
        CF_Modules<KOTH_GameMode>.Get(m_GameMode);
        CF_Modules<KOTH_RoundStatsTracker>.Get(m_StatsTracker);
        CF_Modules<KOTH_PlayerRewardManager>.Get(m_RewardManager);
        CF_Modules<KOTH_ZoneManager>.Get(m_ZoneManager);
        CF_Modules<KOTH_HUDDataSync>.Get(m_HUDSync);
        
        if (!m_HUDSync)
        {
            Error("[KOTH_RoundEndModule] ERROR: Could not get KOTH_HUDDataSync module!");
            return;
        }

        if (!m_GameMode)
        {
            Error("[KOTH_RoundEndModule] Failed to get KOTH_GameMode!");
        }
        else
        {
            Print("[KOTH_RoundEndModule] Got KOTH_GameMode reference");
        }
        
        if (!m_StatsTracker)
        {
            Error("[KOTH_RoundEndModule] Failed to get KOTH_RoundStatsTracker!");
        }
        else
        {
            Print("[KOTH_RoundEndModule] Got KOTH_RoundStatsTracker reference");
        }
        
        if (!m_RewardManager)
        {
            Error("[KOTH_RoundEndModule] Failed to get KOTH_PlayerRewardManager!");
        }
        else
        {
            Print("[KOTH_RoundEndModule] Got KOTH_PlayerRewardManager reference");
        }
        
        if (!m_ZoneManager)
        {
            Error("[KOTH_RoundEndModule] Failed to get KOTH_ZoneManager!");
        }
        else
        {
            Print("[KOTH_RoundEndModule] Got KOTH_ZoneManager reference");
        }
        
        LoadSettings();
        
        if (m_GameMode)
        {
            Print("[KOTH_RoundEndModule] Subscribing to GameMode SI_OnRoundEnd...");
            KOTH_GameMode.SI_OnRoundEnd.Insert(OnRoundEnd);
            Print("[KOTH_RoundEndModule] SUBSCRIBED to GameMode round end event");
        }
        
        Print("[KOTH_RoundEndModule] Module initialized successfully");
        Print("[KOTH_RoundEndModule] ============================================");
    }
    
    void LoadSettings()
    {
        m_Settings = GetExpansionSettings().GetDayZ_KOTH();
        if (!m_Settings)
        {
            m_EndScreenDisplaySeconds = 60;
            m_VoteTimeSeconds = 30;
            m_VotingEnabled = false;
            Print("[KOTH_RoundEndModule] WARNING: Could not load settings, using defaults");
            return;
        }
        
        m_EndScreenDisplaySeconds = m_Settings.EndScreenDisplaySeconds;
        if(m_EndScreenDisplaySeconds < 5)
        {
            m_EndScreenDisplaySeconds = 10;
        }

        m_VoteTimeSeconds = m_Settings.VoteTimeSeconds;
        
        if (m_Settings.ZoneSelectionMode == 2)
            m_VotingEnabled = true;
        else
            m_VotingEnabled = false;
        
        if (m_VoteTimeSeconds > m_EndScreenDisplaySeconds)
        {
            m_VoteTimeSeconds = m_EndScreenDisplaySeconds;
        }
        
        if (m_VoteTimeSeconds < 30)
        {
            m_VoteTimeSeconds = 30;
        }
        
        Print("[KOTH_RoundEndModule] Settings loaded - Display: " + m_EndScreenDisplaySeconds + "s, Vote: " + m_VoteTimeSeconds + "s, Voting: " + m_VotingEnabled);
    }
    
    static KOTH_RoundEndModule GetInstance()
    {
        return s_Instance;
    }
    
    ScriptInvoker GetRoundEndMenuSI()
    {
        return m_RoundEndMenuInvoker;
    }
    
    void OnRoundEnd(string winningTeam, int eastScore, int westScore)
    {
        if (!GetGame().IsServer() || m_RoundEndInProgress)
            return;
        
        m_RoundEndInProgress = true;
        
        Print("[KOTH_RoundEndModule] ============================================");
        Print("[KOTH_RoundEndModule] ROUND END SEQUENCE STARTED");
        Print("[KOTH_RoundEndModule] Winner: " + winningTeam);
        Print("[KOTH_RoundEndModule] Scores - East: " + eastScore + ", West: " + westScore);
        Print("[KOTH_RoundEndModule] ============================================");

        Print("[KOTH_RoundEndModule] STEP 1: Stopping zone triggers...");
        StopZoneTriggers();

        Print("[KOTH_RoundEndModule] STEP 2: Ejecting players and deleting vehicles...");
        EjectPlayersFromVehicles();

        Print("[KOTH_RoundEndModule] STEP 3: Cleaning up AIs...");
        CleanupAIs();

        Print("[KOTH_RoundEndModule] STEP 4: Calculating and awarding bonuses...");
        CalculateAndAwardBonuses(winningTeam);

        Print("[KOTH_RoundEndModule] STEP 5: Preparing next zone...");
        PrepareNextZone();

        Print("[KOTH_RoundEndModule] STEP 6: Showing round end screen...");
        ShowRoundEndScreenToAllClients(winningTeam);

        if (m_HUDSync)
        {
            m_HUDSync.SetEastScore(0);
            m_HUDSync.SetWestScore(0);
            m_HUDSync.SetCaptureProgress(0.0, "None");
        }

        float teleportDelay = m_EndScreenDisplaySeconds - 3;
        if (teleportDelay < 1)
        {
            teleportDelay = 1;
        }
        teleportDelay = teleportDelay * 1000;
        float newRoundDelay = teleportDelay + 5000;

        Print("[KOTH_RoundEndModule] STEP 7: Scheduling teleport...");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TeleportAllPlayers, teleportDelay, false);

        Print("[KOTH_RoundEndModule] STEP 8: Scheduling new round..");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartNewRound, newRoundDelay, false);
        
        Print("[KOTH_RoundEndModule] Round end sequence initiated");
        Print("[KOTH_RoundEndModule] ============================================");
    }

    void EjectPlayersFromVehicles()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] Ejecting players from ALL vehicles and deleting vehicles...");
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        array<Transport> vehiclesToDelete = new array<Transport>;
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
                continue;
            
            Transport transport = Transport.Cast(player.GetParent());
            if (transport)
            {
                Print("[KOTH_RoundEndModule] Found player " + player.GetIdentity().GetName() + " in vehicle: " + transport.GetType());
                
                int crew_index = transport.CrewMemberIndex(player);
                if (crew_index >= 0)
                {
                    Print("[KOTH_RoundEndModule] Ejecting from crew position: " + crew_index);
                    transport.CrewGetOut(crew_index);
                }
                
                if (vehiclesToDelete.Find(transport) == -1)
                {
                    vehiclesToDelete.Insert(transport);
                }
            }
        }
        
        Print("[KOTH_RoundEndModule] Waiting 500ms for ejection animation...");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DeleteAllVehicles, 500, false, vehiclesToDelete);
        
        Print("[KOTH_RoundEndModule] Player ejection initiated");
    }

    void DeleteAllVehicles(array<Transport> vehicles)
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] Deleting " + vehicles.Count() + " vehicles...");
        
        foreach (Transport vehicle : vehicles)
        {
            if (vehicle)
            {
                Print("[KOTH_RoundEndModule] Deleting vehicle: " + vehicle.GetType());
                vehicle.DeleteSafe();
            }
        }
        
        Print("[KOTH_RoundEndModule] All vehicles deleted");
    }

    void StopZoneTriggers()
    {
        KOTH_Area mainZone = KOTH_Area.GetInstance();
        if (mainZone)
        {
            KOTH_AreaTrigger trigger = mainZone.GetMainTrigger();
            if (trigger)
            {
                trigger.SetActive(false);
                Print("[KOTH_RoundEndModule] Main zone trigger deactivated");
            }
        }
        
        KOTH_PriorityZoneManager.StopMovement();
        Print("[KOTH_RoundEndModule] Priority zone movement stopped");
        
        if (m_GameMode)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(m_GameMode.UpdateCapture);
            Print("[KOTH_RoundEndModule] Capture update loop stopped");
        }
        
        Print("[KOTH_RoundEndModule] All zone triggers stopped");
    }
    
    void CleanupAIs()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] Cleaning up AI entities...");
        
        eAIGroup.Admin_ClearAllAI();
        
        Print("[KOTH_RoundEndModule] All AI entities cleared via Expansion method");
    }

    void CalculateAndAwardBonuses(string winningTeam)
    {
        if (!m_StatsTracker || !m_RewardManager)
            return;
        
        Print("[KOTH_RoundEndModule] Calculating end-round bonuses...");

        if (!m_Settings)
            return;
        
        KOTH_RoundTeamStats eastStats = m_StatsTracker.GetTeamStats("East");
        KOTH_RoundTeamStats westStats = m_StatsTracker.GetTeamStats("West");
        
        if (!eastStats || !westStats)
        {
            Error("[KOTH_RoundEndModule] Failed to get team stats!");
            return;
        }
        
        AwardTopPerformers(m_Settings);
        AwardTeamBonuses(winningTeam, eastStats, westStats, m_Settings);
        
        Print("[KOTH_RoundEndModule] Bonuses awarded");
    }
    
    void AwardTopPerformers(KOTH_Settings m_Settings)
    {
        KOTH_RoundPlayerStats mvp = m_StatsTracker.GetMVP();
        if (mvp && mvp.Kills > 0)
        {
            PlayerBase mvpPlayer = GetPlayerByUID(mvp.PlayerUID);
            if (mvpPlayer)
            {
                m_RewardManager.AddPlayerXP(mvpPlayer, m_Settings.MVPBonusXP, "MVP Award");
                m_RewardManager.AddPlayerMoney(mvpPlayer, m_Settings.MVPBonusMoney, "MVP Award");
                KOTH_NotificationModule.ShowNotificationAdvanced("MVP AWARD!", "$" + m_Settings.MVPBonusMoney.ToString(), ARGB(255, 255, 215, 0), m_Settings.MVPBonusXP.ToString() + "XP", ARGB(255, 255, 215, 0), ARGB(255, 218, 165, 32), 5.0, mvpPlayer.GetIdentity());
                Print("[KOTH_RoundEndModule] MVP Award: " + mvp.PlayerName + " (" + mvp.Kills + " kills)");
            }
        }
        
        KOTH_RoundPlayerStats sharpshooter = m_StatsTracker.GetSharpshooter();
        if (sharpshooter && sharpshooter.Headshots > 0)
        {
            PlayerBase ssPlayer = GetPlayerByUID(sharpshooter.PlayerUID);
            if (ssPlayer)
            {
                m_RewardManager.AddPlayerXP(ssPlayer, m_Settings.SharpshooterBonusXP, "Sharpshooter Award");
                m_RewardManager.AddPlayerMoney(ssPlayer, m_Settings.SharpshooterBonusMoney, "Sharpshooter Award");
                KOTH_NotificationModule.ShowNotificationAdvanced("SHARPSHOOTER AWARD!", "$" + m_Settings.SharpshooterBonusMoney.ToString(), ARGB(255, 255, 140, 0), m_Settings.SharpshooterBonusXP.ToString() + "XP", ARGB(255, 255, 140, 0), ARGB(255, 255, 69, 0), 5.0, ssPlayer.GetIdentity());
                Print("[KOTH_RoundEndModule] Sharpshooter Award: " + sharpshooter.PlayerName + " (" + sharpshooter.Headshots + " headshots)");
            }
        }
        
        KOTH_RoundPlayerStats medic = m_StatsTracker.GetTopMedic();
        if (medic && medic.Revives > 0)
        {
            PlayerBase medicPlayer = GetPlayerByUID(medic.PlayerUID);
            if (medicPlayer)
            {
                m_RewardManager.AddPlayerXP(medicPlayer, m_Settings.MedicBonusXP, "Top Medic Award");
                m_RewardManager.AddPlayerMoney(medicPlayer, m_Settings.MedicBonusMoney, "Top Medic Award");
                KOTH_NotificationModule.ShowNotificationAdvanced("TOP MEDIC AWARD!", "$" + m_Settings.MedicBonusMoney.ToString(), ARGB(255, 0, 255, 127), m_Settings.MedicBonusXP.ToString() + "XP", ARGB(255, 0, 255, 127), ARGB(255, 60, 179, 113), 5.0, medicPlayer.GetIdentity());
                Print("[KOTH_RoundEndModule] Top Medic Award: " + medic.PlayerName + " (" + medic.Revives + " revives)");
            }
        }
    }
    
    void AwardTeamBonuses(string winningTeam, KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats, KOTH_Settings m_Settings)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
                continue;
            
            string uid = player.GetIdentity().GetId();
            string team = player.GetKOTHTeam();
            
            KOTH_RoundPlayerStats playerStats = m_StatsTracker.GetPlayerStats(uid);
            if (!playerStats)
                continue;
            
            int bonusXP = 0;
            int bonusMoney = 0;
            
            if (team == winningTeam)
            {
                bonusXP = CalculateWinningBonus(playerStats, team, eastStats, westStats, m_Settings, true);
                bonusMoney = CalculateWinningBonus(playerStats, team, eastStats, westStats, m_Settings, false);
                m_RewardManager.AddPlayerXP(player, bonusXP, "Victory Bonus");
                m_RewardManager.AddPlayerMoney(player, bonusMoney, "Victory Bonus");
                KOTH_NotificationModule.ShowNotificationAdvanced("VICTORY BONUS!", "$" + bonusMoney.ToString(), ARGB(255, 0, 255, 0), bonusXP.ToString() + "XP", ARGB(255, 144, 238, 144), ARGB(255, 34, 139, 34), 5.0, player.GetIdentity());
            }
            else
            {
                bonusXP = CalculateLosingBonus(playerStats, team, eastStats, westStats, m_Settings, true);
                bonusMoney = CalculateLosingBonus(playerStats, team, eastStats, westStats, m_Settings, false);
                m_RewardManager.AddPlayerXP(player, bonusXP, "Participation Bonus");
                m_RewardManager.AddPlayerMoney(player, bonusMoney, "Participation Bonus");
                KOTH_NotificationModule.ShowNotificationAdvanced("Participation Bonus", "$" + bonusMoney.ToString(), ARGB(255, 169, 169, 169), bonusXP.ToString() + "XP", ARGB(255, 169, 169, 169), ARGB(255, 128, 128, 128), 5.0, player.GetIdentity());
            }
            
            Print("[KOTH_RoundEndModule] Team Bonus: " + player.GetIdentity().GetName() + " (" + team + ") - XP: " + bonusXP + ", Money: " + bonusMoney);
        }
    }
    
    int CalculateWinningBonus(KOTH_RoundPlayerStats playerStats, string team, KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats, KOTH_Settings m_Settings, bool isXP)
    {
        int totalTeamXP;
        int totalTeamMoney;
        
        if (team == "East")
        {
            totalTeamXP = eastStats.TotalXPEarned;
            totalTeamMoney = eastStats.TotalMoneyEarned;
        }
        else
        {
            totalTeamXP = westStats.TotalXPEarned;
            totalTeamMoney = westStats.TotalMoneyEarned;
        }
        
        int playerLevel = playerStats.CurrentLevel;
        int maxLevel = m_Settings.MaxPlayerLevel;
        float playTimeRatio = playerStats.PlayTimeRatio;
        float levelRatio = playerLevel / maxLevel;
        
        int bonus;
        if (isXP)
        {
            bonus = totalTeamXP * levelRatio * m_Settings.WinningTeamXPMultiplier * playTimeRatio;
        }
        else
        {
            bonus = totalTeamMoney * levelRatio * m_Settings.WinningTeamMoneyMultiplier * playTimeRatio;
        }
        
        return bonus;
    }
    
    int CalculateLosingBonus(KOTH_RoundPlayerStats playerStats, string team, KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats, KOTH_Settings m_Settings, bool isXP)
    {
        int totalTeamXP;
        int totalTeamMoney;
        int teamFinalScore;
        
        if (team == "East")
        {
            totalTeamXP = eastStats.TotalXPEarned;
            totalTeamMoney = eastStats.TotalMoneyEarned;
            teamFinalScore = eastStats.FinalScore;
        }
        else
        {
            totalTeamXP = westStats.TotalXPEarned;
            totalTeamMoney = westStats.TotalMoneyEarned;
            teamFinalScore = westStats.FinalScore;
        }
        
        int scoreLimit = m_Settings.ScoreLimit;
        int playerLevel = playerStats.CurrentLevel;
        int maxLevel = m_Settings.MaxPlayerLevel;
        float playTimeRatio = playerStats.PlayTimeRatio;
        float scoreRatio = teamFinalScore / scoreLimit;
        float levelRatio = playerLevel / maxLevel;
        
        int bonus;
        if (isXP)
        {
            bonus = totalTeamXP * scoreRatio * levelRatio * m_Settings.LosingTeamXPMultiplier * playTimeRatio;
        }
        else
        {
            bonus = totalTeamMoney * scoreRatio * levelRatio * m_Settings.LosingTeamMoneyMultiplier * playTimeRatio;
        }
        
        return bonus;
    }
    
    void PrepareNextZone()
    {
        if (!m_ZoneManager)
            return;
        
        m_AvailableZones = m_ZoneManager.GetAvailableZones();
        m_ZoneVotes.Clear();
        
        foreach (string zoneName : m_AvailableZones)
        {
            m_ZoneVotes.Set(zoneName, 0);
        }
        
        Print("[KOTH_RoundEndModule] Prepared " + m_AvailableZones.Count() + " zones for next round");
    }
    
    void ShowRoundEndScreenToAllClients(string winningTeam)
    {
        Print("[KOTH_RoundEndModule] ShowRoundEndScreenToAllClients CALLED");
        
        if (!GetGame().IsServer())
        {
            Error("[KOTH_RoundEndModule] ERROR: Not server, aborting");
            return;
        }
        
        if (!m_StatsTracker)
        {
            Error("[KOTH_RoundEndModule] ERROR: Stats tracker not available!");
            return;
        }
        
        KOTH_RoundTeamStats eastStats = m_StatsTracker.GetTeamStats("East");
        KOTH_RoundTeamStats westStats = m_StatsTracker.GetTeamStats("West");
        
        if (!eastStats || !westStats)
        {
            Error("[KOTH_RoundEndModule] ERROR: Could not get team stats!");
            return;
        }
        
        KOTH_RoundPlayerStats mvp = m_StatsTracker.GetMVP();
        KOTH_RoundPlayerStats sharpshooter = m_StatsTracker.GetSharpshooter();
        KOTH_RoundPlayerStats medic = m_StatsTracker.GetTopMedic();
        
        Print("[KOTH_RoundEndModule] Creating RPC with " + m_AvailableZones.Count() + " zones...");
        
        auto rpc = Expansion_CreateRPC("RPC_ShowRoundEndScreen");
        rpc.Write(winningTeam);
        rpc.Write(m_EndScreenDisplaySeconds);
        rpc.Write(m_VotingEnabled);
        rpc.Write(m_AvailableZones.Count());
        
        foreach (string zoneName : m_AvailableZones)
        {
            rpc.Write(zoneName);
        }
        
        string mvpName;
        if (mvp)
            mvpName = mvp.PlayerName;
        else
            mvpName = "";
        rpc.Write(mvpName);
        
        int mvpKills;
        if (mvp)
            mvpKills = mvp.Kills;
        else
            mvpKills = 0;
        rpc.Write(mvpKills);
        
        string sharpshooterName;
        if (sharpshooter)
            sharpshooterName = sharpshooter.PlayerName;
        else
            sharpshooterName = "";
        rpc.Write(sharpshooterName);
        
        int sharpshooterHeadshots;
        if (sharpshooter)
            sharpshooterHeadshots = sharpshooter.Headshots;
        else
            sharpshooterHeadshots = 0;
        rpc.Write(sharpshooterHeadshots);
        
        string medicName;
        if (medic)
            medicName = medic.PlayerName;
        else
            medicName = "";
        rpc.Write(medicName);
        
        int medicRevives;
        if (medic)
            medicRevives = medic.Revives;
        else
            medicRevives = 0;
        rpc.Write(medicRevives);
        
        rpc.Expansion_Send(true, null);
        
        Print("[KOTH_RoundEndModule] Sent round end screen RPC to all clients with top performers");
    }
    
    void RPC_ShowRoundEndScreen(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] CLIENT: RPC_ShowRoundEndScreen RECEIVED");
        
        string winningTeam;
        if (!ctx.Read(winningTeam))
            return;
        
        float displayDuration;
        if (!ctx.Read(displayDuration))
            return;
        
        m_EndScreenDisplaySeconds = displayDuration;
        
        bool votingEnabled;
        if (!ctx.Read(votingEnabled))
            return;
        
        int zoneCount;
        if (!ctx.Read(zoneCount))
            return;
        
        array<string> zones = new array<string>();
        for (int i = 0; i < zoneCount; i++)
        {
            string zoneName;
            if (ctx.Read(zoneName))
            {
                zones.Insert(zoneName);
            }
        }
        
        string mvpName;
        int mvpKills;
        if (!ctx.Read(mvpName))
            return;
        if (!ctx.Read(mvpKills))
            return;
        
        string sharpshooterName;
        int sharpshooterHeadshots;
        if (!ctx.Read(sharpshooterName))
            return;
        if (!ctx.Read(sharpshooterHeadshots))
            return;
        
        string medicName;
        int medicRevives;
        if (!ctx.Read(medicName))
            return;
        if (!ctx.Read(medicRevives))
            return;
        
        Print("[KOTH_RoundEndModule] CLIENT: Scheduling menu display...");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, m_EndScreenDisplaySeconds, votingEnabled, mvpName, mvpKills, sharpshooterName, sharpshooterHeadshots, medicName, medicRevives);
    }
    
    void Exec_ShowRoundEndMenu(array<string> zones, float m_EndDisplaySeconds, bool votingEnabled, string mvpName, int mvpKills, string sharpshooterName, int sharpshooterHeadshots, string medicName, int medicRevives)
    {
        Print("[KOTH_RoundEndModule] CLIENT: Exec_ShowRoundEndMenu called");
        
        if (GetDayZGame().IsLoading() || GetDayZGame().GetMissionState() != DayZGame.MISSION_STATE_GAME)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, m_EndDisplaySeconds, votingEnabled, mvpName, mvpKills, sharpshooterName, sharpshooterHeadshots, medicName, medicRevives);
            return;
        }

        if (!GetGame().GetMission() || !GetGame().GetMission().GetHud())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones,  m_EndDisplaySeconds, votingEnabled, mvpName, mvpKills, sharpshooterName, sharpshooterHeadshots, medicName, medicRevives);
            return;
        }

        UIScriptedMenu blockingMenu = GetGame().GetUIManager().GetMenu();
        if (blockingMenu)
        {
            blockingMenu.Close();
        }
        
        Print("[KOTH_RoundEndModule] CLIENT: All checks passed, creating menu...");
        
        if (!OpenRoundEndMenu())
            return;
        
        m_RoundEndMenuInvoker.Invoke(zones, m_EndDisplaySeconds, votingEnabled, mvpName, mvpKills, sharpshooterName, sharpshooterHeadshots, medicName, medicRevives);
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CloseMenuAfterDuration, m_EndDisplaySeconds * 1000, false);
        
        Print("[KOTH_RoundEndModule] CLIENT: Round end menu created and invoked");
    }
    
    void CloseMenuAfterDuration()
    {
        if (!GetGame().IsClient())
            return;
        
        Print("[KOTH_RoundEndModule] CLIENT: Auto-closing menu after duration");
        
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
    }
    
    bool OpenRoundEndMenu()
    {
        Print("[KOTH_RoundEndModule] ========== MENU CREATION DEBUG ==========");
        Print("[KOTH_RoundEndModule] About to call CreateSVMenu...");
        
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("KOTH_RoundEndMenu");
        
        Print("[KOTH_RoundEndModule] CreateSVMenu called - checking if menu exists...");
        
        ExpansionScriptViewMenuBase Roundmenu = GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu();
        if (Roundmenu)
        {
            Print("[KOTH_RoundEndModule] Roundmenu created successfully: " + Roundmenu.ToString());
        }
        else
        {
            Error("[KOTH_RoundEndModule] Roundmenu is NULL after CreateSVMenu!");
        }
        
        Print("[KOTH_RoundEndModule] ==========================================");
        
        return true;
    }
    
    void SendVote(string zoneName)
    {
        if (GetGame().IsServer())
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_PlayerVote");
        rpc.Write(zoneName);
        rpc.Expansion_Send(true, null);
        
        Print("[KOTH_RoundEndModule] CLIENT: Sent vote for " + zoneName);
    }
    
    void RPC_PlayerVote(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer() || !sender)
            return;
        
        string zoneName;
        if (!ctx.Read(zoneName))
            return;
        
        if (!m_ZoneVotes.Contains(zoneName))
            return;
        
        int currentVotes = m_ZoneVotes.Get(zoneName);
        m_ZoneVotes.Set(zoneName, currentVotes + 1);
        
        Print("[KOTH_RoundEndModule] Vote received from " + sender.GetName() + " for " + zoneName);
        
        BroadcastVoteCounts();
    }
    
    void BroadcastVoteCounts()
    {
        if (!GetGame().IsServer())
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_UpdateVoteCounts");
        rpc.Write(m_ZoneVotes.Count());
        
        foreach (string zoneName, int voteCount : m_ZoneVotes)
        {
            rpc.Write(zoneName);
            rpc.Write(voteCount);
        }
        
        rpc.Expansion_Send(true, null);
    }
    
    void RPC_UpdateVoteCounts(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        int zoneCount;
        if (!ctx.Read(zoneCount))
            return;
        
        map<string, int> votes = new map<string, int>();
        
        for (int i = 0; i < zoneCount; i++)
        {
            string zoneName;
            int voteCount;
            
            if (ctx.Read(zoneName) && ctx.Read(voteCount))
            {
                votes.Set(zoneName, voteCount);
            }
        }
        
        SI_UpdateVoteCounts.Invoke(votes);
    }
    
    static ScriptInvoker GetUpdateVoteCountsSI()
    {
        return SI_UpdateVoteCounts;
    }
    
void TeleportAllPlayers()
{
    if (!GetGame().IsServer() || !m_ZoneManager)
        return;
    
    Print("[KOTH_RoundEndModule] ============================================");
    Print("[KOTH_RoundEndModule] Teleporting all players to new spawn points...");
    
    string nextZone = DetermineNextZone();
    
    if (nextZone != "")
    {
        m_ZoneManager.LoadSpecificZone(nextZone);
    }
    else
    {
        m_ZoneManager.LoadNextZone();
    }
    
    array<Man> players = new array<Man>;
    GetGame().GetPlayers(players);
    
    Print("[KOTH_RoundEndModule] Found " + players.Count() + " players to teleport");
    
    for (int i = 0; i < players.Count(); i++)
    {
        PlayerBase player = PlayerBase.Cast(players.Get(i));
        if (!player || !player.GetIdentity())
        {
            Print("[KOTH_RoundEndModule] Skipping invalid player at index " + i);
            continue;
        }
        
        Print("[KOTH_RoundEndModule] Processing player: " + player.GetIdentity().GetName());
        
        Transport transport = Transport.Cast(player.GetParent());
        if (transport)
        {
            Print("[KOTH_RoundEndModule] WARNING: Player " + player.GetIdentity().GetName() + " still in vehicle during teleport!");
            int crew_index = transport.CrewMemberIndex(player);
            if (crew_index >= 0)
            {
                transport.CrewGetOut(crew_index);
                Print("[KOTH_RoundEndModule] Force ejecting player before teleport");
            }
        }
        
        string m_PlayerTeam = player.GetKOTHTeam();
        vector spawnPos;
        
        if (m_PlayerTeam == "East")
        {
            spawnPos = m_ZoneManager.GetEastSpawnPosition();
        }
        else if (m_PlayerTeam == "West")
        {
            spawnPos = m_ZoneManager.GetWestSpawnPosition();
        }
        else
        {
            Print("[KOTH_RoundEndModule] Player has no team: " + player.GetIdentity().GetName());
            continue;
        }
        
        Print("[KOTH_RoundEndModule] Teleporting " + player.GetIdentity().GetName() + " to " + spawnPos.ToString());
        
        if (spawnPos != "0 0 0")
        {
            KOTH_SpawnUtils.SpawnPlayerAtPosition(player, spawnPos);
            KOTH_PlayerLoadout.SetPlayerStats(player);
            Print("[KOTH_RoundEndModule] ✅ Teleported " + player.GetIdentity().GetName() + " to " + m_PlayerTeam + " spawn");
        }
        else
        {
            Print("[KOTH_RoundEndModule] ❌ Invalid spawn position for " + player.GetIdentity().GetName());
        }
    }
    
    Print("[KOTH_RoundEndModule] All players teleported");
    Print("[KOTH_RoundEndModule] ============================================");
}
    
    void StartNewRound()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] ============================================");
        Print("[KOTH_RoundEndModule] Starting new round...");
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity())
            {
                ExpansionNotification("New Round Starting", "Get ready! Round begins in 5 seconds...").Info(player.GetIdentity());
            }
        }
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ActivateNewRound, 10000, false);
        
        Print("[KOTH_RoundEndModule] New round notification sent");
        Print("[KOTH_RoundEndModule] ============================================");
    }
    
    void ActivateNewRound()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] ============================================");
        Print("[KOTH_RoundEndModule] Activating new round NOW");
        
        m_RoundEndInProgress = false;
        
        if (m_GameMode)
        {
            m_GameMode.StartRound();
            Print("[KOTH_RoundEndModule] GameMode.StartRound() called");
        }
        
        KOTH_Area mainZone = KOTH_Area.GetInstance();
        if (mainZone)
        {
            KOTH_AreaTrigger trigger = mainZone.GetMainTrigger();
            if (trigger)
            {
                trigger.SetActive(true);
                Print("[KOTH_RoundEndModule] Main zone trigger activated");
            }
        }
        
        KOTH_PriorityZoneManager.StartMovement();
        Print("[KOTH_RoundEndModule] Priority zone movement started");
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity())
            {
                ExpansionNotification("Round Started!", "Fight for your team!").Success(player.GetIdentity());
            }
        }
        
        Print("[KOTH_RoundEndModule] New round fully activated");
        Print("[KOTH_RoundEndModule] ============================================");
    }
    
    string DetermineNextZone()
    {
        if (!m_VotingEnabled || m_ZoneVotes.Count() == 0)
        {
            return "";
        }
        
        array<string> tiedZones = new array<string>;
        int highestVotes = 0;
        
        foreach (string zoneName, int voteCount : m_ZoneVotes)
        {
            if (voteCount > highestVotes)
            {
                highestVotes = voteCount;
                tiedZones.Clear();
                tiedZones.Insert(zoneName);
            }
            else if (voteCount == highestVotes && voteCount > 0)
            {
                tiedZones.Insert(zoneName);
            }
        }
        
        if (tiedZones.Count() == 0)
        {
            return "";
        }
        
        if (tiedZones.Count() == 1)
        {
            Print("[KOTH_RoundEndModule] Voting result: " + tiedZones[0] + " with " + highestVotes + " votes");
            return tiedZones[0];
        }
        
        int randomIndex = Math.RandomInt(0, tiedZones.Count());
        string winningZone = tiedZones[randomIndex];
        Print("[KOTH_RoundEndModule] Tie broken randomly: " + winningZone + " with " + highestVotes + " votes (tied with " + tiedZones.Count() + " zones)");
        return winningZone;
    }
    
    PlayerBase GetPlayerByUID(string uid)
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity())
            {
                if (player.GetIdentity().GetId() == uid)
                {
                    return player;
                }
            }
        }
        
        return null;
    }
}