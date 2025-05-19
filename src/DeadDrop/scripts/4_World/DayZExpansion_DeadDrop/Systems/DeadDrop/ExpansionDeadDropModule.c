[CF_RegisterModule(ExpansionDeadDropModule)]
class ExpansionDeadDropModule: CF_ModuleWorld
{
    protected static ExpansionDeadDropModule s_ModuleInstance;
	protected ExpansionDeadDropNPCAIBase m_DeadDropNPC;
	protected ref ScriptInvoker m_DeadDropMenuInvoker = new ScriptInvoker();
	protected ref ExpansionMarketModule m_marketModule;


    void ExpansionDeadDropModule()
	{
        s_ModuleInstance = this;
    }

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);

		if (GetGame().IsServer() && GetGame().IsMultiplayer())
			SpawnDeadDropNPC();

		if (!m_marketModule)
		{
			m_marketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
			if (!m_marketModule)
				Print("[DeadDrop] Failed to get ExpansionMarketModule!");
			else
				Print("[DeadDrop] ExpansionMarketModule found.");
		}

		ExpansionDeadDropSettings m_testSettings = GetExpansionSettings().GetDeadDrop();
		if (!m_testSettings)
		{
			Print("[DeadDrop] Failed to get DeadDrop settings!");
			return;
		}
		Print("[DeadDrop] DeadDrop settings loaded.");
		string m_NPCName = m_testSettings.NPCName;
		Print("[DeadDrop] server settings: " + m_testSettings);
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
		Expansion_RegisterClientRPC("RPC_RequestOpenDeadDropMenu");
		Expansion_RegisterServerRPC("RPC_ConfirmDeadDropRecovery");
	}

	//Spawn Dead Drop NPC
	ExpansionDeadDropNPCAIBase SpawnDeadDropNPCAI()
	{
		ExpansionDeadDropSettings settings = GetExpansionSettings().GetDeadDrop();

		if (!settings || !settings.Active)
			return null;

		vector pos = settings.Position;
		vector ori = settings.Orientation;
		string className = settings.ClassName;

		EntityAI obj = EntityAI.Cast(ExpansionGame.CreateObjectSafe(className, pos));
		if (!obj)
		{
			Print("[DeadDrop] Failed to create NPC object safely");
			return null;
		}

		ExpansionDeadDropNPCAIBase npc = ExpansionDeadDropNPCAIBase.Cast(obj);
		if (!npc)
		{
			Print("[DeadDrop] Failed to cast to ExpansionDeadDropNPCAIBase");
			GetGame().ObjectDelete(obj);
			return null;
		}

		npc.SetPosition(pos);
		npc.SetOrientation(ori);
		npc.Update();

		npc.Expansion_SetEmote(settings.NPCEmoteID, !settings.NPCEmoteIsStatic);
		ExpansionHumanLoadout.Apply(npc, settings.NPCLoadoutFile, false);
		npc.Expansion_SetCanBeLooted(false);
		npc.eAI_SetUnlimitedReload(true);
		npc.eAI_SetAccuracy(1.0, 1.0);
		npc.eAI_SetThreatDistanceLimit(800);

		eAIGroup aiGroup = npc.GetGroup();
		if (settings.NPCFaction != string.Empty)
		{
			eAIFaction faction = eAIFaction.Create(settings.NPCFaction);
			if (faction && aiGroup.GetFaction().Type() != faction.Type())
				aiGroup.SetFaction(faction);
		}

		aiGroup.SetFormation(new eAIFormationColumn(aiGroup));
		aiGroup.SetWaypointBehaviour(eAIWaypointBehavior.ALTERNATE);

		return npc;
	}

	void SpawnDeadDropNPC()
	{
		m_DeadDropNPC = SpawnDeadDropNPCAI();

		if (!m_DeadDropNPC)
		{
			Print("[DeadDrop] NPC spawn failed.");
		}
	}

	ExpansionDeadDropNPCAIBase GetDeadDropNPC()
	{
		return m_DeadDropNPC;
	}
	//End Spawn NPC

	//Start on death write file
	void OnPlayerKilled(PlayerBase player, Object killer)
	{

		if (!GetGame().IsServer() || !player || !player.GetIdentity())
			return;

		string playerId = player.GetIdentity().GetId();
		string steamId = player.GetIdentity().GetPlainId();
		string playerName = player.GetIdentity().GetName();
		string causeOfDeath = "";
		vector deathPos = player.GetPosition();

		ExpansionDeadDropPlayerData deathData = new ExpansionDeadDropPlayerData();
		deathData.SetPlayerID(playerId);
		deathData.SetSteamID(steamId);
		deathData.SetPlayerName(playerName);
		deathData.SetCauseOfDeath(causeOfDeath);
		deathData.SetLocation(deathPos);
		deathData.SetISRecovered(false);
		deathData.SetRecoveryTime("");
		deathData.SetRecoveredBy("");
		Print("[DeadDrop] Saved inventory for player " + playerName + " (" + steamId + ")");

		// Serialize inventory
		ref array<ref DeadDropItem> inventoryData = new array<ref DeadDropItem>();
		if (EntityAI.Cast(player))
		{
				ref set<Object> visited = new set<Object>();
				GetInventoryRecursive(EntityAI.Cast(player), "", inventoryData, false, visited);
		}
		deathData.items = inventoryData;

		string timestamp = GetFormattedDateTime();
		deathData.SetDeathTime(timestamp);
		string fileName = steamId + "-" + timestamp;
		deathData.Save(fileName);
	}

	void GetInventoryRecursive(EntityAI entity, string slotName, ref array<ref DeadDropItem> result, bool wasInHands = false, ref set<Object> visited = null)
	{

		if (!entity)
			return;

		if (!visited)
			visited = new set<Object>();

		if (IsInVisited(entity, visited))
			return;

		visited.Insert(entity);

		DeadDropItem itemData = new DeadDropItem();
		itemData.type = entity.GetType();
		itemData.health = entity.GetHealth("", "");
		itemData.wasInHands = wasInHands;
		itemData.slotName = slotName;

		ItemBase itemBase = ItemBase.Cast(entity);
		if (itemBase)
		{
			itemData.quantity = itemBase.GetQuantity();
			if (itemBase.IsLiquidContainer())
				itemData.liquidType = "" + itemBase.GetLiquidType();
			else
				itemData.liquidType = "";

			itemData.skinIndex = 0;
			Magazine mag = Magazine.Cast(itemBase);
			if (mag)
			{
				itemData.ammo = mag.GetAmmoCount();
			}
			else
			{
				itemData.ammo = 0;
			}

		}

		// Attachments
		ref array<ref DeadDropItem> attachments = new array<ref DeadDropItem>();
		for (int a = 0; a < entity.GetInventory().AttachmentCount(); a++)
		{
			EntityAI attachment = entity.GetInventory().GetAttachmentFromIndex(a);
			if (attachment)
				GetInventoryRecursive(attachment, InventorySlots.GetSlotName(attachment.GetInventory().GetSlotId(0)), attachments, false, visited);

		}
		itemData.attachments = attachments;

		// Cargo
		ref array<ref DeadDropItem> cargo = new array<ref DeadDropItem>();
		for (int i = 0; i < entity.GetInventory().GetCargo().GetItemCount(); i++)
		{
			EntityAI cargoItem = entity.GetInventory().GetCargo().GetItem(i);
			if (cargoItem)
				GetInventoryRecursive(cargoItem, "", cargo, false, visited);

		}
		itemData.cargo = cargo;

		result.Insert(itemData);
	}

	bool IsInVisited(Object obj, set<Object> visited)
	{
		foreach (Object o : visited)
		{
			if (o == obj)
				return true;
		}
		return false;
	}

	string GetFormattedDateTime()
	{
		auto now = CF_Date.Now();
		return string.Format("%1-%2-%3-%4-%5-%6",now.GetYear(), now.GetMonth().ToStringLen(2),now.GetDay().ToStringLen(2),now.GetHours().ToStringLen(2),now.GetMinutes().ToStringLen(2),now.GetSeconds().ToStringLen(2));
	}
	//END on death logic

	//Start dead drop menu
	void RequestOpenDeadDropMenuCB(PlayerIdentity identity)
	{
		PlayerBase player = PlayerBase.GetPlayerByUID(identity.GetId());
		if (!player)
			return;

		ExpansionDeadDropNPCAIBase npc = GetDeadDropNPC();
		if (!npc)
		{
			EXError.Error(this, "[Expansion DeadDrop] Could not find the DeadDrop NPC.");
			return;
		}

		float distance = vector.Distance(player.GetPosition(), npc.GetPosition());
		if (distance > 5)  // Optional: interaction range check
		{
			EXError.Warn(this, "[Expansion DeadDrop] Player too far from DeadDrop NPC.");
			return;
		}

		RequestOpenDeadDropMenu(npc, identity);
	}

	void RequestOpenDeadDropMenu(Object target, PlayerIdentity identity)
	{
		if (!target || !identity)
			return;

		auto npcAI = ExpansionDeadDropNPCAIBase.Cast(target);
		if (!npcAI)
		{
			Print("[DeadDrop] Target is not a valid DeadDrop NPC.");
			return;
		}

		string uid = identity.GetPlainId();
		int PlayerMoney = 0;

		// Get ATM balance
		if (m_marketModule)
		{
			ref ExpansionMarketATM_Data atmData = m_marketModule.GetPlayerATMData(identity.GetId());
			if (atmData)
				PlayerMoney = atmData.MoneyDeposited;
		}

		// Load all death entries for this player
		array<ref ExpansionDeadDropPlayerData> entries = ExpansionDeadDropPlayerData.LoadAllForPlayer(uid);

		if (!entries || entries.Count() == 0)
		{
			ExpansionNotification("DeadDrop", "No recoverable loadouts found.").Error(identity);
			return;
		}

		// Create and send RPC
		auto rpc = Expansion_CreateRPC("RPC_RequestOpenDeadDropMenu");
		rpc.Write(PlayerMoney);
		rpc.Write(entries.Count());

		foreach (ExpansionDeadDropPlayerData data : entries)
		{
			data.OnSend(rpc);
		}

		rpc.Expansion_Send(target, true, identity);

		#ifdef EXPANSIONMODAI
		if (npcAI)
		{
			npcAI.eAI_AddInteractingPlayer(identity.GetPlayer());

			EmoteManager npcEmoteManager = npcAI.GetEmoteManager();
			if (!npcEmoteManager.IsEmotePlaying())
			{
				npcEmoteManager.PlayEmote(GetExpansionSettings().GetDeadDrop().NPCEmoteID);
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(npcEmoteManager.ServerRequestEmoteCancel, 2000);
			}
		}
		#endif
	}

	protected void RPC_RequestOpenDeadDropMenu(PlayerIdentity sender, Object target, ParamsReadContext ctx)
	{
		int playerMoney;
		if (!ctx.Read(playerMoney))
		{
			Error("[DeadDrop] Failed to read PlayerMoney!");
			return;
		}

		int entriesCount;
		if (!ctx.Read(entriesCount))
		{
			Error("[DeadDrop] Failed to read entries count!");
			return;
		}

		array<ref ExpansionDeadDropPlayerData> entries = {};
		for (int i = 0; i < entriesCount; i++)
		{
			ExpansionDeadDropPlayerData entry = new ExpansionDeadDropPlayerData();
			if (!entry.OnRecieve(ctx))  // Manual deserialization
			{
				Error("[DeadDrop] Failed to read entry at index " + i);
				continue;
			}
			entries.Insert(entry);
		}


		if (!entries || entries.Count() == 0)
		{
			Error("[DeadDrop] No dead drop entries received!");
			return;
		}

		if (!GetGame().IsDedicatedServer())
		{
			if (!OpenDeadDropMenu())
				return;
		}

		Exec_ShowDeadDropMenu(playerMoney, entries);
	}

	private void Exec_ShowDeadDropMenu(int playerMoney, array<ref ExpansionDeadDropPlayerData> entries)
	{
		if (!OpenDeadDropMenu())
			return;

		Print("[DeadDrop] Executing menu display. Entries count: " + entries.Count());
		m_DeadDropMenuInvoker.Invoke(playerMoney, entries);
	}

	bool OpenDeadDropMenu()
	{
		Print("[DeadDrop] Creating DeadDrop Menu");
		GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("ExpansionDeadDropMenu");
		return true;
	}

	ScriptInvoker GetDeadDropMenuSI()
	{
		Print("[DeadDrop] GetDeadDropMenuSI Invoked");
		return m_DeadDropMenuInvoker;
	}

	static ExpansionDeadDropModule GetModuleInstance() { return s_ModuleInstance; }
//Module End Here
}
