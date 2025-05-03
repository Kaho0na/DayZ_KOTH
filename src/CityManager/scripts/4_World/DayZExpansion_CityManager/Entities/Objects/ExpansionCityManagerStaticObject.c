/**
 * ExpansionCityManagerStaticObject.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionCityManagerStaticObject
 * @brief		This class handles static CityManager objects used to head-out or take-in CityManagers
 **/
class ExpansionCityManagerStaticObject: ExpansionStaticObjectBase
{
    protected int m_CityManagerNPCID = -1;
	protected ref ExpansionCityManagerNPCData m_CityManagerNPCData;

    void ExpansionCityManagerStaticObject()
	{
		if (IsMissionHost())
			SetAllowDamage(false);

		RegisterNetSyncVariableInt("m_CityManagerNPCID", 1, int.MAX);

		m_Expansion_NetsyncData = new ExpansionNetsyncData(this);
	}

    void ~ExpansionCityManagerStaticObject()
	{

	}

    void SetCityManagerNPCID(int id)
	{
		m_CityManagerNPCID = id;
		
		ExpansionCityManagerModule.AddStaticCityManagerObject(id, this);
		
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


}