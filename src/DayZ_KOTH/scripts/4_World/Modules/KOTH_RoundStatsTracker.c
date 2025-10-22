/**
 * KOTH_RoundStatsTracker.c (PHASE 1A - REAL-TIME STAT TRACKING)
 *
 * King of the Hill by Kahoona
 * Tracks all player and team statistics during a round
 * Includes play time tracking for fair end-round bonuses
 *
 * Place in: 4_World/Modules/KOTH_RoundStatsTracker.c
 */

class KOTH_RoundPlayerStats
{
    string PlayerUID;
    string PlayerName;
    int CurrentLevel;
    
    int Kills;
    int Deaths;
    float KDRatio;
    int Headshots;
    int Teamkills;
    int Revives;
    int LongestKill;
    
    int XPEarned;
    int MoneyEarned;
    
    float JoinTime;
    float TotalPlayTime;
    float PlayTimeRatio;
    
    void KOTH_RoundPlayerStats(string uid, string name, int level, float joinTime)
    {
        PlayerUID = uid;
        PlayerName = name;
        CurrentLevel = level;
        JoinTime = joinTime;
        
        Kills = 0;
        Deaths = 0;
        KDRatio = 0.0;
        Headshots = 0;
        Teamkills = 0;
        Revives = 0;
        LongestKill = 0;
        XPEarned = 0;
        MoneyEarned = 0;
        TotalPlayTime = 0;
        PlayTimeRatio = 0;
    }
    
    void CalculateKDRatio()
    {
        if (Deaths == 0)
        {
            if (Kills > 0)
            {
                KDRatio = Kills;
            }
            else
            {
                KDRatio = 0.0;
            }
        }
        else
        {
            KDRatio = Kills / Deaths;
        }
    }
}

class KOTH_RoundTeamStats
{
    string TeamName;
    
    int TotalKills;
    int TotalDeaths;
    int TotalHeadshots;
    int TotalRevives;
    int TotalTeamkills;
    int FinalScore;
    int TotalXPEarned;
    int TotalMoneyEarned;
    
    void KOTH_RoundTeamStats(string teamName)
    {
        TeamName = teamName;
        TotalKills = 0;
        TotalDeaths = 0;
        TotalHeadshots = 0;
        TotalRevives = 0;
        TotalTeamkills = 0;
        FinalScore = 0;
        TotalXPEarned = 0;
        TotalMoneyEarned = 0;
    }
}

[CF_RegisterModule(KOTH_RoundStatsTracker)]
class KOTH_RoundStatsTracker: CF_ModuleWorld
{
    private static ref KOTH_RoundStatsTracker s_Instance;
    
    static ref ScriptInvoker SI_OnRoundStatsAvailable = new ScriptInvoker();
    static ref ScriptInvoker SI_OnStatsUpdated = new ScriptInvoker();
    
    private ref map<string, ref KOTH_RoundPlayerStats> m_PlayerStats;
    private ref KOTH_RoundTeamStats m_EastTeamStats;
    private ref KOTH_RoundTeamStats m_WestTeamStats;
    
    private float m_RoundStartTime;
    private float m_RoundEndTime;
    private bool m_RoundActive;
    
    void KOTH_RoundStatsTracker()
    {
        s_Instance = this;
        m_PlayerStats = new map<string, ref KOTH_RoundPlayerStats>();
        m_EastTeamStats = new KOTH_RoundTeamStats("East");
        m_WestTeamStats = new KOTH_RoundTeamStats("West");
        m_RoundActive = false;
        
        if (!SI_OnRoundStatsAvailable)
            SI_OnRoundStatsAvailable = new ScriptInvoker();
        if (!SI_OnStatsUpdated)
            SI_OnStatsUpdated = new ScriptInvoker();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        Expansion_RegisterClientRPC("RPC_SyncRoundStats");
        Expansion_RegisterServerRPC("RPC_RequestRoundStats");
        
        Print("[KOTH_RoundStatsTracker] Initialized with real-time tracking and RPC support");
    }
    
    static KOTH_RoundStatsTracker GetInstance()
    {
        return s_Instance;
    }
    
