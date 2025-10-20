/**
 * KOTH_RoundEndModule.c (CORRECTED - EXPANSION MENU STANDARD)
 *
 * King of the Hill by Kahoona
 * Orchestrates round end: stats, bonuses, voting, teleportation, AI cleanup
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
    
    void KOTH_RoundEndModule()
    {
        s_Instance = this;
        m_ZoneVotes = new map<string, int>();
        m_AvailableZones = new array<string>();
        m_RoundEndMenuInvoker = new ScriptInvoker();
        
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
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (!settings)
        {
            m_EndScreenDisplaySeconds = 90;
            m_VoteTimeSeconds = 30;
            m_VotingEnabled = false;
            Print("[KOTH_RoundEndModule] WARNING: Could not load settings, using defaults");
            return;
        }
        
        m_EndScreenDisplaySeconds = settings.EndScreenDisplaySeconds;
        m_VoteTimeSeconds = settings.VoteTimeSeconds;
        m_VotingEnabled = (settings.ZoneSelectionMode == 2);
        
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
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] ============================================");
        Print("[KOTH_RoundEndModule] OnRoundEnd INVOKED");
        Print("[KOTH_RoundEndModule] Winner: " + winningTeam);
        Print("[KOTH_RoundEndModule] Scores - East: " + eastScore + ", West: " + westScore);
        Print("[KOTH_RoundEndModule] ============================================");
        
        Print("[KOTH_RoundEndModule] Step 1: Calculating and awarding bonuses...");
        CalculateAndAwardBonuses(winningTeam);
        
        Print("[KOTH_RoundEndModule] Step 2: Preparing next zone...");
        PrepareNextZone();
        
        Print("[KOTH_RoundEndModule] Step 3: Showing round end screen to all clients...");
        ShowRoundEndScreenToAllClients(winningTeam);
        
        Print("[KOTH_RoundEndModule] Step 4: Scheduling AI cleanup in 5 seconds...");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupAIs, 5000, false);
        
        float teleportDelay = m_EndScreenDisplaySeconds - 5.0;
        if (teleportDelay < 1.0)
            teleportDelay = 1.0;
        
        Print("[KOTH_RoundEndModule] Step 5: Scheduling player teleportation in " + teleportDelay + " seconds...");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TeleportAllPlayers, teleportDelay * 1000, false);
        
        Print("[KOTH_RoundEndModule] All round end steps initiated");
        Print("[KOTH_RoundEndModule] ============================================");
    }

    
    void CalculateAndAwardBonuses(string winningTeam)
    {
        if (!m_StatsTracker || !m_RewardManager)
            return;
        
        Print("[KOTH_RoundEndModule] Calculating end-round bonuses...");
        
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (!settings)
            return;
        
        KOTH_RoundTeamStats eastStats = m_StatsTracker.GetTeamStats("East");
        KOTH_RoundTeamStats westStats = m_StatsTracker.GetTeamStats("West");
        
        if (!eastStats || !westStats)
        {
            Error("[KOTH_RoundEndModule] Failed to get team stats!");
            return;
        }
        
        AwardTopPerformers(settings);
        AwardTeamBonuses(winningTeam, eastStats, westStats, settings);
        
        Print("[KOTH_RoundEndModule] Bonuses awarded");
    }
    
    void AwardTopPerformers(KOTH_Settings settings)
    {
        KOTH_RoundPlayerStats mvp = m_StatsTracker.GetMVP();
        if (mvp && mvp.Kills > 0)
        {
            PlayerBase mvpPlayer = GetPlayerByUID(mvp.PlayerUID);
            if (mvpPlayer)
            {
                m_RewardManager.AddPlayerXP(mvpPlayer, settings.MVPBonusXP, "MVP Award");
                m_RewardManager.AddPlayerMoney(mvpPlayer, settings.MVPBonusMoney, "MVP Award");
                KOTH_NotificationModule.ShowNotificationAdvanced("MVP AWARD!", "$" + settings.MVPBonusMoney.ToString(), ARGB(255, 255, 215, 0), settings.MVPBonusXP.ToString() + "XP", ARGB(255, 255, 215, 0), ARGB(255, 218, 165, 32), 5.0, mvpPlayer.GetIdentity());
                Print("[KOTH_RoundEndModule] MVP Award: " + mvp.PlayerName + " (" + mvp.Kills + " kills)");
            }
        }
        
        KOTH_RoundPlayerStats sharpshooter = m_StatsTracker.GetSharpshooter();
        if (sharpshooter && sharpshooter.Headshots > 0)
        {
            PlayerBase ssPlayer = GetPlayerByUID(sharpshooter.PlayerUID);
            if (ssPlayer)
            {
                m_RewardManager.AddPlayerXP(ssPlayer, settings.SharpshooterBonusXP, "Sharpshooter Award");
                m_RewardManager.AddPlayerMoney(ssPlayer, settings.SharpshooterBonusMoney, "Sharpshooter Award");
                KOTH_NotificationModule.ShowNotificationAdvanced("SHARPSHOOTER AWARD!", "$" + settings.SharpshooterBonusMoney.ToString(), ARGB(255, 255, 140, 0), settings.SharpshooterBonusXP.ToString() + "XP", ARGB(255, 255, 140, 0), ARGB(255, 255, 69, 0), 5.0, ssPlayer.GetIdentity());
                Print("[KOTH_RoundEndModule] Sharpshooter Award: " + sharpshooter.PlayerName + " (" + sharpshooter.Headshots + " headshots)");
            }
        }
        
        KOTH_RoundPlayerStats medic = m_StatsTracker.GetTopMedic();
        if (medic && medic.Revives > 0)
        {
            PlayerBase medicPlayer = GetPlayerByUID(medic.PlayerUID);
            if (medicPlayer)
            {
                m_RewardManager.AddPlayerXP(medicPlayer, settings.MedicBonusXP, "Top Medic Award");
                m_RewardManager.AddPlayerMoney(medicPlayer, settings.MedicBonusMoney, "Top Medic Award");
                KOTH_NotificationModule.ShowNotificationAdvanced("TOP MEDIC AWARD!", "$" + settings.MedicBonusMoney.ToString(), ARGB(255, 0, 255, 127), settings.MedicBonusXP.ToString() + "XP", ARGB(255, 0, 255, 127), ARGB(255, 60, 179, 113), 5.0, medicPlayer.GetIdentity());
                Print("[KOTH_RoundEndModule] Top Medic Award: " + medic.PlayerName + " (" + medic.Revives + " revives)");
            }
        }
    }
    
    void AwardTeamBonuses(string winningTeam, KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats, KOTH_Settings settings)
    {
        ref array<Man> players = new array<Man>;
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
                bonusXP = CalculateWinningBonus(playerStats, team, eastStats, westStats, settings, true);
                bonusMoney = CalculateWinningBonus(playerStats, team, eastStats, westStats, settings, false);
                m_RewardManager.AddPlayerXP(player, bonusXP, "Victory Bonus");
                m_RewardManager.AddPlayerMoney(player, bonusMoney, "Victory Bonus");
                KOTH_NotificationModule.ShowNotificationAdvanced("VICTORY BONUS!", "$" + bonusMoney.ToString(), ARGB(255, 0, 255, 0), bonusXP.ToString() + "XP", ARGB(255, 144, 238, 144), ARGB(255, 34, 139, 34), 5.0, player.GetIdentity());
            }
            else
            {
                bonusXP = CalculateLosingBonus(playerStats, team, eastStats, westStats, settings, true);
                bonusMoney = CalculateLosingBonus(playerStats, team, eastStats, westStats, settings, false);
                m_RewardManager.AddPlayerXP(player, bonusXP, "Participation Bonus");
                m_RewardManager.AddPlayerMoney(player, bonusMoney, "Participation Bonus");
                KOTH_NotificationModule.ShowNotificationAdvanced("Participation Bonus", "$" + bonusMoney.ToString(), ARGB(255, 169, 169, 169), bonusXP.ToString() + "XP", ARGB(255, 169, 169, 169), ARGB(255, 128, 128, 128), 5.0, player.GetIdentity());
            }
            
            Print("[KOTH_RoundEndModule] Team Bonus: " + player.GetIdentity().GetName() + " (" + team + ") - XP: " + bonusXP + ", Money: " + bonusMoney);
        }
    }
    
    int CalculateWinningBonus(KOTH_RoundPlayerStats playerStats, string team, KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats, KOTH_Settings settings, bool isXP)
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
        int maxLevel = settings.MaxPlayerLevel;
        float playTimeRatio = playerStats.PlayTimeRatio;
        float levelRatio = playerLevel / maxLevel;
        
        int bonus;
        if (isXP)
        {
            bonus = totalTeamXP * levelRatio * settings.WinningTeamXPMultiplier * playTimeRatio;
        }
        else
        {
            bonus = totalTeamMoney * levelRatio * settings.WinningTeamMoneyMultiplier * playTimeRatio;
        }
        
        return bonus;
    }
    
    int CalculateLosingBonus(KOTH_RoundPlayerStats playerStats, string team, KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats, KOTH_Settings settings, bool isXP)
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
        
        int scoreLimit = settings.ScoreLimit;
        int playerLevel = playerStats.CurrentLevel;
        int maxLevel = settings.MaxPlayerLevel;
        float playTimeRatio = playerStats.PlayTimeRatio;
        float scoreRatio = teamFinalScore / scoreLimit;
        float levelRatio = playerLevel / maxLevel;
        
        int bonus;
        if (isXP)
        {
            bonus = totalTeamXP * scoreRatio * levelRatio * settings.LosingTeamXPMultiplier * playTimeRatio;
        }
        else
        {
            bonus = totalTeamMoney * scoreRatio * levelRatio * settings.LosingTeamMoneyMultiplier * playTimeRatio;
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
        
        rpc.Expansion_Send(true, null);
        
        Print("[KOTH_RoundEndModule] Sent round end screen RPC to all clients");
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
        
        Print("[KOTH_RoundEndModule] CLIENT: Scheduling menu display...");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, votingEnabled);
    }
    
    void Exec_ShowRoundEndMenu(array<string> zones, bool votingEnabled)
    {
        Print("[KOTH_RoundEndModule] CLIENT: Exec_ShowRoundEndMenu called");
        
        if (GetDayZGame().IsLoading())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, votingEnabled);
            return;
        }

        if (GetGame().GetUIManager().GetMenu())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, votingEnabled);
            return;
        }

        if (GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, votingEnabled);
            return;
        }

        if (GetDayZGame().GetMissionState() != DayZGame.MISSION_STATE_GAME)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, votingEnabled);
            return;
        }

        if (!GetGame().GetMission().GetHud())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowRoundEndMenu, 500, false, zones, votingEnabled);
            return;
        }
        
        Print("[KOTH_RoundEndModule] CLIENT: All checks passed, creating menu...");
        
        if (!OpenRoundEndMenu())
            return;
        
        m_RoundEndMenuInvoker.Invoke(zones, votingEnabled);
        
        Print("[KOTH_RoundEndModule] CLIENT: Round end menu created and invoked");
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
    
    void CleanupAIs()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundEndModule] Cleaning up AI entities...");
        
        int aiCount = 0;
        ref array<Man> entities = new array<Man>;
        GetGame().GetPlayers(entities);
        
        for (int i = 0; i < entities.Count(); i++)
        {
            PlayerBase entity = PlayerBase.Cast(entities.Get(i));
            if (!entity)
                continue;
            
            if (!entity.GetIdentity())
            {
                eAIBase ai = eAIBase.Cast(entity);
                if (ai)
                {
                    GetGame().ObjectDelete(ai);
                    aiCount++;
                }
            }
        }
        
        Print("[KOTH_RoundEndModule] Cleaned up " + aiCount + " AI entities");
    }
    
    void TeleportAllPlayers()
    {
        if (!GetGame().IsServer() || !m_ZoneManager)
            return;
        
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
        
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (!player || !player.GetIdentity())
                continue;
            
            string team = player.GetKOTHTeam();
            vector spawnPos;
            
            if (team == "East")
            {
                spawnPos = m_ZoneManager.GetEastSpawnPosition();
            }
            else if (team == "West")
            {
                spawnPos = m_ZoneManager.GetWestSpawnPosition();
            }
            else
            {
                continue;
            }
            
            if (spawnPos != "0 0 0")
            {
                player.SetPosition(spawnPos);
                Print("[KOTH_RoundEndModule] Teleported " + player.GetIdentity().GetName() + " to " + team + " spawn");
            }
        }
        
        Print("[KOTH_RoundEndModule] All players teleported");
    }
    
    string DetermineNextZone()
    {
        if (!m_VotingEnabled || m_ZoneVotes.Count() == 0)
        {
            return "";
        }
        
        string winningZone = "";
        int highestVotes = 0;
        
        foreach (string zoneName, int voteCount : m_ZoneVotes)
        {
            if (voteCount > highestVotes)
            {
                highestVotes = voteCount;
                winningZone = zoneName;
            }
        }
        
        if (highestVotes > 0)
        {
            Print("[KOTH_RoundEndModule] Voting result: " + winningZone + " with " + highestVotes + " votes");
            return winningZone;
        }
        
        return "";
    }
    
    PlayerBase GetPlayerByUID(string uid)
    {
        ref array<Man> players = new array<Man>;
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