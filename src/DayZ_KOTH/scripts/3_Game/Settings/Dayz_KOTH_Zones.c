/**
 * Dayz_KOTH_Zones.c
 *
 * King of the Hill by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class Dayz_KOTH_ZonesBase
{
	int m_Version;
	[NonSerialized()]
	string m_FileName;
	int ZoneID;
}

class Dayz_KOTH_Zones : Dayz_KOTH_ZonesBase
{
	static const int VERSION = 1;
	string ZoneName;
	autoptr TStringArray EastSpawnBuilding;
	autoptr TStringArray WestSpawnBuilding;
	int EastSafeZoneRadius;
	int WestSafeZoneRadius;
	autoptr TStringArray AOZoneCenter;
	int AOZoneRadius;
	int PriorityAORadius;

	// Constructor
	void Dayz_KOTH_Zones()
	{
		m_Version = VERSION;
		EastSpawnBuilding = new TStringArray;
		WestSpawnBuilding = new TStringArray;
		AOZoneCenter = new TStringArray;
	}

	// Load from file
	static Dayz_KOTH_Zones Load(string name)
	{
		Dayz_KOTH_Zones settings = new Dayz_KOTH_Zones;
		settings.Defaults();
		ExpansionJsonFileParser<Dayz_KOTH_Zones>.Load(EXPANSION_DAYZ_KOTH_ZONES + name + ".json", settings);
		settings.m_FileName = name; // Save the file name so Save() works correctly later
		return settings;
	}

	// Save to file
	void Save()
	{
		JsonFileLoader<Dayz_KOTH_Zones>.JsonSaveFile(EXPANSION_DAYZ_KOTH_ZONES + m_FileName + ".json", this);
	}

	// Defaults if file not found
	void Defaults()
	{
		ZoneID = 1;
		ZoneName = "Chernogorsk";
		EastSpawnBuilding = {"6594.66", "0", "2426.82"};  // TStringArray needs strings
		WestSpawnBuilding = {"6204.66", "0", "2426.82"};
		EastSafeZoneRadius = 50;
		WestSafeZoneRadius = 50;
		AOZoneCenter = {"6400", "0", "2426.82"};
		AOZoneRadius = 400;
		PriorityAORadius = 200;
	}
}