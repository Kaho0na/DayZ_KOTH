/**
 * KOTH_PlayerBase.c (ASSIST + HEADSHOT TRACKING)
 *
 * King of the Hill by Kahoona
 * Track headshot kills and assists (knockdowns)
 *
 * Place in: 4_World/Entities/KOTH_PlayerBase.c
 */

modded class PlayerBase
{
    private EntityAI m_KOTHArmband;
    private string m_KOTHTeam = "";
    private bool m_KOTHHeadshotKill = false;
    private PlayerBase m_KOTHLastAttacker;
    
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
        
        if (attackerTeam == victimTeam)
            return;
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (rewardManager)
        {
            rewardManager.ProcessAssist(m_KOTHLastAttacker, this);
        }
        
        m_KOTHLastAttacker = null;
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
        
        if (!killerPlayer)
            return;
        
        KOTH_PlayerRewardManager rewardManager;
        CF_Modules<KOTH_PlayerRewardManager>.Get(rewardManager);
        
        if (!rewardManager)
        {
            Print("[KOTH_PlayerBase] Reward manager not found!");
            return;
        }
        
        rewardManager.ProcessKill(killerPlayer, this);
        
        m_KOTHHeadshotKill = false;
        m_KOTHLastAttacker = null;
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