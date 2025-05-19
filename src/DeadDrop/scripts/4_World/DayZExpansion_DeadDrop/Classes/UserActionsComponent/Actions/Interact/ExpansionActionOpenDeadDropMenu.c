/**
 * ExpansionActionOpenDeadDropMenu.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionActionOpenDeadDropMenu: ActionInteractBase
{
	protected ExpansionDeadDropModule m_Expansion_DeadDropModule;

	void ExpansionActionOpenDeadDropMenu()
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

		auto npcAI = ExpansionDeadDropNPCAIBase.Cast(targetObject);
		if (!npcAI)
			return false;  // Only allow action for DeadDrop NPC

		if (!GetGame().IsDedicatedServer())
		{
			string actionText = "Talk to the medic";
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

		if (!CF_Modules<ExpansionDeadDropModule>.Get(m_Expansion_DeadDropModule))
		{
			Print("Exiting: Could not get ExpansionDeadDropModule");
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
		Print("Requesting Dead Drop Menu for " + player.GetIdentity().GetName());

		m_Expansion_DeadDropModule.RequestOpenDeadDropMenu(targetObject, player.GetIdentity());
	}

}
