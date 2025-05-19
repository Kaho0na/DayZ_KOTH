class ExpansionDeadDropMenu: ExpansionScriptViewMenu
{
	protected ref ExpansionDeadDropMenuController m_DeadDropMenuController;
	protected ref ExpansionDeadDropModule m_DeadDropModule;
	ref array<ref ExpansionDeadDropPlayerData> m_DeadDropEntries;
	protected ExpansionDeadDropPlayerData m_SelectedBodyData;
	private MissionGameplay m_Mission;

	protected int m_PlayerMoney;
	protected int m_SelectedIndex = -1;
	protected ref ExpansionMarketATM_Data m_ATMData;

	private Widget MapSpacer;
	private MapWidget Map_Widget;
	private ButtonWidget m_ConfirmButton;
	private TextWidget m_ConfirmText;
	private TextWidget m_ConfirmButtonText;
	private ImageWidget m_Background;
	private TextWidget m_RecoveryLabel;

	private int m_NextListIndex = 0;
	private int m_RecoveryCost;

	override string GetLayoutFile() 
	{
		return "DeadDrop/GUI/layouts/ExpansionDeadDropMenu.layout"; 
	}

	override typename GetControllerType() 
	{
		return ExpansionDeadDropMenuController;
	}

	override void OnShow()
	{
		super.OnShow();
		Print("[ExpansionDeadDropMenu] OnShow called");
	
		GetGame().GetInput().ChangeGameFocus(1);
		SetFocus(GetLayoutRoot());
		PPEffects.SetBlurMenu(0.5);
		m_Mission.GetHud().ShowHud(false);
		m_Mission.GetHud().ShowQuickBar(false);
	
		m_DeadDropMenuController.SelectedBody = "NONE";
		m_DeadDropMenuController.NotifyPropertyChanged("SelectedBody");
	
		m_ConfirmButton = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("Confirm"));
		m_ConfirmText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("ConfirmText"));
		m_ConfirmButtonText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("ConfirmButtonText"));
		m_Background = ImageWidget.Cast(GetLayoutRoot().FindAnyWidget("Background"));
		m_Background.LoadImageFile(0, "DeadDrop/GUI/backgrounds/morguefinal.paa");
		m_RecoveryLabel = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("RecoveryCostLabel"));

	}

	override void OnHide()
	{
		super.OnHide();

		GetGame().GetInput().ResetGameFocus();
		PPEffects.SetBlurMenu(0.0);
		m_Mission.GetHud().ShowHud(true);
		m_Mission.GetHud().ShowQuickBar(true);

		Clear();
	}


	void Clear()
	{
		m_DeadDropMenuController.BodyEntries.Clear();
	}

	//Constructor
	void ExpansionDeadDropMenu(int playerMoney, array<ref ExpansionDeadDropPlayerData> entries)
	{
		m_DeadDropMenuController = ExpansionDeadDropMenuController.Cast(GetController());
		m_DeadDropEntries = entries;
		m_PlayerMoney = playerMoney;
		Class.CastTo(m_Mission, MissionGameplay.Cast(GetGame().GetMission()));
		Class.CastTo(m_DeadDropMenuController, GetController());
		Class.CastTo(m_DeadDropModule, CF_ModuleCoreManager.Get(ExpansionDeadDropModule));
		m_ConfirmButton.Show(false);
		m_ConfirmButton.Enable(false);

		int mapSize = GetGame().GetWorld().GetWorldSize() / 2;

		m_DeadDropModule.GetModuleInstance().GetDeadDropMenuSI().Insert(ShowMenuNow);
	}

	//Deconstructor
	void ~ExpansionDeadDropMenu()
	{
		Clear();
	}

	//Show Menu
	void ShowMenuNow(int playerMoney, array<ref ExpansionDeadDropPlayerData> entries)
	{
		Print("[DeadDrop] ShowMenuNow called");
		
		m_PlayerMoney = playerMoney;
		Print("[DeadDrop] Player Money: " + m_PlayerMoney);
		m_DeadDropEntries = entries;

		m_DeadDropMenuController.PlayerMoney = "$" + FormatNumberWithCommas(m_PlayerMoney);
		m_DeadDropMenuController.NotifyPropertyChanged("PlayerMoney");

		ExpansionDeadDropSettings settings = GetExpansionSettings().GetDeadDrop();
		string m_NPCName = settings.NPCName;
		Print("[DeadDrop] NPC Name: " + m_NPCName);
		m_DeadDropMenuController.Stationtitle = m_NPCName;
		m_DeadDropMenuController.NotifyPropertyChanged("Stationtitle");

		string m_NPCText = GetRandomUndertakerQuote();
		m_DeadDropMenuController.NPCName = m_NPCText;
		m_DeadDropMenuController.NotifyPropertyChanged("NPCName");

		m_DeadDropMenuController.UndertakerImage = settings.DeadDropImage;
		m_DeadDropMenuController.NotifyPropertyChanged("UndertakerImage");

		SetDeadDropEntries(entries);
	}

	string GetRandomUndertakerQuote()
	{
		ref array<string> quotes = {
			"Ah... this one bled for a cause. Just not fast enough to make it back.",
			"I tried to stitch this one once. They kept squirming. Not very cooperative.",
			"Still warm when I found them. You'd be surprised what you can salvage in time.",
			"This one? Brave. Foolish. Left a liver I could've bottled.",
			"A familiar scent… sulfur and cordite. This one went out swinging."
		};

		int idx = Math.RandomInt(0, quotes.Count());
		return quotes[idx];
	}


	void SetDeadDropEntries(array<ref ExpansionDeadDropPlayerData> entries)
	{
		Print("[DeadDrop] SetDeadDropEntries called");
		m_DeadDropMenuController.BodyEntries.Clear();



		for (int i = 0; i < entries.Count(); i++)
		{
			ExpansionDeadDropPlayerData data = entries[i];
			ExpansionDeadDropMenuLocationEntry entry = new ExpansionDeadDropMenuLocationEntry(i, data);
			m_DeadDropMenuController.BodyEntries.Insert(entry);
		}

		m_DeadDropMenuController.NotifyPropertyChanged("BodyEntries");
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
	//END Show Menu

	//On Click Body Entry
	void SetRecoveryTarget(int index, ExpansionDeadDropPlayerData data, bool notifyUI = true)
	{
		m_SelectedIndex = index;
		m_SelectedBodyData = data;  // ← Store selected data for recovery use

		// Unselect all entries
		for (int i = 0; i < m_DeadDropMenuController.BodyEntries.Count(); i++)
		{
			ExpansionDeadDropMenuLocationEntry entry = ExpansionDeadDropMenuLocationEntry.Cast(m_DeadDropMenuController.BodyEntries[i]);
			if (entry)
				entry.SetSelected(i == index);
		}

		// Update label
		m_DeadDropMenuController.SelectedBody = data.GetBodyName();
		if (notifyUI)
			m_DeadDropMenuController.NotifyPropertyChanged("SelectedBody");

		// Set recovery cost
		m_RecoveryCost = GetExpansionSettings().GetDeadDrop().RecoveryCost;
		m_DeadDropMenuController.RecoveryCost = "$" + FormatNumberWithCommas(m_RecoveryCost);
		if (notifyUI)
			m_DeadDropMenuController.NotifyPropertyChanged("RecoveryCost");
	}


	//Menu Closing Here
	void CloseMenuButtonClick()
	{
		CloseMenu();
	}

	//On Recover Click Button
	void OnRecoverButtonClick()
	{
		//Click the recover button logic here
	}

	//Confirm Recovery
	void OnConfirmRecovery()
	{
		//Execute recovery after confirm 
	}

	override bool CanClose()
	{
		return true;
	}

	void CloseMenu()
	{
		GetDayZExpansion().GetExpansionUIManager().CloseMenu();
	}
	//END Menu Closing

}

class ExpansionDeadDropMenuController: ExpansionViewController
{
	ref array<ref ExpansionDeadDropPlayerData> m_DeadDropEntries = new array<ref ExpansionDeadDropPlayerData>();
	ref ObservableCollection<ref ExpansionDeadDropMenuLocationEntry> BodyEntries = new ObservableCollection<ref ExpansionDeadDropMenuLocationEntry>(this);
	string SelectedLocation;
	string RecoveryCost;
	string PlayerMoney;
	string Menutitle;
	string NPCName;
	string Stationtitle;
	string UndertakerImage;
	string SelectedBody;
}