    void OnRoundStart()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundStatsTracker] Round started - Initializing stats");
        
        m_RoundStartTime = GetGame().GetTickTime();
        m_RoundActive = true;
        
        ResetAllStats();
        
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity())
            {
                string uid = player.GetIdentity().GetId();
                string name = player.GetIdentity().GetName();
                int level = GetPlayerLevel(uid);
                
                KOTH_RoundPlayerStats stats = new KOTH_RoundPlayerStats(uid, name, level, m_RoundStartTime);
                m_PlayerStats.Set(uid, stats);
                
                Print("[KOTH_RoundStatsTracker] Registered player: " + name + " (Level " + level + ")");
            }
        }
    }
    
    void OnPlayerJoinRound(PlayerBase player)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        
        string uid = player.GetIdentity().GetId();
        
        if (m_PlayerStats.Contains(uid))
        {
            KOTH_RoundPlayerStats existingStats = m_PlayerStats.Get(uid);
            existingStats.JoinTime = GetGame().GetTickTime();
            Print("[KOTH_RoundStatsTracker] Player rejoined: " + existingStats.PlayerName);
        }
        else
        {
            string name = player.GetIdentity().GetName();
            int level = GetPlayerLevel(uid);
            float joinTime = GetGame().GetTickTime();
            
            KOTH_RoundPlayerStats newStats = new KOTH_RoundPlayerStats(uid, name, level, joinTime);
            m_PlayerStats.Set(uid, newStats);
            
            Print("[KOTH_RoundStatsTracker] Player joined mid-round: " + name + " (Level " + level + ")");
        }
    }
    
    void OnRoundEnd()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundStatsTracker] ============================================");
        Print("[KOTH_RoundStatsTracker] >>>>>> OnRoundEnd CALLED <<<<<<");
        Print("[KOTH_RoundStatsTracker] ============================================");
        
        m_RoundEndTime = GetGame().GetTickTime();
        m_RoundActive = false;
        
        CalculateAllPlayTimes();
        
        Print("[KOTH_RoundStatsTracker] Round ended - Final statistics:");
        Print("[KOTH_RoundStatsTracker]   - East Team: " + m_EastTeamStats.TotalKills + " kills, " + m_EastTeamStats.FinalScore + " score, " + m_EastTeamStats.TotalXPEarned + " XP");
        Print("[KOTH_RoundStatsTracker]   - West Team: " + m_WestTeamStats.TotalKills + " kills, " + m_WestTeamStats.FinalScore + " score, " + m_WestTeamStats.TotalXPEarned + " XP");
        Print("[KOTH_RoundStatsTracker]   - Total Players Tracked: " + m_PlayerStats.Count());
        
        Print("[KOTH_RoundStatsTracker] About to sync stats to all clients...");
        SyncRoundStatsToAllClients();
        
        Print("[KOTH_RoundStatsTracker] About to invoke SI_OnRoundStatsAvailable...");
        SI_OnRoundStatsAvailable.Invoke(m_EastTeamStats, m_WestTeamStats);
        Print("[KOTH_RoundStatsTracker] SI_OnRoundStatsAvailable invoked successfully");
        Print("[KOTH_RoundStatsTracker] ============================================");
    }
    
    void CalculateAllPlayTimes()
    {
        float roundDuration = m_RoundEndTime - m_RoundStartTime;
        
        if (roundDuration < 1.0)
            roundDuration = 1.0;
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {
            stats.TotalPlayTime = m_RoundEndTime - stats.JoinTime;
            stats.PlayTimeRatio = stats.TotalPlayTime / roundDuration;
            
            if (stats.PlayTimeRatio > 1.0)
                stats.PlayTimeRatio = 1.0;
            if (stats.PlayTimeRatio < 0.0)
                stats.PlayTimeRatio = 0.0;
            
            Print("[KOTH_RoundStatsTracker] " + stats.PlayerName + " play time: " + stats.TotalPlayTime + "s (" + (stats.PlayTimeRatio * 100) + "%)");
        }
    }
    
    void RecordKill(string killerUID, string killerName, string killerTeam, bool wasHeadshot, int distance)
    {
        if (!GetGame().IsServer() || !m_RoundActive)
            return;
        Print("[KOTH_RoundStatsTracker] RecordKill called - Killer: " + killerName + ", Team: " + killerTeam + ", Headshot: " + wasHeadshot + ", Distance: " + distance);
        KOTH_RoundPlayerStats killerStats = GetOrCreatePlayerStats(killerUID, killerName);
        
        killerStats.Kills = killerStats.Kills + 1;
        Print("[KOTH_RoundStatsTracker] " + killerName + " now has " + killerStats.Kills + " kills");

        
        if (wasHeadshot)
        {
            killerStats.Headshots = killerStats.Headshots + 1;
        }
        
        if (distance > killerStats.LongestKill)
        {
            killerStats.LongestKill = distance;
        }
        
        killerStats.CalculateKDRatio();
        
        if (killerTeam == "East")
        {
            m_EastTeamStats.TotalKills = m_EastTeamStats.TotalKills + 1;
            if (wasHeadshot)
            {
                m_EastTeamStats.TotalHeadshots = m_EastTeamStats.TotalHeadshots + 1;
            }
        }
        else if (killerTeam == "West")
        {
            m_WestTeamStats.TotalKills = m_WestTeamStats.TotalKills + 1;
            if (wasHeadshot)
            {
                m_WestTeamStats.TotalHeadshots = m_WestTeamStats.TotalHeadshots + 1;
            }
        }
    }
    
    void RecordDeath(string victimUID, string victimName, string victimTeam)
    {
        if (!GetGame().IsServer() || !m_RoundActive)
            return;
        
        KOTH_RoundPlayerStats victimStats = GetOrCreatePlayerStats(victimUID, victimName);
        
        victimStats.Deaths = victimStats.Deaths + 1;
        victimStats.CalculateKDRatio();
        
        if (victimTeam == "East")
        {
            m_EastTeamStats.TotalDeaths = m_EastTeamStats.TotalDeaths + 1;
        }
        else if (victimTeam == "West")
        {
            m_WestTeamStats.TotalDeaths = m_WestTeamStats.TotalDeaths + 1;
        }
    }
    
    void RecordTeamkill(string killerUID, string killerName, string killerTeam)
    {
        if (!GetGame().IsServer() || !m_RoundActive)
            return;
        
        KOTH_RoundPlayerStats teamkillerStats = GetOrCreatePlayerStats(killerUID, killerName);
        
        teamkillerStats.Teamkills = teamkillerStats.Teamkills + 1;
        
        if (killerTeam == "East")
        {
            m_EastTeamStats.TotalTeamkills = m_EastTeamStats.TotalTeamkills + 1;
        }
        else if (killerTeam == "West")
        {
            m_WestTeamStats.TotalTeamkills = m_WestTeamStats.TotalTeamkills + 1;
        }
    }
    
    void RecordRevive(string medicUID, string medicName, string medicTeam)
    {
        if (!GetGame().IsServer() || !m_RoundActive)
            return;
        
        KOTH_RoundPlayerStats medicStats = GetOrCreatePlayerStats(medicUID, medicName);
        
        medicStats.Revives = medicStats.Revives + 1;
        
        if (medicTeam == "East")
        {
            m_EastTeamStats.TotalRevives = m_EastTeamStats.TotalRevives + 1;
        }
        else if (medicTeam == "West")
        {
            m_WestTeamStats.TotalRevives = m_WestTeamStats.TotalRevives + 1;
        }
    }
    
    void RecordXPGained(string playerUID, string playerName, int xpAmount)
    {
        if (!GetGame().IsServer() || !m_RoundActive)
            return;
        
        KOTH_RoundPlayerStats xpPlayerStats = GetOrCreatePlayerStats(playerUID, playerName);
        
        xpPlayerStats.XPEarned = xpPlayerStats.XPEarned + xpAmount;
        
        PlayerBase player = GetPlayerByUID(playerUID);
        if (player)
        {
            string team = player.GetKOTHTeam();
            
            if (team == "East")
            {
                m_EastTeamStats.TotalXPEarned = m_EastTeamStats.TotalXPEarned + xpAmount;
            }
            else if (team == "West")
            {
                m_WestTeamStats.TotalXPEarned = m_WestTeamStats.TotalXPEarned + xpAmount;
            }
        }
    }
    
    void RecordMoneyGained(string playerUID, string playerName, int moneyAmount)
    {
        if (!GetGame().IsServer() || !m_RoundActive)
            return;
        
        KOTH_RoundPlayerStats moneyPlayerStats = GetOrCreatePlayerStats(playerUID, playerName);
        
        moneyPlayerStats.MoneyEarned = moneyPlayerStats.MoneyEarned + moneyAmount;
        
        PlayerBase player = GetPlayerByUID(playerUID);
        if (player)
        {
            string team = player.GetKOTHTeam();
            
            if (team == "East")
            {
                m_EastTeamStats.TotalMoneyEarned = m_EastTeamStats.TotalMoneyEarned + moneyAmount;
            }
            else if (team == "West")
            {
                m_WestTeamStats.TotalMoneyEarned = m_WestTeamStats.TotalMoneyEarned + moneyAmount;
            }
        }
    }
    
    void SetTeamFinalScore(string team, int score)
    {
        if (!GetGame().IsServer())
            return;
        
        if (team == "East")
        {
            m_EastTeamStats.FinalScore = score;
        }
        else if (team == "West")
        {
            m_WestTeamStats.FinalScore = score;
        }
    }
    
    KOTH_RoundPlayerStats GetOrCreatePlayerStats(string uid, string name)
    {
        if (m_PlayerStats.Contains(uid))
        {
            return m_PlayerStats.Get(uid);
        }
        
        int level = GetPlayerLevel(uid);
        float joinTime = GetGame().GetTickTime();
        
        KOTH_RoundPlayerStats stats = new KOTH_RoundPlayerStats(uid, name, level, joinTime);
        m_PlayerStats.Set(uid, stats);
        
        return stats;
    }
    
    KOTH_RoundPlayerStats GetPlayerStats(string uid)
    {
        if (m_PlayerStats.Contains(uid))
        {
            return m_PlayerStats.Get(uid);
        }
        return null;
    }
    
    KOTH_RoundTeamStats GetTeamStats(string team)
    {
        if (team == "East")
        {
            return m_EastTeamStats;
        }
        else if (team == "West")
        {
            return m_WestTeamStats;
        }
        return null;
    }
    
    array<ref KOTH_RoundPlayerStats> GetTopPlayersByKills(int count)
    {
        array<ref KOTH_RoundPlayerStats> allPlayers = new array<ref KOTH_RoundPlayerStats>();
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {
            allPlayers.Insert(stats);
        }
        
        for (int i = 0; i < allPlayers.Count() - 1; i++)
        {
            for (int j = i + 1; j < allPlayers.Count(); j++)
            {
                if (allPlayers.Get(j).Kills > allPlayers.Get(i).Kills)
                {
                    KOTH_RoundPlayerStats temp = allPlayers.Get(i);
                    allPlayers.Set(i, allPlayers.Get(j));
                    allPlayers.Set(j, temp);
                }
            }
        }
        
        array<ref KOTH_RoundPlayerStats> topPlayers = new array<ref KOTH_RoundPlayerStats>();
        
        for (int k = 0; k < count && k < allPlayers.Count(); k++)
        {
            topPlayers.Insert(allPlayers.Get(k));
        }
        
        return topPlayers;
    }
    
    KOTH_RoundPlayerStats GetMVP()
    {
        KOTH_RoundPlayerStats mvp = null;
        int highestKills = 0;
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {

            
            if (stats.Kills > highestKills)
            {
                highestKills = stats.Kills;
                mvp = stats;
            }
        }
        
        return mvp;
    }
    
    KOTH_RoundPlayerStats GetSharpshooter()
    {
        KOTH_RoundPlayerStats sharpshooter = null;
        int highestHeadshots = 0;
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {

            
            if (stats.Headshots > highestHeadshots)
            {
                highestHeadshots = stats.Headshots;
                sharpshooter = stats;
            }
        }
        
        return sharpshooter;
    }

    
    KOTH_RoundPlayerStats GetTopMedic()
    {
        KOTH_RoundPlayerStats medic = null;
        int highestRevives = 0;
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {

            
            if (stats.Revives > highestRevives)
            {
                highestRevives = stats.Revives;
                medic = stats;
            }
        }
        
        return medic;
    }
    
    void ResetAllStats()
    {
        m_PlayerStats.Clear();
        
        m_EastTeamStats = new KOTH_RoundTeamStats("East");
        m_WestTeamStats = new KOTH_RoundTeamStats("West");
        
        Print("[KOTH_RoundStatsTracker] All stats reset");
    }
    
    int GetPlayerLevel(string uid)
    {
        KOTH_PlayerRewardManager rewardManager = KOTH_PlayerRewardManager.GetInstance();
        if (!rewardManager)
            return 1;
        
        KOTH_Players data = rewardManager.GetPlayerData(uid);
        if (!data)
            return 1;
        
        return data.CurrentLevel;
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
    
    bool IsRoundActive()
    {
        return m_RoundActive;
    }
    
    map<string, ref KOTH_RoundPlayerStats> GetAllPlayerStats()
    {
        return m_PlayerStats;
    }
    
    void SyncRoundStatsToAllClients()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundStatsTracker] >>>>>> SyncRoundStatsToAllClients CALLED <<<<<<");
        
        auto rpc = Expansion_CreateRPC("RPC_SyncRoundStats");
        
        WriteTeamStats(rpc, m_EastTeamStats);
        WriteTeamStats(rpc, m_WestTeamStats);
        
        rpc.Write(m_PlayerStats.Count());
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {
            WritePlayerStats(rpc, stats);
        }
        
        rpc.Expansion_Send(true, null);
        
        Print("[KOTH_RoundStatsTracker] ✅ Synced round stats to all clients - " + m_PlayerStats.Count() + " players");
    }
    
    void WriteTeamStats(ParamsWriteContext ctx, KOTH_RoundTeamStats teamStats)
    {
        ctx.Write(teamStats.TeamName);
        ctx.Write(teamStats.TotalKills);
        ctx.Write(teamStats.TotalDeaths);
        ctx.Write(teamStats.TotalHeadshots);
        ctx.Write(teamStats.TotalRevives);
        ctx.Write(teamStats.TotalTeamkills);
        ctx.Write(teamStats.FinalScore);
        ctx.Write(teamStats.TotalXPEarned);
        ctx.Write(teamStats.TotalMoneyEarned);
    }
    
    KOTH_RoundTeamStats ReadTeamStats(ParamsReadContext ctx)
    {
        string teamName;
        if (!ctx.Read(teamName))
            return null;
        
        KOTH_RoundTeamStats stats = new KOTH_RoundTeamStats(teamName);
        
        if (!ctx.Read(stats.TotalKills))
            return null;
        if (!ctx.Read(stats.TotalDeaths))
            return null;
        if (!ctx.Read(stats.TotalHeadshots))
            return null;
        if (!ctx.Read(stats.TotalRevives))
            return null;
        if (!ctx.Read(stats.TotalTeamkills))
            return null;
        if (!ctx.Read(stats.FinalScore))
            return null;
        if (!ctx.Read(stats.TotalXPEarned))
            return null;
        if (!ctx.Read(stats.TotalMoneyEarned))
            return null;
        
        return stats;
    }
    
    void WritePlayerStats(ParamsWriteContext ctx, KOTH_RoundPlayerStats stats)
    {
        ctx.Write(stats.PlayerUID);
        ctx.Write(stats.PlayerName);
        ctx.Write(stats.CurrentLevel);
        ctx.Write(stats.Kills);
        ctx.Write(stats.Deaths);
        ctx.Write(stats.KDRatio);
        ctx.Write(stats.Headshots);
        ctx.Write(stats.Teamkills);
        ctx.Write(stats.Revives);
        ctx.Write(stats.LongestKill);
        ctx.Write(stats.XPEarned);
        ctx.Write(stats.MoneyEarned);
        ctx.Write(stats.JoinTime);
        ctx.Write(stats.TotalPlayTime);
        ctx.Write(stats.PlayTimeRatio);
    }
    
    KOTH_RoundPlayerStats ReadPlayerStats(ParamsReadContext ctx)
    {
        string uid;
        if (!ctx.Read(uid))
            return null;
        
        string name;
        if (!ctx.Read(name))
            return null;
        
        int level;
        if (!ctx.Read(level))
            return null;
        
        // CREATE THE OBJECT FIRST with temporary joinTime
        KOTH_RoundPlayerStats playerStats = new KOTH_RoundPlayerStats(uid, name, level, 0);
        
        // NOW READ IN THE SAME ORDER AS WRITE
        if (!ctx.Read(playerStats.Kills))
            return null;
        if (!ctx.Read(playerStats.Deaths))
            return null;
        if (!ctx.Read(playerStats.KDRatio))
            return null;
        if (!ctx.Read(playerStats.Headshots))
            return null;
        if (!ctx.Read(playerStats.Teamkills))
            return null;
        if (!ctx.Read(playerStats.Revives))
            return null;
        if (!ctx.Read(playerStats.LongestKill))
            return null;
        if (!ctx.Read(playerStats.XPEarned))
            return null;
        if (!ctx.Read(playerStats.MoneyEarned))
            return null;
        if (!ctx.Read(playerStats.JoinTime))
            return null;
        if (!ctx.Read(playerStats.TotalPlayTime))
            return null;
        if (!ctx.Read(playerStats.PlayTimeRatio))
            return null;
        
        return playerStats;
    }
    
    void RPC_SyncRoundStats(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        Print("[KOTH_RoundStatsTracker] ============================================");
        Print("[KOTH_RoundStatsTracker] CLIENT: RPC_SyncRoundStats RECEIVED");
        Print("[KOTH_RoundStatsTracker] ============================================");
        
        m_EastTeamStats = ReadTeamStats(ctx);
        if (!m_EastTeamStats)
        {
            Error("[KOTH_RoundStatsTracker] ❌ CLIENT: Failed to read East team stats");
            return;
        }
        
        m_WestTeamStats = ReadTeamStats(ctx);
        if (!m_WestTeamStats)
        {
            Error("[KOTH_RoundStatsTracker] ❌ CLIENT: Failed to read West team stats");
            return;
        }
        
        int playerCount;
        if (!ctx.Read(playerCount))
        {
            Error("[KOTH_RoundStatsTracker] ❌ CLIENT: Failed to read player count");
            return;
        }
        
        m_PlayerStats.Clear();
        
        for (int i = 0; i < playerCount; i++)
        {
            KOTH_RoundPlayerStats stats = ReadPlayerStats(ctx);
            if (stats)
            {
                m_PlayerStats.Set(stats.PlayerUID, stats);
            }
            else
            {
                Error("[KOTH_RoundStatsTracker] ❌ CLIENT: Failed to read player stats #" + i);
            }
        }
        
        Print("[KOTH_RoundStatsTracker] ✅ CLIENT: Received round stats - " + playerCount + " players");
        Print("[KOTH_RoundStatsTracker]   - East: " + m_EastTeamStats.TotalKills + " kills, " + m_EastTeamStats.FinalScore + " score");
        Print("[KOTH_RoundStatsTracker]   - West: " + m_WestTeamStats.TotalKills + " kills, " + m_WestTeamStats.FinalScore + " score");
        
        Print("[KOTH_RoundStatsTracker] CLIENT: Invoking SI_OnRoundStatsAvailable...");
        SI_OnRoundStatsAvailable.Invoke(m_EastTeamStats, m_WestTeamStats);
        Print("[KOTH_RoundStatsTracker] ============================================");
    }
    
    void RPC_RequestRoundStats(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer() || !sender)
            return;
        
        Print("[KOTH_RoundStatsTracker] Client " + sender.GetName() + " requested round stats");
        
        auto rpc = Expansion_CreateRPC("RPC_SyncRoundStats");
        
        WriteTeamStats(rpc, m_EastTeamStats);
        WriteTeamStats(rpc, m_WestTeamStats);
        
        rpc.Write(m_PlayerStats.Count());
        
        foreach (string uid, KOTH_RoundPlayerStats stats : m_PlayerStats)
        {
            WritePlayerStats(rpc, stats);
        }
        
        rpc.Expansion_Send(true, sender);
    }
    
    static ScriptInvoker GetRoundStatsAvailableSI()
    {
        return SI_OnRoundStatsAvailable;
    }
    
    static ScriptInvoker GetStatsUpdatedSI()
    {
        return SI_OnStatsUpdated;
    }
}