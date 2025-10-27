class KOTH_ActionOpenVehicleShop: ActionInteractBase
{
	protected KOTH_VehicleShopModule m_VehicleShopModule;
	
	void KOTH_ActionOpenVehicleShop()
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
		return "Open Vehicle Shop";
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

		KOTH_NPCBoris vehicleNPC = KOTH_NPCBoris.Cast(targetObject);
		if (!vehicleNPC)
			return false;
			
		if (!GetGame().IsDedicatedServer())
		{
			m_Text = "Open Vehicle Shop";
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

		
		m_VehicleShopModule = KOTH_VehicleShopModule.GetInstance();
		
		if (!m_VehicleShopModule)
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

		
		m_VehicleShopModule.RequestVehicleShopOpen(player, player.GetIdentity());
	}
}