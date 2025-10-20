/**
 * KOTH_PlayerRewardManager.c (INTEGRATED WITH STAT TRACKER)
 *
 * King of the Hill by Kahoona
 * Now integrated with KOTH_RoundStatsTracker for real-time stat tracking
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
    private ref map<string, int> m_PlayerKillstreaks;
    private ref map<string, float> m_PlayerLastCaptureReward;
    private ref ExpansionMarketModule m_MarketModule;
    private ref KOTH_RoundStatsTracker m_StatsTracker;
    
    private int m_KillReward = 100;
    private int m_HeadshotReward = 200;
    private int m_AssistReward = 50;
    private int m_ReviveReward = 50;
    private int m_CaptureReward = 300;
    private int m_TeamKillPenalty = 100;
    private int m_TeamKnockdownPenalty = 50;
    private int m_SuicidePenalty = 100;
    private int m_KillXP = 100;
    private int m_HeadshotXP = 200;
    private int m_AssistXP = 50;
    private int m_ReviveXP = 50;
    private int m_CaptureXP = 250;
    private int m_CaptureInterval = 30;
    private float m_GlobalXPMultiplier = 1.0;
    private float m_GlobalMoneyMultiplier = 1.0;
    
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
        m_PlayerKillstreaks = new map<string, int>();
        m_PlayerLastCaptureReward = new map<string, float>();
        
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
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeStatsTracker, 2500, false);
        }
        
        Print("[KOTH_PlayerRewardManager] Initialized with stat tracking integration");
    }
    
    void InitializeMarketModule()
    {
        if (!Class.CastTo(m_MarketModule, CF_ModuleCoreManager.Get(ExpansionMarketModule)))
        {
            Error("[KOTH_PlayerRewardManager] Failed to get ExpansionMarketModule!");
            return;
        }
        
        LoadSettingsValues();
        
        //Print("[KOTH_PlayerRewardManager] Expansion Market Module connected");
    }
    
    void InitializeStatsTracker()
    {
        if (!CF_Modules<KOTH_RoundStatsTracker>.Get(m_StatsTracker))
        {
            Error("[KOTH_PlayerRewardManager] Failed to get KOTH_RoundStatsTracker!");
            return;
        }
        
        Print("[KOTH_PlayerRewardManager] Stats Tracker connected");
    }
    
    void LoadSettingsValues()
    {
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (!settings)
        {
            Print("[KOTH_PlayerRewardManager] WARNING: Could not load settings, using defaults");
            return;
        }
        
        m_KillXP = settings.XPPerKill;
        m_KillReward = settings.MoneyPerKill;
        m_ReviveXP = settings.XPPerRevive;
        m_ReviveReward = settings.MoneyPerRevive;
        m_AssistXP = settings.XPPerAssist;
        m_AssistReward = settings.MoneyPerAssist;
        m_CaptureXP = settings.XPPerCapture;
        m_CaptureReward = settings.MoneyPerCapture;
        m_CaptureInterval = settings.XPCaptureInterval;
        m_HeadshotXP = settings.HeadShotBonusXP;
        m_HeadshotReward = settings.HeadShotMoneyBonus;
        m_TeamKillPenalty = settings.TeamKillMoneyPenalty;
        m_SuicidePenalty = settings.SuicideMoneyPenalty;
        m_GlobalXPMultiplier = settings.GlobalXPMultiplier;
        m_GlobalMoneyMultiplier = settings.GlobalMoneyMultiplier;
        
        Print("[KOTH_PlayerRewardManager] Loaded settings");
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
        
        int finalXP = xpAmount * m_GlobalXPMultiplier;
        
        int oldXP = data.TotalExperienceEarned;
        int oldLevel = data.CurrentLevel;
        
        data.TotalExperienceEarned += finalXP;
        
        int newLevel = CalculateLevel(data.TotalExperienceEarned);
        bool leveledUp = false;
        
        if (newLevel > oldLevel)
        {
            data.CurrentLevel = newLevel;
            leveledUp = true;
            //Print("[KOTH_PlayerRewardManager] Player " + player.GetIdentity().GetName() + " leveled up! Level " + oldLevel + " → " + newLevel);
        }
        
        SavePlayerData(uid);
        
        if (m_StatsTracker)
        {
            m_StatsTracker.RecordXPGained(uid, player.GetIdentity().GetName(), finalXP);
        }
        
        SI_OnPlayerXPGained.Invoke(uid, finalXP, reason);
        
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
        
        int finalMoney = moneyAmount * m_GlobalMoneyMultiplier;
        
        atmData.AddMoney(finalMoney);
        atmData.Save();
        data.TotalMoneyinBank += finalMoney;
        SavePlayerData(uid);
        
        if (m_StatsTracker)
        {
            m_StatsTracker.RecordMoneyGained(uid, ident.GetName(), finalMoney);
        }
        
        SI_OnPlayerMoneyGained.Invoke(uid, finalMoney, reason);
        
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
        
        int currentBalance = atmData.GetMoney();
        
        if (currentBalance < moneyAmount)
        {
            //Print("[KOTH_PlayerRewardManager] Insufficient funds for penalty - Current: $" + currentBalance + ", Penalty: $" + moneyAmount + " - No deduction");
            ExpansionNotification("Team Kill Penalty", "Insufficient funds for penalty (Balance: $" + currentBalance + ")").Error(ident);
            return;
        }
        
        atmData.RemoveMoney(moneyAmount);
        atmData.Save();
        
        KOTH_Players data = GetPlayerData(uid);
        if (data)
        {
            SyncPlayerStatsToClient(ident, data);
        }
        
        //Print("[KOTH_PlayerRewardManager] Removed $" + moneyAmount + " from " + ident.GetName() + " (" + reason + ") - ATM: $" + atmData.GetMoney());
    }
    
    void ProcessCaptureReward(PlayerBase player, bool isCapturing)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;
        
        if (!isCapturing)
            return;
        
        PlayerIdentity ident = player.GetIdentity();
        string uid = ident.GetId();
        
        float currentTime = GetGame().GetTime() / 1000;
        
        if (m_PlayerLastCaptureReward.Contains(uid))
        {
            float lastRewardTime = m_PlayerLastCaptureReward.Get(uid);
            float timeSinceLastReward = currentTime - lastRewardTime;
            
            if (timeSinceLastReward < m_CaptureInterval)
            {
                return;
            }
        }
        
        m_PlayerLastCaptureReward.Set(uid, currentTime);
        
        AddPlayerMoney(player, m_CaptureReward, "Zone Capture");
        AddPlayerXP(player, m_CaptureXP, "Zone Capture");
        
        KOTH_NotificationModule.ShowNotificationAdvanced("Objective Offensive", "$" + m_CaptureReward.ToString(), ARGB(255, 255, 215, 0), m_CaptureXP.ToString() + "XP", ARGB(255, 144, 238, 144), ARGB(255, 0, 255, 0), 3.0, ident);
    }
    
    void ProcessSuicide(PlayerBase player)
    {
        if (!GetGame().IsServer() || !player)
            return;
        
        PlayerIdentity ident = player.GetIdentity();
        if (!ident)
        {
            //Print("[KOTH_PlayerRewardManager] Player has no identity - skipping suicide penalty");
            return;
        }
        
        string uid = ident.GetId();
        
        RemovePlayerMoney(player, m_SuicidePenalty, "Suicide Penalty");
        
        KOTH_NotificationModule.ShowNotificationAdvanced("Suicide Penalty", "-$" + m_SuicidePenalty.ToString(), ARGB(255, 255, 0, 0), "", ARGB(255, 255, 255, 255), ARGB(255, 128, 128, 128), 3.0, ident);
        
        ResetKillstreak(uid);
    }
    
    void ProcessRevive(PlayerBase medic, PlayerBase patient)
    {
        if (!GetGame().IsServer() || !medic || !patient)
            return;
        
        PlayerIdentity medicIdent = medic.GetIdentity();
        if (!medicIdent)
        {
            //Print("[KOTH_PlayerRewardManager] Medic has no identity - skipping revive reward");
            return;
        }
        
        string medicUID = medicIdent.GetId();
        string patientUID;
        
        if (patient.GetIdentity())
        {
            patientUID = patient.GetIdentity().GetId();
        }
        else
        {
            patientUID = patient.GetType();
        }
        
        string lastRevivedUID = medic.GetLastRevivedPlayerUID();
        
        if (lastRevivedUID == patientUID)
        {
            ExpansionNotification("Revive Farming Detected", "Cannot revive the same player twice in a row").Error(medicIdent);
            return;
        }
        
        medic.SetLastRevivedPlayerUID(patientUID);
        
        AddPlayerMoney(medic, m_ReviveReward, "Teammate Revive");
        AddPlayerXP(medic, m_ReviveXP, "Teammate Revive");
        
        if (m_StatsTracker)
        {
            string medicTeam = medic.GetKOTHTeam();
            m_StatsTracker.RecordRevive(medicUID, medicIdent.GetName(), medicTeam);
        }
        
        string patientName = "teammate";
        if (patient.GetIdentity())
        {
            patientName = patient.GetIdentity().GetName();
        }
        else
        {
            patientName = patient.GetType();
        }
        
        KOTH_NotificationModule.ShowNotificationAdvanced("Team Player Revived", "$" + m_ReviveReward.ToString(), ARGB(255, 0, 255, 0), m_ReviveXP.ToString() + "XP", ARGB(255, 144, 238, 144), ARGB(255, 0, 128, 255), 3.0, medicIdent);
    }
    
    void ProcessTeamKnockdown(PlayerBase attacker, PlayerBase victim)
    {
        if (!GetGame().IsServer() || !attacker || !victim)
            return;
        
        PlayerIdentity attackerIdent = attacker.GetIdentity();
        if (!attackerIdent)
        {
            //Print("[KOTH_PlayerRewardManager] Attacker has no identity - skipping team knockdown penalty");
            return;
        }
        
        RemovePlayerMoney(attacker, m_TeamKnockdownPenalty, "Team Knockdown Penalty");
        
        string victimName = "teammate";
        if (victim.GetIdentity())
        {
            victimName = victim.GetIdentity().GetName();
        }
        else
        {
            victimName = victim.GetType();
        }
        
        KOTH_NotificationModule.ShowNotificationAdvanced("Team Attack Penalty", "-$" + m_TeamKnockdownPenalty.ToString(), ARGB(255, 255, 69, 0), "", ARGB(255, 255, 255, 255), ARGB(255, 255, 0, 0), 4.0, attackerIdent);
    }
    
    void ProcessAssist(PlayerBase attacker, PlayerBase victim)
    {
        if (!GetGame().IsServer() || !attacker || !victim)
            return;
        
        PlayerIdentity attackerIdent = attacker.GetIdentity();
        if (!attackerIdent)
        {
            //Print("[KOTH_PlayerRewardManager] Attacker has no identity - skipping assist reward");
            return;
        }
        
        string attackerUID = attackerIdent.GetId();
        
        AddPlayerMoney(attacker, m_AssistReward, "Enemy Assist");
        AddPlayerXP(attacker, m_AssistXP, "Enemy Assist");
        
        string victimName = "enemy";
        if (victim.GetIdentity())
        {
            victimName = victim.GetIdentity().GetName();
        }
        else
        {
            victimName = victim.GetType();
        }
        
        KOTH_NotificationModule.ShowNotificationAdvanced("Team Assist", "$" + m_AssistReward.ToString(), ARGB(255, 144, 238, 144), m_AssistXP.ToString() + "XP", ARGB(255, 173, 216, 230), ARGB(255, 0, 191, 255), 3.0, attackerIdent);
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
            //Print("[KOTH_PlayerRewardManager] Killer has no identity - skipping reward");
            return;
        }
        
        string killerTeam = killer.GetKOTHTeam();
        string victimTeam;
        string victimUID;
        string victimName;
        
        if (victim.GetIdentity())
        {
            victimTeam = victim.GetKOTHTeam();
            victimUID = victim.GetIdentity().GetId();
            victimName = victim.GetIdentity().GetName();
        }
        else
        {
            victimTeam = GetExpansionAIFaction(victim);
            victimUID = victim.GetType();
            victimName = victim.GetType();
        }
        
        if (killerTeam == "" || victimTeam == "" || victimTeam == "Unknown")
        {
            Print("[KOTH_PlayerRewardManager] Skipping reward - invalid team/faction");
            return;
        }
        
        if (killerTeam == victimTeam)
        {
            RemovePlayerMoney(killer, m_TeamKillPenalty, "Team Kill Penalty");
            KOTH_NotificationModule.ShowNotificationAdvanced("Teamkill Penalty", "-$" + m_TeamKillPenalty.ToString(), ARGB(255, 255, 0, 0), "", ARGB(255, 255, 255, 255), ARGB(255, 139, 0, 0), 4.0, killerIdent);
            ResetKillstreak(killerUID);
            
            if (m_StatsTracker)
            {
                m_StatsTracker.RecordTeamkill(killerUID, killerIdent.GetName(), killerTeam);
            }
        }
        else
        {
            float distance = vector.Distance(killer.GetPosition(), victim.GetPosition());
            int distanceInt = distance;
            
            bool wasHeadshot = CheckIfHeadshot(victim);
            
            int moneyReward = m_KillReward;
            int xpReward = m_KillXP;
            string rewardType = "Enemy Kill";
            
            KOTH_Players data = GetPlayerData(killerUID);
            if (!data)
                return;
            
            data.TotalEnemiesKilled = data.TotalEnemiesKilled + 1;
            
            if (distanceInt > data.LongestKill)
            {
                data.LongestKill = distanceInt;
            }
            
            if (wasHeadshot)
            {
                moneyReward = m_KillReward + m_HeadshotReward;
                xpReward = m_KillXP + m_HeadshotXP;
                rewardType = "Headshot Kill";
                
                if (distanceInt > data.LongestHeadshot)
                {
                    data.LongestHeadshot = distanceInt;
                }
            }
            
            IncrementKillstreak(killerUID, data);
            
            SavePlayerData(killerUID);
            
            if (m_StatsTracker)
            {
                m_StatsTracker.RecordKill(killerUID, killerIdent.GetName(), killerTeam, wasHeadshot, distanceInt);
                m_StatsTracker.RecordDeath(victimUID, victimName, victimTeam);
            }
            
            AddPlayerMoney(killer, moneyReward, rewardType);
            AddPlayerXP(killer, xpReward, rewardType);
            
            if (wasHeadshot)
            {
                KOTH_NotificationModule.ShowNotificationAdvanced("Enemy Killed (" + distanceInt.ToString() + "m)", "$" + moneyReward.ToString(), ARGB(255, 0, 255, 0), xpReward.ToString() + "XP", ARGB(255, 144, 238, 144), ARGB(255, 255, 255, 0), 3.0, killerIdent);
                KOTH_NotificationModule.ShowNotificationAdvanced("BONUS HEADSHOT! (" + distanceInt.ToString() + "m)", "$" + moneyReward.ToString(), ARGB(255, 255, 215, 0), xpReward.ToString() + "XP", ARGB(255, 255, 165, 0), ARGB(255, 255, 140, 0), 3.0, killerIdent);
            }
            else
            {
                KOTH_NotificationModule.ShowNotificationAdvanced("Enemy Killed (" + distanceInt.ToString() + "m)", "$" + moneyReward.ToString(), ARGB(255, 0, 255, 0), xpReward.ToString() + "XP", ARGB(255, 144, 238, 144), ARGB(255, 255, 255, 0), 3.0, killerIdent);
            }
        }
    }
    
    void IncrementKillstreak(string uid, KOTH_Players data)
    {
        int currentStreak = 0;
        
        if (m_PlayerKillstreaks.Contains(uid))
        {
            currentStreak = m_PlayerKillstreaks.Get(uid);
        }
        
        currentStreak++;
        m_PlayerKillstreaks.Set(uid, currentStreak);
        
        if (currentStreak > data.HighestKillstreak)
        {
            data.HighestKillstreak = currentStreak;
        }
    }
    
    void ResetKillstreak(string uid)
    {
        if (m_PlayerKillstreaks.Contains(uid))
        {
            int streak = m_PlayerKillstreaks.Get(uid);
            m_PlayerKillstreaks.Set(uid, 0);
        }
    }
    
    int GetCurrentKillstreak(string uid)
    {
        if (m_PlayerKillstreaks.Contains(uid))
        {
            return m_PlayerKillstreaks.Get(uid);
        }
        return 0;
    }
    
    bool CheckIfHeadshot(PlayerBase victim)
    {
        if (!victim)
            return false;
        
        return victim.WasHeadshotKill();
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
        
        SI_OnPlayerStatsChanged.Invoke(xp, money, level);
    }
    
    int CalculateLevel(int totalXP)
    {
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
        if (level < 0)
            level = 0;
        if (level > 99)
            level = 99;
        
        return KOTH_LEVEL_XP_REQUIREMENTS[level];
    }
    
    void SetTeamKnockdownPenalty(int amount)
    {
        m_TeamKnockdownPenalty = amount;
    }
    
    void SetReviveReward(int amount)
    {
        m_ReviveReward = amount;
    }
    
    void SetReviveXP(int amount)
    {
        m_ReviveXP = amount;
    }
    
    void SetAssistReward(int amount)
    {
        m_AssistReward = amount;
    }
    
    void SetAssistXP(int amount)
    {
        m_AssistXP = amount;
    }
    
    void SetKillReward(int amount)
    {
        m_KillReward = amount;
    }
    
    void SetHeadshotReward(int amount)
    {
        m_HeadshotReward = amount;
    }
    
    void SetTeamKillPenalty(int amount)
    {
        m_TeamKillPenalty = amount;
    }
    
    void SetKillXP(int amount)
    {
        m_KillXP = amount;
    }
    
    void SetHeadshotXP(int amount)
    {
        m_HeadshotXP = amount;
    }
    
    int GetTeamKnockdownPenalty()
    {
        return m_TeamKnockdownPenalty;
    }
    
    int GetReviveReward()
    {
        return m_ReviveReward;
    }
    
    int GetReviveXP()
    {
        return m_ReviveXP;
    }
    
    int GetAssistReward()
    {
        return m_AssistReward;
    }
    
    int GetAssistXP()
    {
        return m_AssistXP;
    }
    
    int GetKillReward()
    {
        return m_KillReward;
    }
    
    int GetHeadshotReward()
    {
        return m_HeadshotReward;
    }
    
    int GetTeamKillPenalty()
    {
        return m_TeamKillPenalty;
    }
    
    int GetKillXP()
    {
        return m_KillXP;
    }
    
    int GetHeadshotXP()
    {
        return m_HeadshotXP;
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