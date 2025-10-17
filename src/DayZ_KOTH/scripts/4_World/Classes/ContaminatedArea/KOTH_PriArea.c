/**
 * KOTH_PriArea.c (WITH EXPANSION AI COUNTING - FIXED)
 *
 * Priority zone with Expansion AI counting support
 * Expansion AI are PlayerBase entities without identity
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
    protected ref map<PlayerBase, bool> m_AIStates;
    protected ref map<PlayerBase, float> m_AIEnterTime;
    
    void KOTH_PriAreaTrigger()
    {
        m_PlayerStates = new map<PlayerBase, bool>();
        m_PlayerEnterTime = new map<PlayerBase, float>();
        m_AIStates = new map<PlayerBase, bool>();
        m_AIEnterTime = new map<PlayerBase, float>();
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
        
        if (!insider)
            return;
        
        float currentTime = GetGame().GetTime();
        PlayerBase player;
        bool entityWasInside;
        float lastEnterTime;
        string team;
        string aiFaction;
        
        if (Class.CastTo(player, insider.GetObject()))
        {
            if (player.GetIdentity())
            {
                entityWasInside = false;
                if (m_PlayerStates.Contains(player))
                {
                    entityWasInside = m_PlayerStates.Get(player);
                }
                
                if (!entityWasInside)
                {
                    lastEnterTime = 0;
                    if (m_PlayerEnterTime.Contains(player))
                    {
                        lastEnterTime = m_PlayerEnterTime.Get(player);
                    }
                    
                    if (currentTime - lastEnterTime > 3000)
                    {
                        m_PlayerStates.Set(player, true);
                        m_PlayerEnterTime.Set(player, currentTime);
                        
                        team = player.GetKOTHTeam();
                        Print("[KOTH_PriAreaTrigger] Player entered PRIORITY: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
                        
                        player.MessageStatus("[KOTH PRIORITY] You entered the BONUS POINTS zone!");
                    }
                }
            }
            else
            {
                entityWasInside = false;
                if (m_AIStates.Contains(player))
                {
                    entityWasInside = m_AIStates.Get(player);
                }
                
                if (!entityWasInside)
                {
                    lastEnterTime = 0;
                    if (m_AIEnterTime.Contains(player))
                    {
                        lastEnterTime = m_AIEnterTime.Get(player);
                    }
                    
                    if (currentTime - lastEnterTime > 3000)
                    {
                        m_AIStates.Set(player, true);
                        m_AIEnterTime.Set(player, currentTime);
                        
                        aiFaction = GetExpansionAIFaction(player);
                        Print("[KOTH_PriAreaTrigger] AI entered PRIORITY: " + player.GetType() + " (Faction: " + aiFaction + ")");
                    }
                }
            }
        }
    }
    
    override void OnLeaveServerEvent(TriggerInsider insider)
    {
        super.OnLeaveServerEvent(insider);
        
        if (!insider)
            return;
        
        float currentTime = GetGame().GetTime();
        PlayerBase player;
        bool entityWasInside;
        float lastEnterTime;
        
        if (Class.CastTo(player, insider.GetObject()))
        {
            if (player.GetIdentity())
            {
                entityWasInside = false;
                if (m_PlayerStates.Contains(player))
                {
                    entityWasInside = m_PlayerStates.Get(player);
                }
                
                if (entityWasInside)
                {
                    lastEnterTime = 0;
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
            else
            {
                entityWasInside = false;
                if (m_AIStates.Contains(player))
                {
                    entityWasInside = m_AIStates.Get(player);
                }
                
                if (entityWasInside)
                {
                    lastEnterTime = 0;
                    if (m_AIEnterTime.Contains(player))
                    {
                        lastEnterTime = m_AIEnterTime.Get(player);
                    }
                    
                    if (currentTime - lastEnterTime > 3000)
                    {
                        m_AIStates.Set(player, false);
                        
                        Print("[KOTH_PriAreaTrigger] AI left PRIORITY: " + player.GetType());
                    }
                }
            }
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
        int count = 0;
        int i;
        array<PlayerBase> playerKeys = m_PlayerStates.GetKeyArray();
        array<PlayerBase> aiKeys = m_AIStates.GetKeyArray();
        
        for (i = 0; i < playerKeys.Count(); i++)
        {
            PlayerBase player = playerKeys.Get(i);
            if (player && player.IsAlive() && m_PlayerStates.Get(player))
                count++;
        }
        
        for (i = 0; i < aiKeys.Count(); i++)
        {
            PlayerBase ai = aiKeys.Get(i);
            if (ai && ai.IsAlive() && m_AIStates.Get(ai))
                count++;
        }
        
        return count > 0;
    }
    
    int GetPlayerCount()
    {
        int count = 0;
        int i;
        array<PlayerBase> playerKeys = m_PlayerStates.GetKeyArray();
        array<PlayerBase> aiKeys = m_AIStates.GetKeyArray();
        
        for (i = 0; i < playerKeys.Count(); i++)
        {
            PlayerBase player = playerKeys.Get(i);
            if (player && player.IsAlive() && m_PlayerStates.Get(player))
                count++;
        }
        
        for (i = 0; i < aiKeys.Count(); i++)
        {
            PlayerBase ai = aiKeys.Get(i);
            if (ai && ai.IsAlive() && m_AIStates.Get(ai))
                count++;
        }
        
        return count;
    }
    
    array<PlayerBase> GetPlayersInside()
    {
        array<PlayerBase> result = new array<PlayerBase>();
        array<PlayerBase> playerKeys = m_PlayerStates.GetKeyArray();
        int i;
        
        for (i = 0; i < playerKeys.Count(); i++)
        {
            PlayerBase player = playerKeys.Get(i);
            if (player && player.IsAlive() && m_PlayerStates.Get(player))
                result.Insert(player);
        }
        
        return result;
    }
    
    array<PlayerBase> GetAIInside()
    {
        array<PlayerBase> result = new array<PlayerBase>();
        array<PlayerBase> aiKeys = m_AIStates.GetKeyArray();
        int i;
        
        for (i = 0; i < aiKeys.Count(); i++)
        {
            PlayerBase ai = aiKeys.Get(i);
            if (ai && ai.IsAlive() && m_AIStates.Get(ai))
                result.Insert(ai);
        }
        
        return result;
    }
    
    int GetTeamPlayerCount(string teamName)
    {
        int count = 0;
        int i;
        array<PlayerBase> playerKeys = m_PlayerStates.GetKeyArray();
        array<PlayerBase> aiKeys = m_AIStates.GetKeyArray();
        string factionName;
        
        for (i = 0; i < playerKeys.Count(); i++)
        {
            PlayerBase player = playerKeys.Get(i);
            if (player && player.IsAlive() && m_PlayerStates.Get(player))
            {
                if (player.GetKOTHTeam() == teamName)
                    count++;
            }
        }
        
        for (i = 0; i < aiKeys.Count(); i++)
        {
            PlayerBase ai = aiKeys.Get(i);
            if (!ai || !ai.IsAlive() || !m_AIStates.Get(ai))
                continue;
            
            factionName = GetExpansionAIFaction(ai);
            
            if (factionName == teamName)
                count++;
        }
        
        return count;
    }
}