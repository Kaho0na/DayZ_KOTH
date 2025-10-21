/**
 * KOTH_RoundEndMenu.c (RECEIVES TOP PERFORMERS FROM SERVER)
 *
 * King of the Hill by Kahoona
 * Client receives and displays server-calculated top performers
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
    
    protected string m_MVPName;
    protected int m_MVPKills;
    protected string m_SharpshooterName;
    protected int m_SharpshooterHeadshots;
    protected string m_MedicName;
    protected int m_MedicRevives;
    
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
        if (m_RoundEndMenuController)
        {
            Print("[KOTH_RoundEndMenu] Controller cast SUCCESS");
        }
        else
        {
            Error("[KOTH_RoundEndMenu] Controller cast FAILED!");
        }
        
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
        if (!m_RoundStatsTracker)
        {
            Error("[KOTH_RoundEndMenu] Constructor - Failed to get stats tracker instance!");
            return;
        }
        
        Print("[KOTH_RoundEndMenu] Constructor - Complete");
    }
    
    void ~KOTH_RoundEndMenu()
    {
        if (m_RoundEndModule)
        {
            ScriptInvoker m_GetRoundEndMenuSIRemove = m_RoundEndModule.GetRoundEndMenuSI();
            if (m_GetRoundEndMenuSIRemove)
            {
                m_GetRoundEndMenuSIRemove.Remove(ShowMenuNow);
            }
            
            ScriptInvoker m_GetUpdateVoteCountsSI = m_RoundEndModule.GetUpdateVoteCountsSI();
            if (m_GetUpdateVoteCountsSI)
            {
                m_GetUpdateVoteCountsSI.Remove(OnVoteCountsUpdated);
            }
        }

        //GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateCountdown);
        
        Print("[KOTH_RoundEndMenu] Destructor - Unsubscribed from invokers");
    }
    
    override void OnShow()
    {
        super.OnShow();
        
        Print("[KOTH_RoundEndMenu] OnShow CALLED - Menu displaying");
        
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);
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
        
        Print("[KOTH_RoundEndMenu] OnHide CALLED - Restoring input");
        
        GetGame().GetInput().ChangeGameFocus(0);
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor(false);
        PPEffects.SetBlurMenu(0.0);
        
        if (m_Mission && m_Mission.GetHud())
        {
            m_Mission.GetHud().ShowHud(true);
            m_Mission.GetHud().ShowQuickBar(true);
        }
        
        Clear();
        
        Print("[KOTH_RoundEndMenu] OnHide completed");
    }
    
    void ShowMenuNow(array<string> zones, bool votingEnabled, string mvpName, int mvpKills, string sharpshooterName, int sharpshooterHeadshots, string medicName, int medicRevives)
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
        Print("[KOTH_RoundEndMenu]   - MVP: " + mvpName + " (" + mvpKills + " kills)");
        Print("[KOTH_RoundEndMenu]   - Sharpshooter: " + sharpshooterName + " (" + sharpshooterHeadshots + " headshots)");
        Print("[KOTH_RoundEndMenu]   - Medic: " + medicName + " (" + medicRevives + " revives)");
        
        m_AvailableZones = zones;
        m_MVPName = mvpName;
        m_MVPKills = mvpKills;
        m_SharpshooterName = sharpshooterName;
        m_SharpshooterHeadshots = sharpshooterHeadshots;
        m_MedicName = medicName;
        m_MedicRevives = medicRevives;
        
        LoadRoundData();
        
        SetupVoting(zones, votingEnabled);

        //GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateCountdown, 1000, true);
        
        Print("[KOTH_RoundEndMenu] ============================================");
    }
    
    void LoadRoundData()
    {
        Print("[KOTH_RoundEndMenu] LoadRoundData CALLED");
        
        if (!m_RoundStatsTracker)
        {
            Error("[KOTH_RoundEndMenu] Stats tracker is NULL!");
            return;
        }
        
        m_EastStats = m_RoundStatsTracker.GetTeamStats("East");
        m_WestStats = m_RoundStatsTracker.GetTeamStats("West");
        
        if (!m_EastStats)
        {
            Error("[KOTH_RoundEndMenu] ERROR: Could not get East team stats!");
            return;
        }
        
        if (!m_WestStats)
        {
            Error("[KOTH_RoundEndMenu] ERROR: Could not get West team stats!");
            return;
        }
        
        Print("[KOTH_RoundEndMenu] East Score: " + m_EastStats.FinalScore + ", West Score: " + m_WestStats.FinalScore);
        
        if (m_EastStats.FinalScore > m_WestStats.FinalScore)
        {
            m_WinningTeam = "East";
        }
        else
        {
            m_WinningTeam = "West";
        }
        
        Print("[KOTH_RoundEndMenu] Winning team: " + m_WinningTeam);
        
        UpdateDisplay();
    }
    
    void UpdateDisplay()
    {
        Print("[KOTH_RoundEndMenu] UpdateDisplay called");
        
        if (!m_RoundEndMenuController)
        {
            Error("[KOTH_RoundEndMenu] Controller is NULL!");
            return;
        }
        
        Print("[KOTH_RoundEndMenu] Setting WinnerTitle...");
        
        string winnerText;
        if (m_WinningTeam == "East")
        {
            winnerText = "TEAM EAST WINS!";
        }
        else
        {
            winnerText = "TEAM WEST WINS!";
        }
        
        m_RoundEndMenuController.WinnerTitle = winnerText;
        m_RoundEndMenuController.NotifyPropertyChanged("WinnerTitle");
        Print("[KOTH_RoundEndMenu] WinnerTitle set to: " + m_RoundEndMenuController.WinnerTitle);
        
        string scoreText = "East: " + m_EastStats.FinalScore.ToString() + "  |  West: " + m_WestStats.FinalScore.ToString();
        m_RoundEndMenuController.FinalScoreText = scoreText;
        m_RoundEndMenuController.NotifyPropertyChanged("FinalScoreText");
        Print("[KOTH_RoundEndMenu] FinalScoreText set to: " + m_RoundEndMenuController.FinalScoreText);
        
        m_RoundEndMenuController.EastKills = "Kills: " + m_EastStats.TotalKills.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("EastKills");
        
        m_RoundEndMenuController.EastDeaths = "Deaths: " + m_EastStats.TotalDeaths.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("EastDeaths");
        
        m_RoundEndMenuController.EastHeadshots = "Headshots: " + m_EastStats.TotalHeadshots.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("EastHeadshots");
        
        m_RoundEndMenuController.EastRevives = "Revives: " + m_EastStats.TotalRevives.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("EastRevives");
        
        m_RoundEndMenuController.EastXP = "Total XP: " + m_EastStats.TotalXPEarned.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("EastXP");
        
        m_RoundEndMenuController.WestKills = "Kills: " + m_WestStats.TotalKills.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("WestKills");
        
        m_RoundEndMenuController.WestDeaths = "Deaths: " + m_WestStats.TotalDeaths.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("WestDeaths");
        
        m_RoundEndMenuController.WestHeadshots = "Headshots: " + m_WestStats.TotalHeadshots.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("WestHeadshots");
        
        m_RoundEndMenuController.WestRevives = "Revives: " + m_WestStats.TotalRevives.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("WestRevives");
        
        m_RoundEndMenuController.WestXP = "Total XP: " + m_WestStats.TotalXPEarned.ToString();
        m_RoundEndMenuController.NotifyPropertyChanged("WestXP");
        
        UpdateTopPerformers();
        
        Print("[KOTH_RoundEndMenu] All display data updated");
    }
    
    void UpdateTopPerformers()
    {
        Print("[KOTH_RoundEndMenu] UpdateTopPerformers called - using server data");
        
        if (m_MVPName != "")
        {
            m_RoundEndMenuController.MVPPlayerName = m_MVPName;
            m_RoundEndMenuController.MVPStat = m_MVPKills.ToString() + " Kills";
            Print("[KOTH_RoundEndMenu] MVP: " + m_MVPName + " with " + m_MVPKills + " kills");
        }
        else
        {
            m_RoundEndMenuController.MVPPlayerName = "N/A";
            m_RoundEndMenuController.MVPStat = "";
            Print("[KOTH_RoundEndMenu] No MVP data");
        }
        m_RoundEndMenuController.NotifyPropertyChanged("MVPPlayerName");
        m_RoundEndMenuController.NotifyPropertyChanged("MVPStat");
        
        if (m_SharpshooterName != "")
        {
            m_RoundEndMenuController.SharpshooterPlayerName = m_SharpshooterName;
            m_RoundEndMenuController.SharpshooterStat = m_SharpshooterHeadshots.ToString() + " Headshots";
            Print("[KOTH_RoundEndMenu] Sharpshooter: " + m_SharpshooterName + " with " + m_SharpshooterHeadshots + " headshots");
        }
        else
        {
            m_RoundEndMenuController.SharpshooterPlayerName = "N/A";
            m_RoundEndMenuController.SharpshooterStat = "";
            Print("[KOTH_RoundEndMenu] No Sharpshooter data");
        }
        m_RoundEndMenuController.NotifyPropertyChanged("SharpshooterPlayerName");
        m_RoundEndMenuController.NotifyPropertyChanged("SharpshooterStat");
        
        if (m_MedicName != "")
        {
            m_RoundEndMenuController.MedicPlayerName = m_MedicName;
            m_RoundEndMenuController.MedicStat = m_MedicRevives.ToString() + " Revives";
            Print("[KOTH_RoundEndMenu] Top Medic: " + m_MedicName + " with " + m_MedicRevives + " revives");
        }
        else
        {
            m_RoundEndMenuController.MedicPlayerName = "N/A";
            m_RoundEndMenuController.MedicStat = "";
            Print("[KOTH_RoundEndMenu] No Medic data");
        }
        m_RoundEndMenuController.NotifyPropertyChanged("MedicPlayerName");
        m_RoundEndMenuController.NotifyPropertyChanged("MedicStat");
        
        Print("[KOTH_RoundEndMenu] Top performers updated from server data");
    }
    
    void SetupVoting(array<string> zones, bool votingEnabled)
    {
        Print("[KOTH_RoundEndMenu] SetupVoting called with " + zones.Count() + " zones, voting: " + votingEnabled);
        
        m_AvailableZones = zones;
        
        if (!votingEnabled || zones.Count() == 0)
        {
            if (m_VotingPanel)
            {
                m_VotingPanel.Show(false);
                Print("[KOTH_RoundEndMenu] Voting panel hidden");
            }
            
            if (zones.Count() > 0)
            {
                m_RoundEndMenuController.VotingHeader = "NEXT ZONE: " + zones.Get(0);
                m_RoundEndMenuController.NotifyPropertyChanged("VotingHeader");
                Print("[KOTH_RoundEndMenu] Next zone header set: " + zones.Get(0));
            }
            
            return;
        }
        
        if (m_VotingPanel)
        {
            m_VotingPanel.Show(true);
            Print("[KOTH_RoundEndMenu] Voting panel shown");
        }
        
        m_RoundEndMenuController.VotingHeader = "VOTE FOR NEXT ZONE";
        m_RoundEndMenuController.NotifyPropertyChanged("VotingHeader");
        Print("[KOTH_RoundEndMenu] Voting header set");
        
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
            {
                zoneNameText.SetText(zoneName);
                Print("[KOTH_RoundEndMenu] Button " + index + " zone name set: " + zoneName);
            }
            else
            {
                Error("[KOTH_RoundEndMenu] Could not find ZoneNameText widget!");
            }
            
            if (voteCountText)
            {
                voteCountText.SetText("0 votes");
            }
            else
            {
                Error("[KOTH_RoundEndMenu] Could not find VoteCountText widget!");
            }
            
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