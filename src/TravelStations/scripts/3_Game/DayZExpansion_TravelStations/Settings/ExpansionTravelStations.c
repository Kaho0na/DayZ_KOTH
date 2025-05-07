/**
 * ExpansionTravelStationsBase.c
 *
 * DayZ Expansion Mod - City Loyalty by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionTravelStationsBase
{
	int m_Version;
	[NonSerialized()]
	string m_FileName;
	int CityID;
}

class ExpansionTravelStations : ExpansionTravelStationsBase
{
	static const int VERSION = 1;
	string StationName;
	int StationType;
    string StationManagerImage;

	autoptr TStringArray SpawnPos;

	// Constructor
	void ExpansionTravelStations()
	{
		m_Version = VERSION;
		SpawnPos = new TStringArray; 
	}

	// Load from file
	static ExpansionTravelStations Load(string name)
	{
		ExpansionTravelStations settings = new ExpansionTravelStations;
		settings.Defaults();
		ExpansionJsonFileParser<ExpansionTravelStations>.Load(EXPANSION_TRAVELSTATIONS_FOLDER + name + ".json", settings);
		settings.m_FileName = name; // Save the file name so Save() works correctly later
		return settings;
	}

	// Save to file
	void Save()
	{
		JsonFileLoader<ExpansionTravelStations>.JsonSaveFile(EXPANSION_TRAVELSTATIONS_FOLDER + m_FileName + ".json", this);
	}

	// Defaults if file not found
	void Defaults()
	{
		CityID = 1;
		StationName = "Chernogorsk";
		SpawnPos = {"6594.66", "0", "2426.82"};  // TStringArray needs strings
		StationType = 1;
		StationManagerImage = "Image Here";
	}
}