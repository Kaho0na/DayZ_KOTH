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
	ref ExpansionMarketATM_Data m_ATMData;
	
	ref SyncPlayer m_SelectedPlayer;
	ref ExpansionATMMenuTransferDialog m_TransferDialog;

	int m_Amount = 0;
	int m_PlayerMoney;
	private int m_FactionBankBalance;

    protected ButtonWidget wBtnCancel;
    protected ButtonWidget wBtnTransfer;
	EditBoxWidget AmountValue;


    void FactionBankMenu()
    {

		ExpansionFactionBankModule.GetModuleInstance().GetFactionBankMenuSI().Insert(OpenFactionBankMenu);
		ExpansionFactionBankModule.GetModuleInstance().GetFactionBankCallbackMenuSI().Insert(OnDonateCallback);

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


	void OpenFactionBankMenu(ExpansionMarketATM_Data data, int factionBankBalance, int liberatedCitiesIncome)
	{
		if (!m_FactionBankMenuController)
			m_FactionBankMenuController = FactionBankMenuController.Cast(GetController());

		m_ATMData = data;
		m_FactionBankMenuController.factionBankBalance = FormatNumberWithCommas(factionBankBalance);
		m_FactionBankMenuController.NotifyPropertyChanged("factionBankBalance");
		
		m_FactionBankMenuController.ATMBalanceValue = FormatNumberWithCommas(m_ATMData.MoneyDeposited);
		m_FactionBankMenuController.NotifyPropertyChanged("ATMBalanceValue");

		m_FactionBankMenuController.NPCName = GetExpansionSettings().GetFactionBank().NPCName;
		m_FactionBankMenuController.NotifyPropertyChanged("NPCName");

		m_FactionBankMenuController.NPCImage = GetExpansionSettings().GetFactionBank().NPCImage;
		m_FactionBankMenuController.NotifyPropertyChanged("NPCImage");

		m_FactionBankMenuController.NPCDefaultText = GetExpansionSettings().GetFactionBank().NPCDefaultText;
		m_FactionBankMenuController.NotifyPropertyChanged("NPCDefaultText");

		if (GetGame())
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if (player)
			{
				int salary = ExpansionFactionBankModule.GetModuleInstance().CalculatePlayerSalary(player);
				Print("[FactionBank] Player salary: " + salary);
				m_FactionBankMenuController.SalaryValue = salary.ToString();
				m_FactionBankMenuController.NotifyPropertyChanged("SalaryValue");
			}

			m_FactionBankMenuController.IncomeBalance = liberatedCitiesIncome.ToString();
			m_FactionBankMenuController.NotifyPropertyChanged("IncomeBalance");
		}

		SetFocus(AmountValue);
    }

	void UpdateView(bool updatePlayerMoney = true)
	{
		if (!m_FactionBankMenuController)
			m_FactionBankMenuController = FactionBankMenuController.Cast(GetController());

		m_FactionBankMenuController.factionBankBalance = FormatNumberWithCommas(m_FactionBankBalance);
		m_FactionBankMenuController.NotifyPropertyChanged("factionBankBalance");
		
		m_FactionBankMenuController.ATMBalanceValue = FormatNumberWithCommas(m_ATMData.MoneyDeposited);
		m_FactionBankMenuController.NotifyPropertyChanged("ATMBalanceValue");

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		m_FactionBankMenuController.SalaryValue = FormatNumberWithCommas(ExpansionFactionBankModule.GetModuleInstance().CalculatePlayerSalary(player));
		m_FactionBankMenuController.NotifyPropertyChanged("SalaryValue");
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

		ExpansionFactionBankModule.GetModuleInstance().RequestDonateMoney(m_Amount);
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
	string ATMBalanceValue;
	string factionBankBalance;
	string NPCName;
	string NPCImage;
	string NPCDefaultText;
	string SalaryValue;
	string IncomeBalance;
};