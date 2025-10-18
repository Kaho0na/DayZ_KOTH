/**
 * KOTH_PriArea.c (FIXED COUNT REPORTING)
 *
 * Priority zone with proper player count tracking
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
    protected ref array<PlayerBase> m_PlayersInside;
    protected ref array<PlayerBase> m_AIInside;
    
    void KOTH_PriAreaTrigger()
    {
        m_PlayersInside = new array<PlayerBase>();
        m_AIInside = new array<PlayerBase>();
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
                    Print("[KOTH_PriAreaTrigger] Player entered PRIORITY: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
                    
                    player.MessageStatus("[KOTH PRIORITY] You entered the BONUS POINTS zone!");
                }
            }
            else
            {
                int aiIdx = m_AIInside.Find(player);
                if (aiIdx == -1)
                {
                    m_AIInside.Insert(player);
                    
                    string aiFaction = GetExpansionAIFaction(player);
                    Print("[KOTH_PriAreaTrigger] AI entered PRIORITY: " + player.GetType() + " (Faction: " + aiFaction + ")");
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
                    Print("[KOTH_PriAreaTrigger] Player left PRIORITY: " + player.GetIdentity().GetName());
                    
                    player.MessageStatus("[KOTH PRIORITY] You left the bonus zone");
                }
            }
            else
            {
                int aiIdx = m_AIInside.Find(player);
                if (aiIdx != -1)
                {
                    m_AIInside.Remove(aiIdx);
                    Print("[KOTH_PriAreaTrigger] AI left PRIORITY: " + player.GetType());
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
        
        for (i = 0; i < m_PlayersInside.Count(); i++)
        {
            player = m_PlayersInside.Get(i);
            if (player && player.IsAlive() && player.GetKOTHTeam() == teamName)
            {
                count++;
            }
        }
        
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
        
        Print("[KOTH_PriAreaTrigger] GetTeamPlayerCount(" + teamName + ") = " + count + " (Players: " + m_PlayersInside.Count() + ", AI: " + m_AIInside.Count() + ")");
        
        return count;
    }
}