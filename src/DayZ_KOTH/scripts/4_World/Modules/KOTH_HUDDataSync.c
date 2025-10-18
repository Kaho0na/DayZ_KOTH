/**
 * KOTH_HUDDataSync.c (PHASE 1 - SCRIPTINVOKERS ADDED)
 *
 * Syncs HUD data with event broadcasting
 * Place in: 4_World/Modules/KOTH_HUDDataSync.c
 */

[CF_RegisterModule(KOTH_HUDDataSync)]
class KOTH_HUDDataSync: CF_ModuleWorld
{
    private static ref KOTH_HUDDataSync s_Instance;
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 1: EVENT INVOKERS
    // ═══════════════════════════════════════════════════════════════
    
    static ref ScriptInvoker SI_OnZonePlayersChanged = new ScriptInvoker();
    static ref ScriptInvoker SI_OnScoreUpdate = new ScriptInvoker();
    static ref ScriptInvoker SI_OnCaptureUpdate = new ScriptInvoker();
    
    // Server-side data
    private int m_EastScore = 0;
    private int m_WestScore = 0;
    private int m_EastPlayersInAO = 0;
    private int m_WestPlayersInAO = 0;
    private int m_EastPlayersInPriority = 0;
    private int m_WestPlayersInPriority = 0;
    private float m_CaptureProgress = 0.0;
    private string m_CapturingTeam = "None";
    
    // Client-side cached data
    private int m_ClientEastScore = 0;
    private int m_ClientWestScore = 0;
    private int m_ClientEastPlayers = 0;
    private int m_ClientWestPlayers = 0;
    private int m_ClientEastPriority = 0;
    private int m_ClientWestPriority = 0;
    private float m_ClientCaptureProgress = 0.0;
    private string m_ClientCapturingTeam = "None";
    
    void KOTH_HUDDataSync()
    {
        s_Instance = this;
        
        if (!SI_OnZonePlayersChanged)
            SI_OnZonePlayersChanged = new ScriptInvoker();
        if (!SI_OnScoreUpdate)
            SI_OnScoreUpdate = new ScriptInvoker();
        if (!SI_OnCaptureUpdate)
            SI_OnCaptureUpdate = new ScriptInvoker();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        EnableMissionStart();
        Expansion_EnableRPCManager();
        
        Expansion_RegisterClientRPC("RPC_SyncHUDData");
        
        Print("[KOTH_HUDDataSync] Initialized with ScriptInvokers");
    }
    
