/**
 * KOTH_ZoneData.c
 *
 * King of the Hill by Kahoona
 * Runtime zone data with vector support and network synchronization
 * 
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class KOTH_ZoneData
{
    int ZoneID;
    string ZoneName;
    vector EastSpawnBuilding;
    vector WestSpawnBuilding;
    int EastSafeZoneRadius;
    int WestSafeZoneRadius;
    vector AOZoneCenter;
    int AOZoneRadius;
    int PriorityAORadius;
    
    // Create from KOTH_Zones (JSON format)
    static KOTH_ZoneData CreateFromZoneFile(KOTH_Zones zoneFile)
    {
        if (!zoneFile)
            return null;
            
        KOTH_ZoneData data = new KOTH_ZoneData();
        
        data.ZoneID = zoneFile.ZoneID;
        data.ZoneName = zoneFile.ZoneName;
        
        // Convert TFloatArray to vectors
        data.EastSpawnBuilding = Vector(zoneFile.EastSpawnBuilding[0], zoneFile.EastSpawnBuilding[1], zoneFile.EastSpawnBuilding[2]);
        data.WestSpawnBuilding = Vector(zoneFile.WestSpawnBuilding[0], zoneFile.WestSpawnBuilding[1], zoneFile.WestSpawnBuilding[2]);
        data.AOZoneCenter = Vector(zoneFile.AOZoneCenter[0], zoneFile.AOZoneCenter[1], zoneFile.AOZoneCenter[2]);
        
        data.EastSafeZoneRadius = zoneFile.EastSafeZoneRadius;
        data.WestSafeZoneRadius = zoneFile.WestSafeZoneRadius;
        data.AOZoneRadius = zoneFile.AOZoneRadius;
        data.PriorityAORadius = zoneFile.PriorityAORadius;
        
        return data;
    }
    
    // Load zone from file
    static KOTH_ZoneData Load(string fileName)
    {
        fileName.Replace(" ", "");
        fileName.Replace(".json", "");
        
        Print("[KOTH_ZoneData] Loading zone file: " + fileName);
        
        KOTH_Zones zoneFile = KOTH_Zones.Load(fileName);
        if (!zoneFile)
        {
            Error("[KOTH_ZoneData] Failed to load zone file: " + fileName);
            return null;
        }
        
        KOTH_ZoneData data = CreateFromZoneFile(zoneFile);
        
        Print("[KOTH_ZoneData] Successfully loaded zone: " + data.ZoneName);
        return data;
    }
    
    // Setters
    void SetZoneID(int id) { ZoneID = id; }
    void SetZoneName(string name) { ZoneName = name; }
    void SetEastSpawnBuilding(vector pos) { EastSpawnBuilding = pos; }
    void SetWestSpawnBuilding(vector pos) { WestSpawnBuilding = pos; }
    void SetEastSafeZoneRadius(int radius) { EastSafeZoneRadius = radius; }
    void SetWestSafeZoneRadius(int radius) { WestSafeZoneRadius = radius; }
    void SetAOZoneCenter(vector pos) { AOZoneCenter = pos; }
    void SetAOZoneRadius(int radius) { AOZoneRadius = radius; }
    void SetPriorityAORadius(int radius) { PriorityAORadius = radius; }
    
    // Getters
    int GetZoneID() { return ZoneID; }
    string GetZoneName() { return ZoneName; }
    vector GetEastSpawnBuilding() { return EastSpawnBuilding; }
    vector GetWestSpawnBuilding() { return WestSpawnBuilding; }
    int GetEastSafeZoneRadius() { return EastSafeZoneRadius; }
    int GetWestSafeZoneRadius() { return WestSafeZoneRadius; }
    vector GetAOZoneCenter() { return AOZoneCenter; }
    int GetAOZoneRadius() { return AOZoneRadius; }
    int GetPriorityAORadius() { return PriorityAORadius; }
    
    // Network serialization
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
    
    bool OnReceive(ParamsReadContext ctx)
    {
        if (!ctx.Read(ZoneID)) return false;
        if (!ctx.Read(ZoneName)) return false;
        if (!ctx.Read(EastSpawnBuilding)) return false;
        if (!ctx.Read(WestSpawnBuilding)) return false;
        if (!ctx.Read(EastSafeZoneRadius)) return false;
        if (!ctx.Read(WestSafeZoneRadius)) return false;
        if (!ctx.Read(AOZoneCenter)) return false;
        if (!ctx.Read(AOZoneRadius)) return false;
        if (!ctx.Read(PriorityAORadius)) return false;
        return true;
    }
    
    // Copy data from another zone
    void CopyConfig(KOTH_ZoneData other)
    {
        ZoneID = other.ZoneID;
        ZoneName = other.ZoneName;
        EastSpawnBuilding = other.EastSpawnBuilding;
        WestSpawnBuilding = other.WestSpawnBuilding;
        EastSafeZoneRadius = other.EastSafeZoneRadius;
        WestSafeZoneRadius = other.WestSafeZoneRadius;
        AOZoneCenter = other.AOZoneCenter;
        AOZoneRadius = other.AOZoneRadius;
        PriorityAORadius = other.PriorityAORadius;
    }
}