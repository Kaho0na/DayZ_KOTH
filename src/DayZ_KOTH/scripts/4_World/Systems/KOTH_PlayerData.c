class KOTH_PlayerData
{
    static ref KOTH_PlayerData s_Instance;
    protected ref map<int, ref KOTH_PlayerData> m_PlayerData = new map<int, ref KOTH_PlayerData>();

    string PlayerID;
    string PlayerName;
    int TotalMoneyinBank;
    int TotalExperienceEarned;
    int CurrentLevel;
    int TotalTimePlayed;
    int LongestKill;
    int LongestHeadshot;
    int HighestKillstreak;
    int TotalEnemiesKilled;
    string LastTeamSelection;

    void KOTH_PlayerData()
    {
        s_Instance = this;
        m_PlayerData = new map<int, ref KOTH_PlayerData>();
    }

    static KOTH_PlayerData GetInstance()
    {
        if (!s_Instance)
            s_Instance = new KOTH_PlayerData();
        return s_Instance;
    }

    void SetPlayerID(string id)
    {
        PlayerID = id;
    }

    string GetPlayerID()
    {
        return PlayerID;
    }

    void SetPlayerName(string name)
    {
        PlayerName = name;
    }

    string GetPlayerName()
    {
        return PlayerName;
    }

    void SetTotalMoneyinBank(int money)
    {
        TotalMoneyinBank = money;
    }

    int GetTotalMoneyinBank()
    {
        return TotalMoneyinBank;
    }

    void SetTotalExperienceEarned(int xp)
    {
        TotalExperienceEarned = xp;
    }

    int GetTotalExperienceEarned()
    {
        return TotalExperienceEarned;
    }

    void SetCurrentLevel(int level)
    {
        CurrentLevel = level;
    }

    int GetCurrentLevel()
    {
        return CurrentLevel;
    }

    void SetTotalTimePlayed(int time)
    {
        TotalTimePlayed = time;
    }

    int GetTotalTimePlayed()
    {
        return TotalTimePlayed;
    }

    void SetLongestKill(int distance)
    {
        LongestKill = distance;
    }

    int GetLongestKill()
    {
        return LongestKill;
    }

    void SetLongestHeadshot(int distance)
    {
        LongestHeadshot = distance;
    }

    int GetLongestHeadshot()
    {
        return LongestHeadshot;
    }

    void SetHighestKillstreak(int streak)
    {
        HighestKillstreak = streak;
    }

    int GetHighestKillstreak()
    {
        return HighestKillstreak;
    }

    void SetTotalEnemiesKilled(int kills)
    {
        TotalEnemiesKilled = kills;
    }

    int GetTotalEnemiesKilled()
    {
        return TotalEnemiesKilled;
    }

    void SetLastTeamSelection(string team)
    {
        LastTeamSelection = team;
    }

    string GetLastTeamSelection()
    {
        return LastTeamSelection;
    }

    static KOTH_PlayerData Load(string fileName)
    {
        fileName.Replace(" ", "");
        if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
            fileName += ".json";
        Print("[KOTH_PlayerData] Load existing configuration file:" + fileName);
        KOTH_PlayerData zoneConfig;
        ExpansionJsonFileParser<KOTH_PlayerData>.Load(EXPANSION_KOTH_Players + fileName, zoneConfig);
        Print("[KOTH_PlayerData] KOTH_PlayerData:" + zoneConfig);
        return zoneConfig;
    }

    void Save(string fileName)
    {
        fileName.Replace(" ", "");
        fileName.ToLower();
        if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
            fileName += ".json";

        ExpansionJsonFileParser<KOTH_PlayerData>.Save(EXPANSION_KOTH_Players + fileName, this);
    }

    void CopyConfig(KOTH_PlayerData playerDataBase)
    {
        PlayerID = playerDataBase.PlayerID;
        PlayerName = playerDataBase.PlayerName;
        TotalMoneyinBank = playerDataBase.TotalMoneyinBank;
        TotalExperienceEarned = playerDataBase.TotalExperienceEarned;
        CurrentLevel = playerDataBase.CurrentLevel;
        TotalTimePlayed = playerDataBase.TotalTimePlayed;
        LongestKill = playerDataBase.LongestKill;
        LongestHeadshot = playerDataBase.LongestHeadshot;
        HighestKillstreak = playerDataBase.HighestKillstreak;
        TotalEnemiesKilled = playerDataBase.TotalEnemiesKilled;
        LastTeamSelection = playerDataBase.LastTeamSelection;
    }

    void OnSend(ParamsWriteContext ctx)
    {
        ctx.Write(PlayerID);
        ctx.Write(PlayerName);
        ctx.Write(TotalMoneyinBank);
        ctx.Write(TotalExperienceEarned);
        ctx.Write(CurrentLevel);
        ctx.Write(TotalTimePlayed);
        ctx.Write(LongestKill);
        ctx.Write(LongestHeadshot);
        ctx.Write(HighestKillstreak);
        ctx.Write(TotalEnemiesKilled);
        ctx.Write(LastTeamSelection);
    }

    void OnReceive(ParamsReadContext ctx)
    {
        if(!ctx.Read(PlayerID)) 
            return;

        if(!ctx.Read(PlayerName))
            return;

        if(!ctx.Read(TotalMoneyinBank))
            return; 
        
        if(!ctx.Read(TotalExperienceEarned))
            return;
        
        if(!ctx.Read(CurrentLevel))
            return;
        
        if(!ctx.Read(TotalTimePlayed))
            return;

        if(!ctx.Read(LongestKill))
            return;

        if(!ctx.Read(LongestHeadshot))
            return;

        if(!ctx.Read(HighestKillstreak))
            return;

        if(!ctx.Read(TotalEnemiesKilled))
            return;

        if(!ctx.Read(LastTeamSelection))
            return;
    }


}