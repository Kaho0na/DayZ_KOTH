class KOTH_VehicleSpawn
{
    protected string m_VehicleType;
    protected vector m_SpawnPosition;
    protected vector m_SpawnOrientation;
    protected Car m_SpawnedVehicle;
    protected ref TStringArray m_Attachments;
    protected float m_RespawnCheckInterval;
    protected ref ScriptCallQueue m_CallQueue;
    protected float m_CheckRadius;
    
    void KOTH_VehicleSpawn(string vehicleType, vector spawnPos, vector orientation)
    {
        m_VehicleType = vehicleType;
        m_SpawnPosition = spawnPos;
        m_SpawnOrientation = orientation;
        m_Attachments = new TStringArray;
        m_RespawnCheckInterval = 30000;
        m_CheckRadius = 20.0;
        m_CallQueue = GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM);
        
        SetupVehicleAttachments();
        
        Print("[KOTH_VehicleSpawn] Constructor: " + vehicleType + " at " + spawnPos.ToString());
    }
    
    void ~KOTH_VehicleSpawn()
    {
        StopRespawnTimer();
    }
    
    protected void SetupVehicleAttachments()
    {
        if (m_VehicleType == "Hatchback_02" || m_VehicleType == "Hatchback_02_Blue")
        {
            m_Attachments.Insert("Hatchback_02_Wheel");
            m_Attachments.Insert("Hatchback_02_Wheel");
            m_Attachments.Insert("Hatchback_02_Wheel");
            m_Attachments.Insert("Hatchback_02_Wheel");
            m_Attachments.Insert("Hatchback_02_Wheel");
            
            m_Attachments.Insert("Hatchback_02_Door_1_1");
            m_Attachments.Insert("Hatchback_02_Door_1_2");
            m_Attachments.Insert("Hatchback_02_Door_2_1");
            m_Attachments.Insert("Hatchback_02_Door_2_2");
            m_Attachments.Insert("Hatchback_02_Hood");
            m_Attachments.Insert("Hatchback_02_Trunk");
            
            m_Attachments.Insert("CarBattery");
            m_Attachments.Insert("CarRadiator");
            m_Attachments.Insert("SparkPlug");
            
            m_Attachments.Insert("HeadlightH7");
            m_Attachments.Insert("HeadlightH7");
        }
    }
    
    Car SpawnVehicle()
    {
        if (!GetGame().IsServer())
        {
            Print("[KOTH_VehicleSpawn] ERROR: Not server, cannot spawn vehicle");
            return null;
        }
        
        Print("[KOTH_VehicleSpawn] Attempting to spawn: " + m_VehicleType);
        Print("[KOTH_VehicleSpawn] Position: " + m_SpawnPosition.ToString());
        Print("[KOTH_VehicleSpawn] Orientation: " + m_SpawnOrientation.ToString());
        
        int remainingAmount = 1;
        EntityAI parent = NULL;
        bool attachmentNotAttached = false;
        
        Object spawnedObj = ExpansionItemSpawnHelper.SpawnVehicle(m_VehicleType, NULL, parent, m_SpawnPosition, m_SpawnOrientation, remainingAmount, NULL, -1, "", attachmentNotAttached);
        
        if (!spawnedObj)
        {
            Print("[KOTH_VehicleSpawn] ERROR: ExpansionItemSpawnHelper.SpawnVehicle returned null for: " + m_VehicleType);
            return null;
        }
        
        m_SpawnedVehicle = Car.Cast(spawnedObj);
        
        if (!m_SpawnedVehicle)
        {
            Print("[KOTH_VehicleSpawn] ERROR: Could not cast spawned object to Car");
            return null;
        }
        
        Print("[KOTH_VehicleSpawn] Vehicle spawned successfully using Expansion helper!");
        
        Print("[KOTH_VehicleSpawn] Manually attaching parts...");
        int attached = 0;
        foreach (string part : m_Attachments)
        {
            EntityAI attachment = m_SpawnedVehicle.GetInventory().CreateInInventory(part);
            if (attachment)
            {
                attachment.SetHealth("", "", 1000);
                attached++;
            }
            else
            {
                Print("[KOTH_VehicleSpawn] Failed to attach: " + part);
            }
        }
        Print("[KOTH_VehicleSpawn] Successfully attached " + attached.ToString() + " / " + m_Attachments.Count().ToString() + " parts");
        
        m_SpawnedVehicle.SetAllowDamage(false);
        
        Print("[KOTH_VehicleSpawn] Vehicle fully configured: " + m_VehicleType);
        Print("[KOTH_VehicleSpawn] Final vehicle position: " + m_SpawnedVehicle.GetPosition().ToString());
        Print("[KOTH_VehicleSpawn] Vehicle ID: " + m_SpawnedVehicle.GetID().ToString());
        
        StartRespawnTimer();
        
        return m_SpawnedVehicle;
    }
    
    void StartRespawnTimer()
    {
        if (!GetGame().IsServer())
        {
            return;
        }
        
        StopRespawnTimer();
        m_CallQueue.CallLater(CheckAndRespawnVehicle, m_RespawnCheckInterval, true);
        Print("[KOTH_VehicleSpawn] Started respawn timer - checking every " + (m_RespawnCheckInterval / 1000).ToString() + " seconds");
    }
    
    void StopRespawnTimer()
    {
        if (m_CallQueue)
        {
            m_CallQueue.Remove(CheckAndRespawnVehicle);
        }
    }
    
    protected void CheckAndRespawnVehicle()
    {
        if (!GetGame().IsServer())
        {
            return;
        }
        
        bool vehicleExistsAtSpawn = IsVehicleAtSpawnLocation();
        
        if (!vehicleExistsAtSpawn)
        {
            Print("[KOTH_VehicleSpawn] No vehicle found at spawn location, respawning...");
            SpawnVehicle();
        }
        else
        {
            Print("[KOTH_VehicleSpawn] Vehicle found at spawn location, no respawn needed");
        }
    }
    
    protected bool IsVehicleAtSpawnLocation()
    {
        array<Object> nearbyObjects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        
        GetGame().GetObjectsAtPosition(m_SpawnPosition, m_CheckRadius, nearbyObjects, proxyCargos);
        
        foreach (Object obj : nearbyObjects)
        {
            Car car = Car.Cast(obj);
            if (car && car.GetType() == m_VehicleType)
            {
                return true;
            }
        }
        
        return false;
    }
    
    bool IsVehicleDestroyed()
    {
        if (!m_SpawnedVehicle)
        {
            return true;
        }
        
        if (m_SpawnedVehicle.IsRuined())
        {
            return true;
        }
        
        return false;
    }
    
    void CleanupVehicle()
    {
        if (m_SpawnedVehicle)
        {
            Print("[KOTH_VehicleSpawn] Cleaning up vehicle: " + m_VehicleType);
            GetGame().ObjectDelete(m_SpawnedVehicle);
            m_SpawnedVehicle = null;
        }
    }
    
    Car GetVehicle()
    {
        return m_SpawnedVehicle;
    }
    
    vector GetSpawnPosition()
    {
        return m_SpawnPosition;
    }
    
    void SetRespawnInterval(float intervalMS)
    {
        m_RespawnCheckInterval = intervalMS;
        
        if (GetGame().IsServer() && m_SpawnedVehicle)
        {
            StopRespawnTimer();
            StartRespawnTimer();
        }
    }
}