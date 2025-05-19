class ExpansionDeadDropBase
{
	int m_Version;

	[NonSerialized()]
	string m_FileName;
}

class ExpansionDeadDrop : ExpansionDeadDropBase
{
	string player_id;
	string player_name;
	string steam_id;
	string death_time;
	bool recovered;
	string recovery_time;
	string recovered_by;
	string cause_of_death;
	vector location;

	ref array<ref DeadDropItem> items;


	// Constructor
	void ExpansionDeadDrop()
	{
		items = new array<ref DeadDropItem>();
	}

	// Load from file
	static ExpansionDeadDrop Load(string name)
	{
		ExpansionDeadDrop settings = new ExpansionDeadDrop;
		settings.Defaults();
		
		ExpansionJsonFileParser<ExpansionDeadDrop>.Load(EXPANSION_DEADDROP_FOLDER + "Active" + name + ".json", settings);

		settings.m_FileName = name; // Save the file name so Save() works correctly later

		return settings;
	}

	// Save to file
	void Save()
	{
		JsonFileLoader<ExpansionDeadDrop>.JsonSaveFile(EXPANSION_DEADDROP_FOLDER + "Active" + m_FileName + ".json", this);
	}

	// Defaults if file not found
	void Defaults()
	{
		player_id = "";
		player_name = "";
		steam_id = "";
		death_time = "";
		recovered = false;
		recovery_time = "";
		recovered_by = "";
		cause_of_death = "";
		location = vector.Zero;

		items = new array<ref DeadDropItem>();
	}
}