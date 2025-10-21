// ═══════════════════════════════════════════════════════════════
// KOTH_Zones.c (WITH AUTO HEIGHT CALCULATION)
// Place in: 3_Game/Settings/KOTH_Zones.c
// Supports both [X,Z] and [X,Y,Z] formats in JSON
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
        
        if (tempZone.EastSpawnBuilding.Count() == 2)
        {
            float eastY = GetGame().SurfaceY(tempZone.EastSpawnBuilding[0], tempZone.EastSpawnBuilding[1]);
            zoneData.EastSpawnBuilding = Vector(tempZone.EastSpawnBuilding[0], eastY + 1.0, tempZone.EastSpawnBuilding[1]);
            Print("[KOTH_Zones] East spawn auto-height calculated: " + eastY);
        }
        else if (tempZone.EastSpawnBuilding.Count() == 3)
        {
            zoneData.EastSpawnBuilding = Vector(tempZone.EastSpawnBuilding[0], tempZone.EastSpawnBuilding[1], tempZone.EastSpawnBuilding[2]);
            Print("[KOTH_Zones] East spawn using provided height: " + tempZone.EastSpawnBuilding[1]);
        }
        else
        {
            Error("[KOTH_Zones] Invalid EastSpawnBuilding array size: " + tempZone.EastSpawnBuilding.Count());
        }
        
        if (tempZone.WestSpawnBuilding.Count() == 2)
        {
            float westY = GetGame().SurfaceY(tempZone.WestSpawnBuilding[0], tempZone.WestSpawnBuilding[1]);
            zoneData.WestSpawnBuilding = Vector(tempZone.WestSpawnBuilding[0], westY + 1.0, tempZone.WestSpawnBuilding[1]);
            Print("[KOTH_Zones] West spawn auto-height calculated: " + westY);
        }
        else if (tempZone.WestSpawnBuilding.Count() == 3)
        {
            zoneData.WestSpawnBuilding = Vector(tempZone.WestSpawnBuilding[0], tempZone.WestSpawnBuilding[1], tempZone.WestSpawnBuilding[2]);
            Print("[KOTH_Zones] West spawn using provided height: " + tempZone.WestSpawnBuilding[1]);
        }
        else
        {
            Error("[KOTH_Zones] Invalid WestSpawnBuilding array size: " + tempZone.WestSpawnBuilding.Count());
        }
        
        if (tempZone.AOZoneCenter.Count() == 2)
        {
            float aoY = GetGame().SurfaceY(tempZone.AOZoneCenter[0], tempZone.AOZoneCenter[1]);
            zoneData.AOZoneCenter = Vector(tempZone.AOZoneCenter[0], aoY, tempZone.AOZoneCenter[1]);
            Print("[KOTH_Zones] AO center auto-height calculated: " + aoY);
        }
        else if (tempZone.AOZoneCenter.Count() == 3)
        {
            zoneData.AOZoneCenter = Vector(tempZone.AOZoneCenter[0], tempZone.AOZoneCenter[1], tempZone.AOZoneCenter[2]);
            Print("[KOTH_Zones] AO center using provided height: " + tempZone.AOZoneCenter[1]);
        }
        else
        {
            Error("[KOTH_Zones] Invalid AOZoneCenter array size: " + tempZone.AOZoneCenter.Count());
        }
        
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