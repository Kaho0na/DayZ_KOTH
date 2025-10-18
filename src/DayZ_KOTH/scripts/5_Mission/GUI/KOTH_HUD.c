/**
 * KOTH_HUD.c (PHASE 2 - EVENT-DRIVEN WITH POLLING FALLBACK)
 *
 * King of the Hill by Kahoona
 * HUD now subscribes to events but keeps polling as safety net
 *
 * Place in: 5_Mission/GUI/KOTH_HUD.c
 */

class KOTH_HUD: ExpansionScriptView
{
    private ref KOTH_HUDController m_HUDController;
    private IngameHud m_Hud;
    
    // Top Panel Widgets (Team Scores)
    protected TextWidget westScore;
    protected TextWidget westAOPlayers;
    protected TextWidget westPriorityPlayers;
    protected ProgressBarWidget TeamBar;
    protected TextWidget eastScore;
    protected TextWidget eastAOPlayers;
    protected TextWidget eastPriorityPlayers;
    
    // Level Panel Widgets (Player Stats)
    protected TextWidget currentLevel;
    protected ProgressBarWidget XPBar;
    protected TextWidget currentXP;
    protected TextWidget currentMoney;
    
    // Cache for player data
    private int m_CachedWestScore = 0;
    private int m_CachedEastScore = 0;
    private int m_CachedWestAO = -1;
    private int m_CachedEastAO = -1;
    private int m_CachedWestPriority = -1;
    private int m_CachedEastPriority = -1;
    private float m_CachedCaptureProgress = -1.0;
    private string m_CachedCapturingTeam = "";
    private int m_CachedLevel = -1;
    private int m_CachedXP = -1;
    private int m_CachedMaxXP = -1;
    private int m_CachedMoney = -1;
    
    // PHASE 2: Event-driven flags
    private bool m_UseEventDrivenUpdates = true;
    private bool m_UsePollingFallback = true;
    
    // Player data cache - now only used as fallback
    private ref KOTH_Players m_PlayerDataCache;
    private float m_LastPlayerDataUpdate = 0;
    private float m_PlayerDataUpdateInterval = 5.0;
    
    void KOTH_HUD(IngameHud hud)
    {
        m_Hud = hud;
        m_HUDController = KOTH_HUDController.Cast(GetController());
        
        // PHASE 2: Subscribe to all events
        SubscribeToEvents();
        
        Print("[KOTH_HUD] Initialized with event subscriptions");
    }
    
