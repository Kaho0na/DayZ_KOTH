class ExpansionDeadDropPlayerData
{

	static ref ExpansionDeadDropPlayerData s_Instance;

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

    [NonSerialized()]
	static string FILENAME = "NONE";
	
	bool Active = true;

    void ExpansionDeadDropPlayerData()
	{
		s_Instance = this;
		items = new array<ref DeadDropItem>();
	}

	static ExpansionDeadDropPlayerData GetInstance()
	{
		if (!s_Instance)
			s_Instance = new ExpansionDeadDropPlayerData();
		return s_Instance;
	}

    void SetPlayerID(string id)
	{
		player_id = id;
	}

	string GetPlayerID()
	{
		return player_id;
	}

    void SetPlayerName(string playername)
	{
		player_name = playername;
	}
	
	string GetPlayerName()
	{
		return player_name;
	}

    void SetSteamID(string id)
	{
		steam_id = id;
	}

	string GetSteamID()
	{
		return steam_id;
	}

    void SetDeathTime(string time)
	{
		death_time = time;
	}

	string GetDeathTime()
	{
		return death_time;
	}

    void SetISRecovered(bool rec)
	{
		recovered = rec;
	}

	bool GetISRecovered()
	{
		return recovered;
	}

    void SetRecoveryTime(string recoverytime)
	{
		recovery_time = recoverytime;
	}

	string GetRecoveryTime()
	{
		return recovery_time;
	}
    
    void SetRecoveredBy(string recoveredby)
	{
		recovered_by = recoveredby;
	}

	string GetRecoveredBy()
	{
		return recovered_by;
	}

    void SetCauseOfDeath(string cause)
	{
		cause_of_death = cause;
	}

	string GetCauseOfDeath()
	{
		return cause_of_death;
	}

	void SetLocation(vector loc)
	{
		location = loc;
	}

	vector GetLocation()
	{
		return location;
	}

	string GetBodyName()
	{
		return player_name + " - " + death_time;
	}

	int GetRecoveryTimeInt()
	{
		return recovery_time.ToInt();
	}

	int CompareTo(ExpansionDeadDropPlayerData other)
	{
		return GetRecoveryTimeInt() - other.GetRecoveryTimeInt();
	}

	string m_FileName;

	void SetFileName(string name)
	{
		m_FileName = name;
	}

	string GetFileName()
	{
		return m_FileName;
	}

	static ExpansionDeadDropPlayerData Load(string fileName)
	{
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";

		string fullPath = EXPANSION_DEADDROP_FOLDER + "Active\\" + fileName;
		Print("[ExpansionDeadDropPlayerData] Loading file: " + fullPath);

		ExpansionDeadDropPlayerData playerData;
		if (!ExpansionJsonFileParser<ExpansionDeadDropPlayerData>.Load(fullPath, playerData))
		{
			Print("[ExpansionDeadDropPlayerData] Failed to load: " + fullPath);
			return null;
		}

		playerData.SetFileName(fileName);
		Print("[ExpansionDeadDropPlayerData] Loaded data for: " + playerData.GetPlayerName());
		return playerData;
	}

	static array<ref ExpansionDeadDropPlayerData> LoadAllForPlayer(string steamId)
	{
		array<string> files = ExpansionStatic.FindFilesInLocation(EXPANSION_DEADDROP_FOLDER + "Active\\", ".json");
		array<ref ExpansionDeadDropPlayerData> results = {};

		foreach (string fileName : files)
		{
			// Skip if the file doesn't start with the player's steam ID
			if (!ExpansionString.StartsWith(fileName, steamId))
				continue;

			ExpansionDeadDropPlayerData data = Load(fileName);
			if (data)
				results.Insert(data);
		}

		return results;
	}

    void Save(string fileName)
	{
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";

		ExpansionJsonFileParser<ExpansionDeadDropPlayerData>.Save(EXPANSION_DEADDROP_FOLDER + "Active\\" + fileName, this);
	}

    void CopyConfig(ExpansionDeadDropPlayerData PlayerDataBase)
	{
		player_id = PlayerDataBase.player_id;
        player_name = PlayerDataBase.player_name;
        steam_id = PlayerDataBase.steam_id;
        death_time = PlayerDataBase.death_time;
        recovered = PlayerDataBase.recovered;
        recovery_time = PlayerDataBase.recovery_time;
		recovered_by = PlayerDataBase.recovered_by;
		cause_of_death = PlayerDataBase.cause_of_death;
		location = PlayerDataBase.location;

		items = new array<ref DeadDropItem>();
		foreach (DeadDropItem item : PlayerDataBase.items)
		{
			DeadDropItem copy = new DeadDropItem();
			copy.CopyFrom(item);
			items.Insert(copy);
		}

	}

    void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(player_id);
        ctx.Write(player_name);
        ctx.Write(steam_id);
        ctx.Write(death_time);
        ctx.Write(recovered);
        ctx.Write(recovery_time);
		ctx.Write(recovered_by);
		ctx.Write(cause_of_death);
		ctx.Write(location);
		ctx.Write(items.Count());
		foreach (DeadDropItem item : items)
			ctx.Write(item);

	}

    bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(player_id))
			return false;
        
        if (!ctx.Read(player_name))
            return false;

        if (!ctx.Read(steam_id))
            return false;

        if (!ctx.Read(death_time))
            return false;

        if (!ctx.Read(recovered))
            return false;

        if (!ctx.Read(recovery_time))
            return false;
		
		if (!ctx.Read(recovered_by))
			return false;

		if (!ctx.Read(cause_of_death))
		return false;

		if (!ctx.Read(location))
		return false;

		int count;
		if (!ctx.Read(count)) return false;

		items = new array<ref DeadDropItem>();
		for (int i = 0; i < count; i++)
		{
			DeadDropItem item;
			if (!ctx.Read(item)) return false;
			items.Insert(item);
		}

				
		return true;
	}

};