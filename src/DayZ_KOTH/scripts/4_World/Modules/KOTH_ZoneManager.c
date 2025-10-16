/**
 * KOTH_ZoneManager.c (UPDATED WITH MARKER SUPPORT)
 *
 * King of the Hill by Kahoona
 * Centralized zone management system - handles active zone selection and rotation
 * NOW WITH MAP MARKER INTEGRATION
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
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
    
    void KOTH_ZoneManager()
    {
        s_Instance = this;
        m_AvailableZones = new array<string>();
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
        
        //! Initialize marker system
        if (GetGame().IsServer())
        {
            KOTH_MarkerSystem.Initialize();
        }
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
    
    bool LoadZone(string zoneName, bool despawnOldBases = true)
    {
        if (!GetGame().IsServer())
            return false;
        
        Print("[KOTH_ZoneManager] Loading zone: " + zoneName);
        
        if (despawnOldBases && m_ActiveZone)
        {
            KOTH_SpawnBase.DespawnAll();
            Print("[KOTH_ZoneManager] Despawned previous zone bases");
        }
        
        m_ActiveZone = KOTH_ZoneData.Load(zoneName);
        
        if (!m_ActiveZone)
        {
            Error("[KOTH_ZoneManager] Failed to load zone: " + zoneName);
            return false;
        }
        
        m_ActiveZoneName = zoneName;
        m_CurrentZoneIndex = m_AvailableZones.Find(zoneName);
        
        KOTH_SpawnBasesForZone(m_ActiveZone);
        
        Print("[KOTH_ZoneManager] Successfully loaded zone: " + m_ActiveZone.GetZoneName());
        Print("[KOTH_ZoneManager] East Spawn: " + m_ActiveZone.GetEastSpawnBuilding());
        Print("[KOTH_ZoneManager] West Spawn: " + m_ActiveZone.GetWestSpawnBuilding());
        Print("[KOTH_ZoneManager] AO Center: " + m_ActiveZone.GetAOZoneCenter());
        
        //! ═══════════════════════════════════════════════════════════════
        //! PLACE MAP MARKERS FOR NEW ZONE
        //! ═══════════════════════════════════════════════════════════════
        KOTH_MarkerSystem.PlaceZoneMarkers(m_ActiveZone);
        
        SyncActiveZoneToAllClients();
        NotifyPlayersZoneChange();
        
        return true;
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
        
        return LoadZone(nextZone);
    }
    
    bool LoadSpecificZone(string zoneName)
    {
        if (m_AvailableZones.Find(zoneName) == -1)
        {
            Error("[KOTH_ZoneManager] Zone not found: " + zoneName);
            return false;
        }
        
        return LoadZone(zoneName);
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
        return LoadZone(randomZone);
    }
    
    void EnableAutoRotation(float intervalSeconds, KOTHZoneSelectionMode mode)
    {
        if (!GetGame().IsServer())
            return;
        
        m_AutoRotateZones = true;
        m_ZoneRotationInterval = intervalSeconds;
        m_SelectionMode = mode;
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(AutoRotateZone, m_ZoneRotationInterval * 1000, true);
        
        Print("[KOTH_ZoneManager] Auto-rotation enabled: " + intervalSeconds + "s interval, mode: " + typename.EnumToString(KOTHZoneSelectionMode, mode));
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