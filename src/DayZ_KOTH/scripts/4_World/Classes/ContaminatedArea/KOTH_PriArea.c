// ═══════════════════════════════════════════════════════════════
// KOTH_PriArea.c (HEIGHT FIX)
// Place in: 4_World/Classes/ContaminatedArea/KOTH_PriArea.c
// ═══════════════════════════════════════════════════════════════

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
        
        Print("[KOTH_PriArea] Initializing at " + m_Position + " with radius " + radius);
        Print("[KOTH_PriArea] Cylinder: Sea level (Y=0) to 200m height");
        
        CreateTrigger(m_Position, m_Radius);
    }
    
    override void CreateTrigger(vector pos, int radius)
    {
        if (Class.CastTo(m_KOTH_PriTrigger, GetGame().CreateObjectEx("KOTH_PriAreaTrigger", pos, ECE_NONE)))
        {
            m_KOTH_PriTrigger.SetCollisionCylinder(radius, m_PositiveHeight);
            m_KOTH_PriTrigger.KOTH_Init(this);
            Print("[KOTH_PriArea] Trigger created successfully");
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
    protected ref map<PlayerBase, bool> m_PlayerStates;
    protected ref map<PlayerBase, float> m_PlayerEnterTime;
    
    void KOTH_PriAreaTrigger()
    {
        m_PlayerStates = new map<PlayerBase, bool>();
        m_PlayerEnterTime = new map<PlayerBase, float>();
    }
    
    void KOTH_Init(EffectArea area)
    {
        m_KOTH_EffectArea = area;
        Print("[KOTH_PriAreaTrigger] Initialized");
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
        
        if (insider)
        {
            PlayerBase player;
            if (Class.CastTo(player, insider.GetObject()))
            {
                float currentTime = GetGame().GetTime();
                
                bool wasInside = false;
                if (m_PlayerStates.Contains(player))
                {
                    wasInside = m_PlayerStates.Get(player);
                }
                
                if (!wasInside)
                {
                    float lastEnterTime = 0;
                    if (m_PlayerEnterTime.Contains(player))
                    {
                        lastEnterTime = m_PlayerEnterTime.Get(player);
                    }
                    
                    if (currentTime - lastEnterTime > 3000)
                    {
                        m_PlayerStates.Set(player, true);
                        m_PlayerEnterTime.Set(player, currentTime);
                        
                        string team = player.GetKOTHTeam();
                        Print("[KOTH_PriAreaTrigger] Player entered PRIORITY: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
                        
                        player.MessageStatus("[KOTH PRIORITY] You entered the BONUS POINTS zone!");
                    }
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
                float currentTime = GetGame().GetTime();
                
                bool wasInside = false;
                if (m_PlayerStates.Contains(player))
                {
                    wasInside = m_PlayerStates.Get(player);
                }
                
                if (wasInside)
                {
                    float lastEnterTime = 0;
                    if (m_PlayerEnterTime.Contains(player))
                    {
                        lastEnterTime = m_PlayerEnterTime.Get(player);
                    }
                    
                    if (currentTime - lastEnterTime > 3000)
                    {
                        m_PlayerStates.Set(player, false);
                        
                        Print("[KOTH_PriAreaTrigger] Player left PRIORITY: " + player.GetIdentity().GetName());
                        
                        player.MessageStatus("[KOTH PRIORITY] You left the bonus zone");
                    }
                }
            }
        }
    }
    
    bool HasPlayersInside()
    {
        int count = 0;
        foreach (PlayerBase player, bool isInside : m_PlayerStates)
        {
            if (isInside && player && player.IsAlive())
                count++;
        }
        return count > 0;
    }
    
    int GetPlayerCount()
    {
        int count = 0;
        foreach (PlayerBase player, bool isInside : m_PlayerStates)
        {
            if (isInside && player && player.IsAlive())
                count++;
        }
        return count;
    }
    
    array<PlayerBase> GetPlayersInside()
    {
        array<PlayerBase> result = new array<PlayerBase>();
        foreach (PlayerBase player, bool isInside : m_PlayerStates)
        {
            if (isInside && player && player.IsAlive())
                result.Insert(player);
        }
        return result;
    }
    
    int GetTeamPlayerCount(string teamName)
    {
        int count = 0;
        foreach (PlayerBase player, bool isInside : m_PlayerStates)
        {
            if (isInside && player && player.IsAlive() && player.GetKOTHTeam() == teamName)
                count++;
        }
        return count;
    }
}