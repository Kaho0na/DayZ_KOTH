/**
 * FactionBankMenu.c
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

class FactionBankMenu: ExpansionScriptViewMenu
{
    protected ref FactionBankMenuController  m_FactionBankMenuController;
    protected ref ExpansionMarketModule m_MarketModule;
	ref ExpansionMarketATM_Data m_ATMData;
	private int m_FactionBankBalance;


	EditBoxWidget AmountValue;
	int m_Amount = 0;
	int m_PlayerMoney;
	ref SyncPlayer m_SelectedPlayer;
	ref ExpansionATMMenuTransferDialog m_TransferDialog;

    protected ButtonWidget wBtnCancel;
    protected ButtonWidget wBtnTransfer;



    void FactionBankMenu()
    {
		if (!m_FactionBankMenuController)
		m_FactionBankMenuController = FactionBankMenuController.Cast(GetController());
		
		if (!m_MarketModule)
			m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
		
		ExpansionMarketModule.SI_ATMMenuInvoker.Insert(SetPlayerATMData);
		ExpansionMarketModule.SI_FactionBankMenuCallback.Insert(OnDonateCallback);

    }

    override string GetLayoutFile()
    {
        return "FactionBank/GUI/layouts/FactionBankMenu.layout";
    }

	override typename GetControllerType() 
	{
		return FactionBankMenuController;
	}

	override bool CanShow()
	{
		return GetExpansionSettings().GetMarket().ATMSystemEnabled;
	}

	override void OnHide()
	{
		super.OnHide();
		
	}

	void SetPlayerATMData(ExpansionMarketATM_Data data, int factionBankBalance)
	{
		#ifdef EXPANSIONTRACE
			auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "SetPlayerATMData");
		#endif
		if (!data)
			return;

		m_ATMData = data;
		m_FactionBankBalance = factionBankBalance;
		SetView();
	}

	void SetView()
	{
		#ifdef EXPANSIONTRACE
			auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "SetView");
		#endif
		m_FactionBankMenuController.factionBankBalance = FormatNumberWithCommas(m_FactionBankBalance);
		m_FactionBankMenuController.NotifyPropertyChanged("factionBankBalance");
		
		m_FactionBankMenuController.MoneyDepositValue = FormatNumberWithCommas(m_ATMData.MoneyDeposited);
		m_FactionBankMenuController.NotifyPropertyChanged("MoneyDepositValue");
		SetFocus(AmountValue);
    }

	void UpdateView(bool updatePlayerMoney = true)
	{
		m_FactionBankMenuController.factionBankBalance = FormatNumberWithCommas(m_FactionBankBalance);
		m_FactionBankMenuController.NotifyPropertyChanged("factionBankBalance");
		
		m_FactionBankMenuController.MoneyDepositValue = FormatNumberWithCommas(m_ATMData.MoneyDeposited);
		m_FactionBankMenuController.NotifyPropertyChanged("MoneyDepositValue");

	}

	string FormatNumberWithCommas(int number)
	{
		string numStr = number.ToString();
		string formatted = "";
		int count = 0;

		for (int i = numStr.Length() - 1; i >= 0; i--)
		{
			formatted = numStr[i] + formatted;
			count++;
			if (count % 3 == 0 && i != 0)
			{
				formatted = "," + formatted;
			}
		}

		return formatted;
	}
	// On Transfer button click

	void OnDonateButtonClick()
	{
	
		if (!GetExpansionSettings().GetMarket().ATMPlayerTransferEnabled)	
			return;
		
		//! Only numbers are allowed
		int quantity = GetCheckAmount("ATM_TRANSFER_FAILED");
		if (quantity == -1)
			return;

		m_Amount = quantity;

		//! Make sure we dont send negative or 0 amounts
		if (m_Amount <= 0)
		{
			ExpansionNotification("STR_EXPANSION_ATM_UI_TRANSFER_PLAYER", "STR_EXPANSION_ATM_NONZERO").Error();
			return;
		}

		//! We only can only transfer what we have deposited
		if (m_Amount > m_ATMData.MoneyDeposited)
		{
			ExpansionNotification("STR_EXPANSION_ATM_UI_TRANSFER_PLAYER", "STR_EXPANSION_ATM_AMOUNT_MAX_ERROR").Error();
			return;
		}

		m_MarketModule.RequestDonateMoney(m_Amount);
	}

	void OnDonateCallback(int amount, ExpansionMarketATM_Data data, int factionBankBalance)
	{
		#ifdef EXPANSIONTRACE
			auto trace = CF_Trace_0(ExpansionTracing.MARKET, this, "OnDonateCallback");
		#endif
		if (!data)
			return;

		m_FactionBankBalance = factionBankBalance;
		m_ATMData = data;
		UpdateView();	
	}

	//! Gets amount text and checks if it contains only numbers.
	//! Returns amount as positive int if check passed, else displays error notification and returns -1.
	int GetCheckAmount(string title)
	{
		string quantity = AmountValue.GetText();
		TStringArray allNumbers = {"0","1","2","3","4","5","6","7","8","9"};
		for (int i = 0; i < quantity.Length(); i++)
		{
			if (allNumbers.Find(quantity.Get(i)) == -1)
			{
				ExpansionNotification("STR_EXPANSION_" + title, "STR_EXPANSION_ATM_ONLY_NUMBERS").Error();
				return -1;
			}
		}

		return quantity.ToInt();
	}

    //-------------- CLOSE MENU --------------
	void OnCancelButtonClick()
	{
		OnHide();
		Destroy();
	}

}

class FactionBankMenuController: ExpansionViewController
{
	string MoneyDepositValue;
	string factionBankBalance;
};