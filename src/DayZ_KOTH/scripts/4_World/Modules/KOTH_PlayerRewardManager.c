/**
 * KOTH_PlayerRewardManager.c (SIMPLIFIED COMBAT REWARDS)
 *
 * King of the Hill by Kahoona
 * Centralized player reward and progression management with unified combat system
 *
 * Place in: 4_World/Modules/KOTH_PlayerRewardManager.c
 */

[CF_RegisterModule(KOTH_PlayerRewardManager)]
class KOTH_PlayerRewardManager: CF_ModuleWorld
{
    private static ref KOTH_PlayerRewardManager s_Instance;
    
    static ref ScriptInvoker SI_OnPlayerStatsChanged = new ScriptInvoker();
    static ref ScriptInvoker SI_OnPlayerXPGained = new ScriptInvoker();
    static ref ScriptInvoker SI_OnPlayerMoneyGained = new ScriptInvoker();
    static ref ScriptInvoker SI_OnPlayerLevelUp = new ScriptInvoker();
    
    private ref map<string, ref KOTH_Players> m_PlayerDataCache;
    private ref ExpansionMarketModule m_MarketModule;
    
    private int m_KillReward = 100;
    private int m_TeamKillPenalty = 100;
    
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
        
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeMarketModule, 2000, false);
        }
        
        Print("[KOTH_PlayerRewardManager] Initialized with ScriptInvokers and combat rewards");
    }
    
    void InitializeMarketModule()
    {
        if (!Class.CastTo(m_MarketModule, CF_ModuleCoreManager.Get(ExpansionMarketModule)))
        {
            Error("[KOTH_PlayerRewardManager] Failed to get ExpansionMarketModule!");
            return;
        }
        
        Print("[KOTH_PlayerRewardManager] Expansion Market Module connected");
    }
    
    static KOTH_PlayerRewardManager GetInstance()
    {
        return s_Instance;
    }
    
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
        
        SI_OnPlayerXPGained.Invoke(uid, xpAmount, reason);
        
        if (leveledUp)
        {
            SI_OnPlayerLevelUp.Invoke(uid, newLevel, oldLevel);
        }
        
        SyncPlayerStatsToClient(player.GetIdentity(), data);
    }
    
    void AddPlayerMoney(PlayerBase player, int moneyAmount, string reason)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        
        string uid = player.GetIdentity().GetId();
        PlayerIdentity ident = player.GetIdentity();
        
        if (!m_MarketModule)
        {
            Error("[KOTH_PlayerRewardManager] Market module not initialized!");
            return;
        }
        
        ref ExpansionMarketATM_Data atmData = m_MarketModule.GetPlayerATMData(uid);
        if (!atmData)
        {
            Error("[KOTH_PlayerRewardManager] Failed to get ATM data for player " + ident.GetName());
            return;
        }
        
        KOTH_Players data = GetPlayerData(uid);
        if (!data)
            return;
        
        atmData.AddMoney(moneyAmount);
        atmData.Save();
        data.TotalMoneyinBank += moneyAmount;
        SavePlayerData(uid);
        
        Print("[KOTH_PlayerRewardManager] Added $" + moneyAmount + " to " + ident.GetName() + " (" + reason + ") - ATM: $" + atmData.GetMoney() + " | Total: $" + data.TotalMoneyinBank);
        
        SI_OnPlayerMoneyGained.Invoke(uid, moneyAmount, reason);
        
        SyncPlayerStatsToClient(ident, data);
    }
    
    void RemovePlayerMoney(PlayerBase player, int moneyAmount, string reason)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        
        string uid = player.GetIdentity().GetId();
        PlayerIdentity ident = player.GetIdentity();
        
        if (!m_MarketModule)
        {
            Error("[KOTH_PlayerRewardManager] Market module not initialized!");
            return;
        }
        
        ref ExpansionMarketATM_Data atmData = m_MarketModule.GetPlayerATMData(uid);
        if (!atmData)
        {
            Error("[KOTH_PlayerRewardManager] Failed to get ATM data for player " + ident.GetName());
            return;
        }
        
        atmData.RemoveMoney(moneyAmount);
        atmData.Save();
        Print("[KOTH_PlayerRewardManager] Removed $" + moneyAmount + " from " + ident.GetName() + " (" + reason + ") - ATM: $" + atmData.GetMoney());
    }
    
    void ProcessKill(PlayerBase killer, PlayerBase victim)
    {
        if (!GetGame().IsServer() || !killer || !victim)
            return;
        
        PlayerIdentity killerIdent = killer.GetIdentity();
        string killerUID;
        
        if (killerIdent)
        {
            killerUID = killerIdent.GetId();
        }
        else
        {
            Print("[KOTH_PlayerRewardManager] Killer has no identity - skipping reward");
            return;
        }
        
        string killerTeam = killer.GetKOTHTeam();
        string victimTeam;
        
        if (victim.GetIdentity())
        {
            victimTeam = victim.GetKOTHTeam();
        }
        else
        {
            victimTeam = GetExpansionAIFaction(victim);
        }
        
        if (killerTeam == "" || victimTeam == "" || victimTeam == "Unknown")
        {
            Print("[KOTH_PlayerRewardManager] Skipping reward - invalid team/faction");
            return;
        }
        
        if (killerTeam == victimTeam)
        {
            RemovePlayerMoney(killer, m_TeamKillPenalty, "Team Kill Penalty");
            ExpansionNotification("Team Kill Penalty", "-$" + m_TeamKillPenalty + " removed from your account").Error(killerIdent);
        }
        else
        {
            AddPlayerMoney(killer, m_KillReward, "Enemy Kill");
            
            KOTH_Players playerData = GetPlayerData(killerUID);
            if (playerData)
            {
                playerData.TotalEnemiesKilled = playerData.TotalEnemiesKilled + 1;
                SavePlayerData(killerUID);
            }
            
            ExpansionNotification("Kill Reward", "+$" + m_KillReward + " deposited to your account").Success(killerIdent);
        }
    }
    
    void ProcessAIKill(PlayerBase killer, PlayerBase aiVictim)
    {
        ProcessKill(killer, aiVictim);
    }
    
    string GetExpansionAIFaction(PlayerBase ai)
    {
        if (!ai)
            return "Unknown";
        
        eAIBase eaiEntity = eAIBase.Cast(ai);
        if (eaiEntity)
        {
            eAIGroup group = eaiEntity.GetGroup();
            if (group)
            {
                eAIFaction faction = group.GetFaction();
                if (faction)
                {
                    return faction.GetName();
                }
            }
        }
        
        return "Unknown";
    }
    
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
        
        SI_OnPlayerStatsChanged.Invoke(xp, money, level);
    }
    
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
    
    void SetKillReward(int amount)
    {
        m_KillReward = amount;
        Print("[KOTH_PlayerRewardManager] Kill reward set to $" + amount);
    }
    
    void SetTeamKillPenalty(int amount)
    {
        m_TeamKillPenalty = amount;
        Print("[KOTH_PlayerRewardManager] Team kill penalty set to $" + amount);
    }
    
    int GetKillReward()
    {
        return m_KillReward;
    }
    
    int GetTeamKillPenalty()
    {
        return m_TeamKillPenalty;
    }
    
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