/**
 * ExpansionActionOpenCityManagerMenu.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionActionOpenCityManagerMenu: ActionInteractBase
{
	protected ExpansionCityManagerModule m_Expansion_CityManagerModule;

	void ExpansionActionOpenCityManagerMenu()
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

		auto npc = ExpansionCityManagerNPCBase.Cast(targetObject);
	#ifdef EXPANSIONMODAI
		auto npcAI = ExpansionCityManagerNPCAIBase.Cast(targetObject);
	#endif
		auto npcObject = ExpansionCityManagerStaticObject.Cast(targetObject);

	#ifdef EXPANSIONMODAI
		if (!npc && !npcAI && !npcObject)
	#else
		if (!npc && !npcObject)
	#endif
			return false;

		if (!GetGame().IsDedicatedServer())
		{
			//! Client
			//! Dont show action if menu is already opened
			if (GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu())
				return false;

			string actionText;
		#ifdef EXPANSIONMODAI
			if (npc || npcAI)
		#else
			if (npc)
		#endif
				actionText = "Talk to the manager";
			else
				actionText = "Read from the manager";

			m_Text = actionText + " " + targetObject.GetDisplayName();
		}

		return true;
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);

		PlayerBase player = action_data.m_Player;
		if (!player || !player.GetIdentity())
		{
			Print("Exiting: Player or Player Identity is null");
			return;
		}

		if (!CF_Modules<ExpansionCityManagerModule>.Get(m_Expansion_CityManagerModule))
		{
			Print("Exiting: Could not get ExpansionCityManagerModule");
			return;
		}

		if (!action_data.m_Target)
		{
			Print("Exiting: action_data.m_Target is null");
			return;
		}

		Object targetObject;
		if (!Class.CastTo(targetObject, action_data.m_Target.GetParentOrObject()))
		{
			Print("Failed to cast target object!");
			return;
		}

		Print("Target object: " + targetObject.ToString());
		Print("Requesting City Manager Menu for " + player.GetIdentity().GetName());

		m_Expansion_CityManagerModule.RequestOpenCityManagerMenu(targetObject, player.GetIdentity());
	}

}
