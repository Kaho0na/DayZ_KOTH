/**
 * DayZ_KOTH_ZoneData.c
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

class DayZ_KOTH_ZoneData
{
    static ref DayZ_KOTH_ZoneData s_Instance;
    protected ref map<int, ref DayZ_KOTH_ZoneData> m_ZoneData = new map<int, ref DayZ_KOTH_ZoneData>();


    int ZoneID;
	string ZoneName;
	vector EastSpawnBuilding;
	vector WestSpawnBuilding;
	int EastSafeZoneRadius;
	int WestSafeZoneRadius;
	vector AOZoneCenter;
	int AOZoneRadius;
	int PriorityAORadius;

    [NonSerialized()]
	static string FILENAME = "NONE";
    
    void DayZ_KOTH_ZoneData()
    {
        s_Instance = this;
        m_ZoneData = new map<int, ref DayZ_KOTH_ZoneData>();
    }

	static DayZ_KOTH_ZoneData GetInstance()
	{
		if (!s_Instance)
			s_Instance = new DayZ_KOTH_ZoneData();
		return s_Instance;
	}

    void SetZoneID(int id)
    {
        ZoneID = id;
    }

    int GetZoneID()
    {
        return ZoneID;
    }

    void SetZoneName(string name)
    {
        ZoneName = name;
    }

    string GetZoneName()
    {
        return ZoneName;
    }

    void SetEastSpawnBuilding(vector EastSpawn)
    {
        EastSpawnBuilding = EastSpawn;
    }

    vector GetEastSpawnBuilding()
    {
        return EastSpawnBuilding;
    }

    void SetWestSpawnBuilding(vector WestSpawn)
    {
        WestSpawnBuilding = WestSpawn;
    }

    vector GetWestSpawnBuilding()
    {
        return WestSpawnBuilding;
    }

    void SetEastSafeZoneRadius(int EastRadius)
    {
        EastSafeZoneRadius = EastRadius;
    }

    int GetEastSafeZoneRadius()
    {
        return EastSafeZoneRadius;
    }

    void SetWestSafeZoneRadius(int WestRadius)
    {
        WestSafeZoneRadius = WestRadius;
    }

    int GetWestSafeZoneRadius()
    {
        return WestSafeZoneRadius;
    }

    void SetAOZoneCenter(vector AOZone)
    {
        AOZoneCenter = AOZone;
    }

    vector GetAOZoneCenter()
    {
        return AOZoneCenter;
    }

    void SetAOZoneRadius(int AORadius)
    {
        AOZoneRadius = AORadius;
    }

    int GetAOZoneRadius()
    {
        return AOZoneRadius;
    }

    void SetPriorityAORadius(int PriorityRadius)
    {
        PriorityAORadius = PriorityRadius;
    }

    int GetPriorityAORadius()
    {
        return PriorityAORadius;
    }

    static DayZ_KOTH_ZoneData Load(string fileName)
    {
        fileName.Replace(" ", "");
        if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
            fileName += ".json";
        Print("[DayZ_KOTH_ZoneData] Load existing configuration file:" + fileName);
        DayZ_KOTH_ZoneData zoneConfig;
        ExpansionJsonFileParser<DayZ_KOTH_ZoneData>.Load(EXPANSION_DAYZ_KOTH_ZONES + fileName, zoneConfig);
        Print("[DayZ_KOTH_ZoneData] DayZ_KOTH_ZoneData:" + zoneConfig);
        return zoneConfig;
    }

    void Save(string fileName)
    {
        fileName.Replace(" ", "");
        fileName.ToLower();
        if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
            fileName += ".json";

        ExpansionJsonFileParser<DayZ_KOTH_ZoneData>.Save(EXPANSION_DAYZ_KOTH_ZONES + fileName, this);
    }

    void CopyConfig(DayZ_KOTH_ZoneData zoneDataBase)
    {
        ZoneID = zoneDataBase.ZoneID;
        ZoneName = zoneDataBase.ZoneName;
        EastSpawnBuilding = zoneDataBase.EastSpawnBuilding;
        WestSpawnBuilding = zoneDataBase.WestSpawnBuilding;
        EastSafeZoneRadius = zoneDataBase.EastSafeZoneRadius;
        WestSafeZoneRadius = zoneDataBase.WestSafeZoneRadius;
        AOZoneCenter = zoneDataBase.AOZoneCenter;
        AOZoneRadius = zoneDataBase.AOZoneRadius;
        PriorityAORadius = zoneDataBase.PriorityAORadius;
    }

    void OnSend(ParamsWriteContext ctx)
    {
        ctx.Write(ZoneID);
        ctx.Write(ZoneName);
        ctx.Write(EastSpawnBuilding);
        ctx.Write(WestSpawnBuilding);
        ctx.Write(EastSafeZoneRadius);
        ctx.Write(WestSafeZoneRadius);
        ctx.Write(AOZoneCenter);
        ctx.Write(AOZoneRadius);
        ctx.Write(PriorityAORadius);
    }

    bool OnRecieve(ParamsReadContext ctx)
    {
        if (!ctx.Read(ZoneID))
            return false;

        if (!ctx.Read(ZoneName))
            return false;

        if (!ctx.Read(EastSpawnBuilding))
            return false;

        if (!ctx.Read(WestSpawnBuilding))
            return false;

        if (!ctx.Read(EastSafeZoneRadius))
            return false;

        if (!ctx.Read(WestSafeZoneRadius))
            return false;

        if (!ctx.Read(AOZoneCenter))
            return false;

        if (!ctx.Read(AOZoneRadius))
            return false;

        if (!ctx.Read(PriorityAORadius))
            return false;

        return true;
    }

};