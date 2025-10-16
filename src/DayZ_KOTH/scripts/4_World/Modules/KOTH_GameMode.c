/**
 * KOTH_GameMode.c
 *
 * King of the Hill by Kahoona
 * Main game mode logic - handles scoring, win conditions, and round management
 *
 * Place in: 4_World/Modules/KOTH_GameMode.c
 */

[CF_RegisterModule(KOTH_GameMode)]
class KOTH_GameMode: CF_ModuleWorld
{
    private static ref KOTH_GameMode s_Instance;
    
    // Game state
    private int m_EastScore = 0;
    private int m_WestScore = 0;
    private bool m_RoundActive = false;
    private bool m_RoundEnded = false;
    
    // Settings
    private int m_ScoreLimit = 100;
    private float m_TickInterval = 2.0; // Award points every 2 seconds
    private int m_PointsPerTick = 1;
    
    // References
    private ref KOTH_HUDDataSync m_HUDSync;
    
    void KOTH_GameMode()
    {
        s_Instance = this;
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        
        Expansion_RegisterClientRPC("RPC_RoundEnd");
        
        Print("[KOTH_GameMode] Initialized");
        
        if (GetGame().IsServer())
        {
            // Initialize after a delay to ensure all modules are loaded
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeGameMode, 3000, false);
        }
    }
    
    void InitializeGameMode()
    {
        if (!GetGame().IsServer())
            return;
        
        // Get HUD sync module reference
        CF_Modules<KOTH_HUDDataSync>.Get(m_HUDSync);
        
        if (!m_HUDSync)
        {
            Error("[KOTH_GameMode] ERROR: Could not get KOTH_HUDDataSync module!");
            return;
        }
        
        // Load settings from KOTH_Settings
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (settings)
        {
            m_ScoreLimit = settings.ScoreLimit;
            Print("[KOTH_GameMode] Score limit set to: " + m_ScoreLimit);
        }
        
        // Start the round
        StartRound();
    }
    
    static KOTH_GameMode GetInstance()
    {
        return s_Instance;
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
        
        // Update HUD with reset scores
        if (m_HUDSync)
        {
            m_HUDSync.SetEastScore(0);
            m_HUDSync.SetWestScore(0);
        }
        
        // Start the scoring loop
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TickScore, m_TickInterval * 1000, true);
        
        Print("[KOTH_GameMode] Round started - Score limit: " + m_ScoreLimit);
        NotifyAllPlayers("[KOTH] Round started! First team to " + m_ScoreLimit + " points wins!");
    }
    
    void EndRound(string winningTeam)
    {
        if (!GetGame().IsServer() || m_RoundEnded)
            return;
        
        m_RoundActive = false;
        m_RoundEnded = true;
        
        // Stop the scoring loop
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(TickScore);
        
        Print("[KOTH_GameMode] Round ended - Winner: " + winningTeam);
        
        // Broadcast victory to all clients
        BroadcastRoundEnd(winningTeam);
        
        // Restart round after 30 seconds
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(StartRound, 30000, false);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SCORING LOGIC
    // ═══════════════════════════════════════════════════════════════
    
    void TickScore()
    {
        if (!GetGame().IsServer() || !m_RoundActive || !m_HUDSync)
            return;
        
        // Get current player counts from HUD sync module
        int eastPlayers = m_HUDSync.GetEastPlayersInAO();
        int westPlayers = m_HUDSync.GetWestPlayersInAO();
        
        // Determine which team should score
        if (eastPlayers > westPlayers && eastPlayers > 0)
        {
            // East team has more players - they score
            m_EastScore += m_PointsPerTick;
            m_HUDSync.SetEastScore(m_EastScore);
            
            Print("[KOTH_GameMode] East scores! (" + eastPlayers + " vs " + westPlayers + ") - Score: " + m_EastScore + "/" + m_ScoreLimit);
            
            // Check for victory
            if (m_EastScore >= m_ScoreLimit)
            {
                EndRound("East");
                return;
            }
        }
        else if (westPlayers > eastPlayers && westPlayers > 0)
        {
            // West team has more players - they score
            m_WestScore += m_PointsPerTick;
            m_HUDSync.SetWestScore(m_WestScore);
            
            Print("[KOTH_GameMode] West scores! (" + eastPlayers + " vs " + westPlayers + ") - Score: " + m_WestScore + "/" + m_ScoreLimit);
            
            // Check for victory
            if (m_WestScore >= m_ScoreLimit)
            {
                EndRound("West");
                return;
            }
        }
        else
        {
            // Tie or no players - no points awarded
            if (eastPlayers == 0 && westPlayers == 0)
            {
                Print("[KOTH_GameMode] No players in zone - no points awarded");
            }
            else
            {
                Print("[KOTH_GameMode] Teams tied (" + eastPlayers + " vs " + westPlayers + ") - no points awarded");
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
        rpc.Expansion_Send(true, null); // Broadcast to all
        
        // Also notify on server
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
        
        // Show victory notification on client
        string message = "═══════════════════════════════\n";
        message += "     TEAM " + winningTeam.ToUpper() + " WINS!\n";
        message += "═══════════════════════════════\n";
        message += "Final Score:\n";
        message += "East: " + eastScore + " | West: " + westScore + "\n";
        message += "\nNew round starting in 30 seconds...";
        
        // Get local player and show message
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
        {
            player.MessageStatus(message);
        }
        
        Print("[KOTH_GameMode] CLIENT: " + winningTeam + " team wins! " + eastScore + "-" + westScore);
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
    
    int GetEastScore()
    {
        return m_EastScore;
    }
    
    int GetWestScore()
    {
        return m_WestScore;
    }
    
    bool IsRoundActive()
    {
        return m_RoundActive;
    }
    
    int GetScoreLimit()
    {
        return m_ScoreLimit;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SETTERS (for admin commands or testing)
    // ═══════════════════════════════════════════════════════════════
    
    void SetScoreLimit(int limit)
    {
        m_ScoreLimit = limit;
        Print("[KOTH_GameMode] Score limit changed to: " + limit);
    }
    
    void SetTickInterval(float seconds)
    {
        m_TickInterval = seconds;
        
        // Restart timer with new interval
        if (GetGame().IsServer() && m_RoundActive)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(TickScore);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TickScore, m_TickInterval * 1000, true);
        }
        
        Print("[KOTH_GameMode] Tick interval changed to: " + seconds + " seconds");
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
        }
        else if (team == "West")
        {
            m_WestScore += points;
            if (m_HUDSync)
                m_HUDSync.SetWestScore(m_WestScore);
        }
    }
    
    // Manual round control (for testing/admin)
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
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(TickScore);
        StartRound();
    }
}