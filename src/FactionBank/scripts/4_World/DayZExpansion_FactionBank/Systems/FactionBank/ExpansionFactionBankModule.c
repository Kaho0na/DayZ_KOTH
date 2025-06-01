/**
 * FactionBankSalary.c
 *
 * DayZ Expansion Mod - Faction Bank by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

[CF_RegisterModule(ExpansionFactionBankModule)]
class ExpansionFactionBankModule: CF_ModuleWorld
{
    private ref ExpansionMarketModule m_MarketModule;
    protected static ExpansionFactionBankModule s_FactioBankInstance;
  
    protected ref ScriptInvoker m_FactionBankMenuInvoker; //! Client
	protected ref ScriptInvoker m_FactionBankCallbackInvoker; //! Client

    protected int m_FactionBankBalance;
    protected int m_MaxReputation;
    protected int m_BaseSalaryAmount;
    protected float m_SalaryMultiplier;
    protected int m_MoneyAddTimer;
    protected int maxDepositMoney;


    void ExpansionFactionBankModule()
    {
        s_FactioBankInstance = this;
        Print("[FactionBank] ExpansionFactionBankModule Loaded!");

        m_FactionBankMenuInvoker = new ScriptInvoker(); //! Client
		m_FactionBankCallbackInvoker = new ScriptInvoker();
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

        Expansion_RegisterClientRPC("RPC_RequestFactionBankMenu");
        Expansion_RegisterServerRPC("RPC_RequestDonateMoney");
		Expansion_RegisterClientRPC("RPC_ConfirmDonateMoney");
	}

    //Get Instance
	static ExpansionFactionBankModule GetModuleInstance()
	{
		return s_FactioBankInstance;
	}

	override void OnMissionStart(Class sender, CF_EventArgs args)
	{
		super.OnMissionStart(sender, args);

        if (GetGame().IsServer())
        {
            SpawnFactionBankNPC();
            SetupSalarySystem();
            Print("[FactionBank] ExpansionFactionBankModule loaded and active.");
        }
    }

    //Spawn Faction Bank NPC
	ExpansionFactionBankNPCBase SpawnFactionBankNPC()
	{
        Print("[FactionBank] Spawning Faction Bank NPC...");
		vector pos = GetExpansionSettings().GetFactionBank().NPCPosition;
		vector ori = GetExpansionSettings().GetFactionBank().NPCOrientation;
		string className = GetExpansionSettings().GetFactionBank().NPCClassName;
		string loadout = GetExpansionSettings().GetFactionBank().NPCLoadout;

		EntityAI obj = EntityAI.Cast(ExpansionGame.CreateObjectSafe(className, pos));
		if (!obj)
		{
			Print("[FactionBank Error] Failed to create NPC object safely");
			return null;
		}

		ExpansionFactionBankNPCBase npc = ExpansionFactionBankNPCBase.Cast(obj);
		if (!npc)
		{
			Print("[FactionBank Error] Failed to cast to ExpansionFactionBankNPCBase");
			GetGame().ObjectDelete(obj);
			return null;
		}
		
		npc.SetPosition(pos);
		npc.SetOrientation(ori);
		ExpansionHumanLoadout.Apply(npc, loadout, true);
		npc.Update();

		return npc;
	}

    // ------------------------------------------------------------
    // Expansion OpenFactionMenu
    // ------------------------------------------------------------
    void RequestFactionBankMenu(PlayerIdentity ident)
    {
	#ifdef EXTRACE
			auto trace = EXTrace.Start(EXTrace.MARKET, this);
	#endif

        if (!GetGame().IsServer())
        return;
        
		if (!ident)
		{
			Error("ExpansionFactionBankModule::SendPlayerFactionData - Could not get sender indentity!");
			return;
		}
		
       
        if (!m_MarketModule)
		    m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));

		ExpansionMarketATM_Data data = m_MarketModule.GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionFactionBankModule::SendPlayerFactionData - Could not get ExpansionMarketATM_Data!");
			return;
		}
		
        int FactionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;
        int liberatedCitiesIncome = ExpansionCityManagerModule.GetModuleInstance().GetLiberatedCitiesIncome();

		auto rpc = Expansion_CreateRPC("RPC_RequestFactionBankMenu");
		rpc.Write(data);
		rpc.Write(FactionBankBalance);
        rpc.Write(liberatedCitiesIncome);
		rpc.Expansion_Send(true, ident);
    }
    // ------------------------------------------------------------
    // Expansion RPC_RequestFactionBankMenu
    // ------------------------------------------------------------
    private void RPC_RequestFactionBankMenu(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
    {
    #ifdef EXTRACE
            auto trace = EXTrace.Start(EXTrace.MARKET, this);
    #endif

    ExpansionMarketATM_Data data;
    if (!ctx.Read(data))
    {
        Error("ExpansionFactionBankModule::RPC_RequestFactionBankMenu - Could not get ExpansionMarketATM_Data!");
        return;
    }

	int FactionBankBalance;
	if (!ctx.Read(FactionBankBalance)){
		Error("ExpansionFactionBankModule::RPC_RequestFactionBankMenu - Could not get FactionBankBalance!");
		return;
	}

    int liberatedCitiesIncome;
    if (!ctx.Read(liberatedCitiesIncome)){
        Error("ExpansionFactionBankModule::RPC_RequestFactionBankMenu - Could not get liberatedCitiesIncome!");
        return;
    }
	
	if (!GetGame().IsDedicatedServer())
	{
		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return;
	}
    
    Exec_SendFactionBankMenu(data, FactionBankBalance, liberatedCitiesIncome);
    }

    // ------------------------------------------------------------
    // Expansion Exec_SendPlayerFactionData
    // ------------------------------------------------------------
    private void Exec_SendFactionBankMenu(ExpansionMarketATM_Data data, int FactionBankBalance, int liberatedCitiesIncome)
    {
#ifdef EXTRACE
        auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

        if (!OpenFactionMenu())
            return;

		OpenFactionBankMenu(data, FactionBankBalance, liberatedCitiesIncome);
    }
    
	// ------------------------------------------------------------
	// Expansion SetPlayerFactionData
	// ------------------------------------------------------------
	void OpenFactionBankMenu(ExpansionMarketATM_Data data, int FactionBankBalance, int liberatedCitiesIncome)
	{
	#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
	#endif

		m_FactionBankMenuInvoker.Invoke(data, FactionBankBalance, liberatedCitiesIncome);
	}

    bool OpenFactionMenu()
    {
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("FactionBankMenu");
        return true;
    }


    //Execute donation on server/client
    // ------------------------------------------------------------
	// Expansion RequestDonateMoney from Client
	// ------------------------------------------------------------	
	void RequestDonateMoney(int amount)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!GetGame().IsDedicatedServer())
		{
			auto rpc = Expansion_CreateRPC("RPC_RequestDonateMoney");
			rpc.Write(amount);
			rpc.Expansion_Send(true);
		}
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_RequestDonateMoney
	// ------------------------------------------------------------
	private void RPC_RequestDonateMoney(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return;

		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionFactionBankModule::RPC_RequestDonateMoney - Could not get amount!");
			return;
		}
		
		Exec_RequestDonateMoney(amount, senderRPC);
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_RequestDonateMoney
	// ------------------------------------------------------------
	private void Exec_RequestDonateMoney(int amount, PlayerIdentity ident)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!ident)
		{
			Error("ExpansionFactionBankModule::Exec_RequestDonateMoney - Could not get player identity!");
			return;
		}

		PlayerBase player = PlayerBase.GetPlayerByUID(ident.GetId());
		if (!player)
		{
			Error("ExpansionFactionBankModule::Exec_RequestDonateMoney - Could not get player base enity!");
			return;
		}
				
		ExpansionMarketATM_Data data = m_MarketModule.GetPlayerATMData(ident.GetId());
		if (!data)
		{
			Error("ExpansionFactionBankModule::Exec_RequestDonateMoney - Could not find player atm data!");
			return;
		}

		data.RemoveMoney(amount);
		data.Save();
        int factionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;
		factionBankBalance += amount;
		GetExpansionSettings().GetFactionBank().FactionBankBalance = factionBankBalance;
		GetExpansionSettings().GetFactionBank().Save();
		ExpansionLogATM(string.Format("Player \"%1\" (id=%2) has Donated %3 to the Faction Bank.", ident.GetName(), ident.GetId(), amount));
		ConfirmDonateMoney(amount, ident, data, factionBankBalance);
	}
		
	// ------------------------------------------------------------
	// Expansion ConfirmDonateMoney
	// ------------------------------------------------------------	
	void ConfirmDonateMoney(int amount, PlayerIdentity ident, ExpansionMarketATM_Data data, int factionBankBalance)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif
		auto rpc = Expansion_CreateRPC("RPC_ConfirmDonateMoney");
		rpc.Write(amount);
		rpc.Write(data);
		rpc.Write(factionBankBalance);
		rpc.Expansion_Send(true, ident);
	}
	
	// ------------------------------------------------------------
	// Expansion RPC_ConfirmDonateMoney
	// ------------------------------------------------------------
	private void RPC_ConfirmDonateMoney(PlayerIdentity senderRPC, Object target, ParamsReadContext ctx)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return;

		int amount;
		if (!ctx.Read(amount))
		{
			Error("ExpansionFactionBankModule::RPC_ConfirmDonateMoney - Could not get amount!");
			return;
		}
		
		ExpansionMarketATM_Data data;
		if (!ctx.Read(data))
		{
			Error("ExpansionFactionBankModule::RPC_ConfirmDonateMoney - Could not get player ATM data!");
			return;
		}

		int factionBankBalance;
		if (!ctx.Read(factionBankBalance))
		{
			Error("ExpansionFactionBankModule::RPC_ConfirmDonateMoney - Could not get faction bank balance!");
			return;
		}

		
		Exec_ConfirmDonateMoney(amount, data, factionBankBalance);
	}
	
	// ------------------------------------------------------------
	// Expansion Exec_ConfirmDonateMoney
	// ------------------------------------------------------------
	private void Exec_ConfirmDonateMoney(int amount, ExpansionMarketATM_Data data, int factionBankBalance)
	{
#ifdef EXTRACE
		auto trace = EXTrace.Start(EXTrace.MARKET, this);
#endif

		m_FactionBankCallbackInvoker.Invoke(amount, data, factionBankBalance); // <-- Pass both
	}


    //setup salary system
    private void SetupSalarySystem()
    {
        
       
        if (!m_MarketModule)
		    m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));

        m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
        if (!m_MarketModule)
        {
            ExpansionLogATM("Error: MarketModule not found! Salary system will not work.");
            return;
        }

        
        m_MaxReputation = GetExpansionSettings().GetHardline().MaxReputation;
        m_BaseSalaryAmount = GetExpansionSettings().GetFactionBank().BaseSalaryAmount;
        m_SalaryMultiplier = GetExpansionSettings().GetFactionBank().SalaryMultiplier;
        m_MoneyAddTimer = GetExpansionSettings().GetFactionBank().SalaryTimerInterval;
        maxDepositMoney = GetExpansionSettings().GetMarket().MaxDepositMoney;
        Print("FactionBankSalary: Max Deposit Money: " + maxDepositMoney.ToString() + " Max Reputation: " + m_MaxReputation.ToString() + " Base Salary Amount: " + m_BaseSalaryAmount.ToString() + " Salary Multiplier: " + m_SalaryMultiplier.ToString() + " Money Add Timer: " + m_MoneyAddTimer.ToString());
        StartSalaryTimer();
    }

    private void StartSalaryTimer()
    {
        if (m_MoneyAddTimer <= 0)
            m_MoneyAddTimer = 60; // Default to 60s if config error

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(AddMoneyToActivePlayers, m_MoneyAddTimer * 1000, true);
        ExpansionLogATM("Salary timer started, interval: " + m_MoneyAddTimer.ToString() + " seconds.");
    }

    private void AddMoneyToActivePlayers()
    {
       
        if (!m_MarketModule)
		    m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));

        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        foreach (Man player : players)
        {
            PlayerIdentity identity = player.GetIdentity();
            if (!identity)
                continue;

            ExpansionMarketATM_Data playerATMData = m_MarketModule.GetPlayerATMData(identity.GetId());
            if (!playerATMData)
                continue;
            // Get current faction bank balance from settings
            m_FactionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;

            // calculate salary based on reputation and faction bank balance
            int moneyToAdd = CalculatePlayerSalary(PlayerBase.Cast(player));
            int currentATMMoney = playerATMData.GetMoney();

            // check if the ATM balance is full
            int futureDepositMoney = moneyToAdd + currentATMMoney;

            if (futureDepositMoney > maxDepositMoney)
            {
                int possibleDepositAmount = maxDepositMoney - currentATMMoney;
                if (possibleDepositAmount > 0)
                {
                    // if the ATM balance is full, add only the possible amount 
                    playerATMData.AddMoney(possibleDepositAmount);
                    ExpansionNotification(new StringLocaliser("Payment"), new StringLocaliser("You received a payout only %1. Bank account full!", possibleDepositAmount.ToString()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 7, ExpansionNotificationType.TOAST).Create(identity);
                    ExpansionLogATM(string.Format("Player \"%1\" received only %2 salary (bank full)", player.GetName(), possibleDepositAmount));
                }
                else
                {
                    // if the ATM balance is full, do not add any money and notify the player
                    ExpansionNotification(new StringLocaliser("Payment"), new StringLocaliser("You received no payout. Bank account full!"), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 7, ExpansionNotificationType.TOAST).Create(identity);
                    ExpansionLogATM(string.Format("Player \"%1\" received no salary (bank full)", player.GetName()));
                }
            }
            else // ATM is not full add the full amount 
            {
                playerATMData.AddMoney(moneyToAdd);
                ExpansionNotification(new StringLocaliser("Payment"), new StringLocaliser("You received a payout of %1.", moneyToAdd.ToString()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 7, ExpansionNotificationType.TOAST).Create(identity);
                ExpansionLogATM(string.Format("Player \"%1\" received %2 salary", player.GetName(), moneyToAdd));
            }

            playerATMData.Save();
        }
    }

    int CalculatePlayerSalary(PlayerBase playerBase)
    {
        Print("[FactionBank] Calculating player salary...");
        m_BaseSalaryAmount = GetExpansionSettings().GetFactionBank().BaseSalaryAmount;
        Print("[FactionBank] Base salary amount: " + m_BaseSalaryAmount.ToString());
        if (!playerBase || !playerBase.GetIdentity())
        {
            Print("[FactionBank Error] CalculatePlayerSalary: PlayerBase or Identity is null.");
            return m_BaseSalaryAmount;
        }
        int playerReputation = playerBase.Expansion_GetReputation();
        Print("[FactionBank] Player reputation: " + playerReputation.ToString());
        bool useReputation = GetExpansionSettings().GetHardline().UseReputation;
        bool factionEnabledReputation = GetExpansionSettings().GetFactionBank().FactionBankEnableReputation;
        Print("[FactionBank] Use reputation: " + useReputation.ToString() + ", Faction Bank Enable Reputation: " + factionEnabledReputation.ToString());
        int maxSalaryCap = GetExpansionSettings().GetFactionBank().MaxSalaryCap;
        Print("[FactionBank] Max salary cap: " + maxSalaryCap.ToString());
        if (!useReputation || !factionEnabledReputation)
            return m_BaseSalaryAmount;

        m_MaxReputation = GetExpansionSettings().GetHardline().MaxReputation;
        Print("[FactionBank] Max reputation: " + m_MaxReputation.ToString());
        if (m_MaxReputation == 0 || playerReputation == 0)
            return m_BaseSalaryAmount;

        float reputationRatio = (float)playerReputation / (float)m_MaxReputation;
        Print("[FactionBank] Reputation ratio: " + reputationRatio.ToString());
        m_FactionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;
        Print("[FactionBank] Faction bank balance: " + m_FactionBankBalance.ToString());
        m_SalaryMultiplier = GetExpansionSettings().GetFactionBank().SalaryMultiplier;
        Print("[FactionBank] Salary multiplier: " + m_SalaryMultiplier.ToString());
        int bonus = m_FactionBankBalance * reputationRatio * m_SalaryMultiplier;
        Print("[FactionBank] Calculated bonus: " + bonus.ToString());

        if (bonus > maxSalaryCap)
            return maxSalaryCap;
        else if (bonus > m_BaseSalaryAmount)
            return bonus;
        else
            return m_BaseSalaryAmount;
    }

    private void ExpansionLogATM(string message)
    {
        if (GetExpansionSettings().GetLog().ATM)
            GetExpansionSettings().GetLog().PrintLog("[ATM] " + message);
    }

    ScriptInvoker GetFactionBankCallbackMenuSI()
	{
		return m_FactionBankCallbackInvoker;
	}

	ScriptInvoker GetFactionBankMenuSI()
	{
		return m_FactionBankMenuInvoker;
	}
}
