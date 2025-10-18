/**
 * KOTH_GameMode.c (PHASE 4 - EVENT-DRIVEN ZONE COUNTS)
 *
 * King of the Hill by Kahoona
 * Receives player counts from triggers, not polling
 *
 * Place in: 4_World/Modules/KOTH_GameMode.c
 */

[CF_RegisterModule(KOTH_GameMode)]
class KOTH_GameMode: CF_ModuleWorld
{
    private static ref KOTH_GameMode s_Instance;
    
    static ref ScriptInvoker SI_OnScoreChanged = new ScriptInvoker();
    static ref ScriptInvoker SI_OnCaptureProgressChanged = new ScriptInvoker();
    static ref ScriptInvoker SI_OnRoundEnd = new ScriptInvoker();
    static ref ScriptInvoker SI_OnRoundStart = new ScriptInvoker();
    
    private int m_EastScore = 0;
    private int m_WestScore = 0;
    private bool m_RoundActive = false;
    private bool m_RoundEnded = false;
    
    private float m_CaptureProgress = 0.0;
    private string m_CapturingTeam = "None";
    private float m_LastTickTime = 0;
    
    private int m_ScoreLimit = 100;
    private float m_TickInterval = 2.0;
    private int m_PointsPerTick = 1;
    private float m_PriorityBonusMultiplier = 2.0;
    private int m_MinPlayersToInfluence = 1;
    private float m_PointsPerTickPerPlayer = 1.0;
    
    // PHASE 4: Cached player counts (set by triggers)
    private int m_CurrentEastAO = 0;
    private int m_CurrentWestAO = 0;
    private int m_CurrentEastPriority = 0;
    private int m_CurrentWestPriority = 0;
    
    private ref KOTH_HUDDataSync m_HUDSync;
    
