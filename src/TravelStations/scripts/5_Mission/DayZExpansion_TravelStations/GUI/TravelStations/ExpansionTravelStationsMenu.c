enum TravelStationType
{
	TRAIN = 1,
	BUS,
	TAXI,
}
class ExpansionTravelStationsMenu: ExpansionScriptViewMenu
{
	private ref array<ref ExpansionTravelStationsMenuMapMarker> m_MapMarkers;
	private int m_SelectedLocationIndex = -1;
	protected ref ExpansionTravelStationsMenuController m_TravelStationsMenuController;
	protected ref ExpansionTravelStationsModule m_TravelStationsModule;
	protected ref ExpansionTravelStationsNPCData m_TravelStationNPCData;
	ref TravelStationLocation m_SelectedStation;
	private MissionGameplay m_Mission;


	private Widget MapSpacer;
	private MapWidget Map_Widget;
	private ButtonWidget m_ConfirmButton;
	private TextWidget m_ConfirmText;
	private TextWidget m_ConfirmButtonText;
	private ImageWidget m_Background;
	private ImageWidget m_BlackoutOverlay;
	private TextWidget m_CountdownText;
	private TextWidget m_StoryText;
	private TextWidget m_TravelLabel;

	private int m_NextListIndex = 0;
	private int m_TravelCost;
	private int timeToTravel;
	private bool m_isCooldown = false;
	private int m_PlayerMoney;

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

