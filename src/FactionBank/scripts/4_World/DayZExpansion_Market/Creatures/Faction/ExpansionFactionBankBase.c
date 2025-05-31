/**
 * ExpansionFactionBankBase.c
 *
 * DayZ Expansion Mod - Faction Bank by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionFactionBankBase: BuildingSuper
{
	void ExpansionFactionBankBase()
	{
		if (IsMissionHost())
		{
			SetAllowDamage(false);
		}
	}

	override bool IsBuilding()
	{
		return false;
	}

	override void SetActions()
	{
		super.SetActions();
		
		AddAction(ExpansionActionOpenFactionMenu);
	}
}

class ExpansionFactionLocker: ExpansionFactionBankBase {};