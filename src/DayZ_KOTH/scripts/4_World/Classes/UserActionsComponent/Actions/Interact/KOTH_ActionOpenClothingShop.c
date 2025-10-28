class KOTH_ActionOpenClothingShop: ActionInteractBase
{
    protected KOTH_ClothingShopModule m_ClothingShopModule;
    
    void KOTH_ActionOpenClothingShop()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ATTACHITEM;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone;
        m_ConditionTarget = new CCTCursor;
    }

    override string GetText()
    {
        return "Open Clothing Shop";
    }

    override typename GetInputType()
    {
        return InteractActionInput;
    }

    override bool HasTarget()
    {
        return true;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        Object targetObject;
        if (!Class.CastTo(targetObject, target.GetParentOrObject()))
            return false;

        KOTH_NPCDenis clothingNPC = KOTH_NPCDenis.Cast(targetObject);
        if (!clothingNPC)
            return false;
            
        if (!GetGame().IsDedicatedServer())
        {
            m_Text = "Open Clothing Shop";
        }

        return true;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        super.OnExecuteServer(action_data);

        PlayerBase player = action_data.m_Player;
        
        if (!player || !player.GetIdentity())
        {
            return;
        }

        m_ClothingShopModule = KOTH_ClothingShopModule.GetInstance();
        
        if (!m_ClothingShopModule)
        {
            return;
        }

        if (!action_data.m_Target)
        {
            return;
        }

        Object targetObject;
        if (!Class.CastTo(targetObject, action_data.m_Target.GetParentOrObject()))
        {
            return;
        }

        m_ClothingShopModule.RequestClothingShopOpen(player, player.GetIdentity());
    }
}