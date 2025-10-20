/**
 * KOTH_Area.c (PHASE 4 - EVENT-DRIVEN TRIGGERS)
 *
 * Triggers notify GameMode only when players enter/exit
 * Place in: 4_World/Classes/ContaminatedArea/KOTH_Area.c
 */

class KOTH_Area : EffectArea
{
    KOTH_AreaTrigger m_KOTH_Trigger;
    protected int m_UpdateRate = 1000;
    
    static KOTH_Area s_Instance;
    
    void KOTH_Area()
    {
        s_Instance = this;
    }
    
    void ~KOTH_Area()
    {
        if (s_Instance == this)
            s_Instance = null;
    }
    
    static KOTH_Area GetInstance()
    {
        return s_Instance;
    }
    
    static KOTH_AreaTrigger GetMainTrigger()
    {
        if (s_Instance)
            return s_Instance.m_KOTH_Trigger;
        return null;
    }
    
    void KOTH_Init(vector position, float radius)
    {
        m_Radius = radius;
        m_PositiveHeight = 200;
        m_NegativeHeight = 0;
        
        m_Position = position;
        m_Position[1] = 0;
        
        Print("[KOTH_Area] Initializing at " + m_Position + " with radius " + radius);
        
        CreateTrigger(m_Position, m_Radius);
        
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateZone, m_UpdateRate, true);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateCaptureRewards, m_UpdateRate, true);

        }
    }
    
    override void CreateTrigger(vector pos, int radius)
    {
        if (Class.CastTo(m_KOTH_Trigger, GetGame().CreateObjectEx("KOTH_AreaTrigger", pos, ECE_NONE)))
        {
            m_KOTH_Trigger.SetCollisionCylinder(radius, m_PositiveHeight);
            m_KOTH_Trigger.KOTH_Init(this);
            Print("[KOTH_Area] Trigger created successfully");
        }
        else
        {
            Error("[KOTH_Area] Failed to create trigger!");
        }
    }
    
    override void EEDelete(EntityAI parent)
    {
        if (m_KOTH_Trigger)
            GetGame().ObjectDelete(m_KOTH_Trigger);
        
        super.EEDelete(parent);
    }
    
    override void SetupZoneData(EffectAreaParams params) 
    {
    }
    
    override void OnPlayerEnterServer(PlayerBase player, EffectTrigger trigger)
    {
    }
    
    override void OnPlayerExitServer(PlayerBase player, EffectTrigger trigger)
    {
    }
    
    void UpdateZone()
    {
        if (m_KOTH_Trigger && m_KOTH_Trigger.HasPlayersInside())
        {
            Print("[KOTH_Area] Zone active - players inside: " + m_KOTH_Trigger.GetPlayerCount());
        }
    }

    void UpdateCaptureRewards()
    {
        if (m_KOTH_Trigger)
        {
            m_KOTH_Trigger.ProcessCaptureRewards();
        }
    }
}


class KOTH_AreaTrigger : CylinderTrigger
{
    protected EffectArea m_KOTH_EffectArea;
    protected ref array<PlayerBase> m_PlayersInside;
    protected ref array<PlayerBase> m_AIInside;
    
    void KOTH_AreaTrigger()
    {
        m_PlayersInside = new array<PlayerBase>();
        m_AIInside = new array<PlayerBase>();
    }
    
    void KOTH_Init(EffectArea area)
    {
        m_KOTH_EffectArea = area;
        Print("[KOTH_AreaTrigger] Initialized");
    }
    
    override protected bool CanAddObjectAsInsider(Object object)
    {
        if (PlayerBase.Cast(object))
            return true;
        
        return false;
    }
    
    override bool ShouldRemoveInsider(TriggerInsider insider)
    {
        return !insider.GetObject().IsAlive();
    }
    
