/**
 * ExpansionDeadDropNPCAIBase.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

/**@class		ExpansionDeadDropNPCAIBase
 * @brief		This class DeadDrop AI NPCs
 **/

 #ifdef EXPANSIONMODAI
 class ExpansionDeadDropNPCAIBase: eAINPCBase
 {
     protected int m_DeadDropNPCID = -1;
 
	void ExpansionDeadDropNPCAIBase()
	{
		SetMovementSpeedLimits(1.0);  //! Always walk
	}
    void ~ExpansionDeadDropNPCAIBase()
    {

    }

	override void Expansion_Init()
	{
		if (GetGame().IsServer())
			m_eAI_FactionType = eAIFactionInvincibleObservers;

		super.Expansion_Init();
	}
 
};

class ExpansionDeadDropNPCAIMirek: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIDenis: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIBoris: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAICyril: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIElias: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIFrancis: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIGuo: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIHassan: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIIndar: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIJose: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIKaito: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAILewis: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIManua: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAINiki: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIOliver: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIPeter: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIQuinn: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIRolf: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAISeth: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAITaiki: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAILinda: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIMaria: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIFrida: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIGabi: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIHelga: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIIrena: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIJudy: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIKeiko: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIEva: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAINaomi: ExpansionDeadDropNPCAIBase {};
class ExpansionDeadDropNPCAIBaty: ExpansionDeadDropNPCAIBase {};
#endif