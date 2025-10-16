/**
 * KOTH_Zones.c
 *
 * King of the Hill by Kahoona
 * Basic zone data structure for JSON serialization
 * Keep this for backward compatibility with existing zone files
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class KOTH_ZonesBase
{
    int m_Version;
    [NonSerialized()]
    string m_FileName;
    int ZoneID;
}

class KOTH_Zones : KOTH_ZonesBase
{
    static const int VERSION = 1;
    string ZoneName;
    
    autoptr TFloatArray EastSpawnBuilding;
    autoptr TFloatArray WestSpawnBuilding;
    autoptr TFloatArray AOZoneCenter;
    
    int EastSafeZoneRadius;
    int WestSafeZoneRadius;
    int AOZoneRadius;
    int PriorityAORadius;
    
    void KOTH_Zones()
    {
        m_Version = VERSION;
        EastSpawnBuilding = new TFloatArray;
        WestSpawnBuilding = new TFloatArray;
        AOZoneCenter = new TFloatArray;
    }
    
    static KOTH_Zones Load(string name)
    {
        KOTH_Zones settings = new KOTH_Zones;
        settings.Defaults();
        ExpansionJsonFileParser<KOTH_Zones>.Load(EXPANSION_KOTH_Zones + name + ".json", settings);
        settings.m_FileName = name;
        return settings;
    }
    
    void Save()
    {
        JsonFileLoader<KOTH_Zones>.JsonSaveFile(EXPANSION_KOTH_Zones + m_FileName + ".json", this);
    }
    
    void Defaults()
    {
        ZoneID = 1;
        ZoneName = "Chernogorsk";
        EastSpawnBuilding = {4890.6758, 9.5129, 2600.3997};
        WestSpawnBuilding = {8129.4468, 5.5988, 3299.1377};
        AOZoneCenter = {6585.0332, 6.0, 2439.6404};
        EastSafeZoneRadius = 50;
        WestSafeZoneRadius = 50;
        AOZoneRadius = 400;
        PriorityAORadius = 200;
    }
}