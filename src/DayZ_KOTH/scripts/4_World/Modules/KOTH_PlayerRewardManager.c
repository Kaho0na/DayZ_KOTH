/**
 * KOTH_PlayerRewardManager.c (WITH XP TABLE)
 *
 * King of the Hill by Kahoona
 * Centralized player reward and progression management with hardcoded XP table
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
    private int m_KillXP = 100;
    
    static int KOTH_LEVEL_XP_REQUIREMENTS[100] = {
        0, 1000, 2100, 3200, 4400, 5700, 7000, 8400, 9900, 11500,
        13200, 15000, 16900, 18900, 21000, 23300, 25700, 28200, 30900, 33800,
        36800, 40000, 43400, 47000, 50800, 54800, 59100, 63600, 68400, 73500,
        78900, 84600, 90700, 97200, 104000, 111300, 119000, 127100, 135700, 144900,
        154600, 164900, 175800, 187400, 199700, 212700, 226500, 241100, 256600, 273000,
        290400, 308800, 328300, 349000, 370900, 394200, 418900, 445000, 472700, 502100,
        533200, 566200, 601200, 638300, 677600, 719200, 763300, 810100, 859700, 912300,
        968000, 1027100, 1089700, 1156100, 1226500, 1301100, 1380200, 1464000, 1552800, 1647000,
        1746800, 1852600, 1964700, 2083600, 2209600, 2343200, 2484800, 2634900, 2794000, 2962600,
        3141300, 3330800, 3531600, 3744500, 3970200, 4209400, 4462900, 4731700, 5016600, 5318600
    };
    
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
            AddPlayerXP(killer, m_KillXP, "Enemy Kill");
            
            KOTH_Players playerData = GetPlayerData(killerUID);
            if (playerData)
            {
                playerData.TotalEnemiesKilled = playerData.TotalEnemiesKilled + 1;
                SavePlayerData(killerUID);
            }
            
            ExpansionNotification("Kill Reward", "+$" + m_KillReward + " | +" + m_KillXP + " XP").Success(killerIdent);
        }
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
        
        int atmMoney = 0;
        if (m_MarketModule)
        {
            ref ExpansionMarketATM_Data atmData = m_MarketModule.GetPlayerATMData(ident.GetId());
            if (atmData)
            {
                atmMoney = atmData.GetMoney();
            }
        }
        
        auto rpc = Expansion_CreateRPC("RPC_UpdatePlayerStats");
        rpc.Write(data.TotalExperienceEarned);
        rpc.Write(atmMoney);
        rpc.Write(data.CurrentLevel);
        rpc.Expansion_Send(true, ident);
        
        Print("[KOTH_PlayerRewardManager] Synced stats to client: XP=" + data.TotalExperienceEarned + ", ATM Money=" + atmMoney + ", Level=" + data.CurrentLevel);
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
        // Original formula (kept for reference): 500.0 * Math.Pow(level, 1.35)
        // Now using hardcoded table for performance and consistency
        
        int level = 1;
        for (int i = 1; i < 100; i++)
        {
            if (totalXP < KOTH_LEVEL_XP_REQUIREMENTS[i])
            {
                level = i;
                break;
            }
        }
        
        if (totalXP >= KOTH_LEVEL_XP_REQUIREMENTS[99])
            level = 99;
        
        return level;
    }
    
    int CalculateXPForLevel(int level)
    {
        // Original formula (kept for reference): 500.0 * Math.Pow(level, 1.35), rounded to nearest 100
        // Now using hardcoded table for performance and consistency
        
        if (level < 0)
            level = 0;
        if (level > 99)
            level = 99;
        
        return KOTH_LEVEL_XP_REQUIREMENTS[level];
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
    
    void SetKillXP(int amount)
    {
        m_KillXP = amount;
        Print("[KOTH_PlayerRewardManager] Kill XP set to " + amount);
    }
    
    int GetKillReward()
    {
        return m_KillReward;
    }
    
    int GetTeamKillPenalty()
    {
        return m_TeamKillPenalty;
    }
    
    int GetKillXP()
    {
        return m_KillXP;
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