class KOTH_RoundEndZoneEntry: ExpansionScriptView
{
    protected int m_Index;
    protected string m_ZoneName;
    protected int m_VoteCount;
    protected ref KOTH_RoundEndZoneEntryController m_EntryController;
    protected bool m_IsHighlighted = false;
    protected bool m_HasVoted = false;
    
    protected ButtonWidget vote_button;
    protected Widget background;
    protected TextWidget ZoneNameText;
    protected TextWidget VoteCountText;
    
    void KOTH_RoundEndZoneEntry(int index, string zoneName)
    {
        m_Index = index;
        m_ZoneName = zoneName;
        m_VoteCount = 0;
        
        Class.CastTo(m_EntryController, GetController());
        SetEntry();
    }
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/KOTH_RoundEndMenuEntry.layout";
    }
    
    override typename GetControllerType()
    {
        return KOTH_RoundEndZoneEntryController;
    }
    
    private void SetEntry()
    {
        SetZoneName(m_ZoneName);
        SetVoteCount(0);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == vote_button && button == MouseState.LEFT)
        {
            OnVoteButtonClick();
            return true;
        }
        
        return super.OnClick(w, x, y, button);
    }
    
    void OnVoteButtonClick()
    {
        KOTH_RoundEndMenu roundEndMenu = KOTH_RoundEndMenu.Cast(GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu());
        if (!roundEndMenu)
            return;
        
        // Menu will handle locking all entries
        roundEndMenu.VoteForZone(m_Index, m_ZoneName);
        
        // Visually show THIS entry was selected
        SetVoted();
    }
    
    void Lock()
    {
        m_HasVoted = true;
        vote_button.Enable(false);
        
        // Grey out non-voted entries
        if (!m_IsHighlighted)
        {
            background.SetColor(ARGB(100, 50, 50, 50));
            ZoneNameText.SetColor(ARGB(255, 128, 128, 128));
        }
    }

    override bool OnMouseEnter(Widget w, int x, int y)
    {
        if (w == vote_button)
        {
            SetHighlight();
        }
        
        return super.OnMouseEnter(w, x, y);
    }
    
    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
    {
        if (w == vote_button)
        {
            if (!m_HasVoted)
                SetNormal();
        }
        
        return super.OnMouseLeave(w, enterW, x, y);
    }
    
    void SetHighlight()
    {
        m_IsHighlighted = true;
        background.SetColor(ARGB(255, 220, 220, 220));
        ZoneNameText.SetColor(ARGB(255, 0, 0, 0));
    }
    
    void SetNormal()
    {
        m_IsHighlighted = false;
        background.SetColor(ARGB(100, 0, 0, 0));
        ZoneNameText.SetColor(ARGB(255, 255, 255, 255));
    }
    
    void SetVoted()
    {
        m_HasVoted = true;
        background.SetColor(ARGB(255, 0, 100, 0));
        ZoneNameText.SetColor(ARGB(255, 255, 255, 255));
        vote_button.Enable(false);
    }
    
    void SetZoneName(string zoneName)
    {
        m_EntryController.ZoneName = zoneName;
        m_EntryController.NotifyPropertyChanged("ZoneName");
    }
    
    void SetVoteCount(int count)
    {
        m_VoteCount = count;
        m_EntryController.VoteCount = count.ToString() + " votes";
        m_EntryController.NotifyPropertyChanged("VoteCount");
    }
    
    void UpdateVoteCount(int count)
    {
        SetVoteCount(count);
    }
    
    int GetIndex()
    {
        return m_Index;
    }
    
    string GetZoneName()
    {
        return m_ZoneName;
    }
}

class KOTH_RoundEndZoneEntryController: ExpansionViewController
{
    string ZoneName;
    string VoteCount;
}