/**
 * KOTH_PriArea.c
 *
 * Priority zone following ExpansionAINoGoArea pattern
 * Place in: 4_World/Classes/ContaminatedArea/KOTH_PriArea.c
 */

class KOTH_PriArea : EffectArea
{
    KOTH_PriAreaTrigger m_KOTH_PriTrigger;
    protected int m_UpdateRate = 1000;
    
    void KOTH_Init(vector position, float radius)
    {
        m_Radius = radius;
        m_PositiveHeight = 50;
        m_NegativeHeight = 50;
        m_Position = position;
        
        Print("[KOTH_PriArea] Initializing priority zone at " + position + " with radius " + radius);
        
        CreateTrigger(m_Position, m_Radius);
        
        // Start update loop
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdatePriorityZone, m_UpdateRate, true);
        }
    }
    
    override void CreateTrigger(vector pos, int radius)
    {
        if (Class.CastTo(m_KOTH_PriTrigger, GetGame().CreateObjectEx("KOTH_PriAreaTrigger", pos, ECE_NONE)))
        {
            m_KOTH_PriTrigger.SetCollisionCylinder(radius, m_PositiveHeight);
            m_KOTH_PriTrigger.KOTH_Init(this);
            Print("[KOTH_PriArea] Priority trigger created successfully");
        }
        else
        {
            Error("[KOTH_PriArea] Failed to create priority trigger!");
        }
    }
    
    override void EEDelete(EntityAI parent)
    {
        if (m_KOTH_PriTrigger)
            GetGame().ObjectDelete(m_KOTH_PriTrigger);
        
        super.EEDelete(parent);
    }
    
    // Unused - we initialize manually
    override void SetupZoneData(EffectAreaParams params) 
    {
    }
    
    override void OnPlayerEnterServer(PlayerBase player, EffectTrigger trigger)
    {
        // Not used - handled in trigger
    }
    
    override void OnPlayerExitServer(PlayerBase player, EffectTrigger trigger)
    {
        // Not used - handled in trigger
    }
    
    void UpdatePriorityZone()
    {
        if (m_KOTH_PriTrigger && m_KOTH_PriTrigger.HasPlayersInside())
        {
            Print("[KOTH_PriArea] PRIORITY ZONE ACTIVE - BONUS POINTS! Players: " + m_KOTH_PriTrigger.GetPlayerCount());
        }
    }
}

class KOTH_PriAreaTrigger : CylinderTrigger
{
    protected EffectArea m_KOTH_EffectArea;
    protected ref array<PlayerBase> m_PlayersInside;
    
    void KOTH_PriAreaTrigger()
    {
        m_PlayersInside = new array<PlayerBase>();
    }
    
    void KOTH_Init(EffectArea area)
    {
        m_KOTH_EffectArea = area;
        Print("[KOTH_PriAreaTrigger] Priority trigger initialized");
    }
    
    override protected bool CanAddObjectAsInsider(Object object)
    {
        // Only track players
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
        
        if (insider)
        {
            PlayerBase player;
            if (Class.CastTo(player, insider.GetObject()))
            {
                if (m_PlayersInside.Find(player) == -1)
                {
                    m_PlayersInside.Insert(player);
                    string team = player.GetKOTHTeam();
                    Print("[KOTH_PriAreaTrigger] Player entered PRIORITY: " + player.GetIdentity().GetName() + " (Team: " + team + ") BONUS POINTS!");
                    
                    // Send notification to player
                    player.MessageStatus("[KOTH PRIORITY] You entered the BONUS POINTS zone!");
                }
            }
        }
    }
    
    override void OnLeaveServerEvent(TriggerInsider insider)
    {
        super.OnLeaveServerEvent(insider);
        
        if (insider)
        {
            PlayerBase player;
            if (Class.CastTo(player, insider.GetObject()))
            {
                int idx = m_PlayersInside.Find(player);
                if (idx != -1)
                {
                    m_PlayersInside.Remove(idx);
                    Print("[KOTH_PriAreaTrigger] Player left PRIORITY: " + player.GetIdentity().GetName());
                    
                    player.MessageStatus("[KOTH PRIORITY] You left the bonus zone");
                }
            }
        }
    }
    
    bool HasPlayersInside()
    {
        return m_PlayersInside.Count() > 0;
    }
    
    int GetPlayerCount()
    {
        return m_PlayersInside.Count();
    }
    
    array<PlayerBase> GetPlayersInside()
    {
        return m_PlayersInside;
    }
}