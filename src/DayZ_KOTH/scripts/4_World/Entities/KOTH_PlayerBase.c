/**
 * KOTH_PlayerBase.c
 *
 * King of the Hill by Kahoona
 * Unified PlayerBase modifications for KOTH system
 *
 * Place in: 4_World/Entities/KOTH_PlayerBase.c
 */

modded class PlayerBase
{
    private EntityAI m_KOTHArmband;
    private string m_KOTHTeam = "";
    
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
    }
}