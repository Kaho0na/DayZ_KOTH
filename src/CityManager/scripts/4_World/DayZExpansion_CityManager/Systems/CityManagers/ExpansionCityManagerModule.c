/**
 * ExpansionCityManagerModule.c
 *
 * DayZ Expansion Mod - City Manager by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

[CF_RegisterModule(ExpansionCityManagerModule)]
class ExpansionCityManagerModule: CF_ModuleWorld
{
    static ref map<int, ExpansionCityManagerNPCBase> s_CityManagerNPCEntities = new map<int, ExpansionCityManagerNPCBase>; //! Server & Client
    static ref map<int, ExpansionCityManagerStaticObject> s_CityManagerObjectEntities = new map<int, ExpansionCityManagerStaticObject>; //! Server & Client
    protected ref map<int, ref ExpansionCityManagerNPCData> m_CityManagersNPCs; //! Server
	protected ref ExpansionMarketModule m_MarketModule;
	ref map<string, bool> m_LiberatedCityGrids;


    #ifdef EXPANSIONMODAI
	static ref map<int, ExpansionCityManagerNPCAIBase> s_CityManagerNPCAIEntities = new map<int, ExpansionCityManagerNPCAIBase>; //! Server & Client
    #endif

    protected ref map<int, ExpansionCityManagerNPCBase> m_CityManagerNPCs;
    protected static ExpansionCityManagerModule s_ModuleInstance;

	protected ref ScriptInvoker m_CityManagerMenuInvoker; //! Client
	protected ref ScriptInvoker m_CityManagerMenuCallbackInvoker; //! Client	
	static ref ScriptInvoker SI_CityManagerMenuCallback = new ScriptInvoker();
	ExpansionMarkerModule markerModule;
	private int m_LiberatedCitiesIncome;


    void ExpansionCityManagerModule()
	{
        s_ModuleInstance = this;

		
        m_CityManagersNPCs = new map<int, ref ExpansionCityManagerNPCData>; //! Server
		m_CityManagerMenuInvoker = new ScriptInvoker(); //! Client
		m_CityManagerMenuCallbackInvoker = new ScriptInvoker();
    }

    override void OnMissionLoaded(Class sender, CF_EventArgs args)
	{

		super.OnMissionLoaded(sender, args);

		//! Spawn NPCs late so mapping already loaded
		if (GetGame().IsServer())
        {
            SpawnCityManagerNPCs()
        }
	}

	override void OnInit()
	{
		super.OnInit();

		EnableMissionStart();
		EnableMissionLoaded();
		EnableMissionFinish();
		EnableInvokeConnect();
		EnableClientDisconnect();
		EnableClientNew();
		Expansion_EnableRPCManager();

		Expansion_RegisterClientRPC("RPC_RequestOpenCityManagerMenu");
		Expansion_RegisterServerRPC("RPC_RequestBribeMoney");
		Expansion_RegisterClientRPC("RPC_ConfirmBribeMoney");
	}


    override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);

		if (GetGame().IsServer() && GetGame().IsMultiplayer())
		{

			ServerModuleInit();
		}
	}

    void ServerModuleInit()
    {
        //! Server only
        if (GetGame().IsServer() && GetGame().IsMultiplayer())
        {

            //! CITY MANAGER NPC FILES
            if (FileExist(EXPANSION_CITYMANAGER_FOLDER))
            {
                array<string> managerNPCFiles = ExpansionStatic.FindFilesInLocation(EXPANSION_CITYMANAGER_FOLDER, ".json");
                if (managerNPCFiles.Count() > 0)
                {
                    LoadCityManagerNPCData(managerNPCFiles); //! Server: Load existing NPCs data files from the server and load them into m_CityManagersNPCs.
                }
            }
            else
            {
                MakeDirectory(EXPANSION_CITYMANAGER_FOLDER);
            }
        }
    }

    static ExpansionCityManagerNPCBase GetCityManagerNPCByID(int id)
	{
		return s_CityManagerNPCEntities.Get(id);
	}

    static void AddCityManagerNPC(int id, ExpansionCityManagerNPCBase CityManagerNPC)
	{
		if (!s_CityManagerNPCEntities[id])  //! Can be NULL if object was deleted because it was no longer in network bubble
			s_CityManagerNPCEntities[id] = CityManagerNPC;
	}

    static void AddStaticCityManagerObject(int id, ExpansionCityManagerStaticObject staticQustObject)
	{
		if (!s_CityManagerObjectEntities[id])  //! Can be NULL if object was deleted because it was no longer in network bubble
			s_CityManagerObjectEntities[id] = staticQustObject;
	}

    static void AddCityManagerNPCAI(int id, ExpansionCityManagerNPCAIBase CityManagerNPCAI)
	{
		if (!s_CityManagerNPCAIEntities[id])  //! Can be NULL if object was deleted because it was no longer in network bubble
			s_CityManagerNPCAIEntities[id] = CityManagerNPCAI;
	}

    //! Server
    void SpawnCityManagerNPCs()
    {

		if (!GetGame().IsServer() && !GetGame().IsMultiplayer())
		{
			EXError.Error(this, "[Expansion CityManagers] Tryed to call SpawnCityManagerNPCs on Client!");
			return;
		}

		if (!m_CityManagersNPCs)
		{
			EXError.Error(this, "[Expansion CityManagers] NPC config array is NULL!");
			return;
		}

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PayRecurringIncome, 3600000, true); // 3600s = 1hr

		foreach (int id, ExpansionCityManagerNPCData managerNPCData: m_CityManagersNPCs)
		{
			vector centerPos = managerNPCData.GetPosition();

			switch (managerNPCData.GetNPCType())
			{
				case ExpansionCityManagerNPCType.NORMAL:
				{
					ExpansionCityManagerNPCBase npc = managerNPCData.SpawnNPC();
					if (!npc)
						return;

					npc.SetCityManagerNPCID(managerNPCData.GetID());
					npc.SetCityManagerNPCData(managerNPCData);
					npc.SetPosition(managerNPCData.GetPosition());
					npc.SetOrientation(managerNPCData.GetOrientation());

				}
				break;
				case ExpansionCityManagerNPCType.OBJECT:
				{
					ExpansionCityManagerStaticObject object = managerNPCData.SpawnObject();
					if (!object)
						return;

					object.SetCityManagerNPCID(managerNPCData.GetID());
					object.SetCityManagerNPCData(managerNPCData);
					object.SetPosition(managerNPCData.GetPosition());
					object.SetOrientation(managerNPCData.GetOrientation());
				}
				break;
			#ifdef EXPANSIONMODAI
				case ExpansionCityManagerNPCType.AI:
				{
					ExpansionCityManagerNPCAIBase npcAI = managerNPCData.SpawnNPCAI();
					if (!npcAI)
						return;

					npcAI.SetCityManagerNPCID(managerNPCData.GetID());
					npcAI.SetCityManagerNPCData(managerNPCData);
					npcAI.Expansion_SetEmote(managerNPCData.GetEmoteID(), !managerNPCData.IsEmoteStatic());
				}
				break;
			#endif
			}

			// Spawn Police AI in a circle around the City Manager
			SpawnPolicePatrols(centerPos, managerNPCData);

			//Spawn Traders
			int cityTraders = managerNPCData.GetCityTraders();
			SpawnCityTraders(centerPos, managerNPCData, cityTraders);

			//Place Marker
			PlaceCityMarker(managerNPCData);

			if (managerNPCData && managerNPCData.GetCityLiberated())
			{
				// Add City Income Amout
				m_LiberatedCitiesIncome += managerNPCData.GetCityIncome();
				Print("[Expansion CityManagers] Adding City Income Amount: " + managerNPCData.GetCityIncome() + " for liberated city: " + managerNPCData.GetCityName() + " Total Income: " + m_LiberatedCitiesIncome);
				UpdateLiberatedCityGrids(managerNPCData.Position, managerNPCData.CityRadius);
			}
			
		}

	}

	void SpawnPolicePatrols(vector centerPos, ExpansionCityManagerNPCData managerNPCData)
	{
		if (!GetGame().IsServer() && !GetGame().IsMultiplayer())
		{
			EXError.Error(this, "[Expansion CityManagers] Tryed to call SpawnAIPatrols on Client!");
			return;
		}

		if (!managerNPCData)
		{
			EXError.Error(this, "[Expansion CityManagers] Manager NPC Data is NULL!");
			return;
		}

		if(managerNPCData.GetCityLiberated())
		{
			return;
		}

		int radius = managerNPCData.GetCityRadius();
		vector startPos = centerPos;
		ref ExpansionAIPatrol config = new ExpansionAIPatrol();
		config.Faction = "West";
		config.Loadout = "ChernPolice";
		config.NumberOfAI = 2;
		config.Speed = "WALK";
		config.UnderThreatSpeed = "SPRINT";
		config.CanBeLooted = true;
		config.UnlimitedReload = 1;
		config.AccuracyMin = 0.4;
		config.AccuracyMax = 0.5;
		config.LootingBehaviour = "WEAPONS | UPGRADE";
		config.ThreatDistanceLimit = 300;
		config.NoiseInvestigationDistanceLimit = 75;
		config.MinDistRadius = -1;
		config.MaxDistRadius = -1;
		config.DespawnRadius = -1;
		config.MinSpreadRadius = radius/20;
		config.MaxSpreadRadius = radius/10;
		config.DespawnTime = -1;
		config.RespawnTime = -1;
		config.Behaviour = "ALTERNATE";
		config.LoadBalancingCategory = "Global";
		config.Waypoints = new TVectorArray();

		int numWaypoints = Math.RandomIntInclusive(2, 3); // 2 or 3 waypoints
		config.Waypoints.Insert(startPos); // Always include the starting position

		for (int i = 0; i < numWaypoints; i++)
		{
			float angle = Math.RandomFloat(0, 360);
			float distance = Math.RandomFloat(radius * 0.3, radius); // Keep within city radius

			float offsetX = distance * Math.Cos(angle);
			float offsetZ = distance * Math.Sin(angle);

			vector point = startPos + Vector(offsetX, 0, offsetZ);
			point[1] = GetGame().SurfaceY(point[0], point[2]); // Adjust for terrain height
 
			config.Waypoints.Insert(point);
		}


		auto spawnResult = eAIDynamicPatrol.CreateEx(config, startPos, true);
		Print(string.Format("[CityManager] Guard spawn result: %1", spawnResult != null));
		Print("[Expansion CityManagers] Spawned AIPatrol at " + managerNPCData.GetCityName());
	}

	void SpawnCityTraders(vector centerPos, ExpansionCityManagerNPCData managerNPCData, int cityTraders)
	{
		array<Object> nearby = new array<Object>;
		array<CargoBase> proxy = new array<CargoBase>;
		GetGame().GetObjectsAtPosition(centerPos, 100, nearby, proxy); // Increase range if needed

		ref map<string, ref TraderSpawnInfo> buildingCategoryMap = new map<string, ref TraderSpawnInfo>;
		// Civilian Traders
		buildingCategoryMap.Insert("Land_House_1B01_Pub", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "2.34167 -2.77925 -3.52143", "-95.5222 0 0"));
		buildingCategoryMap.Insert("Land_House_1W01", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "2.64694 -2.80881 1.75133", "-57.5492 0 0"));
		buildingCategoryMap.Insert("Land_House_1W03", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "-0.930491 -2.35122 2.83416", "232.583 0 0"));
		buildingCategoryMap.Insert("Land_House_1W09_Yellow", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "-4.81777 -1.9185 1.06665", "89.4416 0 0"));
		buildingCategoryMap.Insert("Land_House_1W10_Brown", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "-1.4755 -1.26735 -3.05731", "2.42112 0 0"));
		buildingCategoryMap.Insert("Land_House_1W11", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "-2.9483 -2.63168 -2.15116", "113.107 0 0"));
		buildingCategoryMap.Insert("Land_House_2B03", new TraderSpawnInfo({"Civilian1", "Civilian2", "Civilian3", "Civilian4", "Civilian5"}, "0.185622 -5.47631 4.96352", "-159.588 -0 0"));
		// Religious Traders
		buildingCategoryMap.Insert("Land_Church1_Yellow", new TraderSpawnInfo({"Religious1", "Religious2", "Religious3", "Religious4", "Religious5"}, "4.59123 -6.0513 0.0885372", "-110.886 0 0"));
		buildingCategoryMap.Insert("Land_Church2_1", new TraderSpawnInfo({"Religious1", "Religious2", "Religious3", "Religious4", "Religious5"}, "0.951901 -11.0625 1.0617", "91.0491 0 0"));
		buildingCategoryMap.Insert("Land_Church2_2", new TraderSpawnInfo({"Religious1", "Religious2", "Religious3", "Religious4", "Religious5"}, "2.95745 -11.5173 1.30115", "-88.9625 0 0"));
		buildingCategoryMap.Insert("Land_Church3", new TraderSpawnInfo({"Religious1", "Religious2", "Religious3", "Religious4", "Religious5"}, "4.76748 -14.2829 -0.127451", "-91.347 0 0"));
		buildingCategoryMap.Insert("Land_Chapel", new TraderSpawnInfo({"Religious1", "Religious2", "Religious3", "Religious4", "Religious5"}, "-2.2051 -4.00077 -0.0106746", "67.8312 0 0"));
		// Security Traders
		buildingCategoryMap.Insert("Land_Mil_Guardhouse1", new TraderSpawnInfo({"Security1", "Security2", "Security3", "Security4", "Security5"}, "3.59266 -1.86618 -1.96487", "251.205 0 0"));
		buildingCategoryMap.Insert("Land_Guardhouse", new TraderSpawnInfo({"Security1", "Security2", "Security3", "Security4", "Security5"}, "1.73803 -0.74469 -0.110972", "-45.3002 0 0"));
		// Stall Traders
		buildingCategoryMap.Insert("Land_City_Stand_FastFood", new TraderSpawnInfo({"Stall1", "Stall2", "Stall3", "Stall4", "Stall5"}, "0.581398 -1.64164 -2.48368", "-183.045 0.0174412 -0.00933447"));
		buildingCategoryMap.Insert("Land_City_Stand_Grocery", new TraderSpawnInfo({"Stall1", "Stall2", "Stall3", "Stall4", "Stall5"}, "0.180242 -1.64162 -2.31755", "181.611 0 0"));
		buildingCategoryMap.Insert("Land_City_Stand_News1", new TraderSpawnInfo({"Stall1", "Stall2", "Stall3", "Stall4", "Stall5"}, "0.690176 -1.64163 -2.41673", "179.85 -0.0152578 -0.0125907"));
		buildingCategoryMap.Insert("Land_City_Stand_News2", new TraderSpawnInfo({"Stall1", "Stall2", "Stall3", "Stall4", "Stall5"}, "-0.070903 -1.27524 -2.15292", "174.019 0 0"));
		// Medical Traders
		buildingCategoryMap.Insert("Land_Village_HealthCare", new TraderSpawnInfo({"Medical1"}, "-0.225904 -2.92607 -4.85171", "-21.7046 0 0"));
		buildingCategoryMap.Insert("Land_City_Hospital", new TraderSpawnInfo({"Medical1"}, "3.16111 -7.31112 -1.01191", "-116.784 0 0"));
		// School Traders
		buildingCategoryMap.Insert("Land_Office1", new TraderSpawnInfo({"School1"}, "-0.367967 -4.63401 1.22722", "-199.624 0 0"));
		// FireStation Traders
		buildingCategoryMap.Insert("Land_City_FireStation", new TraderSpawnInfo({"FireStation1"}, "-1.53522 0.0772831 2.75326", "178.589 0 0"));
		// Store Traders
		buildingCategoryMap.Insert("Land_City_Store", new TraderSpawnInfo({"Store1", "Store2", "Store3", "Store4", "Store5"}, "-3.55501 -1.17147 -1.34356", "30.0516 0 0"));
		buildingCategoryMap.Insert("Land_Village_store", new TraderSpawnInfo({"Store1", "Store2", "Store3", "Store4", "Store5"}, "1.76496 -2.20532 1.94198", "-188.546 0.0332612 0.00822874"));		
		// City Traders
		buildingCategoryMap.Insert("Land_HouseBlock_1F1", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "3.01204 -4.16531 2.88393", "237.672 0 0"));
		buildingCategoryMap.Insert("Land_HouseBlock_1F_Corner", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "2.58017 -5.71783 -1.63714", "140.131 -0.0236269 0.0248151"));
		buildingCategoryMap.Insert("Land_HouseBlock_2F1", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "3.82719 -6.19455 2.56561", "-120.684 0 0"));
		buildingCategoryMap.Insert("Land_HouseBlock_2F2", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "4.71459 -6.47765 -0.503273", "194.96 0 0"));
		buildingCategoryMap.Insert("Land_HouseBlock_2F5", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "2.79294 -6.4505 3.54867", "177.164 0 0"));
		buildingCategoryMap.Insert("Land_HouseBlock_2F7", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "4.89065 -6.7426 -0.472871", "-142.617 0.0110619 0.0164001"));
		buildingCategoryMap.Insert("Land_HouseBlock_2F_Corner", new TraderSpawnInfo({"City1", "City2", "City3", "City4", "City5"}, "-3.26735 -6.76466 2.86444", "-240.132 -0 0"));
		// Farming Traders
		buildingCategoryMap.Insert("Land_Misc_Greenhouse", new TraderSpawnInfo({"Farmers1"}, "0.0423172 -1.18584 -0.0504445", "-78.2693 0 0"));
		// Power Traders
		buildingCategoryMap.Insert("Land_Power_Station", new TraderSpawnInfo({"Power1"}, "-2.81489 -2.02424 -2.23917", "73.4016 -0.000981407 0.0197458"));
		// Mechanic Traders 
		buildingCategoryMap.Insert("Land_Repair_Center", new TraderSpawnInfo({"Mechanic1"}, "0.831973 -2.38045 0.918157", "189.581 -0.0157339 0.0304379"));
		// Workshop Traders
		buildingCategoryMap.Insert("Land_Shed_Closed", new TraderSpawnInfo({"Workshop1"}, "-2.31394 -4.60081 -2.1958", "64.3298 -0.00184021 0.0342145"));
		buildingCategoryMap.Insert("Land_Workshop3", new TraderSpawnInfo({"Workshop1"}, "-0.174282 -1.20459 -4.83973", "43.9714 0 0"));
		buildingCategoryMap.Insert("Land_Rail_Warehouse_Small", new TraderSpawnInfo({"Workshop1"}, "4.33153 -1.49387 -0.0632473", "-85.0627 -0.0185081 -0.00698397"));
		buildingCategoryMap.Insert("Land_Garage_Row_Big", new TraderSpawnInfo({"Workshop1"}, "-2.05076 -3.18141 4.4766", "-148.565 -0.000124817 -0.0279757"));
		// Building Material Traders
		buildingCategoryMap.Insert("Land_Construction_Building", new TraderSpawnInfo({"Building1"}, "-3.0187 -6.40411 0.372549", "-151.891 0 0"));

		array<Object> validBuildings = new array<Object>;
		array<string> buildingTypes = new array<string>;

		// Filter valid buildings
		foreach (Object obj : nearby)
		{
			if (obj && obj.IsInherited(Building))
			{
				string typeName = obj.GetType();
				if (buildingCategoryMap.Contains(typeName))
				{
					validBuildings.Insert(obj);
					buildingTypes.Insert(typeName);
				}
			}
		}

		if (validBuildings.Count() == 0)
		{
			Print("[CityTraderSpawn] No valid buildings found in town: " + managerNPCData.GetCityName());
			return;
		}

		// Randomly pick cityTraders number of buildings
		array<int> indices = new array<int>;
		for (int i = 0; i < validBuildings.Count(); i++) indices.Insert(i);
		for (int k = 0; k < indices.Count(); k++)
		{
			int randIndex = Math.RandomInt(k, indices.Count());
			int temp = indices[k];
			indices[k] = indices[randIndex];
			indices[randIndex] = temp;
		}


		for (int j = 0; j < Math.Min(cityTraders, indices.Count()); j++)
		{
			int idx = indices[j];
			Object building = validBuildings[idx];
			string buildingType = buildingTypes[idx];
			TraderSpawnInfo info = buildingCategoryMap.Get(buildingType);
			
			string traderCategory = info.GetRandomCategory();
			Print("[CityTraderSpawn] Spawning trader in " + buildingType + " for category: " + traderCategory);
			vector spawnPos = building.ModelToWorld(info.Offset);

			string traderClass;
			string loadout;
			GetRandomTraderClassAndLoadout(traderClass, loadout);

			Object npcObj = ExpansionGame.CreateObjectSafe(traderClass, spawnPos, false, GetGame().IsKindOf(traderClass, "DZ_LightAI"), true);
			if (!npcObj)
				continue;
			vector finalOri = building.GetOrientation() + info.Orientation;
			npcObj.SetOrientation(finalOri);
			npcObj.Update();

			if (npcObj.CanAffectPathgraph())
				GetGame().GetWorld().MarkObjectForPathgraphUpdate(npcObj);

			ExpansionTraderNPCBase traderNPC;
			if (Class.CastTo(traderNPC, npcObj))
			{
				traderNPC.LoadTrader(traderCategory);
				ExpansionHumanLoadout.Apply(traderNPC, loadout, true);
				Print("[CityTraderSpawn] Spawned " + traderClass + " in " + managerNPCData.GetCityName() + " (" + traderCategory + ")");
			}
		}
	}

	void GetRandomTraderClassAndLoadout(out string traderClass, out string loadoutName)
	{
		ref array<string> maleClasses = {
			"ExpansionTraderMirek", "ExpansionTraderDenis", "ExpansionTraderBoris", "ExpansionTraderCyril",
			"ExpansionTraderElias", "ExpansionTraderFrancis", "ExpansionTraderGuo", "ExpansionTraderHassan",
			"ExpansionTraderIndar", "ExpansionTraderJose", "ExpansionTraderKaito", "ExpansionTraderLewis",
			"ExpansionTraderManua", "ExpansionTraderNiki", "ExpansionTraderOliver", "ExpansionTraderPeter",
			"ExpansionTraderQuinn", "ExpansionTraderRolf", "ExpansionTraderSeth", "ExpansionTraderTaiki"
		};

		ref array<string> femaleClasses = {
			"ExpansionTraderLinda", "ExpansionTraderMaria", "ExpansionTraderFrida", "ExpansionTraderGabi",
			"ExpansionTraderHelga", "ExpansionTraderIrena", "ExpansionTraderJudy", "ExpansionTraderKeiko",
			"ExpansionTraderEva", "ExpansionTraderNaomi", "ExpansionTraderBaty"
		};

		bool isMale = Math.RandomInt(0, 2) == 0;

		if (isMale)
		{
			traderClass = maleClasses.GetRandomElement();
			loadoutName = "CivilianMaleLoadout";
		}
		else
		{
			traderClass = femaleClasses.GetRandomElement();
			loadoutName = "CivilianFemaleLoadout";
		}
	}





	void PlaceCityMarker(ExpansionCityManagerNPCData managerNPCData)
	{
		Class.CastTo(markerModule, CF_ModuleCoreManager.Get(ExpansionMarkerModule));
		string markerID = managerNPCData.GetID().ToString();
		ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.SERVER, markerID);
		markerData.ApplyVisibility(4);
		markerData.Set3D(0);
		markerData.SetName(managerNPCData.GetCityName());
		markerData.SetIcon("Base");
		if(managerNPCData.GetCityLiberated())
		{
			markerData.SetColor(ARGB(255, 60, 220, 60)); // Green for liberated city
		}
		else
		{
			markerData.SetColor(ARGB(255, 220, 60, 60)); // Red for non-liberated city
		}
		markerData.SetPosition(managerNPCData.GetPosition());
		GetExpansionSettings().GetMap().AddServerMarker( markerData );
	}

	//Load City Manager NPC Data from Files
	protected void LoadCityManagerNPCData(array<string> managerNPCFiles)
	{
		foreach (string file: managerNPCFiles)
		{
			GetCityManagerNPCData(file);
			
		}
	}

	protected void GetCityManagerNPCData(string fileName)
	{
		if (!GetGame().IsServer() && !GetGame().IsMultiplayer())
		{
			EXError.Error(this, "[Expansion CityManagers] Tryed to call GetCityManagerNPCData on Client!", {});
			return;
		}

		ExpansionCityManagerNPCData managerNPCData = ExpansionCityManagerNPCData.Load(fileName);
		if (managerNPCData)
		{
			if (!managerNPCData.IsActive())
			{
				EXError.Info(this, "[Expansion CityManagers] NPC with ID " + managerNPCData.GetID() + " is set to inactive. Skip..", {});
				return;
			}
			
			ExpansionCityManagerNPCData existingNPCData;
			if (!m_CityManagersNPCs.Find(managerNPCData.GetID(), existingNPCData))
			{
				EXError.Info(this, "[Expansion CityManagers] Adding NPC with ID " + managerNPCData.GetID() + " from loaded file " + fileName, {});
				managerNPCData.FILENAME = fileName;
				m_CityManagersNPCs.Insert(managerNPCData.GetID(), managerNPCData);
			}
			else
			{
				EXError.Warn(this, "[Expansion CityManagers] Can't add NPC from config File=" + fileName + ". There is already a CityManager NPC with the same ID " + managerNPCData.GetID() + " File=" + existingNPCData.FILENAME, {});
			}
		}
	}

	//Get Closest City Manager by ID Depending on Type
	//! Server

	static ExpansionCityManagerNPCBase GetClosestCityManagerNPCByID(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		ExpansionCityManagerNPCBase closestNPC;

		foreach (int npcID: ids)
		{
			ExpansionCityManagerNPCBase npc = s_CityManagerNPCEntities.Get(npcID);
			if (!npc)
				continue;

			npcPos = npc.GetPosition();
			distance = Math.Round(vector.Distance(playerPos, npcPos));
			if (shortestDistance == 0 || shortestDistance > distance)
			{
				shortestDistance = distance;
				closestNPC = npc;
			}
		}

		return closestNPC;
	}
	
	//! Server Type Object
	static ExpansionCityManagerStaticObject GetClosestCityManagerObjectByID(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		ExpansionCityManagerStaticObject closestNPC;

		foreach (int npcID: ids)
		{
			ExpansionCityManagerStaticObject npc = s_CityManagerObjectEntities.Get(npcID);
			if (!npc)
				continue;

			npcPos = npc.GetPosition();
			distance = Math.Round(vector.Distance(playerPos, npcPos));
			if (shortestDistance == 0 || shortestDistance > distance)
			{
				shortestDistance = distance;
				closestNPC = npc;
			}
		}

		return closestNPC;
	}
	
	//!Server Type AI
	static ExpansionCityManagerNPCAIBase GetClosestCityManagerNPCAIByID(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		ExpansionCityManagerNPCAIBase closestNPC;

		foreach (int npcID: ids)
		{
			ExpansionCityManagerNPCAIBase npc = s_CityManagerNPCAIEntities.Get(npcID);
			if (!npc)
				continue;

			npcPos = npc.GetPosition();
			distance = Math.Round(vector.Distance(playerPos, npcPos));
			if (shortestDistance == 0 || shortestDistance > distance)
			{
				shortestDistance = distance;
				closestNPC = npc;
			}
		}

		return closestNPC;
	}

	//City Manager Loyalty Menu Starts Here
	//! Server
	//! Request the opening of the city manager menu on the given client.
	void RequestOpenCityManagerMenuCB(array<int> managerNPCIDs, PlayerIdentity identity)
	{
		PlayerBase player = PlayerBase.GetPlayerByUID(identity.GetId());
		if (!player)
			return;
		Object target = GetClosestCityManagerNPCByID(managerNPCIDs, player.GetPosition());
		if (!target)
			target = GetClosestCityManagerObjectByID(managerNPCIDs, player.GetPosition());
	#ifdef EXPANSIONMODAI
		if (!target)
			target = GetClosestCityManagerNPCAIByID(managerNPCIDs, player.GetPosition());
	#endif

		if (!target)
		{
			EXError.Error(this, "[Expansion CityManagers] Could not find NPC object for NPC with IDs=" + managerNPCIDs.ToString());
			return;
		}

		RequestOpenCityManagerMenu(target, identity);
	}


	//! Server
	void RequestOpenCityManagerMenu(Object target, PlayerIdentity identity)
	{

		auto npc = ExpansionCityManagerNPCBase.Cast(target);
	#ifdef EXPANSIONMODAI
		auto npcAI = ExpansionCityManagerNPCAIBase.Cast(target);
	#endif
		auto npcObject = ExpansionCityManagerStaticObject.Cast(target);

			#ifdef EXPANSIONMODAI
		if (!npc && !npcAI && !npcObject)
	#else
		if (!npc && !npcObject)
	#endif
		{
			EXError.Error(this, "[Expansion CityManagers] Manager NPC object is NULL!");
			return;
		}

		int managerNPCID = -1;
		if (npc)
		{
			managerNPCID = npc.GetCityManagerNPCID();
		}
		else if (npcObject)
		{
			managerNPCID = npcObject.GetCityManagerNPCID();
		}
	#ifdef EXPANSIONMODAI
		else if (npcAI)
		{
			managerNPCID = npcAI.GetCityManagerNPCID();
		}
	#endif

		if (managerNPCID == -1)
		{
			EXError.Error(this, "[Expansion CityManagers] Could not get NPC ID from NPC object!");
			return;
		}

		ExpansionCityManagerNPCData data = GetCityManagerNPCDataByID(managerNPCID);
		data = data.Load(data.CityName);
		if (!data)
		{
			EXError.Error(this, "[Expansion CityManagers] Could not get npc data for NPC with ID=" + managerNPCID);
			return;
		}

		auto rpc = Expansion_CreateRPC("RPC_RequestOpenCityManagerMenu");
		rpc.Write(data);
		rpc.Expansion_Send(target, true, identity);

	#ifdef EXPANSIONMODAI
		if (npcAI)
		{
			npcAI.eAI_AddInteractingPlayer(identity.GetPlayer());

			EmoteManager npcEmoteManager = npcAI.GetEmoteManager();
			if (!npcEmoteManager.IsEmotePlaying())
			{
				npcEmoteManager.PlayEmote(data.NPCInteractionEmoteID);
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(npcEmoteManager.ServerRequestEmoteCancel, 2000);
			}
		}
	#endif
	}

	//! Client RPC Menu
	protected void RPC_RequestOpenCityManagerMenu(PlayerIdentity identity, Object target, ParamsReadContext ctx)
	{

		ExpansionCityManagerNPCData data;
		if (!ctx.Read(data))
		{
			Error("ExpansionCityManagerModule::RPC_SendPlayerFactionData - Could not get ExpansionCityManagerNPCData!");
			return;
		}
		
		if (!GetGame().IsDedicatedServer())
		{
			if (!OpenCityManagerMenu())
				return;
		}

		Exec_SendPlayerCityManagerData(data);
	}

	// ------------------------------------------------------------
    // Expansion Exec_SendPlayerCityManagerData
    // ------------------------------------------------------------
	private void Exec_SendPlayerCityManagerData(ExpansionCityManagerNPCData data)
	{

		if (!OpenCityManagerMenu())
		return;

		SetPlayerCityManagerData(data);
	}

	// ------------------------------------------------------------
    // Expansion SetPlayerCityManagerData
    // ------------------------------------------------------------
	private void SetPlayerCityManagerData(ExpansionCityManagerNPCData data)
	{

		m_CityManagerMenuInvoker.Invoke(data);
	}

	// ------------------------------------------------------------
    // Expansion OpenCityManagerMenu
    // ------------------------------------------------------------
    bool OpenCityManagerMenu()
    {

        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("ExpansionCityManagerMenu");

        return true;
    }

	//! Server & Client
	ExpansionCityManagerNPCData GetCityManagerNPCDataByID(int id)
	{
		ExpansionCityManagerNPCData foundData;
		if (m_CityManagersNPCs.Find(id, foundData))
			return foundData;
		Print("ExpansionCityManagerNPCData GetCityManagerNPCDataByID could not find data");
		return NULL;
	}

	// ------------------------------------------------------------
	// Expansion RequestBribeMoney
	// ------------------------------------------------------------	
	void RequestBribeMoney(int amount, int managerNPCID)
	{
		if (!GetGame().IsDedicatedServer())
		{
			auto rpc = Expansion_CreateRPC("RPC_RequestBribeMoney");
			rpc.Write(amount);
			rpc.Write(managerNPCID);
			rpc.Expansion_Send(true);
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestBribeMoney
	// ------------------------------------------------------------
	private void RPC_RequestBribeMoney(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
	{

		int amount;
		int managerNPCID;

		if (!ctx.Read(amount))
		{
			Error("ExpansionCityManagerModule::RPC_RequestBribeMoney - Could not get amount!");
			return;
		}

		if (!ctx.Read(managerNPCID))
		{
			Error("ExpansionCityManagerModule::RPC_RequestBribeMoney - Could not get managerNPCID!");
			return;
		}
		
		Exec_RequestBribeMoney(amount, managerNPCID, senderRPC);
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestBribeMoney
	// ------------------------------------------------------------
	private void Exec_RequestBribeMoney(int amount, int managerNPCID, PlayerIdentity ident)
	{
		if (!ident)
		{
			Error("Exec_RequestBribeMoney - Player identity is null.");
			return;
		}
	
		PlayerBase player = PlayerBase.GetPlayerByUID(ident.GetId());
		if (!player)
		{
			Error("Exec_RequestBribeMoney - Could not find player base.");
			return;
		}
	
		ExpansionCityManagerNPCData data = GetCityManagerNPCDataByID(managerNPCID);
		data = data.Load(data.CityName);
		if (!data)
		{
			Error("Exec_RequestBribeMoney - Could not find city manager data for ID: " + managerNPCID);
			return;
		}
	
		if (data.CityLiberated)
		{
			ExpansionNotification("Bribing Failed", "This city is already liberated and cannot be bribed.").Error(ident);
			return;
		}
	
		int currentLoyalty = data.CityCurrentLoyalty;
		int maxLoyalty = data.CityMaxReachLoyalty;
	
		int actualBribe = amount;
	
		if (currentLoyalty + amount >= maxLoyalty)
		{
			actualBribe = maxLoyalty - currentLoyalty;
			currentLoyalty = maxLoyalty;
			data.CityLiberated = true;
			
	
			ExpansionNotification(data.CityName + " Joined the Resistance", "Thanks to your efforts, the city has pledged its allegiance.").Info();

			// Pay the faction bank
			if (GetExpansionSettings().GetFactionBank().FactionBankEnabled)
				PayFactionBank(data.CityIncome, data.CityName);
			
			m_LiberatedCitiesIncome += data.CityIncome;
			Print("[Expansion CityManager] Total Liberated Cities Income: " + m_LiberatedCitiesIncome);
			//Update Marker Color
			ExpansionMapSettings mapSettings = GetExpansionSettings().GetMap();
			if (!mapSettings)
				return;

			string markerID = data.GetID().ToString();
			Print("Exec_RequestBribeMoney: Removing marker with ID: " + markerID);
			GetExpansionSettings().GetMap().RemoveServerMarker( markerID );
			Print("Exec_RequestBribeMoney: Placing marker with ID: " + markerID);
			PlaceCityMarker(data);

			// Update liberated city grids and clean up dropped items
			UpdateLiberatedCityGrids(data.Position, data.CityRadius);
			CleanupDroppedItemsInLiberatedCity(data.Position, data.CityRadius);

			// TODO: Run liberation logic:
			// - Disable city manager interaction
			// - Change traders
			// - Disable loot spawning in city radius
			// - Spawn resistance objects
		}
		else
		{
			currentLoyalty += actualBribe;
		}
	
		data.CityCurrentLoyalty = currentLoyalty;
		data.Save(data.CityName);  // Save using city name as filename
	
		
		int m_NewPlayerMoney = DeleteUSDNotes(player, actualBribe);
		Print("Exec_RequestBribeMoney: m_NewPlayerMoney = " + m_NewPlayerMoney);
		ConfirmBribeMoney(ident, data, currentLoyalty, m_NewPlayerMoney);
	}


		
	// ------------------------------------------------------------
	// Expansion ConfirmBribeMoney
	// ------------------------------------------------------------	
	void ConfirmBribeMoney(PlayerIdentity ident, ExpansionCityManagerNPCData data, int m_CurrentLoyalty, int m_NewPlayerMoney)
	{
		auto rpc = Expansion_CreateRPC("RPC_ConfirmBribeMoney");
		rpc.Write(data);
		rpc.Write(m_CurrentLoyalty);
		rpc.Write(m_NewPlayerMoney);
		Print("ConfirmBribeMoney: m_NewPlayerMoney = " + m_NewPlayerMoney);
		rpc.Expansion_Send(true, ident);
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmBribeMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmBribeMoney(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
	{
		ExpansionCityManagerNPCData data;
		if (!ctx.Read(data))
		{
			Error("ExpansionCityManagerModule::RPC_ConfirmBribeMoney - Could not get City Manager data!");
			return;
		}
		int m_CurrentLoyalty;
		if (!ctx.Read(m_CurrentLoyalty))
		{
			Error("ExpansionCityManagerModule::RPC_ConfirmBribeMoney - Could not get Current loyalty!");
			return;
		}

		int m_NewPlayerMoney;
		if (!ctx.Read(m_NewPlayerMoney))
		{
			Error("ExpansionCityManagerModule::RPC_ConfirmBribeMoney - Could not get Player USD Money!");
			return;
		}
		Print("RPC_ConfirmBribeMoney: m_NewPlayerMoney = " + m_NewPlayerMoney);
		Exec_ConfirmBribeMoney(data, m_CurrentLoyalty, m_NewPlayerMoney);
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmBribeMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmBribeMoney(ExpansionCityManagerNPCData data, int m_CurrentLoyalty, int m_NewPlayerMoney)
	{
		Print("Exec_ConfirmBribeMoney: m_NewPlayerMoney = " + m_NewPlayerMoney);
		CallbackPlayerCityManagerData(data, m_CurrentLoyalty, m_NewPlayerMoney);
	}

	private void CallbackPlayerCityManagerData(ExpansionCityManagerNPCData data, int m_CurrentLoyalty, int m_NewPlayerMoney)
	{
		Print("CallbackPlayerCityManagerData: m_NewPlayerMoney = " + m_NewPlayerMoney);
		m_CityManagerMenuCallbackInvoker.Invoke(data, m_CurrentLoyalty, m_NewPlayerMoney);
	}

	int DeleteUSDNotes(PlayerBase player, int amountToDelete)
	{
		if (!player || amountToDelete <= 0)
			return 0;
	
		int deleted = 0;
	
		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
	
		foreach (EntityAI item : items)
		{
			if (deleted >= amountToDelete)
				break;
	
			string moneycurrency = "ExpansionBanknoteUSD";
			ItemBase note;
			if (Class.CastTo(note, item) && note.GetType() == moneycurrency)
			{
				if (!MiscGameplayFunctions.Expansion_IsLooseEntity(note))
					continue;
	
				int quantity = note.GetQuantity();
				int needed = amountToDelete - deleted;
	
				if (quantity > needed)
				{
					note.SetQuantity(quantity - needed);
					deleted += needed;
				}
				else
				{
					deleted += quantity;
					GetGame().ObjectDelete(note);
				}
			}
		}
	
		int m_NewPlayerMoney = GetPlayerMoney(player);
		return m_NewPlayerMoney;
		
	}
	
	int GetPlayerMoney(PlayerBase player)
	{
		if (!player)
			return 0;
	
		array<int> monies = new array<int>();
		TStringArray usdcurrency = {"expansionbanknoteusd"};
		if (!m_MarketModule)
		 	m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
		return m_MarketModule.GetPlayerWorth(player, monies, usdcurrency) / 100;
	}
	
	
	

	ScriptInvoker GetCityManagerCallbackMenuSI()
	{
		return m_CityManagerMenuCallbackInvoker;
	}

	ScriptInvoker GetCityManagerMenuSI()
	{
		return m_CityManagerMenuInvoker;
	}

	//Get Instance
	static ExpansionCityManagerModule GetModuleInstance()
	{
		return s_ModuleInstance;
	}

	bool isCityLiberated(int cityID)
	{
		Print("CityManagerModule: Checking cityID = " + cityID);
		ExpansionCityManagerNPCData data = GetCityManagerNPCDataByID(cityID);

		if (!data)
		{
			Print("CityManagerModule: No data found for cityID = " + cityID);
			return false;
		}

		Print("CityManagerModule: Found data for city = " + data.CityName + " (Liberated = " + data.CityLiberated + ")");
		return data.CityLiberated;
	}

		
	void CleanupDroppedItemsInLiberatedCity(vector pos, float radius)
	{
		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition(pos, radius, objects, null);
		
		foreach (Object obj : objects)
		{
			ItemBase item;
			if (Class.CastTo(item, obj) && item.IsInherited(ItemBase) && !item.GetHierarchyParent())
			{
				item.ExpansionCreateLiberatedCityCleanup();
			}
		}
	}

	void UpdateLiberatedCityGrids(vector pos, float radius)
	{
		if (!m_LiberatedCityGrids)
			m_LiberatedCityGrids = new map<string, bool>();

		// Calculate grid bounds based on position and radius
		int x1 = Math.Floor((pos[0] - radius) / 1000);
		int x2 = Math.Floor((pos[0] + radius) / 1000);
		int z1 = Math.Floor((pos[2] - radius) / 1000);
		int z2 = Math.Floor((pos[2] + radius) / 1000);

		for (int gx = x1; gx <= x2; gx++)
		{
			for (int gz = z1; gz <= z2; gz++)
			{
				string key = gx.ToString() + "_" + gz.ToString();
				m_LiberatedCityGrids.Set(key, true);
			}
		}
	}

	bool IsInsideLiberatedCity(vector pos)
	{
		if (!m_LiberatedCityGrids)
			return false;

		int gx = Math.Floor(pos[0] / 1000);
		int gz = Math.Floor(pos[2] / 1000);
		string key = gx.ToString() + "_" + gz.ToString();

		return m_LiberatedCityGrids.Contains(key);
	}

	void PayFactionBank(int amount, string cityName)
	{
		if (!GetGame().IsServer() && !GetGame().IsMultiplayer())
		{
			Error("PayFactionBank - This function can only be called on the server!");
			return;
		}

		auto settings = GetExpansionSettings().GetFactionBank();
		settings.FactionBankBalance += amount;
		settings.Save();
		ExpansionNotification("Resistance Bank", cityName + " paid the Resistance the daily income of $" + amount).Info();
		return;
	}

	void PayRecurringIncome()
	{
		if (!GetGame().IsServer())
			return;

		if (m_LiberatedCitiesIncome > 0)
		{
			PayFactionBank(m_LiberatedCitiesIncome, "All Liberated Cities");
		}
	}

	int GetLiberatedCitiesIncome()
	{
		Print("[Expansion CityManagerModule] GetLiberatedCitiesIncome called, returning: " + m_LiberatedCitiesIncome);
		return m_LiberatedCitiesIncome;
	}
}
class TraderSpawnInfo
{
    ref array<string> Categories;
    vector Offset;
    vector Orientation;

    void TraderSpawnInfo(array<string> categories, vector offset, vector ori)
    {
        Categories = categories;
        Offset = offset;
        Orientation = ori;
    }

    string GetRandomCategory()
    {
        return Categories.GetRandomElement();
    }
}

