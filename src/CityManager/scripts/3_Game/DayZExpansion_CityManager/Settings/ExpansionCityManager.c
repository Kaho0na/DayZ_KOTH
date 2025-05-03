class ExpansionCityManagerBase
{
	int m_Version;

	[NonSerialized()]
	string m_FileName;

	int CityID;
}

class ExpansionCityManager : ExpansionCityManagerBase
{
	static const int VERSION = 1;

	string CityName;
	int CityRadius;
    int CityCurrentLoyalty;
    int CityMaxReachedLoyalty;
    bool CityLiberated;

	autoptr TStringArray CityPos;

	// Constructor
	void ExpansionCityManager()
	{
		m_Version = VERSION;
		CityPos = new TStringArray; 
	}

	// Load from file
	static ExpansionCityManager Load(string name)
	{
		ExpansionCityManager settings = new ExpansionCityManager;
		settings.Defaults();
		
		ExpansionJsonFileParser<ExpansionCityManager>.Load(EXPANSION_CITYMANAGER_FOLDER + name + ".json", settings);

		settings.m_FileName = name; // Save the file name so Save() works correctly later

		return settings;
	}

	// Save to file
	void Save()
	{
		JsonFileLoader<ExpansionCityManager>.JsonSaveFile(EXPANSION_CITYMANAGER_FOLDER + m_FileName + ".json", this);
	}

	// Defaults if file not found
	void Defaults()
	{
		CityID = 1;
		CityName = "Chernogorsk";
		CityPos = {"6594.66", "0", "2426.82"};  // TStringArray needs strings
		CityRadius = 500;
		CityCurrentLoyalty = 0;
		CityMaxReachedLoyalty = 1000;
		CityLiberated = false;
	}
}