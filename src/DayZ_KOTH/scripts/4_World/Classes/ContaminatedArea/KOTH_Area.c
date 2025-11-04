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
        m_PositiveHeight = 300;
        m_NegativeHeight = -300;
        
        m_Position = position;
        m_Position[1] = GetGame().SurfaceY(position[0], position[2]);
        
        //Print("[KOTH_Area] Initializing at " + m_Position + " with radius " + radius);
        
        CreateTrigger(m_Position, m_Radius);
        
        if (GetGame().IsServer())
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateZone, m_UpdateRate, true);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateCaptureRewards, m_UpdateRate, true);

        }
    }
    
    override void CreateTrigger(vector pos, int radius)
    {
        pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
        // Lower the origin by the negative height so the trigger covers both up and down
        vector triggerPos = pos;
        triggerPos[1] = triggerPos[1] + (m_NegativeHeight / 2);  // center the cylinder vertically

        // Total height of the cylinder should be the full span (top + bottom)
        float totalHeight = m_PositiveHeight - m_NegativeHeight;
        
        if (Class.CastTo(m_KOTH_Trigger, GetGame().CreateObjectEx("KOTH_AreaTrigger", triggerPos, ECE_NONE)))
        {
            m_KOTH_Trigger.SetCollisionCylinder(radius, totalHeight);
            m_KOTH_Trigger.KOTH_Init(this);
            Print("[KOTH_Area] Trigger created successfully at " + triggerPos.ToString());
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
            //Print("[KOTH_Area] Zone active - players inside: " + m_KOTH_Trigger.GetPlayerCount());
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
    private bool m_IsActive = true;
    KOTH_AIManager aiManager;
    
    void KOTH_AreaTrigger()
    {
        m_PlayersInside = new array<PlayerBase>();
        m_AIInside = new array<PlayerBase>();
        aiManager = KOTH_AIManager.GetInstance();
    }

    void SetActive(bool active)
    {
        m_IsActive = active;
        
        if (!active)
        {
            m_PlayersInside.Clear();
            m_AIInside.Clear();
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
        if (!m_IsActive || !insider)
            return;

        PlayerBase player;
        if (!Class.CastTo(player, insider.GetObject()))
            return;

        // Prevent same object being processed twice (player vs AI)
        if (AlreadyRegistered(player))
            return;
        string team = "";
        // Distinguish player vs AI
        if (player.GetIdentity())
        {
            m_PlayersInside.Insert(player);
            team = player.GetKOTHTeam();
            Print("[KOTH_AreaTrigger] Player entered: " + player.GetIdentity().GetName() + " (Team: " + team + ")");
            NotifyPlayerEntered(player);
        }
        else
        {
            m_AIInside.Insert(player);
            team = GetExpansionAIFaction(player);
            if (aiManager)
            {
                aiManager.OnReinforcementArrived(team);
            }
            Print("[KOTH_AreaTrigger] AI entered: " + player.GetType() + " (Faction: " + team + ")");
        }

        // Delay spawn logic slightly to allow all entries in same frame to register first
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(NotifyandSpawnGameMode, 100, false, team);

        // Do NOT call super first (prevents double trigger)
        // super.OnEnterServerEvent(insider);  // <-- keep commented unless absolutely needed
    }
    
    override void OnLeaveServerEvent(TriggerInsider insider)
    {
        if (!m_IsActive)
            return;
        
        super.OnLeaveServerEvent(insider);
        
        if (!insider)
            return;
        
        PlayerBase player;
        string team = "";
        if (Class.CastTo(player, insider.GetObject()))
        {
            
            if (player.GetIdentity())
            {
                int playerIdx = m_PlayersInside.Find(player);
                if (playerIdx != -1)
                {
                    team = player.GetKOTHTeam();
                    if (aiManager)
                        aiManager.OnReinforcementArrived(team);
                    m_PlayersInside.Remove(playerIdx);
                    Print("[KOTH_AreaTrigger] Player left: " + player.GetIdentity().GetName());

                    NotifyPlayerExited(player);

                }
            }
            else
            {
                int aiIdx = m_AIInside.Find(player);
                if (aiIdx != -1)
                {
                    team = GetExpansionAIFaction(player);
                    if (aiManager)
                        aiManager.OnReinforcementArrived(team);
                    m_AIInside.Remove(aiIdx);
                    Print("[KOTH_AreaTrigger] AI left: " + player.GetType());
                }
            }

            if(team == "West")
                team = "East";
            else if(team == "East")
                team = "West";

            NotifyandSpawnGameMode(team);
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

    bool AlreadyRegistered(PlayerBase p)
    {
        return m_PlayersInside.Find(p) != -1 || m_AIInside.Find(p) != -1;
    }

    float m_LastSpawnTime;

    void NotifyandSpawnGameMode(string team)
    {
        if (!GetGame().IsServer())
            return;

        if (GetGame().GetTime() - m_LastSpawnTime < 250)
            return;

        m_LastSpawnTime = GetGame().GetTime();


        int eastCount = GetTeamPlayerCount("East");
        int westCount = GetTeamPlayerCount("West");

        int effectiveEast = eastCount + aiManager.GetReinforcementCount("East");
        int effectiveWest = westCount + aiManager.GetReinforcementCount("West");

        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();

        if (effectiveEast < settings.FactionMaxAI && effectiveWest < settings.FactionMaxAI)
        {
            aiManager.SpawnSingleAI("East");
            aiManager.SpawnSingleAI("West");
        }
        else if (effectiveEast > effectiveWest && effectiveWest < settings.FactionMaxAI)
        {
            aiManager.SpawnSingleAI("West");
        }
        else if (effectiveWest > effectiveEast && effectiveEast < settings.FactionMaxAI)
        {
            aiManager.SpawnSingleAI("East");
        }

        KOTH_GameMode gameMode = KOTH_GameMode.GetInstance();
        if (gameMode)
        {
            gameMode.OnAOZonePlayersChanged(eastCount, westCount);
        }

        Print("[KOTH_AreaTrigger] Faction counts after spawn -> East: " + GetTeamPlayerCount("East") + " | West: " + GetTeamPlayerCount("West"));
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
        PlayerBase player;
        string factionName;

        // --- Count human players ---
        foreach (PlayerBase p : m_PlayersInside)
        {
            if (p && p.IsAlive() && p.GetKOTHTeam() == teamName)
                count++;
        }

        // --- Count AI players ---
        foreach (PlayerBase ai : m_AIInside)
        {
            if (!ai || !ai.IsAlive())
                continue;

            factionName = GetExpansionAIFaction(ai);
            if (factionName == teamName)
                count++;
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