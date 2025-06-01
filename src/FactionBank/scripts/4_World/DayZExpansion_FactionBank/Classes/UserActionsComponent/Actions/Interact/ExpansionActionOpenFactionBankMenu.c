/**
 * ExpansionActionOpenFactionBankMenu.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionActionOpenFactionBankMenu: ActionInteractBase
{
	protected ExpansionFactionBankModule m_FactionBankModule;
	
	void ExpansionActionOpenFactionBankMenu()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ATTACHITEM;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override typename GetInputType()
	{
		return InteractActionInput;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		Object targetObject;
		if (!Class.CastTo(targetObject, target.GetParentOrObject()))
			return false;

		auto npc = ExpansionFactionBankNPCBase.Cast(targetObject);

		if (!npc)
			return false;
			
		if (!GetGame().IsDedicatedServer())
		{
			m_Text = "Talk to the Resistance Bank Manager";
		}

		return true;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);

		PlayerBase player = action_data.m_Player;
		if (!player || !player.GetIdentity())
		{
			Print("[FactionBank Error] Exiting: Player or Player Identity is null");
			return;
		}

		if (!CF_Modules<ExpansionFactionBankModule>.Get(m_FactionBankModule))
		{
			Print("[FactionBank Error] Exiting: Could not get ExpansionFactionBankModule");
			return;
		}

		if (!action_data.m_Target)
		{
			Print("[FactionBank Error] Exiting: action_data.m_Target is null");
			return;
		}

		Object targetObject;
		if (!Class.CastTo(targetObject, action_data.m_Target.GetParentOrObject()))
		{
			Print("[FactionBank Error] Failed to cast target object!");
			return;
		}

		m_FactionBankModule.RequestFactionBankMenu(player.GetIdentity());

	}


}
