/**
 * DayZ_KOTH_Players.c
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

class DayZ_KOTH_PlayersBase
{
	int m_Version;
	[NonSerialized()]
	string m_FileName;
	string PlayerID;
}

class DayZ_KOTH_Players : DayZ_KOTH_PlayersBase
{
	static const int VERSION = 1;
	string PlayerName;
	int TotalMoneyinBank;
	int TotalExperienceEarned;
	int CurrentLevel;
	int TotalTimePlayed;
	int LongestKill;
	int LongestHeadshot;
	int HighestKillstreak;
	int TotalEnemiesKilled;
	string LastTeamSelection;

	// Constructor
	void DayZ_KOTH_Players()
	{
		m_Version = VERSION;
	}

	// Load from file
	static DayZ_KOTH_Players Load(string name)
	{
		DayZ_KOTH_Players settings = new DayZ_KOTH_Players;
		settings.Defaults();
		ExpansionJsonFileParser<DayZ_KOTH_Players>.Load(EXPANSION_DAYZ_KOTH_PLAYERS + name + ".json", settings);
		settings.m_FileName = name; // Save the file name so Save() works correctly later
		return settings;
	}

	// Save to file
	void Save()
	{
		JsonFileLoader<DayZ_KOTH_Players>.JsonSaveFile(EXPANSION_DAYZ_KOTH_PLAYERS + m_FileName + ".json", this);
	}

	// Defaults if file not found
	void Defaults()
	{
		PlayerID = "";
		PlayerName = "New Player";
		TotalMoneyinBank = 0;
		TotalExperienceEarned = 0;
		CurrentLevel = 1;
		TotalTimePlayed = 0;
		LongestKill = 0;
		LongestHeadshot = 0;
		HighestKillstreak = 0;
		TotalEnemiesKilled = 0;
		LastTeamSelection = "None";
	}

}