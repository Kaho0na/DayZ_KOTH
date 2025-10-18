/**
 * KOTH_HUD.c (WITH AO AND PRIORITY COUNTS)
 *
 * King of the Hill by Kahoona
 * HUD Display with AO and priority zone player counts
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
    
    // Player data cache to avoid constant file reads
    private ref KOTH_Players m_PlayerDataCache;
    private float m_LastPlayerDataUpdate = 0;
    private float m_PlayerDataUpdateInterval = 5.0;
    
    void KOTH_HUD(IngameHud hud)
    {
        m_Hud = hud;
        m_HUDController = KOTH_HUDController.Cast(GetController());
        
        Print("[KOTH_HUD] Initialized");
    }
    
    void ~KOTH_HUD()
    {
        Print("[KOTH_HUD] Destroyed");
    }
    
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
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
            return;
        
        UpdateTeamInfo();
        UpdatePlayerStats(player);
    }
    
    void UpdateTeamInfo()
    {
        KOTH_HUDDataSync syncModule;
        CF_Modules<KOTH_HUDDataSync>.Get(syncModule);
        
        int eastAOCount = 0;
        int westAOCount = 0;
        int eastPriorityCount = 0;
        int westPriorityCount = 0;
        int eastScoreValue = 0;
        int westScoreValue = 0;
        float captureProgress = 0.0;
        string capturingTeam = "None";
        
        if (syncModule)
        {
            eastAOCount = syncModule.GetEastPlayersInAO();
            westAOCount = syncModule.GetWestPlayersInAO();
            eastPriorityCount = syncModule.GetEastPlayersInPriority();
            westPriorityCount = syncModule.GetWestPlayersInPriority();
            eastScoreValue = syncModule.GetEastScore();
            westScoreValue = syncModule.GetWestScore();
            captureProgress = syncModule.GetCaptureProgress();
            capturingTeam = syncModule.GetCapturingTeam();
        }
        
        if (m_CachedWestScore != westScoreValue)
        {
            m_CachedWestScore = westScoreValue;
            if (this.westScore)
            {
                this.westScore.SetText(westScoreValue.ToString());
            }
        }
        
        if (m_CachedEastScore != eastScoreValue)
        {
            m_CachedEastScore = eastScoreValue;
            if (this.eastScore)
            {
                this.eastScore.SetText(eastScoreValue.ToString());
            }
        }
        
        if (m_CachedWestAO != westAOCount)
        {
            m_CachedWestAO = westAOCount;
            if (westAOPlayers)
            {
                string westText;
                if (westAOCount == 1)
                    westText = "1 in AO";
                else
                    westText = westAOCount.ToString() + " in AO";
                    
                westAOPlayers.SetText(westText);
            }
        }
        
        if (m_CachedEastAO != eastAOCount)
        {
            m_CachedEastAO = eastAOCount;
            if (eastAOPlayers)
            {
                string eastText;
                if (eastAOCount == 1)
                    eastText = "1 in AO";
                else
                    eastText = eastAOCount.ToString() + " in AO";
                    
                eastAOPlayers.SetText(eastText);
            }
        }
        
        if (m_CachedWestPriority != westPriorityCount)
        {
            m_CachedWestPriority = westPriorityCount;
            if (westPriorityPlayers)
            {
                string westPriText;
                if (westPriorityCount == 1)
                    westPriText = "1 priority";
                else
                    westPriText = westPriorityCount.ToString() + " priority";
                    
                westPriorityPlayers.SetText(westPriText);
            }
        }
        
        if (m_CachedEastPriority != eastPriorityCount)
        {
            m_CachedEastPriority = eastPriorityCount;
            if (eastPriorityPlayers)
            {
                string eastPriText;
                if (eastPriorityCount == 1)
                    eastPriText = "1 priority";
                else
                    eastPriText = eastPriorityCount.ToString() + " priority";
                    
                eastPriorityPlayers.SetText(eastPriText);
            }
        }
        
        if (m_CachedCaptureProgress != captureProgress || m_CachedCapturingTeam != capturingTeam)
        {
            m_CachedCaptureProgress = captureProgress;
            m_CachedCapturingTeam = capturingTeam;
            
            if (TeamBar)
            {
                TeamBar.SetCurrent(captureProgress);
                
                if (capturingTeam == "East")
                {
                    TeamBar.SetColor(ARGB(255, 220, 60, 60));
                }
                else if (capturingTeam == "West")
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
    
    void UpdatePlayerStats(PlayerBase player)
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
                    Print("[KOTH_HUD] Initial player data load - Level: " + m_PlayerDataCache.CurrentLevel + ", XP: " + m_PlayerDataCache.TotalExperienceEarned + ", Money: " + m_PlayerDataCache.TotalMoneyinBank);
                }
            }
            else
            {
                Print("[KOTH_HUD] Creating default player data for UID: " + uid);
                m_PlayerDataCache = new KOTH_Players();
                m_PlayerDataCache.Defaults();
                m_PlayerDataCache.PlayerID = uid;
                m_PlayerDataCache.PlayerName = player.GetIdentity().GetName();
            }
        }
        
        if (!m_PlayerDataCache)
        {
            UpdatePlayerStatsWithDefaults();
            return;
        }
        
        int level = m_PlayerDataCache.CurrentLevel;
        int xp = m_PlayerDataCache.TotalExperienceEarned;
        int money = m_PlayerDataCache.TotalMoneyinBank;
        
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
    
    void UpdatePlayerStatsWithDefaults()
    {
        if (currentLevel)
            currentLevel.SetText("1");
        
        if (currentXP)
            currentXP.SetText("0 / 1,000 XP");
        
        if (XPBar)
            XPBar.SetCurrent(0);
        
        if (currentMoney)
            currentMoney.SetText("$0");
    }
    
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
            Print("[KOTH_IngameHud] KOTH HUD created and shown");
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