    static KOTH_HUDDataSync GetInstance()
    {
        return s_Instance;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Called by KOTH_AreaTrigger when player counts change
    // ═══════════════════════════════════════════════════════════════
    
    void OnPlayerCountsChanged(int eastCount, int westCount)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastPlayersInAO = eastCount;
        m_WestPlayersInAO = westCount;
        
        Print("[KOTH_HUDDataSync] Player counts changed - East: " + eastCount + ", West: " + westCount);
        
        BroadcastHUDData();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Set player counts (called by game mode)
    // ═══════════════════════════════════════════════════════════════
    
    void SetPlayerCounts(int eastAO, int westAO, int eastPriority, int westPriority)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastPlayersInAO = eastAO;
        m_WestPlayersInAO = westAO;
        m_EastPlayersInPriority = eastPriority;
        m_WestPlayersInPriority = westPriority;
        
        // PHASE 1: Invoke zone players changed event
        SI_OnZonePlayersChanged.Invoke(eastAO, westAO, eastPriority, westPriority);
        
        BroadcastHUDData();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Set capture progress
    // ═══════════════════════════════════════════════════════════════
    
    void SetCaptureProgress(float progress, string team)
    {
        if (!GetGame().IsServer())
            return;
        
        m_CaptureProgress = progress;
        m_CapturingTeam = team;
        
        // PHASE 1: Invoke capture update event
        SI_OnCaptureUpdate.Invoke(progress, team);
        
        BroadcastHUDData();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Broadcast data to all clients
    // ═══════════════════════════════════════════════════════════════
    
    void BroadcastHUDData()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_HUDDataSync] Broadcasting - AO: E" + m_EastPlayersInAO + " W" + m_WestPlayersInAO + " | Priority: E" + m_EastPlayersInPriority + " W" + m_WestPlayersInPriority);
        
        auto rpc = Expansion_CreateRPC("RPC_SyncHUDData");
        rpc.Write(m_EastScore);
        rpc.Write(m_WestScore);
        rpc.Write(m_EastPlayersInAO);
        rpc.Write(m_WestPlayersInAO);
        rpc.Write(m_EastPlayersInPriority);
        rpc.Write(m_WestPlayersInPriority);
        rpc.Write(m_CaptureProgress);
        rpc.Write(m_CapturingTeam);
        rpc.Expansion_Send(true, null);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // CLIENT: Receive data from server
    // ═══════════════════════════════════════════════════════════════
    
    void RPC_SyncHUDData(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        if (!ctx.Read(m_ClientEastScore))
            return;
        if (!ctx.Read(m_ClientWestScore))
            return;
        if (!ctx.Read(m_ClientEastPlayers))
            return;
        if (!ctx.Read(m_ClientWestPlayers))
            return;
        if (!ctx.Read(m_ClientEastPriority))
            return;
        if (!ctx.Read(m_ClientWestPriority))
            return;
        if (!ctx.Read(m_ClientCaptureProgress))
            return;
        if (!ctx.Read(m_ClientCapturingTeam))
            return;
        
        Print("[KOTH_HUDDataSync] CLIENT received - AO: East " + m_ClientEastPlayers + ", West " + m_ClientWestPlayers + " | Priority: East " + m_ClientEastPriority + ", West " + m_ClientWestPriority);
        
        // PHASE 1: Invoke client-side events
        SI_OnZonePlayersChanged.Invoke(m_ClientEastPlayers, m_ClientWestPlayers, m_ClientEastPriority, m_ClientWestPriority);
        SI_OnScoreUpdate.Invoke(m_ClientEastScore, m_ClientWestScore);
        SI_OnCaptureUpdate.Invoke(m_ClientCaptureProgress, m_ClientCapturingTeam);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC GETTERS (for HUD to use)
    // ═══════════════════════════════════════════════════════════════
    
    int GetEastScore()
    {
        if (GetGame().IsServer())
            return m_EastScore;
        return m_ClientEastScore;
    }
    
    int GetWestScore()
    {
        if (GetGame().IsServer())
            return m_WestScore;
        return m_ClientWestScore;
    }
    
    int GetEastPlayersInAO()
    {
        if (GetGame().IsServer())
            return m_EastPlayersInAO;
        return m_ClientEastPlayers;
    }
    
    int GetWestPlayersInAO()
    {
        if (GetGame().IsServer())
            return m_WestPlayersInAO;
        return m_ClientWestPlayers;
    }
    
    int GetEastPlayersInPriority()
    {
        if (GetGame().IsServer())
            return m_EastPlayersInPriority;
        return m_ClientEastPriority;
    }
    
    int GetWestPlayersInPriority()
    {
        if (GetGame().IsServer())
            return m_WestPlayersInPriority;
        return m_ClientWestPriority;
    }
    
    float GetCaptureProgress()
    {
        if (GetGame().IsServer())
            return m_CaptureProgress;
        return m_ClientCaptureProgress;
    }
    
    string GetCapturingTeam()
    {
        if (GetGame().IsServer())
            return m_CapturingTeam;
        return m_ClientCapturingTeam;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Set scores (for game mode integration)
    // ═══════════════════════════════════════════════════════════════
    
    void SetEastScore(int score)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastScore = score;
        
        // PHASE 1: Invoke score update event
        SI_OnScoreUpdate.Invoke(m_EastScore, m_WestScore);
        
        BroadcastHUDData();
    }
    
    void SetWestScore(int score)
    {
        if (!GetGame().IsServer())
            return;
        
        m_WestScore = score;
        
        // PHASE 1: Invoke score update event
        SI_OnScoreUpdate.Invoke(m_EastScore, m_WestScore);
        
        BroadcastHUDData();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 1: ACCESSOR FOR SCRIPTINVOKERS
    // ═══════════════════════════════════════════════════════════════
    
    static ScriptInvoker GetZonePlayersChangedSI()
    {
        return SI_OnZonePlayersChanged;
    }
    
    static ScriptInvoker GetScoreUpdateSI()
    {
        return SI_OnScoreUpdate;
    }
    
    static ScriptInvoker GetCaptureUpdateSI()
    {
        return SI_OnCaptureUpdate;
    }
}