    override void OnEnterServerEvent(TriggerInsider insider)
    {
        super.OnEnterServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player;
        
        if (Class.CastTo(player, insider.GetObject()))
        {
            if (player.GetIdentity())
            {
                int playerIdx = m_PlayersInside.Find(player);
                if (playerIdx == -1)
                {
                    m_PlayersInside.Insert(player);
                    string team = player.GetKOTHTeam();
                    Print("[KOTH_AreaTrigger] Player entered: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
                    
                    NotifyPlayerEntered(player);
                    
                    // PHASE 4: Notify GameMode of count change
                    NotifyGameModeCountChanged();
                }
            }
            else
            {
                int aiIdx = m_AIInside.Find(player);
                if (aiIdx == -1)
                {
                    m_AIInside.Insert(player);
                    
                    string aiFaction = GetExpansionAIFaction(player);
                    Print("[KOTH_AreaTrigger] AI entered: " + player.GetType() + " (Faction: " + aiFaction + ")");
                    
                    // PHASE 4: Notify GameMode of count change
                    NotifyGameModeCountChanged();
                }
            }
        }
    }
    
    override void OnLeaveServerEvent(TriggerInsider insider)
    {
        super.OnLeaveServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player;
        
        if (Class.CastTo(player, insider.GetObject()))
        {
            if (player.GetIdentity())
            {
                int playerIdx = m_PlayersInside.Find(player);
                if (playerIdx != -1)
                {
                    m_PlayersInside.Remove(playerIdx);
                    Print("[KOTH_AreaTrigger] Player left: " + player.GetIdentity().GetName());
                    
                    NotifyPlayerExited(player);
                    
                    // PHASE 4: Notify GameMode of count change
                    NotifyGameModeCountChanged();
                }
            }
            else
            {
                int aiIdx = m_AIInside.Find(player);
                if (aiIdx != -1)
                {
                    m_AIInside.Remove(aiIdx);
                    Print("[KOTH_AreaTrigger] AI left: " + player.GetType());
                    
                    // PHASE 4: Notify GameMode of count change
                    NotifyGameModeCountChanged();
                }
            }
        }
    }
    
    // PHASE 4: Event-driven count notification
    void NotifyGameModeCountChanged()
    {
        if (!GetGame().IsServer())
            return;
        
        int eastCount = GetTeamPlayerCount("East");
        int westCount = GetTeamPlayerCount("West");
        
        KOTH_GameMode gameMode = KOTH_GameMode.GetInstance();
        if (gameMode)
        {
            gameMode.OnAOZonePlayersChanged(eastCount, westCount);
        }
    }
    
    string GetExpansionAIFaction(PlayerBase ai)
    {
        if (!ai)
            return "Unknown";
        
        eAIBase eaiEntity = eAIBase.Cast(ai);
        if (eaiEntity)
        {
            eAIGroup group = eaiEntity.GetGroup();
            if (group)
            {
                eAIFaction faction = group.GetFaction();
                if (faction)
                {
                    return faction.GetName();
                }
            }
        }
        
        return "Unknown";
    }
    
    void NotifyPlayerEntered(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
            
        player.MessageStatus("[KOTH] You entered the capture zone!");
    }
    
    void NotifyPlayerExited(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
            
        player.MessageStatus("[KOTH] You left the capture zone");
    }
    
    bool HasPlayersInside()
    {
        return m_PlayersInside.Count() > 0 || m_AIInside.Count() > 0;
    }
    
    int GetPlayerCount()
    {
        return m_PlayersInside.Count() + m_AIInside.Count();
    }
    
    array<PlayerBase> GetPlayersInside()
    {
        return m_PlayersInside;
    }
    
    array<PlayerBase> GetAIInside()
    {
        return m_AIInside;
    }
    
    int GetTeamPlayerCount(string teamName)
    {
        int count = 0;
        int i;
        PlayerBase player;
        string factionName;
        
        // Count human players
        for (i = 0; i < m_PlayersInside.Count(); i++)
        {
            player = m_PlayersInside.Get(i);
            if (player && player.IsAlive() && player.GetKOTHTeam() == teamName)
            {
                count++;
            }
        }
        
        // Count AI players
        for (i = 0; i < m_AIInside.Count(); i++)
        {
            PlayerBase ai = m_AIInside.Get(i);
            if (!ai || !ai.IsAlive())
                continue;
            
            factionName = GetExpansionAIFaction(ai);
            
            if (factionName == teamName)
            {
                count++;
            }
        }
        
        return count;
    }
    
    int GetTeamAICount(string teamName)
    {
        int count = 0;
        int i;
        PlayerBase ai;
        string factionName;
        
        for (i = 0; i < m_AIInside.Count(); i++)
        {
            ai = m_AIInside.Get(i);
            if (!ai || !ai.IsAlive())
                continue;
            
            factionName = GetExpansionAIFaction(ai);
            
            if (factionName == teamName)
            {
                count++;
            }
        }
        return count;
    }

    void ProcessCaptureRewards()
    {
        if (!GetGame().IsServer())
            return;
        
        KOTH_GameMode gameMode = KOTH_GameMode.GetInstance();
        
        if (!gameMode || !gameMode.IsRoundActive())
            return;
        
        string capturingTeam = gameMode.GetCapturingTeam();
        
        if (capturingTeam == "" || capturingTeam == "Neutral")
            return;
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (!rewardManager)
            return;
        
        foreach (PlayerBase player : m_PlayersInside)
        {
            if (!player || !player.IsAlive() || !player.GetIdentity())
                continue;
            
            string playerTeam = player.GetKOTHTeam();
            if (playerTeam != capturingTeam)
                continue;
            
            rewardManager.ProcessCaptureReward(player, true);
        }
    }

}