		m_TravelStationsModule.GetModuleInstance().GetTravelStationMenuSI().Insert(ShowMenuNow);
	}

	void ~ExpansionTravelStationsMenu()
	{
		Clear();
		m_SelectedStation = null;
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
	
		m_ConfirmButton = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("Confirm"));
		m_ConfirmText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("ConfirmText"));
		m_ConfirmButtonText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("ConfirmButtonText"));
		m_Background = ImageWidget.Cast(GetLayoutRoot().FindAnyWidget("Background"));
		m_Background.LoadImageFile(0, "TravelStations/GUI/backgrounds/TravelStation.paa");
		m_BlackoutOverlay = GetLayoutRoot().FindAnyWidget("BlackoutOverlay");
		m_CountdownText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("CountdownText"));
		m_StoryText = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("StoryText"));
		m_TravelLabel = TextWidget.Cast(GetLayoutRoot().FindAnyWidget("TravelCostLabel"));

		if (m_BlackoutOverlay)
			m_BlackoutOverlay.Show(false);  // Force-hide on menu open

	}
	
	override void OnHide()
	{
		super.OnHide();
		Print("[ExpansionTravelStationsMenu] OnHide called");
		GetGame().GetInput().ResetGameFocus();
		PPEffects.SetBlurMenu(0.0);
		m_Mission.GetHud().ShowHud(true);
		m_Mission.GetHud().ShowQuickBar(true);
		if (m_BlackoutOverlay)
			m_BlackoutOverlay.Show(false);

		Clear();
	}

	void ShowMenuNow(int p_PlayerMoney, ExpansionTravelStationsNPCData npcData, array<ref TravelStationLocation> travelStations)
	{
		if(m_isCooldown)
			return;

		m_PlayerMoney = p_PlayerMoney;
		Print("[TravelStations] Player Money = " + m_PlayerMoney);
		m_TravelStationNPCData = npcData;  // ← Update current StationNPC reference
		Print("[TravelStations] Current Station Data! = " + m_TravelStationNPCData.StationName);
	
		ExpansionScriptViewMenuBase activeMenu = ExpansionScriptViewMenuBase.Cast(GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu());
		ExpansionTravelStationsMenu menu = ExpansionTravelStationsMenu.Cast(activeMenu);
	
		if (!menu)
		{
			Error("[TravelStations] ShowMenuNow - TravelStationsMenu is not the active menu.");
			return;
		}
	
		Print("[TravelStations] ShowMenuNow - Binding travelStations to existing menu.");

		m_TravelStationsMenuController.PlayerMoney = "$" + FormatNumberWithCommas(m_PlayerMoney);
		m_TravelStationsMenuController.NotifyPropertyChanged("PlayerMoney");

		//Set Map Focus
		Map_Widget.SetMapPos(Vector(m_TravelStationNPCData.Position[0], 0, m_TravelStationNPCData.Position[2]));
		Map_Widget.SetScale(0.1);

		//Set Stations
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
		m_TravelStationNPCData = npcData;
		m_TravelStationsMenuController.StationLocationEntries.Clear();
		m_MapMarkers.Clear();

		if (!travelStations || travelStations.Count() == 0)
		{
			Print("[ExpansionTravelStationsMenu] No travelStations received.");
			return;
		}

		m_TravelStationsMenuController.Stationtitle = m_TravelStationNPCData.StationName + " " + GetStationTypeName(m_TravelStationNPCData.StationType);
		m_TravelStationsMenuController.NotifyPropertyChanged("Stationtitle");

		m_TravelStationsMenuController.NPCName = m_TravelStationNPCData.DefaultNPCText;
		m_TravelStationsMenuController.NotifyPropertyChanged("NPCName");

		m_TravelStationsMenuController.StationManagerImage = m_TravelStationNPCData.StationManagerImage;
		m_TravelStationsMenuController.NotifyPropertyChanged("StationManagerImage");

		vector playerStationPosition = npcData.Position;
		int currentStation = m_TravelStationNPCData.StationID;
		int npc_StationType = m_TravelStationNPCData.StationType;

		for (int i = 0; i < travelStations.Count(); i++)
		{
			TravelStationLocation thisStation = travelStations[i];
			int curr_StationType = thisStation.StationType;

			if(npc_StationType == curr_StationType)
			{
				if (thisStation.StationID == currentStation)
					continue; //skip station player is in

				ExpansionTravelStationsMenuLocationEntry station_entry = new ExpansionTravelStationsMenuLocationEntry(m_NextListIndex, thisStation);

				station_entry.SetStationKey(thisStation.StationID.ToString());

				m_TravelStationsMenuController.StationLocationEntries.Insert(station_entry);

				//Add New Marker
				int primaryColor = ARGB(255, 220, 60, 60);
				int liberatedColor = ARGB(255, 50, 200, 90);
				int hoverColor = ARGB(255,255,255,255);
				ExpansionTravelStationsMenuMapMarker marker = new ExpansionTravelStationsMenuMapMarker(MapSpacer, Map_Widget, true);
				string markerIcon = "Map Marker";
				marker.SetIcon(markerIcon);
				marker.SetPosition(thisStation.Position);
				if(thisStation.IsLiberated)
					marker.SetPrimaryColor(liberatedColor);
				else
					marker.SetPrimaryColor(primaryColor);
				marker.SetHoverColour(hoverColor);
				marker.SetName(thisStation.StationName);
				marker.SetLocation(m_NextListIndex, thisStation);
				marker.Show();
				m_MapMarkers.Insert(marker);
				m_NextListIndex++;
			}

		}

		m_TravelStationsMenuController.NotifyPropertyChanged("StationLocationEntries");
	}


	void SetTravelDestination(int index, TravelStationLocation travelStation, bool highlight = true)
	{
		m_SelectedStation = travelStation;

		bool m_isLiberated = travelStation.IsLiberated;
		Print("SetTravelDestination : travelStation.CityID" + travelStation.CityID);
		if(!m_isLiberated)
		{
			m_TravelStationsMenuController.TravelCost = "Enemy Territory";
			m_TravelStationsMenuController.NotifyPropertyChanged("TravelCost");
			m_TravelLabel.SetText("Station Status:");

			m_TravelStationsMenuController.SelectedLocation = travelStation.StationName + "(N/A)";
			m_TravelStationsMenuController.NotifyPropertyChanged("SelectedLocation");
    		m_ConfirmButtonText.SetText("UNAVAILABLE");
		}
		else
		{
			//Calculate cost to travel based on distance
			float distance = vector.Distance(m_TravelStationNPCData.Position, m_SelectedStation.Position);
			float m_travelcostpm = GetExpansionSettings().GetTravelStations().TravelCostPerMeter;
			m_TravelCost = Math.Round(distance * m_travelcostpm);
			m_TravelLabel.SetText("Travel Cost:");
			m_TravelStationsMenuController.TravelCost = "$" + m_TravelCost.ToString();
			m_TravelStationsMenuController.NotifyPropertyChanged("TravelCost");

			//Calculate time to travel based on distance
			int distanceTime = Math.Floor(distance / 1000);
			timeToTravel = Math.Clamp(distanceTime, 5, 20);

			//Change selected location on menu
			m_TravelStationsMenuController.SelectedLocation = travelStation.StationName + "(" + timeToTravel + "s)";
			m_TravelStationsMenuController.NotifyPropertyChanged("SelectedLocation");
			m_ConfirmButtonText.SetText("TRAVEL");
		}

		for (int i = 0; i < m_TravelStationsMenuController.StationLocationEntries.Count(); i++)
		{
			ExpansionTravelStationsMenuLocationEntry entry = m_TravelStationsMenuController.StationLocationEntries.Get(i);
			entry.SetSelected(false);
		}

		//Set New Map Focus
		Map_Widget.SetMapPos(Vector(travelStation.Position[0], 0, travelStation.Position[2]));
		Map_Widget.SetScale(0.3);  // optional zoom level

		m_TravelStationsMenuController.StationLocationEntries[index].SetSelected(true);
	}

	MapWidget GetMapWidget()
	{
		return Map_Widget;
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
		if (!m_SelectedStation.IsLiberated)
		{
			Print("OnTravelButtonClick: Target station is not liberated.");

			TStringArray blockMessages = {
				"Travel to this city is currently prohibited. Hostile occupation forces have imposed a lockdown.",
				"This city remains under enemy control. Resistance fighters advise against entering.",
				"Entry into this zone is restricted. Unauthorized travel may result in arrest or worse.",
				"The roads ahead are blocked by checkpoints and patrols. Better to wait until liberation.",
				"City is under occupation. Travel is not allowed."
			};
			
			int index = Math.RandomInt(0, blockMessages.Count());
			ExpansionNotification("Travel Blocked", blockMessages[index]).Error();
			return;
		}

		if (m_PlayerMoney < m_TravelCost)
		{
			ExpansionNotification("Insufficient Funds", "Not enough funds in your ATM account. Maybe try walking?!").Error();
			return;
		}

		if (!m_SelectedStation || !m_SelectedStation.TPPositions || m_SelectedStation.TPPositions.Count() == 0)
		{
			Error("[TravelStations] No valid travel station or teleport positions.");
			return;
		}

		new ExpansionTravelStationsConfirmDialog(this, m_SelectedStation.StationName, m_TravelCost);
	}


	void OnConfirmTravel()
	{
		int index = Math.RandomInt(0, m_SelectedStation.TPPositions.Count());
		vector spawnPos = m_SelectedStation.TPPositions.Get(index);

		ExpansionTravelStationsModule.GetModuleInstance().SendTeleportRequest(spawnPos, m_TravelCost);
		ShowPostTeleportCountdown(timeToTravel);
	}


	void ShowPostTeleportCountdown(int seconds)
	{
		if (m_BlackoutOverlay)
			m_BlackoutOverlay.Show(true);
		
				if (m_BlackoutOverlay)
			m_BlackoutOverlay.Show(true);

		m_CountdownText.SetText("Traveling in " + seconds.ToString() + "...");

		string m_StoryLines;
		string m_StoryImage;
		TStringArray storyLines = {
				"The train pulls away from the station… a gust of wind through shattered windows makes you wonder how Chernarus fell so far.",
				"Steel wheels scream over rusted tracks. You close your eyes. For a second, it feels like the war never happened.",
				"The train moves. Smoke rises. Somewhere out there, someone’s still planting tomatoes like it matters.",
				"The train rattles on. Another town, another bribe. Chernarus doesn’t run on tracks—it runs on corruption.",
				"Wind in your face. Smoke in your lungs. Somewhere in the distance, a town forgets you ever left."
			};
		
		TStringArray loadingImage = {
			"TravelStations/GUI/backgrounds/travelloading.paa",
			"TravelStations/GUI/backgrounds/babushka.paa",
			"TravelStations/GUI/backgrounds/soldiers.paa",
		};

		int Storyindex = Math.RandomInt(0, storyLines.Count());
		m_StoryLines = storyLines[Storyindex];
		m_StoryText.SetText(m_StoryLines);

		int loadingImageindex = Math.RandomInt(0, loadingImage.Count());
		m_StoryImage = loadingImage[loadingImageindex];
		m_BlackoutOverlay.LoadImageFile(0, m_StoryImage);

		m_ConfirmText.SetText("Arriving in " + seconds.ToString() + "...");

		for (int i = 1; i <= seconds; i++)
		{
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateCountdownText, i * 1000, false, seconds - i);
		}

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OnPostTravelCountdownEnd, seconds * 1000, false);
	}

	void OnPostTravelCountdownEnd()
	{
		HideBlackout();
		CloseMenu();
	}


	void UpdateCountdownText(int secondsLeft)
	{
		m_CountdownText.SetText("Traveling in " + secondsLeft.ToString() + "...");
	}

	void HideBlackout()
	{
		if (m_BlackoutOverlay)
		{
			m_BlackoutOverlay.Show(false);
		}
	}
	
	void Clear()
	{
		m_TravelStationsMenuController.StationLocationEntries.Clear();
		m_MapMarkers.Clear();
		if (m_BlackoutOverlay)
			m_BlackoutOverlay.Show(false);

	}

	override void Update(float dt)
	{
		if (GetLayoutRoot().IsVisible())
		{
			for ( int i = 0; i < m_MapMarkers.Count(); i++ )
			{
				m_MapMarkers[i].Update(dt);
			}
		}
	}

	string GetStationTypeName(int type)
	{
		switch (type)
		{
			case TravelStationType.TRAIN:
				return "Train Station";
			case TravelStationType.BUS:
				return "Bus Station";
			case TravelStationType.TAXI:
				return "Taxi Station";
			default:
				return "Unknown Station";
		}

		return "Unknown Station"; 
	}


	ButtonWidget GetConfirmButton()
	{
		return m_ConfirmButton;
	}

	MapWidget GetTravelMapWidget()
	{
		return Map_Widget;
	}
};

class ExpansionTravelStationsMenuController: ExpansionViewController
{
	ref ObservableCollection<ref ExpansionTravelStationsMenuLocationEntry> StationLocationEntries = new ObservableCollection<ref ExpansionTravelStationsMenuLocationEntry>(this);
	string SelectedLocation;
	string TravelCost;
	string PlayerMoney;
	string Stationtitle;
	string NPCName;
	string StationManagerImage;
	string StoryText;
	string StoryImage;
}