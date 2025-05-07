/**
 * ExpansionTravelStationsStaticObject.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionTravelStationsStaticObject
 * @brief		This class handles static TravelStations objects used to head-out or take-in TravelStationss
 **/
class ExpansionTravelStationsStaticObject: ExpansionStaticObjectBase
{
    protected int m_TravelStationsNPCID = -1;
	protected ref ExpansionTravelStationsNPCData m_TravelStationsNPCData;

    void ExpansionTravelStationsStaticObject()
	{
		if (IsMissionHost())
			SetAllowDamage(false);

		RegisterNetSyncVariableInt("m_TravelStationsNPCID", 1, int.MAX);

		m_Expansion_NetsyncData = new ExpansionNetsyncData(this);
	}

    void ~ExpansionTravelStationsStaticObject()
	{

	}

    void SetTravelStationsNPCID(int id)
	{
		m_TravelStationsNPCID = id;
		
		ExpansionTravelStationsModule.AddStaticTravelStationsObject(id, this);
		
		SetSynchDirty();
	}

    int GetTravelStationsNPCID()
	{
		return m_TravelStationsNPCID;
	}

	void SetTravelStationsNPCData(ExpansionTravelStationsNPCData TravelStationsNPCData)
	{
		m_TravelStationsNPCData = TravelStationsNPCData;
	}

	ExpansionTravelStationsNPCData GetTravelStationsNPCData()
	{
		return m_TravelStationsNPCData;
	}


}