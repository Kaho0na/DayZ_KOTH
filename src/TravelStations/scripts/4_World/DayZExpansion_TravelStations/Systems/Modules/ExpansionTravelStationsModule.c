[CF_RegisterModule(ExpansionTravelStationsModule)]
class ExpansionTravelStationsModule: CF_ModuleWorld
{

	protected static ExpansionTravelStationsModule s_ModuleInstance;
	protected ref ExpansionMarketModule m_marketModule;
	protected ref ExpansionCityManagerModule m_cityManagerModule;
	static ref map<int, ExpansionTravelStationsNPCBase> s_TravelStationsNPCEntities = new map<int, ExpansionTravelStationsNPCBase>;
	static ref map<int, ExpansionTravelStationsStaticObject> s_TravelStationsObjectEntities = new map<int, ExpansionTravelStationsStaticObject>;
	private ref map<string, int> m_GlobalTravelCooldown = new map<string, int>();
	#ifdef EXPANSIONMODAI
	static ref map<int, ExpansionTravelStationsNPCAIBase> s_TravelStationsNPCAIEntities = new map<int, ExpansionTravelStationsNPCAIBase>;
	#endif

	protected ref map<int, ref ExpansionTravelStationsNPCData> m_TravelStationsNPCs;
	protected ref ExpansionTravelStationsNPCData m_ActiveTravelStationData;
	
	protected ref ScriptInvoker m_TravelStationMenuInvoker; //! Client



