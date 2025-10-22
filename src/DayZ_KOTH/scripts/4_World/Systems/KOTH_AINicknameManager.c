class KOTH_AINicknameManager
{
    private static ref KOTH_AINicknameManager s_Instance;
    private ref map<string, string> m_AIToNickname;
    private ref array<string> m_AvailableNicknames;
    private ref array<string> m_UsedNicknames;
    
    private static ref array<string> NICKNAME_POOL = {
        "ShadowStrike",
        "IronWolf",
        "TheReaper",
        "NightHawk",
        "SilentKiller",
        "DeadEye",
        "GhostRider",
        "WarMachine",
        "BloodHound",
        "DarkPhoenix",
        "ViperStrike",
        "ThunderBolt",
        "IceBreaker",
        "FireStorm",
        "SteelFang",
        "CrimsonBlade",
        "WildCard",
        "Maverick",
        "RogueOne",
        "ApexPredator",
        "LoneWolf",
        "SkullCrusher",
        "BattleBorn",
        "StormBringer",
        "PhantomFury",
        "WarLord",
        "DeathDealer",
        "BlackOps",
        "HellRaiser",
        "TitanSlayer"
    };
    
    void KOTH_AINicknameManager()
    {
        m_AIToNickname = new map<string, string>();
        m_AvailableNicknames = new array<string>();
        m_UsedNicknames = new array<string>();
        
        foreach (string nick : NICKNAME_POOL)
        {
            m_AvailableNicknames.Insert(nick);
        }
        
        Print("[KOTH_AINicknameManager] Initialized with " + m_AvailableNicknames.Count() + " available nicknames");
    }
    
    static KOTH_AINicknameManager GetInstance()
    {
        if (!s_Instance)
        {
            s_Instance = new KOTH_AINicknameManager();
        }
        return s_Instance;
    }
    
    string GetOrAssignNickname(Object ai)
    {
        if (!ai)
            return "UnknownAI";
        
        string aiID = ai.GetID().ToString();
        
        if (m_AIToNickname.Contains(aiID))
        {
            return m_AIToNickname.Get(aiID);
        }
        
        string nickname;
        
        if (m_AvailableNicknames.Count() > 0)
        {
            int randomIndex = Math.RandomInt(0, m_AvailableNicknames.Count());
            nickname = m_AvailableNicknames.Get(randomIndex);
            m_AvailableNicknames.Remove(randomIndex);
            m_UsedNicknames.Insert(nickname);
        }
        else
        {
            nickname = "AI_" + Math.RandomInt(1000, 9999).ToString();
        }
        
        m_AIToNickname.Set(aiID, nickname);
        
        Print("[KOTH_AINicknameManager] Assigned nickname '" + nickname + "' to AI " + aiID);
        
        return nickname;
    }
    
    string GetUniqueUID(Object ai)
    {
        if (!ai)
            return "UnknownAI_0";
        
        string nickname = GetOrAssignNickname(ai);
        return "AI_" + nickname;
    }
    
    void RemoveAI(Object ai)
    {
        if (!ai)
            return;
        
        string aiID = ai.GetID().ToString();
        
        if (m_AIToNickname.Contains(aiID))
        {
            string nickname = m_AIToNickname.Get(aiID);
            
            m_UsedNicknames.RemoveItem(nickname);
            m_AvailableNicknames.Insert(nickname);
            
            m_AIToNickname.Remove(aiID);
            
            Print("[KOTH_AINicknameManager] Released nickname '" + nickname + "' from AI " + aiID);
        }
    }
    
    void ResetAll()
    {
        m_AIToNickname.Clear();
        m_UsedNicknames.Clear();
        m_AvailableNicknames.Clear();
        
        foreach (string nick : NICKNAME_POOL)
        {
            m_AvailableNicknames.Insert(nick);
        }
        
        Print("[KOTH_AINicknameManager] Reset all nicknames");
    }
}