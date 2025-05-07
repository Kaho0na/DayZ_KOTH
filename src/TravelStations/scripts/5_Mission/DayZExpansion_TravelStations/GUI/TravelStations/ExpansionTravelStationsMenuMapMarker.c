class ExpansionTravelStationsMenuMapMarker : ExpansionMapWidgetBase
{
	protected int m_Index;
	protected ref TravelStationLocation m_Location;
	protected bool m_IsSelected = false;
	protected string m_StationKey;
	protected ExpansionTravelStationsModule m_TravelStationsModule;

	void ExpansionTravelStationsMenuMapMarker(Widget parent, MapWidget mapWidget, bool autoInit = true)
	{
		CF_Modules<ExpansionTravelStationsModule>.Get(m_TravelStationsModule);
	}

	override void OnInit(Widget layoutRoot)
	{
		super.OnInit(layoutRoot);
	}
	
	void SetLocation(int index, TravelStationLocation location)
	{
		m_Index = index;
		m_Location = location;
		SetPosition(m_Location.Position);
	}

	override void Update(float pDt)
	{
		float x, y;
		GetLayoutRoot().GetParent().GetScreenPos(x, y);
		vector mapPos = GetMapWidget().MapToScreen(m_WorldPosition);	
		GetLayoutRoot().SetPos(mapPos[0] - x, mapPos[1] - y, true);
	}

	override void Show()
	{
		super.Show();
	}

	void SetStationKey(string key)
	{
		m_StationKey = key;
	}

	string GetStationKey()
	{
		return m_StationKey;
	}

	int GetIndex()
	{
		return m_Index;
	}

	override void OnMarkerClick()
	{
		if (m_IsSelected)
			return;

		ExpansionTravelStationsMenu travelMenu = ExpansionTravelStationsMenu.Cast(GetDayZExpansion().GetExpansionUIManager().GetMenu());
		if (!travelMenu)
			return;

		travelMenu.SetTravelDestination(m_Index, m_Location);
	}

	void Lock()
	{
		m_IsSelected = true;
	}

	void Unlock()
	{
		m_IsSelected = false;
	}

	override bool CanDrag()
	{
		return false;
	}
}