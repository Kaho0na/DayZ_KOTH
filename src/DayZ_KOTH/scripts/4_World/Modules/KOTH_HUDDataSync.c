/**
 * KOTH_HUDDataSync.c
 *
 * Syncs HUD data from server to all clients
 * Place in: 4_World/Modules/KOTH_HUDDataSync.c
 */

[CF_RegisterModule(KOTH_HUDDataSync)]
class KOTH_HUDDataSync: CF_ModuleWorld
{
    private static ref KOTH_HUDDataSync s_Instance;
    
    // Server-side data
    private int m_EastScore = 0;
    private int m_WestScore = 0;
    private int m_EastPlayersInAO = 0;
    private int m_WestPlayersInAO = 0;
    
    // Client-side cached data
    private int m_ClientEastScore = 0;
    private int m_ClientWestScore = 0;
    private int m_ClientEastPlayers = 0;
    private int m_ClientWestPlayers = 0;
    
    void KOTH_HUDDataSync()
    {
        s_Instance = this;
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        EnableMissionStart();
        Expansion_EnableRPCManager();
        
        Expansion_RegisterClientRPC("RPC_SyncHUDData");
        
        Print("[KOTH_HUDDataSync] Initialized");
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
        
        // Broadcast to all clients
        BroadcastHUDData();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Broadcast data to all clients
    // ═══════════════════════════════════════════════════════════════
    
    void BroadcastHUDData()
    {
        if (!GetGame().IsServer())
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_SyncHUDData");
        rpc.Write(m_EastScore);
        rpc.Write(m_WestScore);
        rpc.Write(m_EastPlayersInAO);
        rpc.Write(m_WestPlayersInAO);
        rpc.Expansion_Send(true, null); // Broadcast to all
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
        
        Print("[KOTH_HUDDataSync] CLIENT received - East: " + m_ClientEastPlayers + ", West: " + m_ClientWestPlayers);
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
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: Set scores (for future game mode integration)
    // ═══════════════════════════════════════════════════════════════
    
    void SetEastScore(int score)
    {
        if (!GetGame().IsServer())
            return;
        
        m_EastScore = score;
        BroadcastHUDData();
    }
    
    void SetWestScore(int score)
    {
        if (!GetGame().IsServer())
            return;
        
        m_WestScore = score;
        BroadcastHUDData();
    }
}