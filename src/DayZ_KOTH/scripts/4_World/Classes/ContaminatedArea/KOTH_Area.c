/**
 * KOTH_Area.c
 *
 * Following ExpansionAINoGoArea pattern exactly
 * Place in: 4_World/Classes/ContaminatedArea/KOTH_Area.c
 */

class KOTH_Area : EffectArea
{
    KOTH_AreaTrigger m_KOTH_Trigger;
    protected int m_UpdateRate = 1000;
    
    void KOTH_Init(vector position, float radius)
    {
        m_Radius = radius;
        m_PositiveHeight = 50;
        m_NegativeHeight = 50;
        m_Position = position;
        
        Print("[KOTH_Area] Initializing at " + position + " with radius " + radius);
        
        CreateTrigger(m_Position, m_Radius);
        
        // Start update loop
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateZone, m_UpdateRate, true);
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
    
    void UpdateZone()
    {
        if (m_KOTH_Trigger && m_KOTH_Trigger.HasPlayersInside())
        {
            Print("[KOTH_Area] Zone active - players inside: " + m_KOTH_Trigger.GetPlayerCount());
        }
    }
}


class KOTH_AreaTrigger : CylinderTrigger
{
    protected EffectArea m_KOTH_EffectArea;
    protected ref array<PlayerBase> m_PlayersInside;
    
    void KOTH_AreaTrigger()
    {
        m_PlayersInside = new array<PlayerBase>();
    }
    
    void KOTH_Init(EffectArea area)
    {
        m_KOTH_EffectArea = area;
        Print("[KOTH_AreaTrigger] Initialized with area");
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
                    Print("[KOTH_AreaTrigger] Player entered: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
                    
                    // Send notification to player
                    NotifyPlayerEntered(player);
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
                    Print("[KOTH_AreaTrigger] Player left: " + player.GetIdentity().GetName());
                    
                    // Send notification to player
                    NotifyPlayerExited(player);
                }
            }
        }
    }
    
    void NotifyPlayerEntered(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;
            
        // Send notification directly to player
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