/**
 * KOTH_ZoneData.c
 *
 * King of the Hill by Kahoona
 * Runtime zone data with vector support and network synchronization
 * 
 * MOVED TO: 3_Game/Settings/KOTH_ZoneData.c (same folder as KOTH_Zones.c)
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
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
    
    void SetZoneID(int id) { ZoneID = id; }
    void SetZoneName(string name) { ZoneName = name; }
    void SetEastSpawnBuilding(vector pos) { EastSpawnBuilding = pos; }
    void SetWestSpawnBuilding(vector pos) { WestSpawnBuilding = pos; }
    void SetEastSafeZoneRadius(int radius) { EastSafeZoneRadius = radius; }
    void SetWestSafeZoneRadius(int radius) { WestSafeZoneRadius = radius; }
    void SetAOZoneCenter(vector pos) { AOZoneCenter = pos; }
    void SetAOZoneRadius(int radius) { AOZoneRadius = radius; }
    void SetPriorityAORadius(int radius) { PriorityAORadius = radius; }
    
    int GetZoneID() { return ZoneID; }
    string GetZoneName() { return ZoneName; }
    vector GetEastSpawnBuilding() { return EastSpawnBuilding; }
    vector GetWestSpawnBuilding() { return WestSpawnBuilding; }
    int GetEastSafeZoneRadius() { return EastSafeZoneRadius; }
    int GetWestSafeZoneRadius() { return WestSafeZoneRadius; }
    vector GetAOZoneCenter() { return AOZoneCenter; }
    int GetAOZoneRadius() { return AOZoneRadius; }
    int GetPriorityAORadius() { return PriorityAORadius; }
    
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