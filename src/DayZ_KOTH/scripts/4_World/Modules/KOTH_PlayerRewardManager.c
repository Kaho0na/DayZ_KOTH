/**
 * KOTH_PlayerRewardManager.c (PHASE 1 - NEW MODULE)
 *
 * King of the Hill by Kahoona
 * Centralized player reward and progression management with event broadcasting
 *
 * Place in: 4_World/Modules/KOTH_PlayerRewardManager.c
 */

[CF_RegisterModule(KOTH_PlayerRewardManager)]
class KOTH_PlayerRewardManager: CF_ModuleWorld
{
    private static ref KOTH_PlayerRewardManager s_Instance;
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 1: EVENT INVOKERS
    // ═══════════════════════════════════════════════════════════════
    
    static ref ScriptInvoker SI_OnPlayerStatsChanged = new ScriptInvoker();
    static ref ScriptInvoker SI_OnPlayerXPGained = new ScriptInvoker();
    static ref ScriptInvoker SI_OnPlayerMoneyGained = new ScriptInvoker();
    static ref ScriptInvoker SI_OnPlayerLevelUp = new ScriptInvoker();
    
    // Player data cache (server-side only)
    private ref map<string, ref KOTH_Players> m_PlayerDataCache;
    
    void KOTH_PlayerRewardManager()
    {
        s_Instance = this;
        m_PlayerDataCache = new map<string, ref KOTH_Players>();
        
        if (!SI_OnPlayerStatsChanged)
            SI_OnPlayerStatsChanged = new ScriptInvoker();
        if (!SI_OnPlayerXPGained)
            SI_OnPlayerXPGained = new ScriptInvoker();
        if (!SI_OnPlayerMoneyGained)
            SI_OnPlayerMoneyGained = new ScriptInvoker();
        if (!SI_OnPlayerLevelUp)
            SI_OnPlayerLevelUp = new ScriptInvoker();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        
        Expansion_RegisterClientRPC("RPC_UpdatePlayerStats");
        
        Print("[KOTH_PlayerRewardManager] Initialized with ScriptInvokers");
    }
    
