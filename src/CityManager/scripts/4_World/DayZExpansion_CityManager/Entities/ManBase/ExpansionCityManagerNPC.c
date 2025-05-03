/**
 * ExpansionCityManagerNpc.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionCityManagerNPCBase: ExpansionNPCBase
{
	protected int m_CityManagerNPCID = -1;
	protected ref ExpansionCityManagerNPCData m_CityManagerNPCData;

	
	void ExpansionCityManagerNPCBase()
	{
		RegisterNetSyncVariableInt("m_CityManagerNPCID", 1, int.MAX);
	}

	void ~ExpansionCityManagerNPCBase()
	{

	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (m_CityManagerNPCID > -1)
		{
			if (!ExpansionCityManagerModule.GetCityManagerNPCByID(m_CityManagerNPCID))
				EXPrint(ToString() + " ::OnVariablesSynchronized - NPC ID: " + m_CityManagerNPCID);
			ExpansionCityManagerModule.AddCityManagerNPC(m_CityManagerNPCID, this);
		}

	}

	void SetCityManagerNPCID(int id)
	{
		m_CityManagerNPCID = id;
		
		ExpansionCityManagerModule.AddCityManagerNPC(id, this);
		
		SetSynchDirty();
	}

	int GetCityManagerNPCID()
	{
		return m_CityManagerNPCID;
	}

	void SetCityManagerNPCData(ExpansionCityManagerNPCData CityManagerNPCData)
	{
		m_CityManagerNPCData = CityManagerNPCData;
	}

	ExpansionCityManagerNPCData GetCityManagerNPCData()
	{
		return m_CityManagerNPCData;
	}

};