	void ExpansionTravelStationsModule()
	{
		s_ModuleInstance = this;
		m_TravelStationMenuInvoker = new ScriptInvoker(); //! Client
		m_TravelStationsNPCs = new map<int, ref ExpansionTravelStationsNPCData>();
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

		Print("[TravelStations] OnInit - Registering RPC on CLIENT");
		Expansion_EnableRPCManager();
		Expansion_RegisterClientRPC("RPC_RequestOpenTravelStationsMenu");
		Expansion_RegisterServerRPC("RPC_TeleportPlayerTo");

	}
	

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);
		if (GetGame().IsServer() && GetGame().IsMultiplayer())
			ServerModuleInit();

		if (!m_marketModule)
		{
			m_marketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
			if (!m_marketModule)
				Print("[TravelStations] Failed to get ExpansionMarketModule!");
			else
				Print("[TravelStations] ExpansionMarketModule found.");
		}

		if (!m_cityManagerModule)
		{
			m_cityManagerModule = ExpansionCityManagerModule.Cast(CF_ModuleCoreManager.Get(ExpansionCityManagerModule));
			if (!m_cityManagerModule)
				Print("[TravelStations] Failed to get ExpansionCityManagerModule!");
			else
				Print("[TravelStations] ExpansionCityManagerModule found.");
		}

	}



	float GetYawFromDirection(vector dir)
	{
		return Math.Atan2(dir[0], dir[2]) * Math.RAD2DEG;
	}

	void ServerModuleInit()
	{
		Print("[ExpansionTravelStationsModule] ServerModuleInit called.");
	
		if (FileExist(EXPANSION_TRAVELSTATIONS_FOLDER))
		{
			Print("[ExpansionTravelStationsModule] TravelStations folder exists: " + EXPANSION_TRAVELSTATIONS_FOLDER);
	
			array<string> managerNPCFiles = ExpansionStatic.FindFilesInLocation(EXPANSION_TRAVELSTATIONS_FOLDER, ".json");
	
			if (managerNPCFiles && managerNPCFiles.Count() > 0)
			{
				Print("[ExpansionTravelStationsModule] Found " + managerNPCFiles.Count() + " TravelStation config files:");
				foreach (string file : managerNPCFiles)
				{
					Print(" - " + file);
				}
	
				LoadTravelStationsNPCData(managerNPCFiles);
				SpawnTravelStationsNPCs();  // Important: make sure this is only called *after* loading
			}
			else
			{
				Print("[ExpansionTravelStationsModule] No JSON files found in: " + EXPANSION_TRAVELSTATIONS_FOLDER);
			}
		}
		else
		{
			Print("[ExpansionTravelStationsModule] TravelStations folder does not exist. Creating...");
			MakeDirectory(EXPANSION_TRAVELSTATIONS_FOLDER);
		}
	}
	

	static ExpansionTravelStationsNPCBase GetTravelStationsNPCByID(int id)
	{
		return s_TravelStationsNPCEntities.Get(id);
	}

    static void AddTravelStationsNPC(int id, ExpansionTravelStationsNPCBase TravelStationsNPC)
	{
		if (!s_TravelStationsNPCEntities[id])  //! Can be NULL if object was deleted because it was no longer in network bubble
			s_TravelStationsNPCEntities[id] = TravelStationsNPC;

	}

    static void AddStaticTravelStationsObject(int id, ExpansionTravelStationsStaticObject staticQustObject)
	{
		if (!s_TravelStationsObjectEntities[id])  //! Can be NULL if object was deleted because it was no longer in network bubble
			s_TravelStationsObjectEntities[id] = staticQustObject;
	}

    static void AddTravelStationsNPCAI(int id, ExpansionTravelStationsNPCAIBase TravelStationsNPCAI)
	{
		if (!s_TravelStationsNPCAIEntities[id])  //! Can be NULL if object was deleted because it was no longer in network bubble
			s_TravelStationsNPCAIEntities[id] = TravelStationsNPCAI;
	}

	void LoadTravelStationsNPCData(array<string> managerNPCFiles)
	{
		foreach (string file: managerNPCFiles)
			GetTravelStationsNPCData(file);
	}

	void GetTravelStationsNPCData(string fileName)
	{
		ExpansionTravelStationsNPCData managerNPCData = ExpansionTravelStationsNPCData.Load(fileName);
		if (!managerNPCData || !managerNPCData.IsActive()) return;
		if (!m_TravelStationsNPCs.Contains(managerNPCData.GetStationID()))
		{
			managerNPCData.FILENAME = fileName;
			m_TravelStationsNPCs.Insert(managerNPCData.GetStationID(), managerNPCData);
			Print("[ExpansionTravelStations] Loaded NPC Data: " + managerNPCData.GetStationName());
		}
	}

	void SpawnTravelStationsNPCs()
	{
		Print("[ExpansionTravelStations] Spawning all configured Travel Station NPCs...");
		foreach (int id, ExpansionTravelStationsNPCData managerNPCData: m_TravelStationsNPCs)
		{
			Print("[ExpansionTravelStations] Attempting to spawn NPC ID=" + id + ", Type=" + managerNPCData.GetNPCType());
			switch (managerNPCData.GetNPCType())
			{
				case ExpansionTravelStationsNPCType.NORMAL:
					ExpansionTravelStationsNPCBase npc = managerNPCData.SpawnNPC();
					if (npc)
					{
						npc.SetTravelStationsNPCID(id);
						npc.SetTravelStationsNPCData(managerNPCData);
						npc.SetPosition(managerNPCData.GetPosition());
						npc.SetOrientation(managerNPCData.GetOrientation());
						Print("[ExpansionTravelStations] Spawned NORMAL NPC: " + managerNPCData.GetStationName());
					}
					break;

				case ExpansionTravelStationsNPCType.OBJECT:
					ExpansionTravelStationsStaticObject obj = managerNPCData.SpawnObject();
					if (obj)
					{
						obj.SetTravelStationsNPCID(id);
						obj.SetTravelStationsNPCData(managerNPCData);
						obj.SetPosition(managerNPCData.GetPosition());
						obj.SetOrientation(managerNPCData.GetOrientation());
						Print("[ExpansionTravelStations] Spawned OBJECT NPC: " + managerNPCData.GetStationName());
					}
					break;

				#ifdef EXPANSIONMODAI
				case ExpansionTravelStationsNPCType.AI:
					ExpansionTravelStationsNPCAIBase ai = managerNPCData.SpawnNPCAI();
					if (ai)
					{
						ai.SetTravelStationsNPCID(id);
						ai.SetTravelStationsNPCData(managerNPCData);
						ai.Expansion_SetEmote(managerNPCData.GetEmoteID(), !managerNPCData.IsEmoteStatic());
						Print("[ExpansionTravelStations] Spawned AI NPC: " + managerNPCData.GetStationName());
					}
					break;
				#endif
			}
		}
	}

	ref TravelStationLocation ConvertNPCDataToTravelStationLocation(ExpansionTravelStationsNPCData npcData)
	{
		Print("[ExpansionTravelStationsModule] Converting NPC data to TravelStationLocation for: " + npcData.GetStationName());
	
		ref TravelStationLocation loc = new TravelStationLocation();
		loc.CityID = npcData.GetCityID();
		loc.StationName = npcData.GetStationName();
		loc.StationID = npcData.GetStationID();
		loc.Position = npcData.GetPosition();
		loc.SetTPPositions(loc.Position);  // Generate TPPositions based on Position
		loc.IsLiberated = checkIfLiberated(loc.CityID);
		loc.StationType = npcData.GetStationType();

		return loc;
	}
	

	array<ref TravelStationLocation> BuildTravelStationLocationList()
	{
	
		array<ref TravelStationLocation> locations = new array<ref TravelStationLocation>();
	
		if (!m_TravelStationsNPCs || m_TravelStationsNPCs.Count() == 0)
		{
			return locations;
		}
	
		foreach (int id, ExpansionTravelStationsNPCData npcData : m_TravelStationsNPCs)
		{
			if (!npcData)
			{
				continue;
			}
	
			if (!npcData.IsActive())
			{
				continue;
			}
	
			ref TravelStationLocation loc = ConvertNPCDataToTravelStationLocation(npcData);
			loc.SetTPPositions(loc.Position);  
	
			locations.Insert(loc);
		}
	
		return locations;
	}
	
	

	map<int, ref ExpansionTravelStationsNPCData> GetAllNPCData()
	{
		return m_TravelStationsNPCs;
	}



	//Get Closest Station Manager by ID Depending on Type
	//! Server

	static ExpansionTravelStationsNPCBase GetClosestTravelStationsNPCByID(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		ExpansionTravelStationsNPCBase closestNPC;

		foreach (int npcID: ids)
		{
			ExpansionTravelStationsNPCBase npc = s_TravelStationsNPCEntities.Get(npcID);
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

	//! Server & Client get NPC Data
	ExpansionTravelStationsNPCData GetTravelStationsNPCDataByID(int id)
	{
		ExpansionTravelStationsNPCData foundData;
		if (m_TravelStationsNPCs.Find(id, foundData))
			return foundData;

		return NULL;
	}
	
	//! Server Type Object
	static ExpansionTravelStationsStaticObject GetClosestTravelStationsObjectByID(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		ExpansionTravelStationsStaticObject closestNPC;

		foreach (int npcID: ids)
		{
			ExpansionTravelStationsStaticObject npc = s_TravelStationsObjectEntities.Get(npcID);
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
	static ExpansionTravelStationsNPCAIBase GetClosestTravelStationsNPCAIByID(array<int> ids, vector playerPos)
	{
		float shortestDistance;
		float distance;
		vector npcPos;
		ExpansionTravelStationsNPCAIBase closestNPC;

		foreach (int npcID: ids)
		{
			ExpansionTravelStationsNPCAIBase npc = s_TravelStationsNPCAIEntities.Get(npcID);
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

	// --- Menu Related ---

	void RequestOpenTravelStationsMenuCB(array<int> npcIDs, PlayerIdentity identity)
	{
		PlayerBase player = PlayerBase.GetPlayerByUID(identity.GetId());
		if (!player) return;

		Object target = GetClosestTravelStationsNPCByID(npcIDs, player.GetPosition());
		if (!target) target = GetClosestTravelStationsObjectByID(npcIDs, player.GetPosition());
		#ifdef EXPANSIONMODAI
		if (!target) target = GetClosestTravelStationsNPCAIByID(npcIDs, player.GetPosition());
		#endif
		if (!target)
		{
			EXError.Error(this, "[Expansion TravelStations] Could not find NPC for IDs: " + npcIDs.ToString());
			return;
		}

		RequestOpenTravelStationsMenu(target, identity);
	}


	void RequestOpenTravelStationsMenu(Object target, PlayerIdentity identity)
	{
		//Get NPC Data

		auto npc = ExpansionTravelStationsNPCBase.Cast(target);
	#ifdef EXPANSIONMODAI
		auto npcAI = ExpansionTravelStationsNPCAIBase.Cast(target);
	#endif
		auto npcObject = ExpansionTravelStationsStaticObject.Cast(target);

	#ifdef EXPANSIONMODAI
		if (!npc && !npcAI && !npcObject)
	#else
		if (!npc && !npcObject)
	#endif
		{
			EXError.Error(this, "[Expansion TravelStations] Travel Station NPC object is NULL!");
			return;
		}

		int npcID = -1;
		if (npc)
		{
			npcID = npc.GetTravelStationsNPCID();
		}
		else if (npcObject)
		{
			npcID = npcObject.GetTravelStationsNPCID();
		}
	#ifdef EXPANSIONMODAI
		else if (npcAI)
		{
			npcID = npcAI.GetTravelStationsNPCID();
		}
	#endif

		if (npcID == -1)
		{
			EXError.Error(this, "[Expansion TravelStations] Could not get quest NPC ID from quest NPC object!");
			return;
		}

		ExpansionTravelStationsNPCData data = GetTravelStationsNPCDataByID(npcID);

		if (!data)
		{
			EXError.Error(this, "[Expansion TravelStations] Could not load NPC data for ID=" + npcID);
			return;
		}

		//Check NPC Data for City Liberation
		int m_cityID = data.CityID;
		Print("RequestOpenTravelStationsMenu: data.CityID = " + m_cityID);
		bool m_isLiberated = checkIfLiberated(m_cityID);
		Print("RequestOpenTravelStationsMenu: checkIfLiberated(m_cityID) = " + m_isLiberated);
		if(!m_isLiberated)
		{
			TStringArray blockMessages = {
				"Travel to this city is currently prohibited. Hostile occupation forces have imposed a lockdown.",
				"This city remains under enemy control. Resistance fighters advise against entering.",
				"Entry into this zone is restricted. Unauthorized travel may result in arrest or worse.",
				"The roads ahead are blocked by checkpoints and patrols. Better to wait until liberation.",
				"City is under occupation. Travel is not allowed."
			};
			
			int index = Math.RandomInt(0, blockMessages.Count());
			ExpansionNotification("Travel Blocked",	blockMessages[index]).Error(identity);
			return;
		}

		//Check Cooldown
		string uid = identity.GetId();
		int now = GetGame().GetTime();
		int cooldownDuration = 60000; // 5 seconds

		if (m_GlobalTravelCooldown.Contains(uid))
		{
			int lastTravelTime = m_GlobalTravelCooldown.Get(uid);

			if (now - lastTravelTime < cooldownDuration)
			{
				int remaining = (cooldownDuration - (now - lastTravelTime)) / 1000;
				ExpansionNotification("Travel Cooldown", "You must wait " + remaining + " seconds before traveling again.").Error(identity);
				return;
			}
		}
		
		//Get Player ATM Data
		int PlayerMoney;
		Print("[TravelStations] m_marketModule: " + m_marketModule);
		Print("[TravelStations] identity: " + identity);
		if (m_marketModule && identity)
		{
			ref ExpansionMarketATM_Data m_ATMData = m_marketModule.GetPlayerATMData(identity.GetId());
			if (m_ATMData)
			{
				PlayerMoney = m_ATMData.MoneyDeposited;
				Print("[TravelStations] Player Money Server: " + PlayerMoney);
			}
			else
			{
				Print("[TravelStations] ATM data not found for player: " + identity.GetId());
			}
		}
	
		// Send all stations that match
		array<ref TravelStationLocation> stationslist = BuildTravelStationLocationList(); // send all, not just active?
		
		//Create RPC
		auto rpc = Expansion_CreateRPC("RPC_RequestOpenTravelStationsMenu");
		rpc.Write(PlayerMoney);
		rpc.Write(data); // active NPC station
		Print("[TravelStations] data = " + data);
		rpc.Write(stationslist.Count());
		Print("[TravelStations] stationslist.Count() = " + stationslist.Count());
	
		foreach (TravelStationLocation tplocation: stationslist)
		{
			tplocation.OnSend(rpc);
		}
		Print("[TravelStations] Sending RPC");
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

	protected void RPC_RequestOpenTravelStationsMenu(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		int PlayerMoney;
		if (!ctx.Read(PlayerMoney))
		{
			Error("[TravelStations] Failed to read NPC data!");
			return;
		}
		
		Print("[TravelStations] RPC_RequestOpenTravelStationsMenu called");
		ExpansionTravelStationsNPCData npcData;
		if (!ctx.Read(npcData))
		{
			Error("[TravelStations] Failed to read NPC data!");
			return;
		}
	
		int stationCount;
		if (!ctx.Read(stationCount))
		{
			Error("[TravelStations] Failed to read station count!");
			return;
		}
	
		array<ref TravelStationLocation> stationslist = {};
		for (int i = 0; i < stationCount; i++)
		{
			TravelStationLocation loc = new TravelStationLocation();
			if (!loc.OnReceive(ctx))
			{
				Error("[TravelStations] Failed to read station location at index " + i);
				continue;
			}
			stationslist.Insert(loc);
		}
	
		// Make sure something was received
		if (!stationslist || !stationslist.Count())
		{
			Error("[TravelStations] No travel stations received!");
			return;
		}

		if (!GetGame().IsDedicatedServer())
		{
			if (!OpenTravelStationsMenu())
				return;
		}

		// Open menu via call queue
		Exec_ShowTravelStationsMenu(PlayerMoney, npcData, stationslist);
	}

	private void Exec_ShowTravelStationsMenu(int PlayerMoney, ExpansionTravelStationsNPCData npcData, array<ref TravelStationLocation> stations)
	{
		if (!OpenTravelStationsMenu())
		return;

		Print("[TravelStations] Invoking " + npcData + stations);
		Print("[TravelStations] Exec_ShowTravelStationsMenu() stations count: " + stations.Count());
		m_TravelStationMenuInvoker.Invoke(PlayerMoney, npcData, stations);
	}

	bool OpenTravelStationsMenu()
    {
		Print("[Travel Stations] Creating Menu ExpansionTravelStationsMenu");
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("ExpansionTravelStationsMenu");

        return true;
    }

	ScriptInvoker GetTravelStationMenuSI()
	{
		Print("[Travel Stations] GetTravelStationMenuSI Invoker is being fired");
		return m_TravelStationMenuInvoker;
	}

	void SetActiveTravelStationData(ExpansionTravelStationsNPCData data)
	{
		m_ActiveTravelStationData = data;
	}

	ExpansionTravelStationsNPCData GetActiveTravelStationData()
	{
		return m_ActiveTravelStationData;
	}

	//Server Side Teleport Player
	void SendTeleportRequest(vector pos, int TravelCost)
	{
		auto rpc = Expansion_CreateRPC("RPC_TeleportPlayerTo");
		rpc.Write(pos);
		rpc.Write(TravelCost);
		rpc.Expansion_Send(null, true, null);  // client → server
	}
	
	
	void RPC_TeleportPlayerTo(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		vector pos;
		if (!ctx.Read(pos))
		{
			Error("[TravelStations] Failed to read position!");
			return;
		}
		
		int TravelCost;
		if (!ctx.Read(TravelCost))
		{
			Error("[TravelStations] Failed to read position!");
			return;
		}

		PlayerBase player = PlayerBase.GetPlayerByUID(sender.GetId());
		if (!player)
		{
			Error("[TravelStations] Server: Player not found for identity " + sender.GetId());
			return;
		}
		if (m_marketModule)
		{
			ExpansionMarketATM_Data data = m_marketModule.GetPlayerATMData(sender.GetId());
			if (!data)
			{
				Error("ExpansionTravelStationsModule::RPC_TeleportPlayerTo - Could not find player atm data!");
				
				return;
			}
			data.RemoveMoney(TravelCost);
			data.Save();
		}


		player.SetPosition(pos);
		int travelNowTime = GetGame().GetTime();
		int globalCooldown = travelNowTime + 10000;
		m_GlobalTravelCooldown.Set(sender.GetId(), globalCooldown);

	}

	bool checkIfLiberated(int cityID)
	{
		ExpansionCityManagerNPCData data = m_cityManagerModule.GetCityManagerNPCDataByID(cityID);
		ExpansionCityManagerNPCData newData = ExpansionCityManagerNPCData.Load(data.CityName);
		if (!newData)
    		return false;
		Print("[TravelStations] Checking CityID: " + cityID + " → Liberated: " + data.CityLiberated);
		return newData.CityLiberated; 
	}




	static ExpansionTravelStationsModule GetModuleInstance() { return s_ModuleInstance; }
}
