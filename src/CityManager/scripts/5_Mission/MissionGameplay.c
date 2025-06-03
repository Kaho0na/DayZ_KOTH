modded class MissionGameplay extends MissionBase
{
	float m_NextDebugTime = 0;

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (KeyState(KeyCode.KC_L) && GetGame().GetTime() > m_NextDebugTime)
		{
			m_NextDebugTime = GetGame().GetTime() + 1000; // 1 second cooldown
			PrintTraderOffsetDebug();
		}
	}


	void PrintTraderOffsetDebug()
	{
		if (!KeyState(KeyCode.KC_L))
			return;

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		vector playerPosWorld = player.GetPosition();
		vector playerOri = player.GetOrientation();

		array<Object> results = new array<Object>;
		array<CargoBase> proxy = new array<CargoBase>;
		GetGame().GetObjectsAtPosition(playerPosWorld, 5.0, results, proxy);

		foreach (Object obj : results)
		{
			if (obj.IsBuilding())
			{
				string type = obj.GetType();
				vector buildingPos = obj.GetPosition();
				vector buildingOri = obj.GetOrientation();
				vector relativeOri = playerOri - buildingOri;

				// Offset relative to building
				vector offset = playerPosWorld - buildingPos;
				vector offsetModel = obj.WorldToModel(playerPosWorld); // this is more accurate
				vector backToWorld = obj.ModelToWorld(offsetModel);     // sanity check

				Print("[OffsetTool] BuildingType=" + type);
				Print("   BuildingPOS = " + buildingPos);
				Print("   BuildingORI = " + buildingOri);
				Print("   PlayerPOS   = " + playerPosWorld);
				Print("   PlayerORI   = " + playerOri);
				Print("   RelativeORI = " + relativeOri);
				Print("   Offset (Manual Subtract)    = " + offset);
				Print("   Offset (WorldToModel)       = " + offsetModel);
				Print("   Back-Check (ModelToWorld)   = " + backToWorld);

				Print(string.Format("buildingCategoryMap.Insert(\"%1\", new TraderSpawnInfo({\"Consumables1\", \"Consumables2\", \"Consumables3\", \"Consumables4\", \"Consumables5\"}, \"%2 %3 %4\", \"%5 %6 %7\"));", type, offsetModel[0].ToString(), offsetModel[1].ToString(), offsetModel[2].ToString(), relativeOri[0].ToString(), relativeOri[1].ToString(), relativeOri[2].ToString()));


				break;
			}
		}
	}
}
