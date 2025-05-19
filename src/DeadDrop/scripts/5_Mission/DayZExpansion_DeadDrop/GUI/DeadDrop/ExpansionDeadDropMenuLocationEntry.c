/**
 * ExpansionDeadDropMenuLocationEntry.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionDeadDropMenuLocationEntry: ExpansionScriptView
{
	protected int m_Index;
	protected ref ExpansionDeadDropPlayerData m_DeadDropData;
	protected ref ExpansionDeadDropMenuLocationEntryController m_DeadDropEntryController;
	protected ExpansionDeadDropModule m_DeadDropModule;
	protected bool m_IsLocked = false;
	protected bool m_IsHighlighted = false;
	protected bool m_IsSelected = false;
	protected string m_EntryKey;
	
	protected ButtonWidget body_entry;
	protected Widget background;
	protected TextWidget BodyName;
	protected ImageWidget icon;
	protected ImageWidget icon_locked;
	protected ImageWidget cooldown_icon;
	protected TextWidget cooldown;
	
	void ExpansionDeadDropMenuLocationEntry(int index, ExpansionDeadDropPlayerData data)
	{
		m_Index = index;
		m_DeadDropData = data;
		
		Class.CastTo(m_DeadDropEntryController, GetController());
		CF_Modules<ExpansionDeadDropModule>.Get(m_DeadDropModule);
		
		SetEntry();
	}

	ExpansionDeadDropPlayerData GetDeadDropData()
	{
		return m_DeadDropData;
	}

	override string GetLayoutFile()
	{
		return "DeadDrop/GUI/layouts/ExpansionDeadDropMenuEntry.layout";
	}
	
	override typename GetControllerType() 
	{
		return ExpansionDeadDropMenuLocationEntryController;
	}

	private void SetEntry()
	{
		SetDisplayName(m_DeadDropData.GetBodyName());
		SetIcon(ExpansionIcons.GetPath("Skull"));
		
		if (m_DeadDropData.recovered)
			SetLocked();
		else
			SetUnlocked();

		SetDisplayTime(m_DeadDropData.GetRecoveryTimeInt());

	}
		
	void SetLocked()
	{
		m_IsLocked = true;
		icon.Show(false);
		icon_locked.Show(true);
		cooldown_icon.Show(true);
		cooldown.Show(true);
		background.SetColor(ARGB(255,106,0,0));
	}
	
	void SetUnlocked()
	{
		icon.Show(true);
		icon_locked.Show(false);
		cooldown_icon.Show(false);
		cooldown.Show(false);
		if (m_IsHighlighted)
			background.SetColor(ARGB(255,220,220,220));
		else
			background.SetColor(ARGB(255,0,0,0));

	}
	
	void OnEntryButtonClick()
	{
		if (m_IsSelected)
			return;
		
		ExpansionDeadDropMenu menu = ExpansionDeadDropMenu.Cast(GetDayZExpansion().GetExpansionUIManager().GetMenu());
		if (!menu)
			return;
		
		menu.SetRecoveryTarget(m_Index, m_DeadDropData, true);
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w == body_entry)
			SetHighlight();
		
		return super.OnMouseEnter(w, x, y);
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w == body_entry && !m_IsSelected)
			SetNormal();
		
		return super.OnMouseLeave(w, enterW, x, y);
	}
	
	void SetHighlight()
	{
		m_IsHighlighted = true;
		background.SetColor(ARGB(255,220,220,220));
		BodyName.SetColor(ARGB(255,0,0,0));
		icon.SetColor(ARGB(255,0,0,0));
	}
	
	void SetNormal()
	{
		m_IsHighlighted = false;
		background.SetColor(ARGB(0,0,0,0));
		BodyName.SetColor(ARGB(255,255,255,255));
		icon.SetColor(ARGB(255,220,220,220));
	}

	void SetSelected(bool selected)
	{
		m_IsSelected = selected;
		if (selected)
		{
			background.SetColor(ARGB(255, 100, 0, 0));
			BodyName.SetColor(ARGB(255, 255, 255, 255));
			icon.SetColor(ARGB(255, 255, 255, 255));
		}
		else
		{
			SetNormal();
		}
	}
	
	void SetDisplayName(string name)
	{
		m_DeadDropEntryController.LocationName = name;
		m_DeadDropEntryController.NotifyPropertyChanged("LocationName");
	}
	
	void SetDisplayTime(int seconds)
	{
		m_DeadDropEntryController.Cooldown = ExpansionStatic.FormatTime(seconds, false);
		m_DeadDropEntryController.NotifyPropertyChanged("Cooldown");
	}
	
	void SetIcon(string iconName)
	{
		m_DeadDropEntryController.EntryIcon = iconName;
		m_DeadDropEntryController.NotifyPropertyChanged("EntryIcon");
	}
	
	override float GetUpdateTickRate()
	{
		return 1.0;
	}
	
	void Lock() { m_IsSelected = true; }
	void Unlock() { m_IsSelected = false; }
	int GetIndex() { return m_Index; }

	void SetEntryKey(string key) { m_EntryKey = key; }
	string GetEntryKey() { return m_EntryKey; }
};

class ExpansionDeadDropMenuLocationEntryController: ExpansionViewController 
{
	string EntryIcon;
	string LocationName;
	string Cooldown;
};