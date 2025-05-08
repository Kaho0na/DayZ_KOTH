class ExpansionTravelStationsMenu: ExpansionScriptViewMenu
{
	private ref array<ref ExpansionTravelStationsMenuMapMarker> m_MapMarkers;
	private int m_SelectedLocationIndex = -1;
	protected ref ExpansionTravelStationsMenuController m_TravelStationsMenuController;
	protected ref ExpansionTravelStationsModule m_TravelStationsModule;
	protected ref ExpansionTravelStationsNPCData m_TravelStationNPCData;
	ref TravelStationLocation m_SelectedStation;
	private MissionGameplay m_Mission;


	private Widget m_MapSpacer;
	private MapWidget m_MapWidget;
	private ButtonWidget m_ConfirmButton;
	private TextWidget m_ConfirmText;
	private ImageWidget m_Background;

	private int m_NextListIndex = 0;
	private int m_TravelCost;
	private int timeToTravel;

	void ExpansionTravelStationsMenu()
	{
		Print("[ExpansionTravelStationsMenu] Constructor called");
		m_MapMarkers = new array<ref ExpansionTravelStationsMenuMapMarker>();
		Class.CastTo(m_Mission, MissionGameplay.Cast(GetGame().GetMission()));
		Class.CastTo(m_TravelStationsMenuController, GetController());
		Class.CastTo(m_TravelStationsModule, CF_ModuleCoreManager.Get(ExpansionTravelStationsModule));
		m_ConfirmButton.Show(false);
		m_ConfirmButton.Enable(false);

		int mapSize = GetGame().GetWorld().GetWorldSize() / 2;
		m_MapWidget.SetMapPos(Vector(mapSize, 0, mapSize));

		m_TravelStationsModule.GetModuleInstance().GetTravelStationMenuSI().Insert(ShowMenuNow);
	}

	void ~ExpansionTravelStationsMenu()
	{
		Clear();
	}

	override string GetLayoutFile()
	{
		return "TravelStations/GUI/layouts/ExpansionTravelStationsMenu.layout";
	}

	override typename GetControllerType()
	{
		return ExpansionTravelStationsMenuController;
	}

	override void OnShow()
	{
		super.OnShow();
		Print("[ExpansionTravelStationsMenu] OnShow called");
	
		GetGame().GetInput().ChangeGameFocus(1);
		SetFocus(GetLayoutRoot());
		PPEffects.SetBlurMenu(0.5);
		m_Mission.GetHud().ShowHud(false);
		m_Mission.GetHud().ShowQuickBar(false);
	
		m_TravelStationsMenuController.SelectedLocation = "NONE";
		m_TravelStationsMenuController.NotifyPropertyChanged("SelectedLocation");
	
		m_MapSpacer = GetLayoutRoot().FindAnyWidget("MapSpacer");
		m_MapWidget = MapWidget.Cast(GetLayoutRoot().FindAnyWidget("Map_Widget"));
		m_ConfirmButton = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("Confirm"));
		m_ConfirmText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("ConfirmText"));
		m_Background = ImageWidget.Cast(GetLayoutRoot().FindAnyWidget("Background"));
		m_Background.LoadImageFile(0, "DayZExpansion/SpawnSelection/GUI/textures/wood_background.edds");
	
	}
	

	override void OnHide()
	{
		super.OnHide();
		Print("[ExpansionTravelStationsMenu] OnHide called");
		GetGame().GetInput().ResetGameFocus();
		PPEffects.SetBlurMenu(0.0);
		m_Mission.GetHud().ShowHud(true);
		m_Mission.GetHud().ShowQuickBar(true);
		Clear();
	}

	void ShowMenuNow(int PlayerMoney, ExpansionTravelStationsNPCData npcData, array<ref TravelStationLocation> travelStations)
	{
		m_TravelStationNPCData = npcData;  // ← Update current StationNPC reference
		Print("[TravelStations] Current Station Data!" + m_TravelStationNPCData.StationName);
	
		ExpansionScriptViewMenuBase activeMenu = ExpansionScriptViewMenuBase.Cast(GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu());
		ExpansionTravelStationsMenu menu = ExpansionTravelStationsMenu.Cast(activeMenu);
	
		if (!menu)
		{
			Error("[TravelStations] ShowMenuNow - TravelStationsMenu is not the active menu.");
			return;
		}
	
		Print("[TravelStations] ShowMenuNow - Binding travelStations to existing menu.");

		m_TravelStationsMenuController.PlayerMoney = "$" + FormatNumberWithCommas(PlayerMoney);
		m_TravelStationsMenuController.NotifyPropertyChanged("PlayerMoney");
		
		menu.SetStations(npcData, travelStations);
	}
	

	void CloseMenuButtonClick()
	{
		CloseMenu();
	}

	override bool CanClose()
	{
		return true;
	}

	void CloseMenu()
	{
		GetDayZExpansion().GetExpansionUIManager().CloseMenu();
	}

	void SetStations(ExpansionTravelStationsNPCData npcData, array<ref TravelStationLocation> travelStations)
	{
		Print("[ExpansionTravelStationsMenu] SetStations called");
		int primaryColor = ARGB(255,226,65,66);
		int hoverColor = ARGB(255,255,255,255);
		m_TravelStationNPCData = npcData;
		m_TravelStationsMenuController.StationLocationEntries.Clear();
		m_MapMarkers.Clear();
		string markerIcon = "Map Marker";
		if (!travelStations || travelStations.Count() == 0)
		{
			Print("[ExpansionTravelStationsMenu] No travelStations received.");
			return;
		}

		vector playerStationPosition = npcData.Position;
		m_MapWidget.SetMapPos(Vector(playerStationPosition[0], 0, playerStationPosition[2]));

		for (int i = 0; i < travelStations.Count(); i++)
		{
			TravelStationLocation thisStation = travelStations[i];
			int currentStation = m_TravelStationNPCData.StationID;

			if (thisStation.StationID == currentStation)
				continue; //skip station player is in

			ExpansionTravelStationsMenuLocationEntry station_entry = new ExpansionTravelStationsMenuLocationEntry(m_NextListIndex, thisStation);

			station_entry.SetStationKey(thisStation.StationID.ToString());

			m_TravelStationsMenuController.StationLocationEntries.Insert(station_entry);

			/*vector thisStationPos = thisStation.Position;

			ExpansionTravelStationsMenuMapMarker marker = new ExpansionTravelStationsMenuMapMarker(m_MapSpacer, m_MapWidget, true);
			marker.SetIcon(markerIcon);
			marker.SetPosition(thisStationPos);
			marker.SetPrimaryColor(primaryColor);
			marker.SetHoverColour(hoverColor);
			marker.SetName(thisStation.StationName);
			marker.SetLocation(m_NextListIndex, thisStation);
			marker.Show();

			m_MapMarkers.Insert(marker);*/
			m_NextListIndex++;
		}

		m_TravelStationsMenuController.NotifyPropertyChanged("StationLocationEntries");
	}


	void SetTravelDestination(int index, TravelStationLocation travelStation, bool highlight = true)
	{
		m_SelectedStation = travelStation;
		
		//Change selected location on menu
		Print("[TravelStations] Selected travel destination: " + travelStation.StationName);
		m_TravelStationsMenuController.SelectedLocation = travelStation.StationName;
		m_TravelStationsMenuController.NotifyPropertyChanged("SelectedLocation");

		//Calculate cost to travel based on distance
		float distance = vector.Distance(m_TravelStationNPCData.Position, m_SelectedStation.Position);
		float m_travelcostpm = GetExpansionSettings().GetTravelStations().TravelCostPerMeter;
		m_TravelCost = Math.Round(distance * m_travelcostpm);
		m_TravelStationsMenuController.TravelCost = "$" + m_TravelCost.ToString();
		m_TravelStationsMenuController.NotifyPropertyChanged("TravelCost");

		//Calculate time to travel based on distance
		timeToTravel = Math.Clamp(Math.Floor(distance / 200), 3, 10);

		Print("[TravelStations] Distance to selected station: " + distance.ToString());

		for (int i = 0; i < m_TravelStationsMenuController.StationLocationEntries.Count(); i++)
		{
			ExpansionTravelStationsMenuLocationEntry entry = m_TravelStationsMenuController.StationLocationEntries.Get(i);
			entry.SetSelected(false);
		}

		m_TravelStationsMenuController.StationLocationEntries[index].SetSelected(true);

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

	
	void OnTravelButtonClick()
	{
		if (!m_SelectedStation || !m_SelectedStation.TPPositions || m_SelectedStation.TPPositions.Count() == 0)
		{
			Error("[TravelStations] No valid travel travelStation or positions.");
			return;
		}
	
		int index = Math.RandomInt(0, m_SelectedStation.TPPositions.Count());
		vector spawnPos = m_SelectedStation.TPPositions.Get(index);
	
		Print("[TravelStations] Requesting teleport to: " + spawnPos);
	
		// Send RPC to server
		ExpansionTravelStationsModule.GetModuleInstance().SendTeleportRequest(spawnPos, m_TravelCost, timeToTravel);
	
		CloseMenu();
	}
	
	

	void Clear()
	{
		m_TravelStationsMenuController.StationLocationEntries.Clear();
		m_MapMarkers.Clear();
	}

	ButtonWidget GetConfirmButton()
	{
		return m_ConfirmButton;
	}

	MapWidget GetTravelMapWidget()
	{
		return m_MapWidget;
	}
};

class ExpansionTravelStationsMenuController: ExpansionViewController
{
	ref ObservableCollection<ref ExpansionTravelStationsMenuLocationEntry> StationLocationEntries = new ObservableCollection<ref ExpansionTravelStationsMenuLocationEntry>(this);
	string SelectedLocation;
	string TravelCost;
	string PlayerMoney;
}