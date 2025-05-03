/*modded class MissionServer
{
	override void OnMissionLoaded()
	{
		super.OnMissionLoaded();

		//! Wait 5 seconds before spawning NPCs
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(ExpansionCityManagerNPCData.GetInstance().SpawnCityManagerNPCs, 5000, false);
        
	}
}
*/