/**
 * ExpansionTravelStationsNPCAIBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionTravelStationsNPCAIBase
 * @brief		This class TravelStations AI NPCs
 **/

 #ifdef EXPANSIONMODAI
 class ExpansionTravelStationsNPCAIBase: eAINPCBase
 {
     protected int m_TravelStationsNPCID = -1;
     protected ref ExpansionTravelStationsNPCData m_TravelStationsNPCData;
 
     void ExpansionTravelStationsNPCAIBase()
     {
         SetMovementSpeedLimits(1.0);  //! Always walk
         RegisterNetSyncVariableInt("m_TravelStationsNPCID", 1, int.MAX);
     }
     void ~ExpansionTravelStationsNPCAIBase()
     {

     }

     override void Expansion_Init()
     {
         if (GetGame().IsServer())
         {
             m_eAI_FactionType = eAIFactionInvincibleObservers;
         }
 
         super.Expansion_Init();
     }

     void SetTravelStationsNPCID(int id)
     {
         m_TravelStationsNPCID = id;
         
         ExpansionTravelStationsModule.AddTravelStationsNPCAI(id, this);
         
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

class ExpansionTravelStationsNPCAIMirek: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIDenis: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIBoris: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAICyril: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIElias: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIFrancis: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIGuo: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIHassan: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIIndar: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIJose: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIKaito: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAILewis: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIManua: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAINiki: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIOliver: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIPeter: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIQuinn: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIRolf: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAISeth: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAITaiki: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAILinda: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIMaria: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIFrida: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIGabi: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIHelga: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIIrena: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIJudy: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIKeiko: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIEva: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAINaomi: ExpansionTravelStationsNPCAIBase {};
class ExpansionTravelStationsNPCAIBaty: ExpansionTravelStationsNPCAIBase {};
#endif