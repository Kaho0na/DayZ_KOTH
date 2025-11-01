/**
 * KOTH_PriArea.c (PHASE 4 - EVENT-DRIVEN TRIGGERS - OPTIMIZED)
 *
 * Priority zone triggers notify GameMode only on enter/exit
 * Uses built-in trigger insider tracking instead of custom arrays
 * Place in: 4_World/Classes/ContaminatedArea/KOTH_PriArea.c
 */

class KOTH_PriArea : EffectArea
{
    KOTH_PriAreaTrigger m_KOTH_PriTrigger;
    
    void KOTH_Init(vector position, float radius)
    {
        m_Radius = radius;
        m_PositiveHeight = 200;
        m_NegativeHeight = 0;
        
        m_Position = position;
        m_Position[1] = 0;
        
        CreateTrigger(m_Position, m_Radius);
    }
    
    override void CreateTrigger(vector pos, int radius)
    {
        if (Class.CastTo(m_KOTH_PriTrigger, GetGame().CreateObjectEx("KOTH_PriAreaTrigger", pos, ECE_NONE)))
        {
            m_KOTH_PriTrigger.SetCollisionCylinder(radius, m_PositiveHeight);
            m_KOTH_PriTrigger.KOTH_Init(this);
        }
        else
        {
            Error("[KOTH_PriArea] Failed to create trigger!");
        }
    }
    
    override void EEDelete(EntityAI parent)
    {
        if (m_KOTH_PriTrigger)
            GetGame().ObjectDelete(m_KOTH_PriTrigger);
        
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
    
    KOTH_PriAreaTrigger GetTrigger()
    {
        return m_KOTH_PriTrigger;
    }
}

class KOTH_PriAreaTrigger : CylinderTrigger
{
    protected EffectArea m_KOTH_EffectArea;
    
    void KOTH_Init(EffectArea area)
    {
        m_KOTH_EffectArea = area;
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
        super.OnEnterServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (!player)
            return;
        
        if (player.GetIdentity())
        {
            string team = player.GetKOTHTeam();
            player.MessageStatus("[KOTH PRIORITY] You entered the BONUS POINTS zone!");
        }
        
        NotifyGameModeCountChanged();
    }
    
    override void OnLeaveServerEvent(TriggerInsider insider)
    {
        super.OnLeaveServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player = PlayerBase.Cast(insider.GetObject());
        if (!player)
            return;
        
        if (player.GetIdentity())
        {
            player.MessageStatus("[KOTH PRIORITY] You left the bonus zone");
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
            gameMode.OnPriorityZonePlayersChanged(eastCount, westCount);
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
}