    static KOTH_PlayerRewardManager GetInstance()
    {
        return s_Instance;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: PLAYER DATA OPERATIONS
    // ═══════════════════════════════════════════════════════════════
    
    KOTH_Players GetPlayerData(string uid)
    {
        if (!GetGame().IsServer())
            return null;
        
        if (m_PlayerDataCache.Contains(uid))
        {
            return m_PlayerDataCache.Get(uid);
        }
        
        KOTH_Players data = KOTH_Players.Load(uid);
        if (data)
        {
            m_PlayerDataCache.Set(uid, data);
        }
        
        return data;
    }
    
    void SavePlayerData(string uid)
    {
        if (!GetGame().IsServer())
            return;
        
        if (!m_PlayerDataCache.Contains(uid))
            return;
        
        KOTH_Players data = m_PlayerDataCache.Get(uid);
        if (data)
        {
            data.Save();
        }
    }
    
    void ClearPlayerCache(string uid)
    {
        if (!GetGame().IsServer())
            return;
        
        m_PlayerDataCache.Remove(uid);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: ADD XP WITH EVENTS
    // ═══════════════════════════════════════════════════════════════
    
    void AddPlayerXP(PlayerBase player, int xpAmount, string reason)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        
        string uid = player.GetIdentity().GetId();
        KOTH_Players data = GetPlayerData(uid);
        
        if (!data)
            return;
        
        int oldXP = data.TotalExperienceEarned;
        int oldLevel = data.CurrentLevel;
        
        data.TotalExperienceEarned += xpAmount;
        
        int newLevel = CalculateLevel(data.TotalExperienceEarned);
        bool leveledUp = false;
        
        if (newLevel > oldLevel)
        {
            data.CurrentLevel = newLevel;
            leveledUp = true;
            Print("[KOTH_PlayerRewardManager] Player " + player.GetIdentity().GetName() + " leveled up! Level " + oldLevel + " → " + newLevel);
        }
        
        SavePlayerData(uid);
        
        Print("[KOTH_PlayerRewardManager] Added " + xpAmount + " XP to " + player.GetIdentity().GetName() + " (" + reason + ") - Total: " + data.TotalExperienceEarned);
        
        // PHASE 1: Broadcast XP gain event
        SI_OnPlayerXPGained.Invoke(uid, xpAmount, reason);
        
        if (leveledUp)
        {
            SI_OnPlayerLevelUp.Invoke(uid, newLevel, oldLevel);
        }
        
        // Send update to client
        SyncPlayerStatsToClient(player.GetIdentity(), data);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: ADD MONEY WITH EVENTS
    // ═══════════════════════════════════════════════════════════════
    
    void AddPlayerMoney(PlayerBase player, int moneyAmount, string reason)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        
        string uid = player.GetIdentity().GetId();
        KOTH_Players data = GetPlayerData(uid);
        
        if (!data)
            return;
        
        data.TotalMoneyinBank += moneyAmount;
        SavePlayerData(uid);
        
        Print("[KOTH_PlayerRewardManager] Added $" + moneyAmount + " to " + player.GetIdentity().GetName() + " (" + reason + ") - Total: $" + data.TotalMoneyinBank);
        
        // PHASE 1: Broadcast money gain event
        SI_OnPlayerMoneyGained.Invoke(uid, moneyAmount, reason);
        
        // Send update to client
        SyncPlayerStatsToClient(player.GetIdentity(), data);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // SERVER: SYNC TO CLIENT
    // ═══════════════════════════════════════════════════════════════
    
    void SyncPlayerStatsToClient(PlayerIdentity ident, KOTH_Players data)
    {
        if (!GetGame().IsServer() || !ident || !data)
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_UpdatePlayerStats");
        rpc.Write(data.TotalExperienceEarned);
        rpc.Write(data.TotalMoneyinBank);
        rpc.Write(data.CurrentLevel);
        rpc.Expansion_Send(true, ident);
        
        Print("[KOTH_PlayerRewardManager] Synced stats to client: XP=" + data.TotalExperienceEarned + ", Money=" + data.TotalMoneyinBank + ", Level=" + data.CurrentLevel);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // CLIENT: RECEIVE STATS UPDATE
    // ═══════════════════════════════════════════════════════════════
    
    void RPC_UpdatePlayerStats(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        int xp;
        int money;
        int level;
        
        if (!ctx.Read(xp))
            return;
        if (!ctx.Read(money))
            return;
        if (!ctx.Read(level))
            return;
        
        Print("[KOTH_PlayerRewardManager] CLIENT received stats update - XP: " + xp + ", Money: " + money + ", Level: " + level);
        
        // PHASE 1: Invoke client-side stats changed event
        SI_OnPlayerStatsChanged.Invoke(xp, money, level);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // HELPER: CALCULATE LEVEL FROM XP
    // ═══════════════════════════════════════════════════════════════
    
    int CalculateLevel(int totalXP)
    {
        float baseMult = 1000.0;
        float exponent = 1.5;
        
        int level = 1;
        while (level < 100)
        {
            float xpNeeded = baseMult * Math.Pow(level + 1, exponent);
            if (totalXP < xpNeeded)
                break;
            level++;
        }
        
        return level;
    }
    
    int CalculateXPForLevel(int level)
    {
        float baseMult = 1000.0;
        float exponent = 1.5;
        
        float xpNeeded = baseMult * Math.Pow(level, exponent);
        return xpNeeded;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 1: ACCESSOR FOR SCRIPTINVOKERS
    // ═══════════════════════════════════════════════════════════════
    
    static ScriptInvoker GetPlayerStatsChangedSI()
    {
        return SI_OnPlayerStatsChanged;
    }
    
    static ScriptInvoker GetPlayerXPGainedSI()
    {
        return SI_OnPlayerXPGained;
    }
    
    static ScriptInvoker GetPlayerMoneyGainedSI()
    {
        return SI_OnPlayerMoneyGained;
    }
    
    static ScriptInvoker GetPlayerLevelUpSI()
    {
        return SI_OnPlayerLevelUp;
    }
}