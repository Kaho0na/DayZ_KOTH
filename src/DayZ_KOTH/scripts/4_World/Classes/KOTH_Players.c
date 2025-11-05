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
	static KOTH_Players Load(string fileName)
	{
		// Clean up input (remove spaces, ensure .json extension)
		fileName.Replace(" ", "");
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";

		string path = EXPANSION_KOTH_Players + fileName;

		KOTH_Players settings = new KOTH_Players;

		if (FileExist(path))
		{
			ExpansionJsonFileParser<KOTH_Players>.Load(path, settings);
			Print("[KOTH_Players] Loaded existing player data: " + path);
		}
		else
		{
			Print("[KOTH_Players] No existing player file found for " + fileName + ". Creating new defaults.");
			settings.Defaults();
			settings.m_FileName = fileName.Substring(0, fileName.IndexOf(".json")); // store raw UID for later saves
			settings.Save();
		}

		// Always set metadata
		settings.m_Version = VERSION;
		settings.m_FileName = fileName.Substring(0, fileName.IndexOf(".json"));

		return settings;
	}


	// Save to file
	void Save()
	{
		string path = EXPANSION_KOTH_Players + m_FileName + ".json";

		// Ensure the directory exists before saving
		if (!FileExist(EXPANSION_KOTH_Players))
		{
			MakeDirectory(EXPANSION_KOTH_Players);
			Print("[KOTH_Players] Created directory: " + EXPANSION_KOTH_Players);
		}

		// Save player data
		JsonFileLoader<KOTH_Players>.JsonSaveFile(path, this);
		Print("[KOTH_Players] Saved player data: " + path);
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
		OwnedWeapons = new array<string>();
		OwnedAttachments = new array<string>();
	}

	void SetPlayerID(string id)
    {
        PlayerID = id;
    }

    string GetPlayerID()
    {
        return PlayerID;
    }

    void SetPlayerName(string name)
    {
        PlayerName = name;
    }

    string GetPlayerName()
    {
        return PlayerName;
    }

    void SetTotalMoneyinBank(int money)
    {
        TotalMoneyinBank = money;
    }

    int GetTotalMoneyinBank()
    {
        return TotalMoneyinBank;
    }

    void SetTotalExperienceEarned(int xp)
    {
        TotalExperienceEarned = xp;
    }

    int GetTotalExperienceEarned()
    {
        return TotalExperienceEarned;
    }

    void SetCurrentLevel(int level)
    {
        CurrentLevel = level;
    }

    int GetCurrentLevel()
    {
        return CurrentLevel;
    }

    void SetTotalTimePlayed(int time)
    {
        TotalTimePlayed = time;
    }

    int GetTotalTimePlayed()
    {
        return TotalTimePlayed;
    }

    void SetLongestKill(int distance)
    {
        LongestKill = distance;
    }

    int GetLongestKill()
    {
        return LongestKill;
    }

    void SetLongestHeadshot(int distance)
    {
        LongestHeadshot = distance;
    }

    int GetLongestHeadshot()
    {
        return LongestHeadshot;
    }

    void SetHighestKillstreak(int streak)
    {
        HighestKillstreak = streak;
    }

    int GetHighestKillstreak()
    {
        return HighestKillstreak;
    }

    void SetTotalEnemiesKilled(int kills)
    {
        TotalEnemiesKilled = kills;
    }

    int GetTotalEnemiesKilled()
    {
        return TotalEnemiesKilled;
    }

    void SetLastTeamSelection(string team)
    {
        LastTeamSelection = team;
    }

    string GetLastTeamSelection()
    {
        return LastTeamSelection;
    }

	// ---------------------------
	// OwnedWeapons Accessors
	// ---------------------------

	void SetOwnedWeapons(array<string> weapons)
	{
		if (!OwnedWeapons)
			OwnedWeapons = new array<string>();

		OwnedWeapons.Clear();
		foreach (string w : weapons)
		{
			OwnedWeapons.Insert(w);
		}
	}

	array<string> GetOwnedWeapons()
	{
		if (!OwnedWeapons)
			OwnedWeapons = new array<string>();
		return OwnedWeapons;
	}

	// Add or remove single items
	void AddOwnedWeapon(string weapon)
	{
		if (!OwnedWeapons)
			OwnedWeapons = new array<string>();

		if (OwnedWeapons.Find(weapon) == -1)
			OwnedWeapons.Insert(weapon);
	}

	void RemoveOwnedWeapon(string weapon)
	{
		if (OwnedWeapons)
			OwnedWeapons.RemoveItem(weapon);
	}

	// ---------------------------
	// OwnedAttachments Accessors
	// ---------------------------

	void SetOwnedAttachments(array<string> attachments)
	{
		if (!OwnedAttachments)
			OwnedAttachments = new array<string>();

		OwnedAttachments.Clear();
		foreach (string a : attachments)
		{
			OwnedAttachments.Insert(a);
		}
	}

	array<string> GetOwnedAttachments()
	{
		if (!OwnedAttachments)
			OwnedAttachments = new array<string>();
		return OwnedAttachments;
	}

	void AddOwnedAttachment(string attachment)
	{
		if (!OwnedAttachments)
			OwnedAttachments = new array<string>();

		if (OwnedAttachments.Find(attachment) == -1)
			OwnedAttachments.Insert(attachment);
	}

	void RemoveOwnedAttachment(string attachment)
	{
		if (OwnedAttachments)
			OwnedAttachments.RemoveItem(attachment);
	}

	void CopyConfig(KOTH_Players playerDataBase)
	{
		PlayerID = playerDataBase.PlayerID;
		PlayerName = playerDataBase.PlayerName;
		TotalMoneyinBank = playerDataBase.TotalMoneyinBank;
		TotalExperienceEarned = playerDataBase.TotalExperienceEarned;
		CurrentLevel = playerDataBase.CurrentLevel;
		TotalTimePlayed = playerDataBase.TotalTimePlayed;
		LongestKill = playerDataBase.LongestKill;
		LongestHeadshot = playerDataBase.LongestHeadshot;
		HighestKillstreak = playerDataBase.HighestKillstreak;
		TotalEnemiesKilled = playerDataBase.TotalEnemiesKilled;
		LastTeamSelection = playerDataBase.LastTeamSelection;

		// --- Deep copy arrays to avoid shared reference bugs ---
		if (!OwnedWeapons)
			OwnedWeapons = new array<string>();
		else
			OwnedWeapons.Clear();

		if (playerDataBase.OwnedWeapons)
		{
			foreach (string w : playerDataBase.OwnedWeapons)
				OwnedWeapons.Insert(w);
		}

		if (!OwnedAttachments)
			OwnedAttachments = new array<string>();
		else
			OwnedAttachments.Clear();

		if (playerDataBase.OwnedAttachments)
		{
			foreach (string a : playerDataBase.OwnedAttachments)
				OwnedAttachments.Insert(a);
		}
	}

	void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(PlayerID);
		ctx.Write(PlayerName);
		ctx.Write(TotalMoneyinBank);
		ctx.Write(TotalExperienceEarned);
		ctx.Write(CurrentLevel);
		ctx.Write(TotalTimePlayed);
		ctx.Write(LongestKill);
		ctx.Write(LongestHeadshot);
		ctx.Write(HighestKillstreak);
		ctx.Write(TotalEnemiesKilled);
		ctx.Write(LastTeamSelection);

		// --- OwnedWeapons ---
		if (OwnedWeapons)
		{
			ctx.Write(OwnedWeapons.Count());
			foreach (string w : OwnedWeapons)
				ctx.Write(w);
		}
		else
		{
			ctx.Write(0);
		}

		// --- OwnedAttachments ---
		if (OwnedAttachments)
		{
			ctx.Write(OwnedAttachments.Count());
			foreach (string a : OwnedAttachments)
				ctx.Write(a);
		}
		else
		{
			ctx.Write(0);
		}
	}

	void OnReceive(ParamsReadContext ctx)
	{
		ctx.Read(PlayerID);
		ctx.Read(PlayerName);
		ctx.Read(TotalMoneyinBank);
		ctx.Read(TotalExperienceEarned);
		ctx.Read(CurrentLevel);
		ctx.Read(TotalTimePlayed);
		ctx.Read(LongestKill);
		ctx.Read(LongestHeadshot);
		ctx.Read(HighestKillstreak);
		ctx.Read(TotalEnemiesKilled);
		ctx.Read(LastTeamSelection);

		int weaponCount;
		if (ctx.Read(weaponCount))
		{
			OwnedWeapons = new array<string>();
			for (int i = 0; i < weaponCount; i++)
			{
				string w;
				ctx.Read(w);
				OwnedWeapons.Insert(w);
			}
		}

		int attachmentCount;
		if (ctx.Read(attachmentCount))
		{
			OwnedAttachments = new array<string>();
			for (int j = 0; j < attachmentCount; j++)
			{
				string a;
				ctx.Read(a);
				OwnedAttachments.Insert(a);
			}
		}
	}

}