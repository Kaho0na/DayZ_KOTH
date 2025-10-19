/**
 * KOTH_HUD.c (FIXED XP CALCULATION)
 *
 * Fixed XP progress calculation to use correct level thresholds
 * Place in: 5_Mission/GUI/KOTH_HUD.c
 */

class KOTH_HUD: ExpansionScriptView
{
    private ref KOTH_HUDController m_HUDController;
    private IngameHud m_Hud;
    
    protected TextWidget westScore;
    protected TextWidget westAOPlayers;
    protected TextWidget westPriorityPlayers;
    protected ProgressBarWidget TeamBar;
    protected TextWidget eastScore;
    protected TextWidget eastAOPlayers;
    protected TextWidget eastPriorityPlayers;
    
    protected TextWidget currentLevel;
    protected ProgressBarWidget XPBar;
    protected TextWidget currentXP;
    protected TextWidget currentMoney;
    
    private int m_CachedWestScore = -1;
    private int m_CachedEastScore = -1;
    private int m_CachedWestAO = -1;
    private int m_CachedEastAO = -1;
    private int m_CachedWestPriority = -1;
    private int m_CachedEastPriority = -1;
    private float m_CachedCaptureProgress = -1.0;
    private string m_CachedCapturingTeam = "";
    private int m_CachedLevel = -1;
    private int m_CachedXP = -1;
    private int m_CachedMoney = -1;
    
    void KOTH_HUD(IngameHud hud)
    {
        m_Hud = hud;
        m_HUDController = KOTH_HUDController.Cast(GetController());
        
        //Print("[KOTH_HUD] Constructing HUD...");
        SubscribeToEvents();
        //Print("[KOTH_HUD] HUD initialized (event-driven mode)");
    }
    
    void ~KOTH_HUD()
    {
        UnsubscribeFromEvents();
        //Print("[KOTH_HUD] Destroyed");
    }
    
    void SubscribeToEvents()
    {
        //Print("[KOTH_HUD] Subscribing to events...");
        
        if (KOTH_GameMode.SI_OnScoreChanged)
        {
            KOTH_GameMode.SI_OnScoreChanged.Insert(OnScoreChanged);
            //Print("[KOTH_HUD] - Subscribed to SI_OnScoreChanged");
        }
        
        if (KOTH_GameMode.SI_OnCaptureProgressChanged)
        {
            KOTH_GameMode.SI_OnCaptureProgressChanged.Insert(OnCaptureProgressChanged);
            //Print("[KOTH_HUD] - Subscribed to SI_OnCaptureProgressChanged");
        }
        
        if (KOTH_HUDDataSync.SI_OnZonePlayersChanged)
        {
            KOTH_HUDDataSync.SI_OnZonePlayersChanged.Insert(OnZonePlayersChanged);
            //Print("[KOTH_HUD] - Subscribed to SI_OnZonePlayersChanged");
        }
        
        if (KOTH_HUDDataSync.SI_OnScoreUpdate)
        {
            KOTH_HUDDataSync.SI_OnScoreUpdate.Insert(OnScoreUpdate);
            //Print("[KOTH_HUD] - Subscribed to SI_OnScoreUpdate");
        }
        
        if (KOTH_HUDDataSync.SI_OnCaptureUpdate)
        {
            KOTH_HUDDataSync.SI_OnCaptureUpdate.Insert(OnCaptureUpdate);
            //Print("[KOTH_HUD] - Subscribed to SI_OnCaptureUpdate");
        }
        
        if (KOTH_PlayerRewardManager.SI_OnPlayerStatsChanged)
        {
            KOTH_PlayerRewardManager.SI_OnPlayerStatsChanged.Insert(OnPlayerStatsChanged);
            //Print("[KOTH_HUD] - Subscribed to SI_OnPlayerStatsChanged");
        }
        
        //Print("[KOTH_HUD] All subscriptions complete");
    }
    
    void UnsubscribeFromEvents()
    {
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
    }
    
