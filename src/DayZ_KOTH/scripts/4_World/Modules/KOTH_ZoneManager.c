/**
 * KOTH_ZoneManager.c
 *
 * King of the Hill by Kahoona
 * Centralized zone management system - handles active zone selection and rotation
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

[CF_RegisterModule(KOTH_ZoneManager)]
class KOTH_ZoneManager: CF_ModuleWorld
{
    private static ref KOTH_ZoneManager s_Instance;
    private ref KOTH_Zones m_ActiveZone;
    private string m_ActiveZoneName;
    private ref array<string> m_AvailableZones;

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
    }

    static KOTH_ZoneManager GetInstance()
    {
        if (!s_Instance)
        {
            Print("[KOTH_ZoneManager] ERROR: Instance not initialized!");
        }
        return s_Instance;
    }

    //! ═══════════════════════════════════════════════════════════════
    //! ZONE DISCOVERY & LOADING
    //! ═══════════════════════════════════════════════════════════════

    /**
     * Scans the KOTH_Zones folder for available zone JSON files
     */
    void DiscoverAvailableZones()
    {
        m_AvailableZones.Clear();

        string zonePath = EXPANSION_KOTH_Zones;
        string fileName;
        FileAttr fileAttr;

        // Find all .json files in the zones directory
        // Use 0 for files (FILES constant doesn't exist in Enforce)
        FindFileHandle handle = FindFile(zonePath + "*.json", fileName, fileAttr, 0);

        if (handle)
        {
            // First file
            if (fileName != "")
            {
                string zoneName = fileName;
                zoneName.Replace(".json", "");
                m_AvailableZones.Insert(zoneName);
                Print("[KOTH_ZoneManager] Discovered zone: " + zoneName);
            }

            // Additional files
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

    /**
     * Loads a specific zone by name
     * @param zoneName Name of the zone (without .json extension)
     * @return true if loaded successfully
     */
    bool LoadZone(string zoneName)
    {
        if (!GetGame().IsServer())
            return false;

        Print("[KOTH_ZoneManager] Attempting to load zone: " + zoneName);

        KOTH_Zones zone = KOTH_Zones.Load(zoneName);
        
        if (!zone)
        {
            Error("[KOTH_ZoneManager] Failed to load zone: " + zoneName);
            return false;
        }

        m_ActiveZone = zone;
        m_ActiveZoneName = zoneName;

        Print("[KOTH_ZoneManager] Successfully loaded zone: " + m_ActiveZoneName);
        Print("[KOTH_ZoneManager] Zone Name: " + m_ActiveZone.ZoneName);
        Print("[KOTH_ZoneManager] East Spawn: " + m_ActiveZone.EastSpawnBuilding.ToString());
        Print("[KOTH_ZoneManager] West Spawn: " + m_ActiveZone.WestSpawnBuilding.ToString());

        return true;
    }

    //! ═══════════════════════════════════════════════════════════════
    //! ZONE SELECTION LOGIC
    //! ═══════════════════════════════════════════════════════════════

    /**
     * Selects and loads the first available zone (for initial startup)
     * @return true if a zone was loaded
     */
    bool LoadFirstAvailableZone()
    {
        DiscoverAvailableZones();

        if (m_AvailableZones.Count() == 0)
        {
            Error("[KOTH_ZoneManager] No zones found in " + EXPANSION_KOTH_Zones);
            return false;
        }

        string firstZone = m_AvailableZones.Get(0);
        return LoadZone(firstZone);
    }

    /**
     * Selects next zone randomly (for future rotation)
     * @return true if a zone was loaded
     */
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

    /**
     * Selects next zone in rotation sequence (for future rotation)
     * @return true if a zone was loaded
     */
    bool LoadNextZoneInRotation()
    {
        if (m_AvailableZones.Count() == 0)
        {
            Print("[KOTH_ZoneManager] No zones available for rotation");
            return false;
        }

        int currentIndex = m_AvailableZones.Find(m_ActiveZoneName);
        int nextIndex = (currentIndex + 1) % m_AvailableZones.Count();
        string nextZone = m_AvailableZones.Get(nextIndex);

        Print("[KOTH_ZoneManager] Rotating to next zone: " + nextZone);
        return LoadZone(nextZone);
    }

    //! ═══════════════════════════════════════════════════════════════
    //! GETTERS
    //! ═══════════════════════════════════════════════════════════════

    /**
     * Returns the currently active zone
     */
    KOTH_Zones GetActiveZone()
    {
        return m_ActiveZone;
    }

    /**
     * Returns the name of the active zone
     */
    string GetActiveZoneName()
    {
        return m_ActiveZoneName;
    }

    /**
     * Returns list of all available zone names
     */
    array<string> GetAvailableZones()
    {
        return m_AvailableZones;
    }

    /**
     * Checks if a zone is currently active
     */
    bool IsZoneActive()
    {
        return m_ActiveZone != null;
    }

    //! ═══════════════════════════════════════════════════════════════
    //! CONVENIENCE METHODS (for backward compatibility)
    //! ═══════════════════════════════════════════════════════════════

    /**
     * Gets East spawn position from active zone
     */
    vector GetEastSpawnPosition()
    {
        if (!m_ActiveZone)
            return "0 0 0";

        return Vector(m_ActiveZone.EastSpawnBuilding[0], m_ActiveZone.EastSpawnBuilding[1], m_ActiveZone.EastSpawnBuilding[2]);
    }

    /**
     * Gets West spawn position from active zone
     */
    vector GetWestSpawnPosition()
    {
        if (!m_ActiveZone)
            return "0 0 0";

        return Vector(m_ActiveZone.WestSpawnBuilding[0], m_ActiveZone.WestSpawnBuilding[1], m_ActiveZone.WestSpawnBuilding[2]);
    }

    /**
     * Gets AO center position from active zone
     */
    vector GetAOCenterPosition()
    {
        if (!m_ActiveZone)
            return "0 0 0";

        return Vector(m_ActiveZone.AOZoneCenter[0], m_ActiveZone.AOZoneCenter[1], m_ActiveZone.AOZoneCenter[2]);
    }
}