/**
 * KOTH_MarkerSystem.c
 *
 * King of the Hill by Kahoona
 * Handles map markers for spawn bases and capture zones
 * Place in: 4_World/Systems/KOTH_MarkerSystem.c
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 */

class KOTH_MarkerSystem
{
    private static ref ExpansionMarkerModule s_MarkerModule;
    
    // Marker IDs for cleanup
    private static const string MARKER_ID_EAST_SPAWN = "KOTH_EastSpawn";
    private static const string MARKER_ID_WEST_SPAWN = "KOTH_WestSpawn";
    private static const string MARKER_ID_CAPTURE_ZONE = "KOTH_CaptureZone";
    private static const string MARKER_ID_PRIORITY_ZONE = "KOTH_PriorityZone";
    
    //! ═══════════════════════════════════════════════════════════════
    //! INITIALIZATION
    //! ═══════════════════════════════════════════════════════════════
    
    static void Initialize()
    {
        if (!GetGame().IsServer())
            return;
        
        Class.CastTo(s_MarkerModule, CF_ModuleCoreManager.Get(ExpansionMarkerModule));
        
        if (!s_MarkerModule)
        {
            Error("[KOTH_MarkerSystem] Failed to get ExpansionMarkerModule!");
            return;
        }
        
        Print("[KOTH_MarkerSystem] Marker system initialized");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! PLACE ALL MARKERS FOR ACTIVE ZONE
    //! ═══════════════════════════════════════════════════════════════
    
    static void PlaceZoneMarkers(KOTH_ZoneData zoneData)
    {
        if (!GetGame().IsServer() || !zoneData)
            return;
        
        if (!s_MarkerModule)
            Initialize();
        
        Print("[KOTH_MarkerSystem] Placing markers for zone: " + zoneData.GetZoneName());
        
        // Remove old markers first
        RemoveAllMarkers();
        
        // Place spawn base markers
        PlaceEastSpawnMarker(zoneData.GetEastSpawnBuilding(), zoneData.GetZoneName());
        PlaceWestSpawnMarker(zoneData.GetWestSpawnBuilding(), zoneData.GetZoneName());
        
        // Place capture zone markers
        PlaceCaptureZoneMarker(zoneData.GetAOZoneCenter(), zoneData.GetZoneName(), zoneData.GetAOZoneRadius());
        
        // Place priority zone marker if configured
        if (zoneData.GetPriorityAORadius() > 0)
        {
            PlacePriorityZoneMarker(zoneData.GetAOZoneCenter(), zoneData.GetZoneName(), zoneData.GetPriorityAORadius());
        }
        
        Print("[KOTH_MarkerSystem] All markers placed successfully");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! EAST SPAWN MARKER (RED)
    //! ═══════════════════════════════════════════════════════════════
    
    static void PlaceEastSpawnMarker(vector position, string zoneName)
    {
        if (!s_MarkerModule)
            return;
        
        ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.SERVER, MARKER_ID_EAST_SPAWN);
        markerData.ApplyVisibility(4);
        markerData.Set3D(1);
        markerData.SetName("EAST BASE - " + zoneName);
        markerData.SetIcon("Base");
        markerData.SetColor(ARGB(255, 220, 60, 60)); // Red
        markerData.SetPosition(position);
        
        GetExpansionSettings().GetMap().AddServerMarker(markerData);
        
        Print("[KOTH_MarkerSystem] Placed East spawn marker at: " + position);
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! WEST SPAWN MARKER (BLUE)
    //! ═══════════════════════════════════════════════════════════════
    
    static void PlaceWestSpawnMarker(vector position, string zoneName)
    {
        if (!s_MarkerModule)
            return;
        
        ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.SERVER, MARKER_ID_WEST_SPAWN);
        markerData.ApplyVisibility(4);
        markerData.Set3D(1);
        markerData.SetName("WEST BASE - " + zoneName);
        markerData.SetIcon("Base");
        markerData.SetColor(ARGB(255, 60, 120, 220)); // Blue
        markerData.SetPosition(position);
        
        GetExpansionSettings().GetMap().AddServerMarker(markerData);
        
        Print("[KOTH_MarkerSystem] Placed West spawn marker at: " + position);
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CAPTURE ZONE MARKER (PURPLE)
    //! ═══════════════════════════════════════════════════════════════
    
    static void PlaceCaptureZoneMarker(vector position, string zoneName, int radius)
    {
        if (!s_MarkerModule)
            return;
        
        ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.SERVER, MARKER_ID_CAPTURE_ZONE);
        markerData.ApplyVisibility(4);
        markerData.Set3D(1);
        markerData.SetName("CAPTURE ZONE - " + zoneName);
        markerData.SetIcon("Map Marker");
        markerData.SetColor(ARGB(255, 180, 60, 220)); // Purple
        markerData.SetPosition(position);
        
        GetExpansionSettings().GetMap().AddServerMarker(markerData);
        
        Print("[KOTH_MarkerSystem] Placed capture zone marker at: " + position + " (radius: " + radius + "m)");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! PRIORITY ZONE MARKER (YELLOW)
    //! ═══════════════════════════════════════════════════════════════
    
    static void PlacePriorityZoneMarker(vector position, string zoneName, int radius)
    {
        if (!s_MarkerModule)
            return;
        
        ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.SERVER, MARKER_ID_PRIORITY_ZONE);
        markerData.ApplyVisibility(4);
        markerData.Set3D(1);
        markerData.SetName("PRIORITY ZONE - " + zoneName);
        markerData.SetIcon("Skull 1");
        markerData.SetColor(ARGB(255, 220, 200, 60)); // Yellow
        markerData.SetPosition(position);
        
        GetExpansionSettings().GetMap().AddServerMarker(markerData);
        
        Print("[KOTH_MarkerSystem] Placed priority zone marker at: " + position + " (radius: " + radius + "m)");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! REMOVE MARKERS
    //! ═══════════════════════════════════════════════════════════════
    
    static void RemoveAllMarkers()
    {
        if (!GetGame().IsServer())
            return;
        
        RemoveMarker(MARKER_ID_EAST_SPAWN);
        RemoveMarker(MARKER_ID_WEST_SPAWN);
        RemoveMarker(MARKER_ID_CAPTURE_ZONE);
        RemoveMarker(MARKER_ID_PRIORITY_ZONE);
        
        Print("[KOTH_MarkerSystem] Removed all KOTH markers");
    }
    
    static void RemoveMarker(string markerID)
    {
        ExpansionMarkerModule markerModule;
        Class.CastTo(markerModule, CF_ModuleCoreManager.Get(ExpansionMarkerModule));
        
        if (!markerModule)
            return;
        
        GetExpansionSettings().GetMap().RemoveServerMarker(markerID);
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! UPDATE MARKER (for zone rotation)
    //! ═══════════════════════════════════════════════════════════════
    
    static void UpdateMarkersForZone(KOTH_ZoneData zoneData)
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_MarkerSystem] Updating markers for new zone: " + zoneData.GetZoneName());
        PlaceZoneMarkers(zoneData);
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! UPDATE PRIORITY MARKER (for moving priority zone)
    //! ═══════════════════════════════════════════════════════════════
    
    static void UpdatePriorityMarker(vector position, int radius)
    {
        if (!GetGame().IsServer())
            return;
        
        // Remove old priority marker
        RemoveMarker(MARKER_ID_PRIORITY_ZONE);
        
        // Create new marker at new position
        ExpansionMarkerData markerData = ExpansionMarkerData.Create(ExpansionMapMarkerType.SERVER, MARKER_ID_PRIORITY_ZONE);
        markerData.ApplyVisibility(4);
        markerData.Set3D(0);
        markerData.SetName("PRIORITY ZONE - BONUS POINTS");
        markerData.SetIcon("Skull 1");
        markerData.SetColor(ARGB(255, 220, 200, 60)); // Yellow
        markerData.SetPosition(position);
        
        GetExpansionSettings().GetMap().AddServerMarker(markerData);
        
        Print("[KOTH_MarkerSystem] Updated priority zone marker to: " + position);
    }
}