    void ~KOTH_HUD()
    {
        // PHASE 2: Unsubscribe from events
        UnsubscribeFromEvents();
        
        Print("[KOTH_HUD] Destroyed");
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 2: EVENT SUBSCRIPTION
    // ═══════════════════════════════════════════════════════════════
    
    void SubscribeToEvents()
    {
        if (!m_UseEventDrivenUpdates)
            return;
        
        // Subscribe to GameMode events
        if (KOTH_GameMode.SI_OnScoreChanged)
            KOTH_GameMode.SI_OnScoreChanged.Insert(OnScoreChanged);
        
        if (KOTH_GameMode.SI_OnCaptureProgressChanged)
            KOTH_GameMode.SI_OnCaptureProgressChanged.Insert(OnCaptureProgressChanged);
        
        // Subscribe to HUDDataSync events
        if (KOTH_HUDDataSync.SI_OnZonePlayersChanged)
            KOTH_HUDDataSync.SI_OnZonePlayersChanged.Insert(OnZonePlayersChanged);
        
        if (KOTH_HUDDataSync.SI_OnScoreUpdate)
            KOTH_HUDDataSync.SI_OnScoreUpdate.Insert(OnScoreUpdate);
        
        if (KOTH_HUDDataSync.SI_OnCaptureUpdate)
            KOTH_HUDDataSync.SI_OnCaptureUpdate.Insert(OnCaptureUpdate);
        
        // Subscribe to PlayerRewardManager events
        if (KOTH_PlayerRewardManager.SI_OnPlayerStatsChanged)
            KOTH_PlayerRewardManager.SI_OnPlayerStatsChanged.Insert(OnPlayerStatsChanged);
        
        Print("[KOTH_HUD] Subscribed to all ScriptInvoker events");
    }
    
    void UnsubscribeFromEvents()
    {
        if (!m_UseEventDrivenUpdates)
            return;
        
        if (KOTH_GameMode.SI_OnScoreChanged)
            KOTH_GameMode.SI_OnScoreChanged.Remove(OnScoreChanged);
        
        if (KOTH_GameMode.SI_OnCaptureProgressChanged)
            KOTH_GameMode.SI_OnCaptureProgressChanged.Remove(OnCaptureProgressChanged);
        
        if (KOTH_HUDDataSync.SI_OnZonePlayersChanged)
            KOTH_HUDDataSync.SI_OnZonePlayersChanged.Remove(OnZonePlayersChanged);
        
        if (KOTH_HUDDataSync.SI_OnScoreUpdate)
            KOTH_HUDDataSync.SI_OnScoreUpdate.Remove(OnScoreUpdate);
        
        if (KOTH_HUDDataSync.SI_OnCaptureUpdate)
            KOTH_HUDDataSync.SI_OnCaptureUpdate.Remove(OnCaptureUpdate);
        
        if (KOTH_PlayerRewardManager.SI_OnPlayerStatsChanged)
            KOTH_PlayerRewardManager.SI_OnPlayerStatsChanged.Remove(OnPlayerStatsChanged);
        
        Print("[KOTH_HUD] Unsubscribed from all events");
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 2: EVENT HANDLERS
    // ═══════════════════════════════════════════════════════════════
    
    void OnScoreChanged(int eastScore, int westScore)
    {
        Print("[KOTH_HUD] EVENT: OnScoreChanged - East: " + eastScore + ", West: " + westScore);
        
        if (m_CachedEastScore != eastScore)
        {
            m_CachedEastScore = eastScore;
            if (this.eastScore)
            {
                this.eastScore.SetText(eastScore.ToString());
            }
        }
        
        if (m_CachedWestScore != westScore)
        {
            m_CachedWestScore = westScore;
            if (this.westScore)
            {
                this.westScore.SetText(westScore.ToString());
            }
        }
    }
    
    void OnScoreUpdate(int eastScore, int westScore)
    {
        Print("[KOTH_HUD] EVENT: OnScoreUpdate - East: " + eastScore + ", West: " + westScore);
        OnScoreChanged(eastScore, westScore);
    }
    
    void OnCaptureProgressChanged(float progress, string team)
    {
        Print("[KOTH_HUD] EVENT: OnCaptureProgressChanged - Progress: " + progress + "%, Team: " + team);
        
        if (m_CachedCaptureProgress != progress || m_CachedCapturingTeam != team)
        {
            m_CachedCaptureProgress = progress;
            m_CachedCapturingTeam = team;
            
            if (TeamBar)
            {
                TeamBar.SetCurrent(progress);
                
                if (team == "East")
                {
                    TeamBar.SetColor(ARGB(255, 220, 60, 60));
                }
                else if (team == "West")
                {
                    TeamBar.SetColor(ARGB(255, 60, 120, 220));
                }
                else
                {
                    TeamBar.SetColor(ARGB(255, 100, 100, 100));
                }
            }
        }
    }
    
    void OnCaptureUpdate(float progress, string team)
    {
        Print("[KOTH_HUD] EVENT: OnCaptureUpdate - Progress: " + progress + "%, Team: " + team);
        OnCaptureProgressChanged(progress, team);
    }
    
    void OnZonePlayersChanged(int eastAO, int westAO, int eastPriority, int westPriority)
    {
        Print("[KOTH_HUD] EVENT: OnZonePlayersChanged - AO: E" + eastAO + " W" + westAO + " | Priority: E" + eastPriority + " W" + westPriority);
        
        if (m_CachedEastAO != eastAO)
        {
            m_CachedEastAO = eastAO;
            if (eastAOPlayers)
            {
                string eastText;
                if (eastAO == 1)
                    eastText = "1 in AO";
                else
                    eastText = eastAO.ToString() + " in AO";
                    
                eastAOPlayers.SetText(eastText);
            }
        }
        
        if (m_CachedWestAO != westAO)
        {
            m_CachedWestAO = westAO;
            if (westAOPlayers)
            {
                string westText;
                if (westAO == 1)
                    westText = "1 in AO";
                else
                    westText = westAO.ToString() + " in AO";
                    
                westAOPlayers.SetText(westText);
            }
        }
        
        if (m_CachedEastPriority != eastPriority)
        {
            m_CachedEastPriority = eastPriority;
            if (eastPriorityPlayers)
            {
                string eastPriText;
                if (eastPriority == 1)
                    eastPriText = "1 priority";
                else
                    eastPriText = eastPriority.ToString() + " priority";
                    
                eastPriorityPlayers.SetText(eastPriText);
            }
        }
        
        if (m_CachedWestPriority != westPriority)
        {
            m_CachedWestPriority = westPriority;
            if (westPriorityPlayers)
            {
                string westPriText;
                if (westPriority == 1)
                    westPriText = "1 priority";
                else
                    westPriText = westPriority.ToString() + " priority";
                    
                westPriorityPlayers.SetText(westPriText);
            }
        }
    }
    
    void OnPlayerStatsChanged(int xp, int money, int level)
    {
        Print("[KOTH_HUD] EVENT: OnPlayerStatsChanged - XP: " + xp + ", Money: " + money + ", Level: " + level);
        
        int maxXP = CalculateXPForLevel(level + 1);
        int currentLevelXP = CalculateXPForLevel(level);
        
        int xpProgress;
        int xpNeeded;
        if (level == 1)
        {
            xpProgress = xp;
            xpNeeded = maxXP;
        }
        else
        {
            xpProgress = xp - currentLevelXP;
            xpNeeded = maxXP - currentLevelXP;
        }
        
        if (m_CachedLevel != level)
        {
            m_CachedLevel = level;
            if (currentLevel)
            {
                currentLevel.SetText(level.ToString());
            }
        }
        
        if (m_CachedXP != xp || m_CachedMaxXP != maxXP)
        {
            m_CachedXP = xp;
            m_CachedMaxXP = maxXP;
            
            if (currentXP)
            {
                string xpText = FormatNumber(xpProgress) + " / " + FormatNumber(xpNeeded) + " XP";
                currentXP.SetText(xpText);
            }
        }
        
        if (XPBar)
        {
            float xpPercentage;
            if (xpNeeded > 0)
            {
                xpPercentage = ((float)xpProgress / (float)xpNeeded) * 100.0;
                if (xpPercentage < 0)
                    xpPercentage = 0;
                if (xpPercentage > 100)
                    xpPercentage = 100;
            }
            else
            {
                xpPercentage = 0;
            }
            XPBar.SetCurrent(xpPercentage);
        }
        
        if (m_CachedMoney != money)
        {
            m_CachedMoney = money;
            if (currentMoney)
            {
                currentMoney.SetText("$" + FormatNumber(money));
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // OVERRIDE METHODS
    // ═══════════════════════════════════════════════════════════════
    
    override typename GetControllerType()
    {
        return KOTH_HUDController;
    }
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/hud/KOTH_HUD.layout";
    }
    
    override float GetUpdateTickRate()
    {
        return 0.1;
    }
    
    override void Expansion_Update()
    {
        // PHASE 2: Only use polling as fallback if events are disabled
        if (!m_UsePollingFallback)
            return;
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
            return;
        
        // Keep polling team info as fallback
        UpdateTeamInfoPolling();
        
        // Keep polling player stats as fallback
        UpdatePlayerStatsPolling(player);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 2: POLLING FALLBACK (LEGACY CODE)
    // ═══════════════════════════════════════════════════════════════
    
    void UpdateTeamInfoPolling()
    {
        KOTH_HUDDataSync syncModule;
        CF_Modules<KOTH_HUDDataSync>.Get(syncModule);
        
        if (!syncModule)
            return;
        
        int eastAOCount = syncModule.GetEastPlayersInAO();
        int westAOCount = syncModule.GetWestPlayersInAO();
        int eastPriorityCount = syncModule.GetEastPlayersInPriority();
        int westPriorityCount = syncModule.GetWestPlayersInPriority();
        int eastScoreValue = syncModule.GetEastScore();
        int westScoreValue = syncModule.GetWestScore();
        float captureProgress = syncModule.GetCaptureProgress();
        string capturingTeam = syncModule.GetCapturingTeam();
        
        // Only update if cache is stale (events might not have fired)
        if (m_CachedWestScore != westScoreValue)
        {
            OnScoreChanged(eastScoreValue, westScoreValue);
        }
        
        if (m_CachedWestAO != westAOCount || m_CachedEastAO != eastAOCount || m_CachedWestPriority != westPriorityCount || m_CachedEastPriority != eastPriorityCount)
        {
            OnZonePlayersChanged(eastAOCount, westAOCount, eastPriorityCount, westPriorityCount);
        }
        
        if (m_CachedCaptureProgress != captureProgress || m_CachedCapturingTeam != capturingTeam)
        {
            OnCaptureProgressChanged(captureProgress, capturingTeam);
        }
    }
    
    void UpdatePlayerStatsPolling(PlayerBase player)
    {
        string uid = player.GetIdentity().GetId();
        
        float currentTime = GetGame().GetTime();
        bool shouldReload = false;
        
        if (!m_PlayerDataCache)
        {
            shouldReload = true;
        }
        else
        {
            float timeSinceLastUpdate = currentTime - m_LastPlayerDataUpdate;
            if (timeSinceLastUpdate > (m_PlayerDataUpdateInterval * 1000))
            {
                shouldReload = true;
            }
        }
        
        if (shouldReload)
        {
            m_PlayerDataCache = KOTH_Players.Load(uid);
            m_LastPlayerDataUpdate = currentTime;
            
            if (m_PlayerDataCache)
            {
                if (m_CachedLevel == -1)
                {
                    Print("[KOTH_HUD] POLLING: Initial player data load - Level: " + m_PlayerDataCache.CurrentLevel + ", XP: " + m_PlayerDataCache.TotalExperienceEarned + ", Money: " + m_PlayerDataCache.TotalMoneyinBank);
                }
                
                // Only update if cache is stale (events might not have fired)
                if (m_CachedXP != m_PlayerDataCache.TotalExperienceEarned || m_CachedMoney != m_PlayerDataCache.TotalMoneyinBank || m_CachedLevel != m_PlayerDataCache.CurrentLevel)
                {
                    OnPlayerStatsChanged(m_PlayerDataCache.TotalExperienceEarned, m_PlayerDataCache.TotalMoneyinBank, m_PlayerDataCache.CurrentLevel);
                }
            }
            else
            {
                Print("[KOTH_HUD] POLLING: Creating default player data for UID: " + uid);
                m_PlayerDataCache = new KOTH_Players();
                m_PlayerDataCache.Defaults();
                m_PlayerDataCache.PlayerID = uid;
                m_PlayerDataCache.PlayerName = player.GetIdentity().GetName();
                
                OnPlayerStatsChanged(0, 0, 1);
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // HELPER METHODS
    // ═══════════════════════════════════════════════════════════════
    
    int CalculateXPForLevel(int level)
    {
        float baseMult = 1000.0;
        float levelFloat = level;
        float exponent = 1.5;
        
        float xpNeeded = baseMult * Math.Pow(levelFloat, exponent);
        return xpNeeded;
    }
    
    string FormatNumber(int number)
    {
        string numStr = number.ToString();
        string result = "";
        int len = numStr.Length();
        int count = 0;
        
        for (int i = len - 1; i >= 0; i--)
        {
            if (count > 0 && count % 3 == 0)
            {
                result = "," + result;
            }
            result = numStr.Get(i) + result;
            count++;
        }
        
        return result;
    }
    
    void ShowHud(bool state)
    {
        if (IsVisible() == state)
            return;
        
        if (state)
        {
            Show();
            Print("[KOTH_HUD] HUD shown");
        }
        else
        {
            Hide();
            Print("[KOTH_HUD] HUD hidden");
        }
    }
    
    void ForceUpdate()
    {
        m_CachedWestScore = -1;
        m_CachedEastScore = -1;
        m_CachedWestAO = -1;
        m_CachedEastAO = -1;
        m_CachedWestPriority = -1;
        m_CachedEastPriority = -1;
        m_CachedCaptureProgress = -1.0;
        m_CachedCapturingTeam = "";
        m_CachedLevel = -1;
        m_CachedXP = -1;
        m_CachedMaxXP = -1;
        m_CachedMoney = -1;
        
        m_PlayerDataCache = null;
        m_LastPlayerDataUpdate = 0;
        
        Expansion_Update();
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PHASE 2: DEBUG TOGGLES
    // ═══════════════════════════════════════════════════════════════
    
    void EnableEventDrivenUpdates(bool enable)
    {
        m_UseEventDrivenUpdates = enable;
        
        if (enable)
        {
            SubscribeToEvents();
            Print("[KOTH_HUD] Event-driven updates ENABLED");
        }
        else
        {
            UnsubscribeFromEvents();
            Print("[KOTH_HUD] Event-driven updates DISABLED");
        }
    }
    
    void EnablePollingFallback(bool enable)
    {
        m_UsePollingFallback = enable;
        
        string status;
        if (enable)
            status = "ENABLED";
        else
            status = "DISABLED";
        
        Print("[KOTH_HUD] Polling fallback " + status);
    }
}

class KOTH_HUDController: ExpansionViewController
{
    string WestScore;
    string EastScore;
    string WestAOPlayers;
    string EastAOPlayers;
    string WestPriorityPlayers;
    string EastPriorityPlayers;
    string CurrentLevel;
    string CurrentXP;
    string CurrentMoney;
}

modded class IngameHud
{
    private ref KOTH_HUD m_KOTH_HUD;
    
    void IngameHud()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(InitKOTHHUD, 1000, false);
    }
    
    void ~IngameHud()
    {
        if (m_KOTH_HUD)
        {
            m_KOTH_HUD.Destroy();
            m_KOTH_HUD = null;
        }
    }
    
    void InitKOTHHUD()
    {
        m_KOTH_HUD = new KOTH_HUD(this);
        
        if (m_KOTH_HUD)
        {
            m_KOTH_HUD.ShowHud(true);
            Print("[KOTH_IngameHud] KOTH HUD created with event subscriptions");
        }
        else
        {
            Error("[KOTH_IngameHud] Failed to create KOTH HUD!");
        }
    }
    
    KOTH_HUD GetKOTHHUD()
    {
        return m_KOTH_HUD;
    }
    
    void ShowKOTHHUD(bool state)
    {
        if (m_KOTH_HUD)
        {
            m_KOTH_HUD.ShowHud(state);
        }
    }
    
    void UpdateKOTHHUD()
    {
        if (m_KOTH_HUD)
        {
            m_KOTH_HUD.ForceUpdate();
        }
    }
    
    override void ShowHud(bool show)
    {
        super.ShowHud(show);
        
        if (m_KOTH_HUD)
        {
            m_KOTH_HUD.ShowHud(show);
        }
    }
}