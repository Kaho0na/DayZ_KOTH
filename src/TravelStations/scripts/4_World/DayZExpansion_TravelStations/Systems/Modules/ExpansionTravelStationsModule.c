[CF_RegisterModule(ExpansionTravelStationsModule)]
class ExpansionTravelStationsModule: CF_ModuleWorld
{

	static ref map<int, ExpansionTravelStationsNPCBase> s_TravelStationsNPCEntities = new map<int, ExpansionTravelStationsNPCBase>;
	static ref map<int, ExpansionTravelStationsStaticObject> s_TravelStationsObjectEntities = new map<int, ExpansionTravelStationsStaticObject>;

	#ifdef EXPANSIONMODAI
	static ref map<int, ExpansionTravelStationsNPCAIBase> s_TravelStationsNPCAIEntities = new map<int, ExpansionTravelStationsNPCAIBase>;
	#endif

	protected ref map<int, ref ExpansionTravelStationsNPCData> m_TravelStationsNPCs;
	protected static ExpansionTravelStationsModule s_ModuleInstance;

	protected ref ScriptInvoker m_TravelStationMenuInvoker; //! Client

	protected ref ExpansionTravelStationsNPCData m_ActiveTravelStationData;



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
	
		return loc;
	}
	

	array<ref TravelStationLocation> BuildTravelStationLocationList(bool requireLiberated = true)
	{
		Print("[ExpansionTravelStationsModule] Building travel station location list...");
	
		array<ref TravelStationLocation> locations = new array<ref TravelStationLocation>();
	
		if (!m_TravelStationsNPCs || m_TravelStationsNPCs.Count() == 0)
		{
			Print("[ExpansionTravelStationsModule] No NPC data available in m_TravelStationsNPCs!");
			return locations;
		}
	
		foreach (int id, ExpansionTravelStationsNPCData npcData : m_TravelStationsNPCs)
		{
			if (!npcData)
			{
				Print("[ExpansionTravelStationsModule] NPC data null for ID: " + id);
				continue;
			}
	
			if (requireLiberated && !npcData.IsActive())
			{
				Print("[ExpansionTravelStationsModule] Skipping station " + npcData.GetStationName() + " (not active)");
				continue;
			}
	
			ref TravelStationLocation loc = ConvertNPCDataToTravelStationLocation(npcData);
			loc.SetTPPositions(loc.Position);  
	
			Print("[ExpansionTravelStationsModule] Added station: " + loc.StationName + " with " + loc.TPPositions.Count() + " TP points");
			locations.Insert(loc);
		}
	
		Print("[ExpansionTravelStationsModule] Final station count: " + locations.Count());
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
		ExpansionMarketModule module = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
		if (module && identity)
		{
			ref ExpansionMarketATM_Data m_ATMData = module.GetPlayerATMData(identity.GetId());
			if (m_ATMData)
			{
				int PlayerMoney = m_ATMData.MoneyDeposited;
			}
			else
			{
				Print("[TravelStations] ATM data not found for player: " + identity.GetId());
			}
		}

		auto npcAI = ExpansionTravelStationsNPCAIBase.Cast(target);
		int npcID = npcAI.GetTravelStationsNPCID();
		Print("[TravelStations] npcID = " + npcID);
		if (npcID == -1)
		{
			EXError.Error(this, "[Expansion TravelStations] Invalid NPC ID!");
			return;
		}
	
		ExpansionTravelStationsNPCData data = GetTravelStationsNPCDataByID(npcID);
		Print("[TravelStations] Station Name = " + data.StationName);
		if (!data)
		{
			EXError.Error(this, "[Expansion TravelStations] Could not load NPC data for ID=" + npcID);
			return;
		}
	
		// Send all stations that match
		array<ref TravelStationLocation> stationslist = BuildTravelStationLocationList(false); // send all, not just active?
		
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
		Print("[Travel Stations] Creating Menu ExpansionTravelStationsMenu")
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("ExpansionTravelStationsMenu");

        return true;
    }

	ScriptInvoker GetTravelStationMenuSI()
	{
		Print("[Travel Stations] GetTravelStationMenuSI Invoker is being fired")
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
	void SendTeleportRequest(vector pos, int TravelCost, int timeToTravel)
	{
		auto rpc = Expansion_CreateRPC("RPC_TeleportPlayerTo");
		rpc.Write(pos);
		rpc.Write(TravelCost);
		rpc.Write(timeToTravel);
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

		int timeToTravel;
		if (!ctx.Read(timeToTravel))
		{
			Error("[TravelStations] Failed to read travel time!");
			return;
		}

		PlayerBase player = PlayerBase.GetPlayerByUID(sender.GetId());
		if (!player)
		{
			Error("[TravelStations] Server: Player not found for identity " + sender.GetId());
			return;
		}
		ExpansionMarketModule module = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
		if (module)
		{
			ExpansionMarketATM_Data data = module.GetPlayerATMData(sender.GetId());
			if (!data)
			{
				Error("ExpansionMarketModule::Exec_RequestDonateMoney - Could not find player atm data!");
				
				return;
			}
			data.RemoveMoney(TravelCost);
			data.Save();
		}

		Print("[TravelStations] Travel delay: " + timeToTravel + "s");

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TeleportPlayerAfterDelay, timeToTravel * 1000, false, player, pos);

	}

	void TeleportPlayerAfterDelay(PlayerBase player, vector pos)
	{
		if (!player) return;

		Print("[TravelStations] Teleporting player to: " + pos);
		player.SetPosition(pos);
	}
		
	

	static ExpansionTravelStationsModule GetModuleInstance() { return s_ModuleInstance; }
}
