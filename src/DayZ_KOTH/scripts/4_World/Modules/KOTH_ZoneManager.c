/**
 * KOTH_ZoneManager.c (DYNAMIC PRIORITY ZONE DATA)
 *
 * King of the Hill by Kahoona
 * Updates priority zone position data on every client request
 * Place in: 4_World/Modules/KOTH_ZoneManager.c
 */

enum KOTHZoneSelectionMode
{
    SEQUENTIAL,
    RANDOM,
    VOTE
}

[CF_RegisterModule(KOTH_ZoneManager)]
class KOTH_ZoneManager: CF_ModuleWorld
{
    private static ref KOTH_ZoneManager s_Instance;
    private ref KOTH_ZoneData m_ActiveZone;
    private string m_ActiveZoneName;
    private ref array<string> m_AvailableZones;
    private int m_CurrentZoneIndex = 0;
    private KOTHZoneSelectionMode m_SelectionMode = KOTHZoneSelectionMode.SEQUENTIAL;
    
    private float m_ZoneRotationInterval = 1800.0;
    private bool m_AutoRotateZones = false;
    
    private KOTH_Area m_MainZoneTrigger;
    
    ref array<string> zoneNames;
    ref array<vector> zonePositions;
    ref array<float> zoneRadii;
    ref array<int> zoneColors;
    ref array<bool> zoneDrawCircles;
    
    void KOTH_ZoneManager()
    {
        s_Instance = this;
        m_AvailableZones = new array<string>();
        zoneNames = new array<string>();
        zonePositions = new array<vector>();
        zoneRadii = new array<float>();
        zoneColors = new array<int>();
        zoneDrawCircles = new array<bool>();
    }
    
    override void OnInit()
    {
        super.OnInit();
        Print("[KOTH_ZoneManager] Zone Manager initialized");
        
        EnableMissionStart();
        EnableMissionFinish();
        Expansion_EnableRPCManager();
        
        Expansion_RegisterClientRPC("RPC_SyncActiveZone");
        Expansion_RegisterServerRPC("RPC_RequestZoneInfo");
        Expansion_RegisterServerRPC("RPC_AdminChangeZone");
    }
    
    override void OnMissionStart(Class sender, CF_EventArgs args)
    {
        super.OnMissionStart(sender, args);
        InitRPC();
    }
    
    void InitRPC()
    {
        if (GetGame().IsServer())
        {
            GetRPCManager().AddRPC("KOTH_MapMenu", "RequestKOTHZones", this, SingleplayerExecutionType.Server);
            GetRPCManager().AddRPC("KOTH_MapMenu", "ReceiveKOTHZones", this, SingleplayerExecutionType.Server);
            Print("[KOTH_ZoneManager] Server-side RPC initialized");
        }
        else
        {
            GetRPCManager().AddRPC("KOTH_MapMenu", "ReceiveKOTHZones", this, SingleplayerExecutionType.Client);
            Print("[KOTH_ZoneManager] Client-side RPC initialized");
        }
    }
    
    void RequestKOTHZones(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Server)
            return;

        if (sender == null)
            return;

        Print("[KOTH_ZoneManager] Client " + sender.GetName() + " requested zone data - updating priority position");

        PrepareMapCircleData();

