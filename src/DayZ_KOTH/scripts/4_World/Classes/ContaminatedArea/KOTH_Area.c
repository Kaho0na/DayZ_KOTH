/**
 * KOTH_Area.c (PHASE 4 - EVENT-DRIVEN TRIGGERS - OPTIMIZED)
 *
 * Triggers notify GameMode only when players enter/exit
 * Uses built-in trigger insider tracking instead of custom arrays
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
    private bool m_IsActive = true;
    
    void SetActive(bool active)
    {
        m_IsActive = active;
        
        if (!active)
        {
            NotifyGameModeCountChanged();
            Print("[KOTH_AreaTrigger] Trigger deactivated and cleared");
        }
        else
        {
            Print("[KOTH_AreaTrigger] Trigger activated");
        }
    }

    bool IsActive()
    {
        return m_IsActive;
    }
    
    void KOTH_Init(EffectArea area)
    {
        m_KOTH_EffectArea = area;
        Print("[KOTH_AreaTrigger] Initialized");
    }
    
    override protected bool CanAddObjectAsInsider(Object object)
    {
        return PlayerBase.Cast(object) != null;
    }
    
    override bool ShouldRemoveInsider(TriggerInsider insider)
    {
        return !insider.GetObject().IsAlive();
    }
    
    override void OnEnterServerEvent(TriggerInsider insider)
    {
        if (!m_IsActive)
            return;
        
        super.OnEnterServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (!player)
            return;
        
        if (player.GetIdentity())
        {
            string team = player.GetKOTHTeam();
            Print("[KOTH_AreaTrigger] Player entered: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
            NotifyPlayerEntered(player);
        }
        else
        {
            string aiFaction = GetExpansionAIFaction(player);
            Print("[KOTH_AreaTrigger] AI entered: " + player.GetType() + " (Faction: " + aiFaction + ")");
        }
        
        NotifyGameModeCountChanged();
    }
    
    override void OnLeaveServerEvent(TriggerInsider insider)
    {
        if (!m_IsActive)
            return;
        
        super.OnLeaveServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (!player)
            return;
        
        if (player.GetIdentity())
        {
            Print("[KOTH_AreaTrigger] Player left: " + player.GetIdentity().GetName());
            NotifyPlayerExited(player);
        }
        else
        {
            Print("[KOTH_AreaTrigger] AI left: " + player.GetType());
        }
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(NotifyGameModeCountChanged, 50, false);
    }
    
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
        return GetInsiders().Count() > 0;
    }
    
    int GetPlayerCount()
    {
        return GetInsiders().Count();
    }
    
    array<PlayerBase> GetPlayersInside()
    {
        array<PlayerBase> players = new array<PlayerBase>;
        array<ref TriggerInsider> insiders = GetInsiders();
        
        for (int i = 0; i < insiders.Count(); i++)
        {
            TriggerInsider insider = insiders[i];
            if (!insider)
                continue;
            
            PlayerBase player = PlayerBase.Cast(insider.GetObject());
            if (player && player.IsAlive() && player.GetIdentity())
                players.Insert(player);
        }
        
        return players;
    }
    
    array<PlayerBase> GetAIInside()
    {
        array<PlayerBase> ais = new array<PlayerBase>;
        array<ref TriggerInsider> insiders = GetInsiders();
        
        for (int i = 0; i < insiders.Count(); i++)
        {
            TriggerInsider insider = insiders[i];
            if (!insider)
                continue;
            
            PlayerBase player = PlayerBase.Cast(insider.GetObject());
            if (player && player.IsAlive() && !player.GetIdentity())
                ais.Insert(player);
        }
        
        return ais;
    }
    
    int GetTeamPlayerCount(string teamName)
    {
        int count = 0;
        array<ref TriggerInsider> insiders = GetInsiders();
        
        for (int i = 0; i < insiders.Count(); i++)
        {
            TriggerInsider insider = insiders[i];
            if (!insider)
                continue;
            
            PlayerBase player = PlayerBase.Cast(insider.GetObject());
            if (!player || !player.IsAlive())
                continue;
            
            if (player.GetIdentity())
            {
                if (player.GetKOTHTeam() == teamName)
                    count++;
            }
            else
            {
                string factionName = GetExpansionAIFaction(player);
                if (factionName == teamName)
                    count++;
            }
        }
        
        return count;
    }
    
    int GetTeamAICount(string teamName)
    {
        int count = 0;
        array<ref TriggerInsider> insiders = GetInsiders();
        
        for (int i = 0; i < insiders.Count(); i++)
        {
            TriggerInsider insider = insiders[i];
            if (!insider)
                continue;
            
            PlayerBase ai = PlayerBase.Cast(insider.GetObject());
            if (!ai || !ai.IsAlive() || ai.GetIdentity())
                continue;
            
            string factionName = GetExpansionAIFaction(ai);
            if (factionName == teamName)
                count++;
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
        
        array<ref TriggerInsider> insiders = GetInsiders();
        for (int i = 0; i < insiders.Count(); i++)
        {
            TriggerInsider insider = insiders[i];
            if (!insider)
                continue;
            
            PlayerBase player = PlayerBase.Cast(insider.GetObject());
            if (!player || !player.IsAlive() || !player.GetIdentity())
                continue;
            
            string playerTeam = player.GetKOTHTeam();
            if (playerTeam != capturingTeam)
                continue;
            
            rewardManager.ProcessCaptureReward(player, true);
        }
    }
}