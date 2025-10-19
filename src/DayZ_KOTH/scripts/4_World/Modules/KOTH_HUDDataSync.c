/**
 * KOTH_HUDDataSync.c (PHASE 4 - RPC BROADCAST FIX)
 *
 * Fixed: Added debug logging to trace RPC issues
 * Place in: 4_World/Modules/KOTH_HUDDataSync.c
 */

[CF_RegisterModule(KOTH_HUDDataSync)]
class KOTH_HUDDataSync: CF_ModuleWorld
{
    private static ref KOTH_HUDDataSync s_Instance;
    
    static ref ScriptInvoker SI_OnZonePlayersChanged = new ScriptInvoker();
    static ref ScriptInvoker SI_OnScoreUpdate = new ScriptInvoker();
    static ref ScriptInvoker SI_OnCaptureUpdate = new ScriptInvoker();
    
    private int m_EastScore = 0;
    private int m_WestScore = 0;
    private int m_EastPlayersInAO = 0;
    private int m_WestPlayersInAO = 0;
    private int m_EastPlayersInPriority = 0;
    private int m_WestPlayersInPriority = 0;
    private float m_CaptureProgress = 0.0;
    private string m_CapturingTeam = "None";
    
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
        
        //Print("[KOTH_HUDDataSync] Module initialized");
    }
    
    static KOTH_HUDDataSync GetInstance()
    {
        return s_Instance;
    }
    
    void OnPlayerCountsChanged(int eastCount, int westCount)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastPlayersInAO = eastCount;
        m_WestPlayersInAO = westCount;
        
        
        BroadcastHUDData();
    }
    
    void SetPlayerCounts(int eastAO, int westAO, int eastPriority, int westPriority)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastPlayersInAO = eastAO;
        m_WestPlayersInAO = westAO;
        m_EastPlayersInPriority = eastPriority;
        m_WestPlayersInPriority = westPriority;
        
        SI_OnZonePlayersChanged.Invoke(eastAO, westAO, eastPriority, westPriority);

        BroadcastHUDData();
    }
    
    void SetCaptureProgress(float progress, string team)
    {
        if (!GetGame().IsServer())
            return;
        
        m_CaptureProgress = progress;
        m_CapturingTeam = team;
        
        SI_OnCaptureUpdate.Invoke(progress, team);
        
        BroadcastHUDData();
    }
    
    void BroadcastHUDData()
    {
        if (!GetGame().IsServer())
            return;

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
    
    void RPC_SyncHUDData(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        if (!ctx.Read(m_ClientEastScore))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read EastScore");
            return;
        }
        if (!ctx.Read(m_ClientWestScore))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read WestScore");
            return;
        }
        if (!ctx.Read(m_ClientEastPlayers))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read EastPlayers");
            return;
        }
        if (!ctx.Read(m_ClientWestPlayers))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read WestPlayers");
            return;
        }
        if (!ctx.Read(m_ClientEastPriority))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read EastPriority");
            return;
        }
        if (!ctx.Read(m_ClientWestPriority))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read WestPriority");
            return;
        }
        if (!ctx.Read(m_ClientCaptureProgress))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read CaptureProgress");
            return;
        }
        if (!ctx.Read(m_ClientCapturingTeam))
        {
            Error("[KOTH_HUDDataSync] CLIENT: Failed to read CapturingTeam");
            return;
        }
        
        //Print("[KOTH_HUDDataSync] CLIENT: Received RPC - AO: East " + m_ClientEastPlayers + ", West " + m_ClientWestPlayers + " | Priority: East " + m_ClientEastPriority + ", West " + m_ClientWestPriority);
        
        //Print("[KOTH_HUDDataSync] CLIENT: Invoking events...");
        SI_OnZonePlayersChanged.Invoke(m_ClientEastPlayers, m_ClientWestPlayers, m_ClientEastPriority, m_ClientWestPriority);
        SI_OnScoreUpdate.Invoke(m_ClientEastScore, m_ClientWestScore);
        SI_OnCaptureUpdate.Invoke(m_ClientCaptureProgress, m_ClientCapturingTeam);
        //Print("[KOTH_HUDDataSync] CLIENT: Events invoked");
    }
    
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
    
    void SetEastScore(int score)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastScore = score;
        SI_OnScoreUpdate.Invoke(m_EastScore, m_WestScore);
        BroadcastHUDData();
    }
    
    void SetWestScore(int score)
    {
        if (!GetGame().IsServer())
            return;
        
        m_WestScore = score;
        SI_OnScoreUpdate.Invoke(m_EastScore, m_WestScore);
        BroadcastHUDData();
    }
    
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