        GetRPCManager().SendRPC("KOTH_MapMenu", "ReceiveKOTHZones", new Param5<array<string>, array<vector>, array<float>, array<int>, array<bool>>(zoneNames, zonePositions, zoneRadii, zoneColors, zoneDrawCircles), true, sender);
    }
    
    void ReceiveKOTHZones(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
        if (type != CallType.Client)
            return;

        Param5<array<string>, array<vector>, array<float>, array<int>, array<bool>> data;
        if (!ctx.Read(data))
            return;

        zoneNames = data.param1;
        zonePositions = data.param2;
        zoneRadii = data.param3;
        zoneColors = data.param4;
        zoneDrawCircles = data.param5;

        Print("[KOTH_ZoneManager] Client received zone data - " + zoneNames.Count() + " zones");
    }
    
    static KOTH_ZoneManager GetInstance()
    {
        if (!s_Instance)
        {
            Print("[KOTH_ZoneManager] ERROR: Instance not initialized!");
        }
        return s_Instance;
    }
    
    void DiscoverAvailableZones()
    {
        m_AvailableZones.Clear();
        
        string zonePath = EXPANSION_KOTH_Zones;
        string fileName;
        FileAttr fileAttr;
        
        FindFileHandle handle = FindFile(zonePath + "*.json", fileName, fileAttr, 0);
        
        if (handle)
        {
            if (fileName != "")
            {
                string zoneName = fileName;
                zoneName.Replace(".json", "");
                m_AvailableZones.Insert(zoneName);
                Print("[KOTH_ZoneManager] Discovered zone: " + zoneName);
            }
            
            while (FindNextFile(handle, fileName, fileAttr))
            {
                if (fileName != "")
                {
                    zoneName = fileName;
                    zoneName.Replace(".json", "");
                    m_AvailableZones.Insert(zoneName);
                    Print("[KOTH_ZoneManager] Discovered zone: " + zoneName);
                }
            }
            
            CloseFindFile(handle);
        }
        
        Print("[KOTH_ZoneManager] Total zones discovered: " + m_AvailableZones.Count());
    }
    
    bool LoadZone(string zoneName, bool despawnOldZone = true)
    {
        if (!GetGame().IsServer())
            return false;
        
        Print("[KOTH_ZoneManager] ═══════════════════════════════════════");
        Print("[KOTH_ZoneManager] Loading zone: " + zoneName);
        
        if (despawnOldZone && m_ActiveZone)
        {
            CleanupCurrentZone();
        }
        
        m_ActiveZone = KOTH_Zones.LoadZone(zoneName);
        
        if (!m_ActiveZone)
        {
            Error("[KOTH_ZoneManager] Failed to load zone data: " + zoneName);
            return false;
        }
        
        m_ActiveZoneName = zoneName;
        m_CurrentZoneIndex = m_AvailableZones.Find(zoneName);
        
        Print("[KOTH_ZoneManager] Zone data loaded successfully");
        Print("[KOTH_ZoneManager] - Name: " + m_ActiveZone.GetZoneName());
        Print("[KOTH_ZoneManager] - East Spawn: " + m_ActiveZone.GetEastSpawnBuilding());
        Print("[KOTH_ZoneManager] - West Spawn: " + m_ActiveZone.GetWestSpawnBuilding());
        Print("[KOTH_ZoneManager] - AO Center: " + m_ActiveZone.GetAOZoneCenter());
        Print("[KOTH_ZoneManager] - AO Radius: " + m_ActiveZone.GetAOZoneRadius());
        Print("[KOTH_ZoneManager] - Priority Radius: " + m_ActiveZone.GetPriorityAORadius());
        
        SpawnZoneBuildings();
        CreateZoneTriggers();
        PrepareMapCircleData();
        
        SyncActiveZoneToAllClients();
        NotifyPlayersZoneChange();
        
        Print("[KOTH_ZoneManager] Zone fully initialized");
        Print("[KOTH_ZoneManager] ═══════════════════════════════════════");
        
        return true;
    }
    
    private void SpawnZoneBuildings()
    {
        if (!m_ActiveZone)
            return;
        
        Print("[KOTH_ZoneManager] Spawning base buildings...");
        KOTH_SpawnBasesForZone(m_ActiveZone);
        Print("[KOTH_ZoneManager] Base buildings spawned");
    }
    
    private void CreateZoneTriggers()
    {
        if (!m_ActiveZone)
            return;
        
        vector aoCenter = m_ActiveZone.GetAOZoneCenter();
        float aoRadius = m_ActiveZone.GetAOZoneRadius();
        float priRadius = m_ActiveZone.GetPriorityAORadius();
        
        Print("[KOTH_ZoneManager] Creating main capture zone trigger...");
        if (Class.CastTo(m_MainZoneTrigger, GetGame().CreateObjectEx("KOTH_Area", aoCenter, ECE_NONE)))
        {
            m_MainZoneTrigger.KOTH_Init(aoCenter, aoRadius);
            Print("[KOTH_ZoneManager] Main zone trigger created");
        }
        else
        {
            Error("[KOTH_ZoneManager] Failed to create main zone trigger!");
        }
        
        if (priRadius > 0)
        {
            Print("[KOTH_ZoneManager] Initializing priority zone...");
            KOTH_PriorityZoneManager.Initialize(aoCenter, aoRadius, priRadius);
            Print("[KOTH_ZoneManager] Priority zone initialized");
        }
    }
    
    private void PrepareMapCircleData()
    {
        if (!m_ActiveZone)
            return;
        
        zoneNames.Clear();
        zonePositions.Clear();
        zoneRadii.Clear();
        zoneColors.Clear();
        zoneDrawCircles.Clear();
        
        zoneNames.Insert("East Base - " + m_ActiveZone.GetZoneName());
        zonePositions.Insert(m_ActiveZone.GetEastSpawnBuilding());
        zoneRadii.Insert(m_ActiveZone.GetEastSafeZoneRadius());
        zoneColors.Insert(ARGB(255, 220, 60, 60));
        zoneDrawCircles.Insert(true);
        
        zoneNames.Insert("West Base - " + m_ActiveZone.GetZoneName());
        zonePositions.Insert(m_ActiveZone.GetWestSpawnBuilding());
        zoneRadii.Insert(m_ActiveZone.GetWestSafeZoneRadius());
        zoneColors.Insert(ARGB(255, 60, 120, 220));
        zoneDrawCircles.Insert(true);
        
        zoneNames.Insert("AO Zone - " + m_ActiveZone.GetZoneName());
        zonePositions.Insert(m_ActiveZone.GetAOZoneCenter());
        zoneRadii.Insert(m_ActiveZone.GetAOZoneRadius());
        zoneColors.Insert(ARGB(255, 180, 60, 220));
        zoneDrawCircles.Insert(true);
        
        if (m_ActiveZone.GetPriorityAORadius() > 0)
        {
            vector priorityPos = KOTH_PriorityZoneManager.GetCurrentPosition();
            
            zoneNames.Insert("Priority Zone");
            zonePositions.Insert(priorityPos);
            zoneRadii.Insert(m_ActiveZone.GetPriorityAORadius());
            zoneColors.Insert(ARGB(255, 220, 200, 60));
            zoneDrawCircles.Insert(true);
            
            Print("[KOTH_ZoneManager] Priority zone marker position updated to: " + priorityPos);
        }
        
        Print("[KOTH_ZoneManager] Map circle data prepared - " + zoneNames.Count() + " circles");
    }
    
    private void CleanupCurrentZone()
    {
        Print("[KOTH_ZoneManager] Cleaning up current zone...");
        
        KOTH_SpawnBase.DespawnAll();
        KOTH_PriorityZoneManager.Cleanup();
        
        if (m_MainZoneTrigger)
        {
            GetGame().ObjectDelete(m_MainZoneTrigger);
            m_MainZoneTrigger = null;
        }
        
        Print("[KOTH_ZoneManager] Cleanup complete");
    }
    
    bool LoadFirstAvailableZone()
    {
        DiscoverAvailableZones();
        
        if (m_AvailableZones.Count() == 0)
        {
            Error("[KOTH_ZoneManager] No zones found in " + EXPANSION_KOTH_Zones);
            return false;
        }
        
        string firstZone = m_AvailableZones.Get(0);
        return LoadZone(firstZone, false);
    }
    
    bool LoadNextZone()
    {
        if (m_AvailableZones.Count() == 0)
        {
            Print("[KOTH_ZoneManager] No zones available for rotation");
            return false;
        }
        
        string nextZone;
        
        switch (m_SelectionMode)
        {
            case KOTHZoneSelectionMode.SEQUENTIAL:
                m_CurrentZoneIndex = (m_CurrentZoneIndex + 1) % m_AvailableZones.Count();
                nextZone = m_AvailableZones.Get(m_CurrentZoneIndex);
                Print("[KOTH_ZoneManager] Rotating to next zone in sequence: " + nextZone);
                break;
                
            case KOTHZoneSelectionMode.RANDOM:
                int randomIndex = Math.RandomInt(0, m_AvailableZones.Count());
                
                if (m_AvailableZones.Count() > 1)
                {
                    while (randomIndex == m_CurrentZoneIndex)
                    {
                        randomIndex = Math.RandomInt(0, m_AvailableZones.Count());
                    }
                }
                
                nextZone = m_AvailableZones.Get(randomIndex);
                Print("[KOTH_ZoneManager] Randomly selected zone: " + nextZone);
                break;
                
            case KOTHZoneSelectionMode.VOTE:
                Print("[KOTH_ZoneManager] Vote mode not yet implemented, using sequential");
                m_CurrentZoneIndex = (m_CurrentZoneIndex + 1) % m_AvailableZones.Count();
                nextZone = m_AvailableZones.Get(m_CurrentZoneIndex);
                break;
        }
        
        return LoadZone(nextZone, true);
    }
    
    bool LoadSpecificZone(string zoneName)
    {
        if (m_AvailableZones.Find(zoneName) == -1)
        {
            Error("[KOTH_ZoneManager] Zone not found: " + zoneName);
            return false;
        }
        
        return LoadZone(zoneName, true);
    }
    
    bool LoadRandomZone()
    {
        if (m_AvailableZones.Count() == 0)
        {
            Print("[KOTH_ZoneManager] No zones available for random selection");
            return false;
        }
        
        int randomIndex = Math.RandomInt(0, m_AvailableZones.Count());
        string randomZone = m_AvailableZones.Get(randomIndex);
        
        Print("[KOTH_ZoneManager] Randomly selected zone: " + randomZone);
        return LoadZone(randomZone, true);
    }
    
    void EnableAutoRotation(float intervalSeconds, KOTHZoneSelectionMode mode)
    {
        if (!GetGame().IsServer())
            return;
        
        m_AutoRotateZones = true;
        m_ZoneRotationInterval = intervalSeconds;
        m_SelectionMode = mode;
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(AutoRotateZone, m_ZoneRotationInterval * 1000, true);
        
        Print("[KOTH_ZoneManager] Auto-rotation enabled: " + intervalSeconds + "s interval");
    }
    
    void DisableAutoRotation()
    {
        m_AutoRotateZones = false;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(AutoRotateZone);
        Print("[KOTH_ZoneManager] Auto-rotation disabled");
    }
    
    private void AutoRotateZone()
    {
        if (!m_AutoRotateZones)
            return;
        
        Print("[KOTH_ZoneManager] Auto-rotating to next zone");
        LoadNextZone();
    }
    
    void SyncActiveZoneToAllClients()
    {
        if (!GetGame().IsServer() || !m_ActiveZone)
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_SyncActiveZone");
        rpc.Write(m_ActiveZoneName);
        m_ActiveZone.OnSend(rpc);
        rpc.Expansion_Send(true, null);
        
        Print("[KOTH_ZoneManager] Synced zone to all clients: " + m_ActiveZoneName);
    }
    
    void RPC_SyncActiveZone(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        string zoneName;
        if (!ctx.Read(zoneName))
            return;
        
        m_ActiveZoneName = zoneName;
        m_ActiveZone = new KOTH_ZoneData();
        
        if (m_ActiveZone.OnReceive(ctx))
        {
            Print("[KOTH_ZoneManager] Client received zone: " + m_ActiveZone.GetZoneName());
        }
    }
    
    void RPC_RequestZoneInfo(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer() || !sender)
            return;
        
        auto rpc = Expansion_CreateRPC("RPC_SyncActiveZone");
        rpc.Write(m_ActiveZoneName);
        m_ActiveZone.OnSend(rpc);
        rpc.Expansion_Send(true, sender);
    }
    
    void RPC_AdminChangeZone(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer())
            return;
        
        string zoneName;
        if (!ctx.Read(zoneName))
            return;
        
        Print("[KOTH_ZoneManager] Admin " + sender.GetName() + " changing zone to: " + zoneName);
        LoadSpecificZone(zoneName);
    }
    
    void NotifyPlayersZoneChange()
    {
        if (!GetGame().IsServer())
            return;
        
        string message = "Zone changed to: " + m_ActiveZone.GetZoneName();
        
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player)
            {
                player.MessageStatus(message);
            }
        }
    }
    
    KOTH_ZoneData GetActiveZone() { return m_ActiveZone; }
    string GetActiveZoneName() { return m_ActiveZoneName; }
    array<string> GetAvailableZones() { return m_AvailableZones; }
    bool IsZoneActive() { return m_ActiveZone != null; }
    KOTHZoneSelectionMode GetSelectionMode() { return m_SelectionMode; }
    void SetSelectionMode(KOTHZoneSelectionMode mode) { m_SelectionMode = mode; }
    
    vector GetEastSpawnPosition()
    {
        if (!m_ActiveZone) return "0 0 0";
        return m_ActiveZone.GetEastSpawnBuilding();
    }
    
    vector GetWestSpawnPosition()
    {
        if (!m_ActiveZone) return "0 0 0";
        return m_ActiveZone.GetWestSpawnBuilding();
    }
    
    vector GetAOCenterPosition()
    {
        if (!m_ActiveZone) return "0 0 0";
        return m_ActiveZone.GetAOZoneCenter();
    }
}