/**
 * ExpansionTravelStationsMenuLocationEntry.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionTravelStationsMenuLocationEntry: ExpansionScriptView
{
	protected int m_Index;
	protected ref TravelStationLocation m_Station;
	protected ref ExpansionTravelStationsMenuLocationEntryController m_TravelStationsEntryController;
	protected ExpansionTravelStationsModule m_TravelStationsModule;
	protected bool m_IsLocked = false;
	protected bool m_IsHighlighted = false;
	protected bool m_IsSelected = false;
	protected string m_StationKey;
	
	protected ButtonWidget spawn_entry;
	protected Widget background;
	protected TextWidget StationName;
	protected ImageWidget icon;
	protected ImageWidget icon_locked;
	protected ImageWidget cooldown_icon;
	protected TextWidget cooldown;
	
	void ExpansionTravelStationsMenuLocationEntry(int index, TravelStationLocation location)
	{
		m_Index = index;
		m_Station = location;
		
		Class.CastTo(m_TravelStationsEntryController, GetController());
		CF_Modules<ExpansionTravelStationsModule>.Get(m_TravelStationsModule);
		
		SetEntry();
	}

    TravelStationLocation GetStation()
    {
        return m_Station;
    }

	override string GetLayoutFile()
	{
		return "TravelStations/GUI/layouts/ExpansionTravelStationsMenuEntry.layout";
	}
	
	override typename GetControllerType() 
	{
		return ExpansionTravelStationsMenuLocationEntryController;
	}

	private void SetEntry()
	{
		SetDisplayName(m_Station.StationName);
		SetIcon(ExpansionIcons.GetPath("Marker"));
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
		if (!m_IsHighlighted)
			background.SetColor(ARGB(255,0,0,0));
		else
			background.SetColor(ARGB(255,220,220,220));
	}
	
	void OnEntryButtonClick()
	{
		if (m_IsSelected)
			return;
		
		ExpansionTravelStationsMenu travelStationsMenu = ExpansionTravelStationsMenu.Cast(GetDayZExpansion().GetExpansionUIManager().GetMenu());
		if (!travelStationsMenu)
			return;
		
		travelStationsMenu.SetTravelDestination(m_Index, m_Station, true);
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		switch (w)
		{
		case spawn_entry:
			SetHighlight();
			break;
		}
		
		return super.OnMouseEnter(w, x, y);;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		switch (w)
		{
		case spawn_entry:
			if (!m_IsSelected)
			SetNormal();
			break;
		}
		
		return super.OnMouseLeave(w, enterW, x, y);
	}
	
	void SetHighlight()
	{
		m_IsHighlighted = true;
		background.SetColor(ARGB(255,220,220,220));
		StationName.SetColor(ARGB(255,0,0,0));
		icon.SetColor(ARGB(255,0,0,0));
	}
	
	void SetNormal()
	{
		m_IsHighlighted = false;
		background.SetColor(ARGB(0,0,0,0));
		StationName.SetColor(ARGB(255,255,255,255));
		icon.SetColor(ARGB(255,220,220,220));
	}

	void SetSelected(bool selected)
	{
		m_IsSelected = selected;

		if (selected)
		{
			background.SetColor(ARGB(255, 100, 0, 0));
			StationName.SetColor(ARGB(255, 255, 255, 255));
			icon.SetColor(ARGB(255, 255, 255, 255));
		}
		else
		{
			SetNormal();  // fallback to normal coloring
		}
	}
	
	void SetDisplayName(string locationName)
	{
		m_TravelStationsEntryController.LocationName = locationName;
		m_TravelStationsEntryController.NotifyPropertyChanged("LocationName");
	}
	
	void SetDisplayTime(int cooldownTime)
	{
		m_TravelStationsEntryController.Cooldown = ExpansionStatic.FormatTime(cooldownTime, false);
		m_TravelStationsEntryController.NotifyPropertyChanged("Cooldown");
	}
	
	void SetIcon(string iconName)
	{
		m_TravelStationsEntryController.EntryIcon = iconName;
		m_TravelStationsEntryController.NotifyPropertyChanged("EntryIcon");
	}
	
	override float GetUpdateTickRate()
	{
		return 1.0;
	}
	
	void Lock()
	{
		m_IsSelected = true;
	}
	
	void Unlock()
	{
		m_IsSelected = false;
	}
	
	int GetIndex()
	{
		return m_Index;
	}
	
	void SetStationKey(string key)
	{
		m_StationKey = key;
	}
	
	string GetStationKey()
	{
		return m_StationKey;
	}
};

class ExpansionTravelStationsMenuLocationEntryController: ExpansionViewController 
{
	string EntryIcon;
	string LocationName;
	string Cooldown;
};