    void OnScoreChanged(int eastScore, int westScore)
    {
        //Print("[KOTH_HUD] OnScoreChanged called - East: " + eastScore + ", West: " + westScore);
        
        if (m_CachedEastScore != eastScore)
        {
            m_CachedEastScore = eastScore;
            if (this.eastScore)
            {
                this.eastScore.SetText(eastScore.ToString());
                //Print("[KOTH_HUD] Updated East score display to: " + eastScore);
            }
        }
        
        if (m_CachedWestScore != westScore)
        {
            m_CachedWestScore = westScore;
            if (this.westScore)
            {
                this.westScore.SetText(westScore.ToString());
                //Print("[KOTH_HUD] Updated West score display to: " + westScore);
            }
        }
    }
    
    void OnScoreUpdate(int eastScore, int westScore)
    {
        //Print("[KOTH_HUD] OnScoreUpdate called - East: " + eastScore + ", West: " + westScore);
        OnScoreChanged(eastScore, westScore);
    }
    
    void OnCaptureProgressChanged(float progress, string team)
    {
        if (m_CachedCaptureProgress == progress && m_CachedCapturingTeam == team)
            return;
        
        m_CachedCaptureProgress = progress;
        m_CachedCapturingTeam = team;
        
        if (TeamBar)
        {
            TeamBar.SetCurrent(progress);
            
            if (team == "East")
                TeamBar.SetColor(ARGB(255, 220, 60, 60));
            else if (team == "West")
                TeamBar.SetColor(ARGB(255, 60, 120, 220));
            else
                TeamBar.SetColor(ARGB(255, 100, 100, 100));
        }
    }
    
    void OnCaptureUpdate(float progress, string team)
    {
        OnCaptureProgressChanged(progress, team);
    }
    
    void OnZonePlayersChanged(int eastAO, int westAO, int eastPriority, int westPriority)
    {
        //Print("[KOTH_HUD] OnZonePlayersChanged called - AO: E" + eastAO + " W" + westAO + " | Priority: E" + eastPriority + " W" + westPriority);
        //Print("[KOTH_HUD] Current cache - AO: E" + m_CachedEastAO + " W" + m_CachedWestAO + " | Priority: E" + m_CachedEastPriority + " W" + m_CachedWestPriority);
        
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
                //Print("[KOTH_HUD] Updated East AO display to: " + eastText);
            }
            else
            {
                //Print("[KOTH_HUD] WARNING: eastAOPlayers widget is NULL!");
            }
        }
        else
        {
            //Print("[KOTH_HUD] East AO unchanged, skipping update");
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
                //Print("[KOTH_HUD] Updated West AO display to: " + westText);
            }
            else
            {
                //Print("[KOTH_HUD] WARNING: westAOPlayers widget is NULL!");
            }
        }
        else
        {
            //Print("[KOTH_HUD] West AO unchanged, skipping update");
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
                //Print("[KOTH_HUD] Updated East Priority display to: " + eastPriText);
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
                //Print("[KOTH_HUD] Updated West Priority display to: " + westPriText);
            }
        }
        
        //Print("[KOTH_HUD] OnZonePlayersChanged complete");
    }
    
    void OnPlayerStatsChanged(int xp, int money, int level)
    {
        int nextLevelXP = CalculateXPForLevel(level);
        int currentLevelXP = CalculateXPForLevel(level - 1);
        
        int xpProgress = xp - currentLevelXP;
        int xpNeeded = nextLevelXP - currentLevelXP;
        
        if (m_CachedLevel != level)
        {
            m_CachedLevel = level;
            if (currentLevel)
                currentLevel.SetText(level.ToString());
        }
        
        if (m_CachedXP != xp)
        {
            m_CachedXP = xp;
            
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
                currentMoney.SetText("$" + FormatNumber(money));
        }
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
        return 1.0;
    }
    
    override void Expansion_Update()
    {
    }
    
    int CalculateXPForLevel(int level)
    {
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (rewardManager)
        {
            return rewardManager.CalculateXPForLevel(level);
        }
        
        return 1000;
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
            Show();
        else
            Hide();
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
        m_CachedMoney = -1;
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
        //Print("[KOTH_IngameHud] Creating KOTH HUD...");
        m_KOTH_HUD = new KOTH_HUD(this);
        
        if (m_KOTH_HUD)
        {
            m_KOTH_HUD.ShowHud(true);
            //Print("[KOTH_IngameHud] KOTH HUD created successfully");
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