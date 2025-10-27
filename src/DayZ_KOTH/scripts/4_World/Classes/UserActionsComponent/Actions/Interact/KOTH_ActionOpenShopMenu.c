/**
 * KOTH_ActionOpenShopMenu.c
 *
 * Action to open the shop menu when interacting with shop NPCs
 */

class KOTH_ActionOpenShopMenu: ActionInteractBase
{
	protected KOTH_ShopModule m_ShopModule;
	
	void KOTH_ActionOpenShopMenu()
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
		return "Open Shop Menu";
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

		KOTH_NPCMirek shopNPC = KOTH_NPCMirek.Cast(targetObject);
		if (!shopNPC)
			return false;
			
		if (!GetGame().IsDedicatedServer())
		{
			m_Text = "Open Shop Menu";
		}

		return true;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		Print("[KOTH_ActionOpenShopMenu] ========== START ==========");
		super.OnExecuteServer(action_data);
		Print("[KOTH_ActionOpenShopMenu] super.OnExecuteServer() completed");

		PlayerBase player = action_data.m_Player;
		Print("[KOTH_ActionOpenShopMenu] Player pointer: " + player);
		
		if (!player || !player.GetIdentity())
		{
			Print("[KOTH_ActionOpenShopMenu] ERROR: Player or Player Identity is null");
			return;
		}

		Print("[KOTH_ActionOpenShopMenu] Player identity: " + player.GetIdentity().GetName());
		Print("[KOTH_ActionOpenShopMenu] Getting KOTH_ShopModule instance...");
		
		m_ShopModule = KOTH_ShopModule.GetInstance();
		Print("[KOTH_ActionOpenShopMenu] GetInstance() returned: " + m_ShopModule);
		
		if (!m_ShopModule)
		{
			Print("[KOTH_ActionOpenShopMenu] ERROR: Could not get KOTH_ShopModule via GetInstance()");
			return;
		}

		Print("[KOTH_ActionOpenShopMenu] Module retrieved successfully!");
		Print("[KOTH_ActionOpenShopMenu] Module class type: " + m_ShopModule.GetType());

		if (!action_data.m_Target)
		{
			Print("[KOTH_ActionOpenShopMenu] ERROR: action_data.m_Target is null");
			return;
		}

		Print("[KOTH_ActionOpenShopMenu] Target exists");

		Object targetObject;
		if (!Class.CastTo(targetObject, action_data.m_Target.GetParentOrObject()))
		{
			Print("[KOTH_ActionOpenShopMenu] ERROR: Failed to cast target object");
			return;
		}

		Print("[KOTH_ActionOpenShopMenu] Target object type: " + targetObject.GetType());
		Print("[KOTH_ActionOpenShopMenu] About to call RequestShopOpen...");
		Print("[KOTH_ActionOpenShopMenu] Player param: " + player);
		Print("[KOTH_ActionOpenShopMenu] Identity param: " + player.GetIdentity().GetName());
		
		m_ShopModule.RequestShopOpen(player, player.GetIdentity());
		
		Print("[KOTH_ActionOpenShopMenu] RequestShopOpen() call completed!");
		Print("[KOTH_ActionOpenShopMenu] ========== END ==========");
	}
}