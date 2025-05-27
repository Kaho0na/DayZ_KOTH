/**
 * ExpansionTravelStationsNpc.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionTravelStationsNPCBase: ExpansionNPCBase
{
	protected int m_TravelStationsNPCID = -1;
	protected ref ExpansionTravelStationsNPCData m_TravelStationsNPCData;

	
	void ExpansionTravelStationsNPCBase()
	{
		RegisterNetSyncVariableInt("m_TravelStationsNPCID", 1, int.MAX);
	}

	void ~ExpansionTravelStationsNPCBase()
	{

	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (m_TravelStationsNPCID > -1)
		{
			if (!ExpansionTravelStationsModule.GetTravelStationsNPCByID(m_TravelStationsNPCID))
				EXPrint(ToString() + " ::OnVariablesSynchronized - NPC ID: " + m_TravelStationsNPCID);
			ExpansionTravelStationsModule.AddTravelStationsNPC(m_TravelStationsNPCID, this);
		}

	}

	void SetTravelStationsNPCID(int id)
	{
		m_TravelStationsNPCID = id;
		
		ExpansionTravelStationsModule.AddTravelStationsNPC(id, this);
		
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

};

class ExpansionTravelStationsNPCMirek: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCDenis: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCBoris: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCCyril: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCElias: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCFrancis: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCGuo: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCHassan: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCIndar: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCJose: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCKaito: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCLewis: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCManua: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCNiki: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCOliver: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCPeter: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCQuinn: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCRolf: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCSeth: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCTaiki: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCLinda: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCMaria: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCFrida: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCGabi: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCHelga: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCIrena: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCJudy: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCKeiko: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCEva: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCNaomi: ExpansionTravelStationsNPCBase {};
class ExpansionTravelStationsNPCBaty: ExpansionTravelStationsNPCBase {};