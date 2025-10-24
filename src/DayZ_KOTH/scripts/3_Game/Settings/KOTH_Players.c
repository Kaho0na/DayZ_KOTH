/**
 * KOTH_Players.c
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

class KOTH_PlayersBase
{
	int m_Version;
	[NonSerialized()]
	string m_FileName;
	string PlayerID;
}

class KOTH_Players : KOTH_PlayersBase
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

	ref array<string> OwnedWeapons;
	ref array<string> OwnedAttachments;

	// Constructor
	void KOTH_Players()
	{
		m_Version = VERSION;
		OwnedWeapons = new array<string>();
    	OwnedAttachments = new array<string>();
	}

	// Load from file
	static KOTH_Players Load(string name)
	{
		KOTH_Players settings = new KOTH_Players;
		settings.Defaults();
		ExpansionJsonFileParser<KOTH_Players>.Load(EXPANSION_KOTH_Players + name + ".json", settings);
		settings.m_FileName = name; // Save the file name so Save() works correctly later

		return settings;
	}

	// Save to file
	void Save()
	{
		JsonFileLoader<KOTH_Players>.JsonSaveFile(EXPANSION_KOTH_Players + m_FileName + ".json", this);
	}

	void AddOwnedItem(string itemClass)
	{
		if (!HasOwnedItem(itemClass))
		{
			OwnedWeapons.Insert(itemClass);
		}
	}

	bool HasOwnedItem(string itemClass)
	{
		return OwnedWeapons.Find(itemClass) != -1;
	}

	array<string> GetOwnedWeapons()
	{
		return OwnedWeapons;
	}

	void RemoveOwnedItem(string itemClass)
	{
		int idx = OwnedWeapons.Find(itemClass);
		if (idx != -1)
		{
			OwnedWeapons.Remove(idx);
		}
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