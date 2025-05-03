/**
 * ExpansionCityManagerNPCAIBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionCityManagerNPCAIBase
 * @brief		This class CityManager AI NPCs
 **/

 #ifdef EXPANSIONMODAI
 class ExpansionCityManagerNPCAIBase: eAINPCBase
 {
     protected int m_CityManagerNPCID = -1;
     protected ref ExpansionCityManagerNPCData m_CityManagerNPCData;
 
     void ExpansionCityManagerNPCAIBase()
     {
         SetMovementSpeedLimits(1.0);  //! Always walk
         RegisterNetSyncVariableInt("m_CityManagerNPCID", 1, int.MAX);
     }
     void ~ExpansionCityManagerNPCAIBase()
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

     void SetCityManagerNPCID(int id)
     {
         m_CityManagerNPCID = id;
         
         ExpansionCityManagerModule.AddCityManagerNPCAI(id, this);
         
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

class ExpansionCityManagerNPCAIMirek: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIDenis: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIBoris: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAICyril: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIElias: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIFrancis: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIGuo: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIHassan: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIIndar: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIJose: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIKaito: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAILewis: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIManua: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAINiki: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIOliver: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIPeter: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIQuinn: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIRolf: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAISeth: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAITaiki: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAILinda: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIMaria: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIFrida: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIGabi: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIHelga: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIIrena: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIJudy: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIKeiko: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIEva: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAINaomi: ExpansionCityManagerNPCAIBase {};
class ExpansionCityManagerNPCAIBaty: ExpansionCityManagerNPCAIBase {};
#endif