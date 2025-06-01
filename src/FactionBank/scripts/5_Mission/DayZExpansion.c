/**
 * DayzExpansion.c
 *
 * DayZ Expansion Mod | Fation Bank by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		DayZExpansion
 * @brief		
 **/
modded class DayZExpansion
{
	// ------------------------------------------------------------
	// Expansion OnLoaded
	// ------------------------------------------------------------
	override void OnLoaded()
	{
		if ( IsMissionHost() )
		{
			ExpansionSettings.SI_FactionBank.Invoke();
		}

		super.OnLoaded();
	}
};