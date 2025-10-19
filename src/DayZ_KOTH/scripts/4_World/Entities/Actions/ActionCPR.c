/**
 * ActionCPR.c (REVIVE TRACKING - AI SUPPORT)
 *
 * King of the Hill by Kahoona
 * Track who performs CPR to award revive XP (works on players and AI)
 *
 * Place in: 4_World/Entities/Actions/ActionCPR.c
 */

modded class ActionCPR
{
    override void OnFinishProgressServer(ActionData action_data)
    {
        super.OnFinishProgressServer(action_data);
        
        PlayerBase target = PlayerBase.Cast(action_data.m_Target.GetObject());
        PlayerBase medic = PlayerBase.Cast(action_data.m_Player);
        
        if (target && medic)
        {
            target.SetReviver(medic);
            
            string targetName = "Unknown";
            if (target.GetIdentity())
            {
                targetName = target.GetIdentity().GetName();
            }
            else
            {
                targetName = target.GetType();
            }
            
            string medicName = "Unknown";
            if (medic.GetIdentity())
            {
                medicName = medic.GetIdentity().GetName();
            }
            
            Print("[KOTH_ActionCPR] CPR completed - Medic: " + medicName + " on Target: " + targetName);
        }
    }
}