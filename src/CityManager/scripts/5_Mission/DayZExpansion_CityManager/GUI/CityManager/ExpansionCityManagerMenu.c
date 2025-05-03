/**
 * ExpansionQuestMenu.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionCityManagerMenu: ExpansionScriptViewMenu
{
    protected ref ExpansionCityManagerMenuController m_CityManagerMenuController;
	protected ref ExpansionCityManagerModule m_CityManagerModule;
    ref ExpansionCityManagerNPCData m_CityManagerData;
	protected ref ExpansionMarketModule m_MarketModule;
	ref ExpansionATMMenuTransferDialog m_TransferDialog;

    protected string m_CurrentNPCName;
	protected string m_CurrentNPCText;
    protected string m_CityName;
    protected int m_CurrentNPCID = -1;
    protected int m_CityManager;
    protected int m_MaxCityManager;
    protected bool m_isLiberated;

    protected TextWidget CityName;
    protected TextWidget CityCurrentLoyalty;
    protected TextWidget LoyaltyCurrencyValue;
    protected EditBoxWidget AmountValue;
    protected ButtonWidget BtnTransfer;

	private int m_PlayerMoney;


    void ExpansionCityManagerMenu()
	{
		if (!m_CityManagerMenuController)
		m_CityManagerMenuController = ExpansionCityManagerMenuController.Cast(GetController());
	
		if (!m_CityManagerModule)
		m_CityManagerModule = ExpansionCityManagerModule.Cast(CF_ModuleCoreManager.Get(ExpansionCityManagerModule));
	
		if (!m_MarketModule)
		m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));

		m_CityManagerModule.GetModuleInstance().GetCityManagerMenuSI().Insert(SetCityManagerData);
		m_CityManagerModule.GetModuleInstance().GetCityManagerCallbackMenuSI().Insert(OnBribeCallback);

	}

    void ~ExpansionCityManagerMenu()
	{
		m_CurrentNPCID = -1;
	}

    override string GetLayoutFile()
    {
        return "CityManager/GUI/layouts/CityManagerMenu.layout";
    }

    override typename GetControllerType() 
	{
		return ExpansionCityManagerMenuController;
	}

    override bool CanShow()
	{
		return true;
	}

	override void OnHide()
	{
		super.OnHide();
	}

	void SetCityManagerData(ExpansionCityManagerNPCData data)
	{
		if (!data)
			return;
	
		m_CityManagerData = data;
	
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
		{
			Print("Player could not be found");
			Error("SetCityManagerData - Could not find local player.");
			return;
		}
	
		m_PlayerMoney = m_CityManagerModule.GetModuleInstance().GetPlayerMoney(player);
		SetView(m_CityManagerData);
	}
	



    void SetView(ExpansionCityManagerNPCData m_CityManagerData)
	{
		m_CityManagerMenuController.CityManagerImage = m_CityManagerData.CityManagerImage;
		m_CityManagerMenuController.NotifyPropertyChanged("CityManagerImage");

		m_CityManagerMenuController.NPCName = m_CityManagerData.NPCName;
		m_CityManagerMenuController.NotifyPropertyChanged("NPCName");

		m_CityManagerMenuController.DefaultNPCText = m_CityManagerData.DefaultNPCText;
		m_CityManagerMenuController.NotifyPropertyChanged("DefaultNPCText");

		m_CityManagerMenuController.CityIncome = "$" + m_CityManagerData.CityIncome.ToString() + " per hour";
		m_CityManagerMenuController.NotifyPropertyChanged("CityIncome");

		if (m_CityManagerData.CityTrain)
			m_CityManagerMenuController.CityTrain = "Available";
		else
			m_CityManagerMenuController.CityTrain = "Not Available";
		m_CityManagerMenuController.NotifyPropertyChanged("CityTrain");

		if (m_CityManagerData.CityBus)
			m_CityManagerMenuController.CityBus = "Available";
		else
			m_CityManagerMenuController.CityBus = "Not Available";
		m_CityManagerMenuController.NotifyPropertyChanged("CityBus");

		if (m_CityManagerData.CityLiberated)
			m_CityManagerMenuController.CityLoyalty = "Resistance";
		else
			m_CityManagerMenuController.CityLoyalty = "Enemy";
		m_CityManagerMenuController.NotifyPropertyChanged("CityLoyalty");

		m_CityManagerMenuController.CityName = m_CityManagerData.CityName;
		m_CityManagerMenuController.NotifyPropertyChanged("CityName");

        m_CityManagerMenuController.CityCurrentLoyalty = m_CityManagerData.CityCurrentLoyalty.ToString();
		m_CityManagerMenuController.NotifyPropertyChanged("CityCurrentLoyalty");

		m_CityManagerMenuController.LoyaltyCurrencyValue = m_PlayerMoney.ToString();
		m_CityManagerMenuController.NotifyPropertyChanged("LoyaltyCurrencyValue");
		SetFocus(AmountValue);
    }

	void UpdateView(ExpansionCityManagerNPCData m_CityManagerData, int m_CurrentLoyalty, int m_NewPlayerMoney, bool updatePlayerMoney = true)
	{
		if (m_CityManagerData.CityTrain && m_CityManagerData.CityLiberated)
			m_CityManagerMenuController.CityTrain = "Available";
		else
			m_CityManagerMenuController.CityTrain = "Not Available";
		m_CityManagerMenuController.NotifyPropertyChanged("CityTrain");

		if (m_CityManagerData.CityBus && m_CityManagerData.CityLiberated)
			m_CityManagerMenuController.CityBus = "Available";
		else
			m_CityManagerMenuController.CityBus = "Not Available";
		m_CityManagerMenuController.NotifyPropertyChanged("CityBus");

		if (m_CityManagerData.CityLiberated)
			m_CityManagerMenuController.CityLoyalty = "Resistance";
		else
			m_CityManagerMenuController.CityLoyalty = "Enemy";
		m_CityManagerMenuController.NotifyPropertyChanged("CityLoyalty");

		m_CityManagerMenuController.CityName = m_CityManagerData.CityName;
		m_CityManagerMenuController.NotifyPropertyChanged("CityName");

        m_CityManagerMenuController.CityCurrentLoyalty = m_CurrentLoyalty.ToString();
		m_CityManagerMenuController.NotifyPropertyChanged("CityCurrentLoyalty");

		m_CityManagerMenuController.LoyaltyCurrencyValue = m_NewPlayerMoney.ToString();
		m_CityManagerMenuController.NotifyPropertyChanged("LoyaltyCurrencyValue");

		m_CityManagerMenuController.AmountValue = 0;
		m_CityManagerMenuController.NotifyPropertyChanged("AmountValue");
		SetFocus(AmountValue);
	}

	// On Transfer button click

	void OnBribeButtonClick()
	{
		//! Only numbers are allowed
		int quantity = GetCheckAmount("ATM_TRANSFER_FAILED");

		if (quantity == -1)
			return;

		int m_Amount = quantity;

		//! Make sure we dont send negative or 0 amounts
		if (m_Amount <= 0)
		{
			ExpansionNotification("STR_EXPANSION_ATM_UI_TRANSFER_PLAYER", "STR_EXPANSION_ATM_NONZERO").Error();
			return;
		}
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
		{
			Print("Player could not be found");
			Error("SetCityManagerData - Could not find local player.");
			return;
		}
	
		m_PlayerMoney = m_CityManagerModule.GetModuleInstance().GetPlayerMoney(player);
		//! We only can only transfer what we have deposited
		if (m_Amount > m_PlayerMoney)
		{
			ExpansionNotification("You don't have that kind of cash", "SOMETHING HERE").Error();
			return;
		}

		m_CityManagerModule.RequestBribeMoney(m_Amount, m_CityManagerData.CityID);

	}

	void OnBribeCallback(ExpansionCityManagerNPCData data, int m_CurrentLoyalty, int m_NewPlayerMoney)
	{
		Print("OnBribeCallback: m_NewPlayerMoney = " + m_NewPlayerMoney);
		UpdateView(data, m_CurrentLoyalty, m_NewPlayerMoney);
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

}


class ExpansionCityManagerMenuController: ExpansionViewController
{
	string CityName;
	string CityCurrentLoyalty;
	string CityMaxReachLoyalty;
	string CityLoyalty;
	string NPCName;
	string DefaultNPCText;
	string LoyaltyCurrencyValue;
	int AmountValue;
	string CityIncome;
	string CityTrain;
	string CityBus;
	string CityManagerImage;
};