    void KOTH_GameMode()
    {
        s_Instance = this;
        
        if (!SI_OnScoreChanged)
            SI_OnScoreChanged = new ScriptInvoker();
        if (!SI_OnCaptureProgressChanged)
            SI_OnCaptureProgressChanged = new ScriptInvoker();
        if (!SI_OnRoundEnd)
            SI_OnRoundEnd = new ScriptInvoker();
        if (!SI_OnRoundStart)
            SI_OnRoundStart = new ScriptInvoker();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        Expansion_RegisterClientRPC("RPC_RoundEnd");
        
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeGameMode, 3000, false);
        }
    }
    
    void InitializeGameMode()
    {
        if (!GetGame().IsServer())
            return;
        
        CF_Modules<KOTH_HUDDataSync>.Get(m_HUDSync);
        
        if (!m_HUDSync)
        {
            Error("[KOTH_GameMode] ERROR: Could not get KOTH_HUDDataSync module!");
            return;
        }
        
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (settings)
        {
            m_ScoreLimit = settings.ScoreLimit;
            m_TickInterval = settings.CaptureTickSeconds;
            m_PriorityBonusMultiplier = settings.PriorityZoneBonusMultiplier;
            m_MinPlayersToInfluence = settings.MinPlayersToInfluence;
            m_PointsPerTickPerPlayer = settings.PointsPerTickPerPlayer;
        }
        
        StartRound();
    }
    
    static KOTH_GameMode GetInstance()
    {
        return s_Instance;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 4: EVENT-DRIVEN PLAYER COUNT UPDATES
    // ═══════════════════════════════════════════════════════════════
    
    void OnAOZonePlayersChanged(int eastCount, int westCount)
    {
        if (!GetGame().IsServer())
            return;
        
        m_CurrentEastAO = eastCount;
        m_CurrentWestAO = westCount;
        
        // Broadcast to clients immediately
        if (m_HUDSync)
        {
            m_HUDSync.SetPlayerCounts(m_CurrentEastAO, m_CurrentWestAO, m_CurrentEastPriority, m_CurrentWestPriority);
        }
    }
    
    void OnPriorityZonePlayersChanged(int eastCount, int westCount)
    {
        if (!GetGame().IsServer())
            return;
        
        m_CurrentEastPriority = eastCount;
        m_CurrentWestPriority = westCount;
        
        // Broadcast to clients immediately
        if (m_HUDSync)
        {
            m_HUDSync.SetPlayerCounts(m_CurrentEastAO, m_CurrentWestAO, m_CurrentEastPriority, m_CurrentWestPriority);
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // ROUND MANAGEMENT
    // ═══════════════════════════════════════════════════════════════
    
    void StartRound()
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastScore = 0;
        m_WestScore = 0;
        m_RoundActive = true;
        m_RoundEnded = false;
        m_CaptureProgress = 0.0;
        m_CapturingTeam = "None";
        m_LastTickTime = 0;
        
        if (m_HUDSync)
        {
            m_HUDSync.SetEastScore(0);
            m_HUDSync.SetWestScore(0);
            m_HUDSync.SetCaptureProgress(0.0, "None");
        }
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateCapture, 100, true);
        
        Print("[KOTH_GameMode] Round started - Score limit: " + m_ScoreLimit);
        NotifyAllPlayers("[KOTH] Round started! First team to " + m_ScoreLimit + " points wins!");
        
        SI_OnRoundStart.Invoke(m_ScoreLimit);
    }
    
    void EndRound(string winningTeam)
    {
        if (!GetGame().IsServer() || m_RoundEnded)
            return;
        
        m_RoundActive = false;
        m_RoundEnded = true;
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateCapture);
        
        Print("[KOTH_GameMode] Round ended - Winner: " + winningTeam);
        
        BroadcastRoundEnd(winningTeam);
        SI_OnRoundEnd.Invoke(winningTeam, m_EastScore, m_WestScore);
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartRound, 30000, false);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // CAPTURE TIMER (USES CACHED COUNTS)
    // ═══════════════════════════════════════════════════════════════
    
    void UpdateCapture()
    {
        if (!GetGame().IsServer() || !m_RoundActive || !m_HUDSync)
            return;
        
        // PHASE 4: Use cached values (set by trigger events)
        int eastAO = m_CurrentEastAO;
        int westAO = m_CurrentWestAO;
        int eastPriority = m_CurrentEastPriority;
        int westPriority = m_CurrentWestPriority;
        
        int eastOuter = eastAO - eastPriority;
        int westOuter = westAO - westPriority;
        
        if (eastOuter < 0) eastOuter = 0;
        if (westOuter < 0) westOuter = 0;
        
        float eastPlayerPoints = (eastOuter * m_PointsPerTickPerPlayer) + (eastPriority * m_PointsPerTickPerPlayer * m_PriorityBonusMultiplier);
        float westPlayerPoints = (westOuter * m_PointsPerTickPerPlayer) + (westPriority * m_PointsPerTickPerPlayer * m_PriorityBonusMultiplier);
        
        float playerPointDiff = eastPlayerPoints - westPlayerPoints;
        string controllingTeam = "None";
        
        if (playerPointDiff >= m_MinPlayersToInfluence)
        {
            controllingTeam = "East";
        }
        else if (playerPointDiff <= -m_MinPlayersToInfluence)
        {
            controllingTeam = "West";
        }
        
        if (controllingTeam == "None")
        {
            m_CaptureProgress = 0.0;
            m_CapturingTeam = "None";
            
            if (m_HUDSync)
            {
                m_HUDSync.SetCaptureProgress(0.0, "None");
            }
            
            SI_OnCaptureProgressChanged.Invoke(0.0, "None");
            
            return;
        }
        
        if (m_CapturingTeam != controllingTeam)
        {
            m_CaptureProgress = 0.0;
            m_CapturingTeam = controllingTeam;
        }
        
        m_CaptureProgress = m_CaptureProgress + 0.1;
        
        float progressPercent = (m_CaptureProgress / m_TickInterval) * 100.0;
        if (progressPercent > 100.0)
            progressPercent = 100.0;
        
        if (m_HUDSync)
        {
            m_HUDSync.SetCaptureProgress(progressPercent, m_CapturingTeam);
        }
        
        SI_OnCaptureProgressChanged.Invoke(progressPercent, m_CapturingTeam);
        
        if (m_CaptureProgress >= m_TickInterval)
        {
            AwardPoint(controllingTeam);
            m_CaptureProgress = 0.0;
        }
    }
    
    void AwardPoint(string team)
    {
        if (team == "East")
        {
            m_EastScore += m_PointsPerTick;
            if (m_HUDSync)
                m_HUDSync.SetEastScore(m_EastScore);
            
            SI_OnScoreChanged.Invoke(m_EastScore, m_WestScore);
            
            if (m_EastScore >= m_ScoreLimit)
            {
                EndRound("East");
                return;
            }
        }
        else if (team == "West")
        {
            m_WestScore += m_PointsPerTick;
            if (m_HUDSync)
                m_HUDSync.SetWestScore(m_WestScore);
            
            SI_OnScoreChanged.Invoke(m_EastScore, m_WestScore);
            
            if (m_WestScore >= m_ScoreLimit)
            {
                EndRound("West");
                return;
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // NOTIFICATIONS
    // ═══════════════════════════════════════════════════════════════
    
    void BroadcastRoundEnd(string winningTeam)
    {
        if (!GetGame().IsServer())
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_RoundEnd");
        rpc.Write(winningTeam);
        rpc.Write(m_EastScore);
        rpc.Write(m_WestScore);
        rpc.Expansion_Send(true, null);
        
        string message = "[KOTH] Team " + winningTeam + " wins the round! Final score - East: " + m_EastScore + " | West: " + m_WestScore;
        NotifyAllPlayers(message);
    }
    
    void RPC_RoundEnd(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        string winningTeam;
        int eastScore;
        int westScore;
        
        if (!ctx.Read(winningTeam))
            return;
        if (!ctx.Read(eastScore))
            return;
        if (!ctx.Read(westScore))
            return;
        
        string message = "═══════════════════════════════\n";
        message += "     TEAM " + winningTeam.ToUpper() + " WINS!\n";
        message += "═══════════════════════════════\n";
        message += "Final Score:\n";
        message += "East: " + eastScore + " | West: " + westScore + "\n";
        message += "\nNew round starting in 30 seconds...";
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            player.MessageStatus(message);
        }
        
        SI_OnRoundEnd.Invoke(winningTeam, eastScore, westScore);
    }
    
    void NotifyAllPlayers(string message)
    {
        if (!GetGame().IsServer())
            return;
        
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player)
            {
                player.MessageStatus(message);
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // GETTERS
    // ═══════════════════════════════════════════════════════════════
    
    int GetEastScore() { return m_EastScore; }
    int GetWestScore() { return m_WestScore; }
    bool IsRoundActive() { return m_RoundActive; }
    int GetScoreLimit() { return m_ScoreLimit; }
    float GetCaptureProgress() { return m_CaptureProgress; }
    string GetCapturingTeam() { return m_CapturingTeam; }
    
    // ═══════════════════════════════════════════════════════════════
    // ADMIN COMMANDS
    // ═══════════════════════════════════════════════════════════════
    
    void SetScoreLimit(int limit)
    {
        m_ScoreLimit = limit;
    }
    
    void SetTickInterval(float seconds)
    {
        m_TickInterval = seconds;
    }
    
    void AddScoreToTeam(string team, int points)
    {
        if (!GetGame().IsServer())
            return;
        
        if (team == "East")
        {
            m_EastScore += points;
            if (m_HUDSync)
                m_HUDSync.SetEastScore(m_EastScore);
            
            SI_OnScoreChanged.Invoke(m_EastScore, m_WestScore);
        }
        else if (team == "West")
        {
            m_WestScore += points;
            if (m_HUDSync)
                m_HUDSync.SetWestScore(m_WestScore);
            
            SI_OnScoreChanged.Invoke(m_EastScore, m_WestScore);
        }
    }
    
    void ForceEndRound(string winningTeam)
    {
        if (!GetGame().IsServer())
            return;
        
        EndRound(winningTeam);
    }
    
    void ForceRestartRound()
    {
        if (!GetGame().IsServer())
            return;
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateCapture);
        StartRound();
    }
}