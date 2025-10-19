/**
 * KOTH_PlayerBase.c (HEADSHOT TRACKING)
 *
 * King of the Hill by Kahoona
 * Track headshot kills by monitoring hit zones before death
 *
 * Place in: 4_World/Entities/KOTH_PlayerBase.c
 */

modded class PlayerBase
{
    private EntityAI m_KOTHArmband;
    private string m_KOTHTeam = "";
    private bool m_KOTHHeadshotKill = false;
    
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
    
    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
        
        if (!GetGame().IsServer())
            return;
        
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