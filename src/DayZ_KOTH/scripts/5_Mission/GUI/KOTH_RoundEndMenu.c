/**
 * KOTH_RoundEndMenu.c (CORRECTED - PERSISTENT MODULE REFERENCE)
 *
 * King of the Hill by Kahoona
 * Round end menu with ViewBinding controller pattern
 *
 * Place in: 5_Mission/GUI/KOTH_RoundEndMenu.c
 */

class KOTH_RoundEndMenu: ExpansionScriptViewMenu
{
    protected ref KOTH_RoundEndMenuController m_RoundEndMenuController;
    protected MissionGameplay m_Mission;
    protected KOTH_RoundStatsTracker m_RoundStatsTracker;
    protected KOTH_RoundEndModule m_RoundEndModule;
    
    protected Widget m_VotingPanel;
    protected GridSpacerWidget m_ZoneVoteGrid;
    
    protected ref KOTH_RoundTeamStats m_EastStats;
    protected ref KOTH_RoundTeamStats m_WestStats;
    protected string m_WinningTeam;
    
    protected float m_CountdownTimer;
    protected float m_DisplayDuration;
    
    protected ref array<string> m_AvailableZones;
    protected ref map<string, int> m_ZoneVotes;
    protected string m_PlayerVotedZone;
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/KOTH_RoundEndMenu.layout";
    }
    
    override typename GetControllerType()
    {
        return KOTH_RoundEndMenuController;
    }
    
    void KOTH_RoundEndMenu()
    {
        Print("[KOTH_RoundEndMenu] Constructor - Starting...");
        
        m_RoundEndMenuController = KOTH_RoundEndMenuController.Cast(GetController());
        Class.CastTo(m_Mission, MissionGameplay.Cast(GetGame().GetMission()));
        
        m_AvailableZones = new array<string>();
        m_ZoneVotes = new map<string, int>();
        m_PlayerVotedZone = "";
        
        m_RoundEndModule = KOTH_RoundEndModule.GetInstance();
        if (!m_RoundEndModule)
        {
            Error("[KOTH_RoundEndMenu] Constructor - Failed to get module instance!");
            return;
        }
        
        ScriptInvoker m_GetRoundEndMenuSIInsert = m_RoundEndModule.GetRoundEndMenuSI();
        if (m_GetRoundEndMenuSIInsert)
        {
            m_GetRoundEndMenuSIInsert.Insert(ShowMenuNow);
            Print("[KOTH_RoundEndMenu] Subscribed m_GetRoundEndMenuSIInsert to menu invoker");
        }
        else
        {
            Error("[KOTH_RoundEndMenu] Menu invoker m_GetRoundEndMenuSIInsert is NULL!");
        }

        ScriptInvoker m_GetUpdateVoteCountsSIInsert = m_RoundEndModule.GetUpdateVoteCountsSI();
        if (m_GetUpdateVoteCountsSIInsert)
        {
            m_GetUpdateVoteCountsSIInsert.Insert(OnVoteCountsUpdated);
            Print("[KOTH_RoundEndMenu] Subscribed m_GetUpdateVoteCountsSIInsert to menu invoker");
        }
        else
        {
            Error("[KOTH_RoundEndMenu] Menu invoker m_GetUpdateVoteCountsSIInsert is NULL!");
        }

        m_RoundStatsTracker = KOTH_RoundStatsTracker.GetInstance();
        if (m_RoundStatsTracker)
        {
            ScriptInvoker m_GetRoundStatsAvailableSIInsert = m_RoundStatsTracker.GetRoundStatsAvailableSI();
            if (m_GetRoundStatsAvailableSIInsert)
            {
                m_GetRoundStatsAvailableSIInsert.Insert(OnStatsReady);
                Print("[KOTH_RoundEndMenu] Subscribed m_GetRoundStatsAvailableSIInsert to menu invoker");
            }
            else
            {
                Error("[KOTH_RoundEndMenu] Menu invoker m_GetRoundStatsAvailableSIInsert is NULL!");
            }
            Print("[KOTH_RoundEndMenu] Constructor - Subscribed to stats tracker");
        }
        else
        {
            Error("[KOTH_RoundEndMenu] Constructor - Failed to get stats tracker instance!");
            return;
        }
        Print("[KOTH_RoundEndMenu] Constructor - Subscribed to module invokers");
    }
    
    void ~KOTH_RoundEndMenu()
    {
        if (m_RoundStatsTracker)
        {
            ScriptInvoker m_GetRoundStatsAvailableSIRemove = m_RoundStatsTracker.GetRoundStatsAvailableSI();
            if (m_GetRoundStatsAvailableSIRemove)
            {
                m_GetRoundStatsAvailableSIRemove.Remove(OnStatsReady);
                Print("[KOTH_RoundEndMenu] Subscribed m_GetRoundStatsAvailableSIRemove to menu invoker");
            }
            else
            {
                Error("[KOTH_RoundEndMenu] Menu invoker m_GetRoundStatsAvailableSIRemove is NULL!");
            }
        }

        if (m_RoundEndModule)
        {
            ScriptInvoker m_GetRoundEndMenuSIRemove = m_RoundEndModule.GetRoundEndMenuSI();
            if (m_GetRoundEndMenuSIRemove)
            {
                m_GetRoundEndMenuSIRemove.Remove(ShowMenuNow);
                Print("[KOTH_RoundEndMenu] Subscribed m_GetRoundEndMenuSIRemove to menu invoker");
            }
            else
            {
                Error("[KOTH_RoundEndMenu] Menu invoker m_GetRoundEndMenuSIRemove is NULL!");
            }
        }
        
        ScriptInvoker m_GetUpdateVoteCountsSI = m_RoundEndModule.GetUpdateVoteCountsSI();
        if (m_GetUpdateVoteCountsSI)
        {
            m_GetUpdateVoteCountsSI.Remove(OnVoteCountsUpdated);
            Print("[KOTH_RoundEndMenu] Subscribed m_GetUpdateVoteCountsSI to menu invoker");
        }
        else
        {
            Error("[KOTH_RoundEndMenu] Menu invoker m_GetUpdateVoteCountsSI is NULL!");
        }
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateCountdown);
        
        Print("[KOTH_RoundEndMenu] Destructor - Unsubscribed from invokers");
    }
    
    override void OnShow()
    {
        super.OnShow();
        
        Print("[KOTH_RoundEndMenu] OnShow CALLED - Menu displaying");
        
        GetGame().GetInput().ChangeGameFocus(1);
        SetFocus(GetLayoutRoot());
        PPEffects.SetBlurMenu(0.5);
        m_Mission.GetHud().ShowHud(false);
        m_Mission.GetHud().ShowQuickBar(false);
        
        m_VotingPanel = GetLayoutRoot().FindAnyWidget("VotingPanel");
        m_ZoneVoteGrid = GridSpacerWidget.Cast(GetLayoutRoot().FindAnyWidget("ZoneVoteGrid"));
        
        Print("[KOTH_RoundEndMenu] OnShow completed");
    }
    
    override void OnHide()
    {
        super.OnHide();
        
        GetGame().GetInput().ResetGameFocus();
        PPEffects.SetBlurMenu(0.0);
        m_Mission.GetHud().ShowHud(true);
        m_Mission.GetHud().ShowQuickBar(true);
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateCountdown);
        
        Clear();
    }
    
    void ShowMenuNow(array<string> zones, bool votingEnabled)
    {
        Print("[KOTH_RoundEndMenu] ============================================");
        Print("[KOTH_RoundEndMenu] ShowMenuNow INVOKED!");
        
        if (!zones || zones.Count() == 0)
        {
            Error("[KOTH_RoundEndMenu] ShowMenuNow - No zones received!");
            return;
        }
        
        Print("[KOTH_RoundEndMenu]   - Zones: " + zones.Count());
        Print("[KOTH_RoundEndMenu]   - Voting Enabled: " + votingEnabled);
        
        m_AvailableZones = zones;
        
        SetupVoting(zones, votingEnabled);
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateCountdown, 1000, true);
        
        Print("[KOTH_RoundEndMenu] ============================================");
    }

    void OnStatsReady(KOTH_RoundTeamStats eastStats, KOTH_RoundTeamStats westStats)
    {
        Print("[KOTH_RoundEndMenu] OnStatsReady - Stats are now available");
        LoadRoundData();
    }
    
    void LoadRoundData()
    {
        Print("[KOTH_RoundEndMenu] LoadRoundData CALLED");
            
        m_EastStats = m_RoundStatsTracker.GetTeamStats("East");
        m_WestStats = m_RoundStatsTracker.GetTeamStats("West");
        
        if (!m_EastStats || !m_WestStats)
        {
            Error("[KOTH_RoundEndMenu] ERROR: Could not get team stats!");
            return;
        }
        
        if (m_EastStats.FinalScore > m_WestStats.FinalScore)
        {
            m_WinningTeam = "East";
        }
        else
        {
            m_WinningTeam = "West";
        }
        
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (settings)
        {
            m_DisplayDuration = settings.EndScreenDisplaySeconds;
        }
        else
        {
            m_DisplayDuration = 30;
        }
        
        m_CountdownTimer = m_DisplayDuration;
        
        UpdateDisplay();
    }
    
    void UpdateDisplay()
    {
        Print("[KOTH_RoundEndMenu] UpdateDisplay called");
        
        m_RoundEndMenuController.WinnerTitle = "TEAM " + m_WinningTeam.ToUpper() + " WINS!";
        m_RoundEndMenuController.NotifyPropertyChanged("WinnerTitle");
        
        m_RoundEndMenuController.FinalScoreText = "East: " + m_EastStats.FinalScore + "  |  West: " + m_WestStats.FinalScore;
        m_RoundEndMenuController.NotifyPropertyChanged("FinalScoreText");
        
        m_RoundEndMenuController.EastKills = "Kills: " + m_EastStats.TotalKills;
        m_RoundEndMenuController.NotifyPropertyChanged("EastKills");
        
        m_RoundEndMenuController.EastDeaths = "Deaths: " + m_EastStats.TotalDeaths;
        m_RoundEndMenuController.NotifyPropertyChanged("EastDeaths");
        
        m_RoundEndMenuController.EastHeadshots = "Headshots: " + m_EastStats.TotalHeadshots;
        m_RoundEndMenuController.NotifyPropertyChanged("EastHeadshots");
        
        m_RoundEndMenuController.EastRevives = "Revives: " + m_EastStats.TotalRevives;
        m_RoundEndMenuController.NotifyPropertyChanged("EastRevives");
        
        m_RoundEndMenuController.EastXP = "Total XP: " + m_EastStats.TotalXPEarned;
        m_RoundEndMenuController.NotifyPropertyChanged("EastXP");
        
        m_RoundEndMenuController.WestKills = "Kills: " + m_WestStats.TotalKills;
        m_RoundEndMenuController.NotifyPropertyChanged("WestKills");
        
        m_RoundEndMenuController.WestDeaths = "Deaths: " + m_WestStats.TotalDeaths;
        m_RoundEndMenuController.NotifyPropertyChanged("WestDeaths");
        
        m_RoundEndMenuController.WestHeadshots = "Headshots: " + m_WestStats.TotalHeadshots;
        m_RoundEndMenuController.NotifyPropertyChanged("WestHeadshots");
        
        m_RoundEndMenuController.WestRevives = "Revives: " + m_WestStats.TotalRevives;
        m_RoundEndMenuController.NotifyPropertyChanged("WestRevives");
        
        m_RoundEndMenuController.WestXP = "Total XP: " + m_WestStats.TotalXPEarned;
        m_RoundEndMenuController.NotifyPropertyChanged("WestXP");
        
        UpdateTopPerformers();
    }
    
    void UpdateTopPerformers()
    {
        KOTH_RoundStatsTracker tracker = KOTH_RoundStatsTracker.GetInstance();
        if (!tracker)
            return;
        
        KOTH_RoundPlayerStats mvp = tracker.GetMVP();
        if (mvp)
        {
            m_RoundEndMenuController.MVPPlayerName = mvp.PlayerName;
            m_RoundEndMenuController.MVPStat = mvp.Kills.ToString() + " Kills";
        }
        else
        {
            m_RoundEndMenuController.MVPPlayerName = "N/A";
            m_RoundEndMenuController.MVPStat = "";
        }
        m_RoundEndMenuController.NotifyPropertyChanged("MVPPlayerName");
        m_RoundEndMenuController.NotifyPropertyChanged("MVPStat");
        
        KOTH_RoundPlayerStats sharpshooter = tracker.GetSharpshooter();
        if (sharpshooter)
        {
            m_RoundEndMenuController.SharpshooterPlayerName = sharpshooter.PlayerName;
            m_RoundEndMenuController.SharpshooterStat = sharpshooter.Headshots.ToString() + " Headshots";
        }
        else
        {
            m_RoundEndMenuController.SharpshooterPlayerName = "N/A";
            m_RoundEndMenuController.SharpshooterStat = "";
        }
        m_RoundEndMenuController.NotifyPropertyChanged("SharpshooterPlayerName");
        m_RoundEndMenuController.NotifyPropertyChanged("SharpshooterStat");
        
        KOTH_RoundPlayerStats medic = tracker.GetTopMedic();
        if (medic)
        {
            m_RoundEndMenuController.MedicPlayerName = medic.PlayerName;
            m_RoundEndMenuController.MedicStat = medic.Revives.ToString() + " Revives";
        }
        else
        {
            m_RoundEndMenuController.MedicPlayerName = "N/A";
            m_RoundEndMenuController.MedicStat = "";
        }
        m_RoundEndMenuController.NotifyPropertyChanged("MedicPlayerName");
        m_RoundEndMenuController.NotifyPropertyChanged("MedicStat");
        
        Print("[KOTH_RoundEndMenu] Top performers updated");
    }
    
    void SetupVoting(array<string> zones, bool votingEnabled)
    {
        Print("[KOTH_RoundEndMenu] SetupVoting called with " + zones.Count() + " zones, voting: " + votingEnabled);
        
        m_AvailableZones = zones;
        
        if (!votingEnabled || zones.Count() == 0)
        {
            if (m_VotingPanel)
                m_VotingPanel.Show(false);
            
            if (zones.Count() > 0)
            {
                m_RoundEndMenuController.VotingHeader = "NEXT ZONE: " + zones.Get(0);
                m_RoundEndMenuController.NotifyPropertyChanged("VotingHeader");
            }
            
            return;
        }
        
        if (m_VotingPanel)
            m_VotingPanel.Show(true);
        
        m_RoundEndMenuController.VotingHeader = "VOTE FOR NEXT ZONE";
        m_RoundEndMenuController.NotifyPropertyChanged("VotingHeader");
        
        CreateVoteButtons();
    }
    
    void CreateVoteButtons()
    {
        if (!m_ZoneVoteGrid)
        {
            Error("[KOTH_RoundEndMenu] ZoneVoteGrid is NULL!");
            return;
        }
        
        Print("[KOTH_RoundEndMenu] Creating vote buttons for " + m_AvailableZones.Count() + " zones");
        
        Widget child = m_ZoneVoteGrid.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            delete child;
            child = next;
        }
        
        int index = 0;
        foreach (string zoneName : m_AvailableZones)
        {
            Widget buttonPanel = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/GUI/layouts/KOTH_VoteButton.layout", m_ZoneVoteGrid);
            if (!buttonPanel)
            {
                Error("[KOTH_RoundEndMenu] Failed to create vote button panel!");
                continue;
            }
            
            buttonPanel.SetUserID(index);
            
            TextWidget zoneNameText = TextWidget.Cast(buttonPanel.FindAnyWidget("ZoneNameText"));
            TextWidget voteCountText = TextWidget.Cast(buttonPanel.FindAnyWidget("VoteCountText"));
            
            if (zoneNameText)
                zoneNameText.SetText(zoneName);
            
            if (voteCountText)
                voteCountText.SetText("0 votes");
            
            m_ZoneVotes.Set(zoneName, 0);
            index++;
        }
        
        Print("[KOTH_RoundEndMenu] Created " + index + " vote buttons");
    }
    
    void OnVoteCountsUpdated(map<string, int> votes)
    {
        UpdateVoteCounts(votes);
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (button != MouseState.LEFT)
            return false;
        
        string widgetName = w.GetName();
        
        if (widgetName == "VoteButton")
        {
            Widget parent = w.GetParent();
            if (parent)
            {
                int zoneIndex = parent.GetUserID();
                if (zoneIndex >= 0 && zoneIndex < m_AvailableZones.Count())
                {
                    string zoneName = m_AvailableZones.Get(zoneIndex);
                    VoteForZone(zoneName);
                }
            }
            return true;
        }
        
        return false;
    }
    
    void VoteForZone(string zoneName)
    {
        if (m_PlayerVotedZone != "")
        {
            Print("[KOTH_RoundEndMenu] Player already voted for: " + m_PlayerVotedZone);
            return;
        }
        
        m_PlayerVotedZone = zoneName;
        
        Print("[KOTH_RoundEndMenu] Player voted for zone: " + zoneName);
        
        m_RoundEndModule.SendVote(zoneName);
    }
    
    void UpdateVoteCounts(map<string, int> votes)
    {
        if (!m_ZoneVoteGrid)
            return;
        
        Widget child = m_ZoneVoteGrid.GetChildren();
        int index = 0;
        
        while (child)
        {
            if (index < m_AvailableZones.Count())
            {
                string zoneName = m_AvailableZones.Get(index);
                TextWidget voteCountText = TextWidget.Cast(child.FindAnyWidget("VoteCountText"));
                
                if (voteCountText && votes.Contains(zoneName))
                {
                    int count = votes.Get(zoneName);
                    voteCountText.SetText(count.ToString() + " votes");
                }
            }
            
            child = child.GetSibling();
            index++;
        }
    }
    
    void UpdateCountdown()
    {
        m_CountdownTimer = m_CountdownTimer - 1.0;
        
        if (m_CountdownTimer <= 0)
        {
            CloseMenu();
            return;
        }
        
        int seconds = m_CountdownTimer;
        m_RoundEndMenuController.CountdownText = "Next round starts in: " + seconds.ToString() + " seconds";
        m_RoundEndMenuController.NotifyPropertyChanged("CountdownText");
    }
    
    void CloseMenu()
    {
        Hide();
    }
    
    void Clear()
    {
        m_AvailableZones.Clear();
        m_ZoneVotes.Clear();
        m_PlayerVotedZone = "";
    }
}

class KOTH_RoundEndMenuController: ExpansionViewController
{
    string WinnerTitle;
    string FinalScoreText;
    
    string EastKills;
    string EastDeaths;
    string EastHeadshots;
    string EastRevives;
    string EastXP;
    
    string WestKills;
    string WestDeaths;
    string WestHeadshots;
    string WestRevives;
    string WestXP;
    
    string MVPPlayerName;
    string MVPStat;
    
    string SharpshooterPlayerName;
    string SharpshooterStat;
    
    string MedicPlayerName;
    string MedicStat;
    
    string VotingHeader;
    string CountdownText;
}