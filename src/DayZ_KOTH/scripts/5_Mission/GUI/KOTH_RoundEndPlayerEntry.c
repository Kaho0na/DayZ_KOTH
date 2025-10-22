class KOTH_RoundEndPlayerEntry: ExpansionScriptView
{
    protected int m_Index;
    protected ref KOTH_RoundPlayerStats m_Stats;
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/KOTH_RoundEndPlayerEntry.layout";
    }
    
    void KOTH_RoundEndPlayerEntry(int index, KOTH_RoundPlayerStats stats)
    {
        m_Index = index;
        m_Stats = stats;
    }
    
    override typename GetControllerType()
    {
        return KOTH_RoundEndPlayerEntryController;
    }
    
    int GetIndex()
    {
        return m_Index;
    }
    
    KOTH_RoundPlayerStats GetStats()
    {
        return m_Stats;
    }
}

class KOTH_RoundEndPlayerEntryController: ExpansionViewController
{
    string PlayerName;
    string PlayerLevel;
    string PlayerKills;
    string PlayerDeaths;
    string PlayerKD;
    string PlayerHeadshots;
    string PlayerRevives;
    string PlayerXP;
    string PlayerMoney;
}