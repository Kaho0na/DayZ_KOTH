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
		
		m_PlayerMoney = playerMoney;
		m_DeadDropEntries = entries;

		m_DeadDropMenuController.PlayerMoney = "$" + FormatNumberWithCommas(m_PlayerMoney);
		m_DeadDropMenuController.NotifyPropertyChanged("PlayerMoney");

		ExpansionDeadDropSettings settings = GetExpansionSettings().GetDeadDrop();
		string m_NPCName = settings.NPCName;
		m_DeadDropMenuController.NPCName = m_NPCName;
		m_DeadDropMenuController.NotifyPropertyChanged("NPCName");

		string m_NPCText = GetRandomUndertakerQuote();
		m_DeadDropMenuController.Stationtitle = m_NPCText;
		m_DeadDropMenuController.NotifyPropertyChanged("Stationtitle");

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

void SetRecoveryTarget(int index, ExpansionDeadDropPlayerData data, bool notifyUI = true)
{
	// Store selection
	m_SelectedIndex = index;
	m_SelectedBodyData = data;

	Print("[DeadDrop] Selected body: " + data.steam_id + " at " + data.death_time);

	// Deselect all entries and highlight selected
	for (int i = 0; i < m_DeadDropMenuController.BodyEntries.Count(); i++)
	{
		ExpansionDeadDropMenuLocationEntry entry = ExpansionDeadDropMenuLocationEntry.Cast(m_DeadDropMenuController.BodyEntries[i]);
		if (entry)
			entry.SetSelected(i == index);
	}

	// Update UI labels
	m_DeadDropMenuController.SelectedBody = data.GetBodyName();
	if (notifyUI)
		m_DeadDropMenuController.NotifyPropertyChanged("SelectedBody");

	m_RecoveryCost = GetExpansionSettings().GetDeadDrop().RecoveryCost;
	m_DeadDropMenuController.RecoveryCost = "$" + FormatNumberWithCommas(m_RecoveryCost);
	if (notifyUI)
		m_DeadDropMenuController.NotifyPropertyChanged("RecoveryCost");

	// Delete old preview
	if (m_DeadDropMenuController.DeadPlayerPreview)
	{
		GetGame().ObjectDelete(m_DeadDropMenuController.DeadPlayerPreview);
		m_DeadDropMenuController.DeadPlayerPreview = null;
	}

	// Always use static preview base type (e.g., Mirek)
	string previewType = "SurvivorM_Mirek";

	// Create preview object locally
	EntityAI previewEntity = EntityAI.Cast(GetGame().CreateObjectEx(previewType, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME));
	if (!previewEntity)
		return;

	// Equip gear
	foreach (ExpansionDeadDropItemData itemData : data.items)
	{
		SpawnPreviewItemRecursive(previewEntity, itemData);
	}

	// Bind to UI
	m_DeadDropMenuController.DeadPlayerPreview = previewEntity;
	m_DeadDropMenuController.NotifyPropertyChanged("DeadPlayerPreview");
}



	void SpawnPreviewItemRecursive(EntityAI parent, ExpansionDeadDropItemData itemData)
	{
		if (!itemData || !parent)
			return;

		EntityAI item = EntityAI.Cast(parent.GetInventory().CreateInInventory(itemData.type));
		if (!item)
			return;

		ItemBase itemBase = ItemBase.Cast(item);
		if (itemBase)
		{
			itemBase.SetHealth01("", "", itemData.health / itemBase.GetMaxHealth("", ""));
			itemBase.SetQuantity(itemData.quantity);
		}

		// Attachments
		foreach (ExpansionDeadDropItemData attData : itemData.attachments)
		{
			SpawnPreviewItemRecursive(item, attData);
		}

		// Cargo
		foreach (ExpansionDeadDropItemData cargoItem : itemData.cargo)
		{
			SpawnPreviewItemRecursive(item, cargoItem);
		}
	}

	//Menu Closing Here
	void CloseMenuButtonClick()
	{
		CloseMenu();
	}

	// On Recover Click Button
	void OnRecoverButtonClick()
	{
		if (!m_SelectedBodyData)
		{
			ExpansionNotification("DeadDrop", "Please select a body to recover.").Error();
			return;
		}

		if (m_PlayerMoney < m_RecoveryCost)
		{
			ExpansionNotification("Insufficient Funds", "Not enough funds in your ATM account. Maybe try looting?!").Error();
			return;
		}

		Print("[DeadDrop] Recover button pressed for: " + m_SelectedBodyData.GetBodyName());

		new ExpansionDeadDropConfirmDialog(this);
	}


	//Confirm Recovery
	void OnConfirmRecovery()
	{
		if (!m_SelectedBodyData)
		{
			ExpansionNotification("DeadDrop", "No recovery target selected.").Error();
			return;
		}

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		// Send RPC to server with selected entry file name
		string fileName = m_SelectedBodyData.steam_id + "-" + m_SelectedBodyData.death_time + ".json";
		ExpansionDeadDropModule.GetModuleInstance().ExecuteRecoveryRequest(fileName);

		// Close menu
		CloseMenu();
	}


	override bool CanClose()
	{
		return true;
	}

	void CloseMenu()
	{
		if (m_DeadDropMenuController.DeadPlayerPreview)
		{
			GetGame().ObjectDelete(m_DeadDropMenuController.DeadPlayerPreview);
			m_DeadDropMenuController.DeadPlayerPreview = null;
		}
		GetDayZExpansion().GetExpansionUIManager().CloseMenu();
	}
	//END Menu Closing

}

class ExpansionDeadDropMenuController: ExpansionViewController
{
	ref array<ref ExpansionDeadDropPlayerData> m_DeadDropEntries = new array<ref ExpansionDeadDropPlayerData>();
	ref ObservableCollection<ref ExpansionDeadDropMenuLocationEntry> BodyEntries = new ObservableCollection<ref ExpansionDeadDropMenuLocationEntry>(this);
	Object DeadPlayerPreview;
	string SelectedLocation;
	string RecoveryCost;
	string PlayerMoney;
	string Menutitle;
	string NPCName;
	string Stationtitle;
	string UndertakerImage;
	string SelectedBody;
}
