/**
 * KOTH_PlayerBase.c (ASSIST + REVIVE + HEADSHOT TRACKING)
 *
 * King of the Hill by Kahoona
 * Track headshot kills, assists (knockdowns), and revives with anti-farm
 *
 * Place in: 4_World/Entities/KOTH_PlayerBase.c
 */

modded class PlayerBase
{
    private EntityAI m_KOTHArmband;
    private string m_KOTHTeam = "";
    private bool m_KOTHHeadshotKill = false;
    private PlayerBase m_KOTHLastAttacker;
    private PlayerBase m_KOTHReviver;
    private string m_KOTHLastRevivedPlayerUID = "";
    
    void SetKOTHArmband(EntityAI armband)
    {
        m_KOTHArmband = armband;
    }
    
    EntityAI GetKOTHArmband()
    {
        return m_KOTHArmband;
    }
    
    void SetKOTHTeam(string team)
    {
        m_KOTHTeam = team;
    }
    
    string GetKOTHTeam()
    {
        return m_KOTHTeam;
    }
    
    bool WasHeadshotKill()
    {
        return m_KOTHHeadshotKill;
    }
    
    PlayerBase GetLastAttacker()
    {
        return m_KOTHLastAttacker;
    }
    
    void SetReviver(PlayerBase reviver)
    {
        m_KOTHReviver = reviver;
    }
    
    PlayerBase GetReviver()
    {
        return m_KOTHReviver;
    }
    
    void SetLastRevivedPlayerUID(string uid)
    {
        m_KOTHLastRevivedPlayerUID = uid;
    }
    
    string GetLastRevivedPlayerUID()
    {
        return m_KOTHLastRevivedPlayerUID;
    }
    
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        
        if (!GetGame().IsServer())
            return;
        
        PlayerBase attacker = PlayerBase.Cast(source);
        
        if (!attacker)
        {
            EntityAI sourceEntity = EntityAI.Cast(source);
            if (sourceEntity)
            {
                attacker = PlayerBase.Cast(sourceEntity.GetHierarchyRootPlayer());
            }
        }
        
        if (attacker && attacker != this)
        {
            m_KOTHLastAttacker = attacker;
        }
        
        if (dmgZone == "Head" || dmgZone == "Brain")
        {
            float headHealth = GetHealth("", "Head");
            float brainHealth = GetHealth("", "Brain");
            
            if (headHealth <= 0 || brainHealth <= 0)
            {
                m_KOTHHeadshotKill = true;
                Print("[KOTH_PlayerBase] Headshot detected on " + GetType() + " - Hit zone: " + dmgZone);
            }
        }
    }
    
    override void OnUnconsciousStart()
    {
        super.OnUnconsciousStart();
        
        if (!GetGame().IsServer())
            return;
        
        if (!m_KOTHLastAttacker)
            return;
        
        string victimTeam;
        if (GetIdentity())
        {
            victimTeam = GetKOTHTeam();
        }
        else
        {
            KOTH_PlayerRewardManager rewardMgr;
            CF_Modules<KOTH_PlayerRewardManager>.Get(rewardMgr);
            if (rewardMgr)
            {
                victimTeam = rewardMgr.GetExpansionAIFaction(this);
            }
        }
        
        string attackerTeam = m_KOTHLastAttacker.GetKOTHTeam();
        
        if (attackerTeam == "" || victimTeam == "" || victimTeam == "Unknown")
            return;
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (!rewardManager)
            return;
        
        if (attackerTeam == victimTeam)
        {
            rewardManager.ProcessTeamKnockdown(m_KOTHLastAttacker, this);
        }
        else
        {
            rewardManager.ProcessAssist(m_KOTHLastAttacker, this);
        }
        
        m_KOTHLastAttacker = null;
    }
    
    override void OnUnconsciousStop(int pCurrentCommandID)
    {
        super.OnUnconsciousStop(pCurrentCommandID);
        
        if (!GetGame().IsServer())
            return;
        
        if (!m_KOTHReviver)
            return;
        
        string revivedTeam;
        if (GetIdentity())
        {
            revivedTeam = GetKOTHTeam();
        }
        else
        {
            KOTH_PlayerRewardManager rewardMgr;
            CF_Modules<KOTH_PlayerRewardManager>.Get(rewardMgr);
            if (rewardMgr)
            {
                revivedTeam = rewardMgr.GetExpansionAIFaction(this);
            }
        }
        
        string reviverTeam = m_KOTHReviver.GetKOTHTeam();
        
        if (revivedTeam == "" || reviverTeam == "" || revivedTeam == "Unknown")
            return;
        
        if (revivedTeam != reviverTeam)
            return;
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (rewardManager)
        {
            rewardManager.ProcessRevive(m_KOTHReviver, this);
        }
        
        m_KOTHReviver = null;
    }
    
    override void EEInit()
    {
        super.EEInit();
        
        if (!GetGame().IsServer())
            return;
        
        // Register player/AI in stats tracker when they spawn
        KOTH_RoundStatsTracker statsTracker = KOTH_RoundStatsTracker.GetInstance();
        if (statsTracker && statsTracker.IsRoundActive())
        {
            string uid;
            string name;
            string team = GetKOTHTeam();
            
            if (GetIdentity())
            {
                uid = GetIdentity().GetId();
                name = GetIdentity().GetName();
            }
            else
            {
                // AI without identity
                KOTH_AINicknameManager nickManager = KOTH_AINicknameManager.GetInstance();
                uid = nickManager.GetUniqueUID(this);
                name = nickManager.GetOrAssignNickname(this);
            }
            
            statsTracker.RegisterPlayer(uid, name, team);
        }
    }

    override void EEKilled(Object killer)
    {
        super.EEKilled(killer);
        
        if (!GetGame().IsServer())
            return;
        
        PlayerBase killerPlayer = PlayerBase.Cast(killer);
        
        if (!killerPlayer)
        {
            EntityAI killerEntity = EntityAI.Cast(killer);
            if (killerEntity)
            {
                killerPlayer = PlayerBase.Cast(killerEntity.GetHierarchyRootPlayer());
            }
        }
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (!rewardManager)
        {
            Print("[KOTH_PlayerBase] Reward manager not found!");
            return;
        }
        
        if (killerPlayer == this)
        {
            rewardManager.ProcessSuicide(this);
        }
        else if (killerPlayer)
        {
            rewardManager.ProcessKill(killerPlayer, this);
        }
        
        m_KOTHHeadshotKill = false;
        m_KOTHLastAttacker = null;
        m_KOTHReviver = null;
    }
    
    override bool CanDropEntity(notnull EntityAI item)
    {
        if (item && (item.IsKindOf("Armband_Red") || item.IsKindOf("Armband_Blue")))
        {
            if (m_KOTHTeam == "East" || m_KOTHTeam == "West")
            {
                return false;
            }
        }
        
        return super.CanDropEntity(item);
    }
}