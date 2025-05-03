[CF_RegisterModule(ExpansionCityManagerModule)]
class ExpansionCityManagerModule: CF_ModuleWorld
{
    static ref map<int, ExpansionCityManagerNPCBase> s_CityManagerNPCEntities = new map<int, ExpansionCityManagerNPCBase>; //! Server & Client
    static ref map<int, ExpansionCityManagerStaticObject> s_CityManagerObjectEntities = new map<int, ExpansionCityManagerStaticObject>; //! Server & Client
    protected ref map<int, ref ExpansionCityManagerNPCData> m_CityManagersNPCs; //! Server
	protected ref ExpansionMarketModule m_MarketModule;


    #ifdef EXPANSIONMODAI
	static ref map<int, ExpansionCityManagerNPCAIBase> s_CityManagerNPCAIEntities = new map<int, ExpansionCityManagerNPCAIBase>; //! Server & Client
    #endif

    protected ref map<int, ExpansionCityManagerNPCBase> m_CityManagerNPCs;
    protected static ExpansionCityManagerModule s_ModuleInstance;

	protected ref ScriptInvoker m_CityManagerMenuInvoker; //! Client
	protected ref ScriptInvoker m_CityManagerMenuCallbackInvoker; //! Client	
	static ref ScriptInvoker SI_CityManagerMenuCallback = new ScriptInvoker();

	#ifdef EXPANSIONMODNAVIGATION
   //declares
	ExpansionMarkerModule m_MarkerModule;
	ExpansionMarkerData m_ServerMarker;
	#endif

    void ExpansionCityManagerModule()
	{
        s_ModuleInstance = this;

		if (!m_MarketModule)
		m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
		
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

		foreach (int id, ExpansionCityManagerNPCData managerNPCData: m_CityManagersNPCs)
		{
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

					#ifdef EXPANSIONMODNAVIGATION
					if (CF_Modules<ExpansionMarkerModule>.Get(m_MarkerModule))
				{
					m_ServerMarker = m_MarkerModule.CreateServerMarker(managerNPCData.GetCityName(), "Territory", managerNPCData.GetPosition(), ARGB(255, 235, 59, 90), true);
				}
				#endif
			}
		}
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

		auto npcAI = ExpansionCityManagerNPCAIBase.Cast(target);

		int managerNPCID = -1;
		managerNPCID = npcAI.GetCityManagerNPCID();

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
			
	
			ExpansionNotification(data.CityName + " Joined the Resistance", "Thanks to your efforts, the city has pledged its allegiance.").Info(ident);
	
			// TODO: Run liberation logic:
			// - Disable city manager interaction
			// - Change traders
			// - Disable loot spawning in city radius
			// - Spawn resistance objects
			// - Enable bus/train travel
			// - Show city marker joined resistance
			// - Show notification joined resistance
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

	
	
	

	
}