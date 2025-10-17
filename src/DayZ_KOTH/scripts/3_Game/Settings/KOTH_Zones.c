
// ═══════════════════════════════════════════════════════════════
// KOTH_Zones.c (UNCHANGED - stays in same folder)
// Place in: 3_Game/Settings/KOTH_Zones.c
// ═══════════════════════════════════════════════════════════════

class KOTH_Zones
{
    static const int VERSION = 1;
    
    int ZoneID;
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
        EastSpawnBuilding = new TFloatArray;
        WestSpawnBuilding = new TFloatArray;
        AOZoneCenter = new TFloatArray;
    }
    
    static KOTH_ZoneData LoadZone(string fileName)
    {
        fileName.Replace(" ", "");
        fileName.Replace(".json", "");
        
        Print("[KOTH_Zones] Loading zone file: " + fileName);
        
        KOTH_Zones tempZone = new KOTH_Zones;
        
        if (!ExpansionJsonFileParser<KOTH_Zones>.Load(EXPANSION_KOTH_Zones + fileName + ".json", tempZone))
        {
            Error("[KOTH_Zones] Failed to load zone file: " + fileName);
            return null;
        }
        
        KOTH_ZoneData zoneData = new KOTH_ZoneData();
        zoneData.ZoneID = tempZone.ZoneID;
        zoneData.ZoneName = tempZone.ZoneName;
        zoneData.EastSpawnBuilding = Vector(tempZone.EastSpawnBuilding[0], tempZone.EastSpawnBuilding[1], tempZone.EastSpawnBuilding[2]);
        zoneData.WestSpawnBuilding = Vector(tempZone.WestSpawnBuilding[0], tempZone.WestSpawnBuilding[1], tempZone.WestSpawnBuilding[2]);
        zoneData.AOZoneCenter = Vector(tempZone.AOZoneCenter[0], tempZone.AOZoneCenter[1], tempZone.AOZoneCenter[2]);
        zoneData.EastSafeZoneRadius = tempZone.EastSafeZoneRadius;
        zoneData.WestSafeZoneRadius = tempZone.WestSafeZoneRadius;
        zoneData.AOZoneRadius = tempZone.AOZoneRadius;
        zoneData.PriorityAORadius = tempZone.PriorityAORadius;
        
        Print("[KOTH_Zones] Successfully loaded zone: " + zoneData.ZoneName);
        return zoneData;
    }
    
    static bool SaveZone(string fileName, KOTH_ZoneData zoneData)
    {
        fileName.Replace(" ", "");
        fileName.Replace(".json", "");
        
        KOTH_Zones tempZone = new KOTH_Zones;
        tempZone.ZoneID = zoneData.ZoneID;
        tempZone.ZoneName = zoneData.ZoneName;
        tempZone.EastSpawnBuilding = {zoneData.EastSpawnBuilding[0], zoneData.EastSpawnBuilding[1], zoneData.EastSpawnBuilding[2]};
        tempZone.WestSpawnBuilding = {zoneData.WestSpawnBuilding[0], zoneData.WestSpawnBuilding[1], zoneData.WestSpawnBuilding[2]};
        tempZone.AOZoneCenter = {zoneData.AOZoneCenter[0], zoneData.AOZoneCenter[1], zoneData.AOZoneCenter[2]};
        tempZone.EastSafeZoneRadius = zoneData.EastSafeZoneRadius;
        tempZone.WestSafeZoneRadius = zoneData.WestSafeZoneRadius;
        tempZone.AOZoneRadius = zoneData.AOZoneRadius;
        tempZone.PriorityAORadius = zoneData.PriorityAORadius;
        
        Print("[KOTH_Zones] Saving zone file: " + fileName);
        JsonFileLoader<KOTH_Zones>.JsonSaveFile(EXPANSION_KOTH_Zones + fileName + ".json", tempZone);
        return true;
    }
}