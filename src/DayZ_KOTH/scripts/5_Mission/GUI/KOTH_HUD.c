/**
 * KOTH_HUD.c
 *
 * King of the Hill by Kahoona
 * HUD Display for team scores, player stats, and zone information
 *
 * Place in: 5_Mission/GUI/KOTH_HUD.c
 */

class KOTH_HUD: ExpansionScriptView
{
    private ref KOTH_HUDController m_HUDController;
    private IngameHud m_Hud;
    
    // Top Panel Widgets (Team Scores)
    protected TextWidget eastScore;
    protected TextWidget eastPlayers;
    protected ProgressBarWidget TeamBar;
    protected TextWidget westScore;
    protected TextWidget westPlayers;
    
    // Level Panel Widgets (Player Stats)
    protected TextWidget currentLevel;
    protected ProgressBarWidget XPBar;
    protected TextWidget currentXP;
    protected TextWidget currentMoney;
    
    // Cache for player data
    private int m_CachedEastScore = 0;
    private int m_CachedWestScore = 0;
    private int m_CachedEastPlayers = 0;
    private int m_CachedWestPlayers = 0;
    private int m_CachedLevel = 1;
    private int m_CachedXP = 0;
    private int m_CachedMaxXP = 1000;
    private int m_CachedMoney = 0;
    
    // Player data cache to avoid constant file reads
    private ref KOTH_Players m_PlayerDataCache;
    private float m_LastPlayerDataUpdate = 0;
    private float m_PlayerDataUpdateInterval = 5.0;
    
    // ═══════════════════════════════════════════════════════════════
    // INITIALIZATION
    // ═══════════════════════════════════════════════════════════════
    
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
    
    // ═══════════════════════════════════════════════════════════════
    // EXPANSION SCRIPTVIEW OVERRIDES
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
        return 1.0;
    }
    
    // ═══════════════════════════════════════════════════════════════
    // UPDATE LOOP
    // ═══════════════════════════════════════════════════════════════
    
    override void Expansion_Update()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
            return;
        
        UpdateTeamInfo();
        UpdatePlayerStats(player);
    }
    
    // ═══════════════════════════════════════════════════════════════
    // TEAM INFO UPDATE
    // ═══════════════════════════════════════════════════════════════
    
    void UpdateTeamInfo()
    {
        int eastScoreValue = 45;
        int westScoreValue = 38;
        int eastPlayerCount = 12;
        int westPlayerCount = 10;
        
        if (m_CachedEastScore != eastScoreValue)
        {
            m_CachedEastScore = eastScoreValue;
            if (this.eastScore)
            {
                this.eastScore.SetText(eastScoreValue.ToString());
            }
        }
        
        if (m_CachedWestScore != westScoreValue)
        {
            m_CachedWestScore = westScoreValue;
            if (this.westScore)
            {
                this.westScore.SetText(westScoreValue.ToString());
            }
        }
        
        if (m_CachedEastPlayers != eastPlayerCount)
        {
            m_CachedEastPlayers = eastPlayerCount;
            if (eastPlayers)
            {
                eastPlayers.SetText(eastPlayerCount.ToString() + " players");
            }
        }
        
        if (m_CachedWestPlayers != westPlayerCount)
        {
            m_CachedWestPlayers = westPlayerCount;
            if (westPlayers)
            {
                westPlayers.SetText(westPlayerCount.ToString() + " players");
            }
        }
        
        if (TeamBar)
        {
            int totalScore = eastScoreValue + westScoreValue;
            float westPercentage;
            if (totalScore > 0)
            {
                westPercentage = ((float)westScoreValue / (float)totalScore) * 100.0;
            }
            else
            {
                westPercentage = 50.0;
            }
            TeamBar.SetCurrent(westPercentage);
        }
    }
    
    // ═══════════════════════════════════════════════════════════════
    // PLAYER STATS UPDATE
    // ═══════════════════════════════════════════════════════════════
    
    void UpdatePlayerStats(PlayerBase player)
    {
        string uid = player.GetIdentity().GetId();
        
        // Only reload player data every 5 seconds to avoid constant file reads
        float currentTime = GetGame().GetTime();
        if (!m_PlayerDataCache || (currentTime - m_LastPlayerDataUpdate) > (m_PlayerDataUpdateInterval * 1000))
        {
            m_PlayerDataCache = KOTH_Players.Load(uid);
            m_LastPlayerDataUpdate = currentTime;
            
            if (!m_PlayerDataCache)
            {
                // If still NULL after load attempt, create default data
                Print("[KOTH_HUD] Creating default player data for UID: " + uid);
                m_PlayerDataCache = new KOTH_Players();
                m_PlayerDataCache.Defaults();
                m_PlayerDataCache.PlayerID = uid;
                m_PlayerDataCache.PlayerName = player.GetIdentity().GetName();
            }
        }
        
        if (!m_PlayerDataCache)
        {
            // Fallback to defaults if still NULL
            UpdatePlayerStatsWithDefaults();
            return;
        }
        
        // Get player stats from cache
        int level = m_PlayerDataCache.CurrentLevel;
        int xp = m_PlayerDataCache.TotalExperienceEarned;
        int money = m_PlayerDataCache.TotalMoneyinBank;
        
        int maxXP = CalculateXPForLevel(level + 1);
        int currentLevelXP = CalculateXPForLevel(level);
        int xpProgress = xp - currentLevelXP;
        int xpNeeded = maxXP - currentLevelXP;
        
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
        // Show default values when player data unavailable
        if (currentLevel)
            currentLevel.SetText("1");
        
        if (currentXP)
            currentXP.SetText("0 / 1,000 XP");
        
        if (XPBar)
            XPBar.SetCurrent(0);
        
        if (currentMoney)
            currentMoney.SetText("$0");
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
    
    // ═══════════════════════════════════════════════════════════════
    // PUBLIC METHODS
    // ═══════════════════════════════════════════════════════════════
    
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
        m_CachedEastScore = -1;
        m_CachedWestScore = -1;
        m_CachedEastPlayers = -1;
        m_CachedWestPlayers = -1;
        m_CachedLevel = -1;
        m_CachedXP = -1;
        m_CachedMaxXP = -1;
        m_CachedMoney = -1;
        
        // Force reload player data
        m_PlayerDataCache = null;
        m_LastPlayerDataUpdate = 0;
        
        Expansion_Update();
    }
}

// ═══════════════════════════════════════════════════════════════
// CONTROLLER
// ═══════════════════════════════════════════════════════════════

class KOTH_HUDController: ExpansionViewController
{
    string EastScore;
    string WestScore;
    string EastPlayers;
    string WestPlayers;
    string CurrentLevel;
    string CurrentXP;
    string CurrentMoney;
}

// ═══════════════════════════════════════════════════════════════
// MODDED INGAMEHUD - Integration
// ═══════════════════════════════════════════════════════════════

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