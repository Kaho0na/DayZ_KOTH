[CF_RegisterModule(KOTH_AIManager)]
class KOTH_AIManager: CF_ModuleWorld
{
    private static ref KOTH_AIManager s_Instance;
    private KOTH_ZoneData m_CurrentZone;
    private bool m_AISystemActive;
    int m_EastReinforcements;
    int m_WestReinforcements;

    // Pre-calculated spawn points (calculated once per zone)
    private vector m_EastSpawnPoint;
    private vector m_WestSpawnPoint;
    
    void KOTH_AIManager()
    {
        s_Instance = this;
    }
    
    void ~KOTH_AIManager()
    {
        if (s_Instance == this)
            s_Instance = null;
    }
    
    static KOTH_AIManager GetInstance()
    {
        return s_Instance;
    }
    
    void OnZoneActivated(KOTH_ZoneData zone)
    {
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (!GetGame().IsServer() || !settings.EnableAI)
            return;
        
        m_CurrentZone = zone;
        m_AISystemActive = true;
        
        Print("[KOTH_AIManager] ==========================================");
        Print("[KOTH_AIManager] Zone activated, calculating spawn points...");
        
        vector aoCenter = m_CurrentZone.GetAOZoneCenter();
        float aoRadius = m_CurrentZone.GetAOZoneRadius();
        vector eastBase = m_CurrentZone.GetEastSpawnBuilding();
        vector westBase = m_CurrentZone.GetWestSpawnBuilding();
        
        m_EastSpawnPoint = CalculateAISpawnPoint(aoCenter, eastBase, aoRadius, "East");
        m_WestSpawnPoint = CalculateAISpawnPoint(aoCenter, westBase, aoRadius, "West");
        
        Print("[KOTH_AIManager] East spawn point: " + m_EastSpawnPoint.ToString());
        Print("[KOTH_AIManager] West spawn point: " + m_WestSpawnPoint.ToString());
        Print("[KOTH_AIManager] AI system activated");
        Print("[KOTH_AIManager] ==========================================");
    }
    
    void SpawnSingleAI(string faction)
    {
        if (!m_AISystemActive || !m_CurrentZone)
        {
            Print("[KOTH_AIManager] Cannot spawn - system inactive or no zone");
            return;
        }

        if (faction == "East")
            m_EastReinforcements++;
        else if (faction == "West")
            m_WestReinforcements++;
        
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        vector baseSpawnPoint;
        string loadout;
        
        if (faction == "East")
        {
            baseSpawnPoint = m_EastSpawnPoint;
            loadout = settings.EastLoadout;
        }
        else
        {
            baseSpawnPoint = m_WestSpawnPoint;
            loadout = settings.WestLoadout;
        }
        
        // Add small random offset (5-15m) to avoid AIs stacking
        float randomAngle = Math.RandomFloat(0, 360);
        float randomDist = Math.RandomFloat(5, 15);
        float offsetX = randomDist * Math.Cos(randomAngle * Math.DEG2RAD);
        float offsetZ = randomDist * Math.Sin(randomAngle * Math.DEG2RAD);
        
        vector finalSpawn = Vector(baseSpawnPoint[0] + offsetX, baseSpawnPoint[1], baseSpawnPoint[2] + offsetZ);
        finalSpawn[1] = GetGame().SurfaceY(finalSpawn[0], finalSpawn[2]);
        
        // Safety check - if random offset puts us in water, use base point
        if (GetGame().SurfaceIsSea(finalSpawn[0], finalSpawn[2]))
        {
            Print("[KOTH_AIManager] WARNING: Random offset in water, using base spawn point");
            finalSpawn = baseSpawnPoint;
        }
        
        // Use Expansion's placement system for final validation
        finalSpawn = ExpansionAISpawnBase.GetPlacementPosition(finalSpawn);

        ExpansionAIPatrol config = new ExpansionAIPatrol();
        config.Faction = faction;
        config.Loadout = loadout;
        config.NumberOfAI = 1;
        config.Speed = settings.AISpeed;
        config.UnderThreatSpeed = settings.AIThreatSpeed;
        config.CanBeLooted = settings.AICanBeLooted == 1;
        if (settings.AIUnlimitedReload == 1)
        {
            // 50 % chance to enable unlimited reload
            if (Math.RandomIntInclusive(0, 1) == 0)
                config.UnlimitedReload = 1;
            else
                config.UnlimitedReload = 0;
        }
        else
        {
            config.UnlimitedReload = settings.AIUnlimitedReload;
        }
        config.AccuracyMin = settings.AIAccuracyMin;
        config.AccuracyMax = settings.AIAccuracyMax;
        config.LootingBehaviour = settings.AILootingBehaviour;
        config.ThreatDistanceLimit = settings.AIThreatDistance;
        config.UseRandomWaypointAsStartPoint = false;
        config.MinDistRadius = 1;
        config.MaxDistRadius = 3000;
        config.DespawnRadius = 2500;
        config.DespawnTime = 100;
        config.RespawnTime = -1;
        config.Behaviour = "ONCE";
        config.LoadBalancingCategory = "KOTH";
        config.Waypoints = new TVectorArray();
        // -------------------------
        // Waypoint selection logic
        // -------------------------
        config.Waypoints = new TVectorArray();
        config.Waypoints.Insert(finalSpawn);  // Always start at spawn

        // Try to get priority zone position
        vector priorityPos = KOTH_PriorityZoneManager.GetCurrentPosition();

        // Determine if it's valid (non-zero, finite, not too far)
        bool hasValidPriority = priorityPos != vector.Zero && priorityPos[0] != 0 && priorityPos[2] != 0 && vector.Distance(priorityPos, m_CurrentZone.GetAOZoneCenter()) < 10000;

        // 50% random choice if priority is valid
        bool attackPriority = hasValidPriority && (Math.RandomIntInclusive(0, 1) == 0);

        // Choose destination
        vector targetPos;
        if (attackPriority)
        {
            targetPos = priorityPos;
            Print("[KOTH_AIManager] AI assigned to attack PRIORITY zone at: " + targetPos.ToString());
        }
        else
        {
            targetPos = m_CurrentZone.GetAOZoneCenter();
            Print("[KOTH_AIManager] AI assigned to attack AO center at: " + targetPos.ToString());
        }

        // Add chosen destination
        config.Waypoints.Insert(targetPos);

        
        auto patrol = eAIDynamicPatrol.CreateEx(config, finalSpawn, true);
        
        if (patrol)
        {
            Print("[KOTH_AIManager] Successfully spawned " + faction + " AI at " + finalSpawn.ToString());
        }
        else
        {
            Print("[KOTH_AIManager] ERROR: Failed to spawn " + faction + " AI at " + finalSpawn.ToString());
        }
    }
    
    vector CalculateAISpawnPoint(vector aoCenter, vector factionBase, float aoRadius, string faction)
    {
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        
        vector direction = vector.Direction(factionBase, aoCenter);
        float dirLength = direction.Length();
        
        // CRITICAL SAFETY CHECK: Validate direction vector
        if (dirLength < 10)
        {
            Print("[KOTH_AIManager] ERROR: Faction base too close to AO center (" + dirLength + "m)!");
            Print("[KOTH_AIManager] Using default direction for " + faction);
            
            if (faction == "East")
                direction = Vector(1, 0, 0);
            else
                direction = Vector(-1, 0, 0);
        }
        else
        {
            direction = direction.Normalized();
        }
        
        // Final safety check on normalized direction
        if (direction[0] == 0 && direction[2] == 0)
        {
            Print("[KOTH_AIManager] ERROR: Invalid normalized direction vector!");
            direction = Vector(1, 0, 0);
        }
        
        // Apply random angle variation
        float angleOffset = Math.RandomFloat(-settings.AISpawnAngleVariation, settings.AISpawnAngleVariation);
        float angleRad = angleOffset * Math.DEG2RAD;
        
        float cosAngle = Math.Cos(angleRad);
        float sinAngle = Math.Sin(angleRad);
        float newX = direction[0] * cosAngle - direction[2] * sinAngle;
        float newZ = direction[0] * sinAngle + direction[2] * cosAngle;
        direction = Vector(newX, 0, newZ).Normalized();
        
        float spawnDistance = aoRadius + (settings.AISpawnBuffer * 0.5);
        vector spawnPos = aoCenter - (direction * spawnDistance);
        spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]);
        
        // Water check with progressively closer attempts
        if (GetGame().SurfaceIsSea(spawnPos[0], spawnPos[2]))
        {
            Print("[KOTH_AIManager] WARNING: " + faction + " spawn in water at " + spawnPos.ToString());
            Print("[KOTH_AIManager] Attempting to find land...");
            
            bool foundLand = false;
            
            for (int attempt = 1; attempt <= 5; attempt++)
            {
                float reducedDistance = spawnDistance - (attempt * 30);
                
                if (reducedDistance < aoRadius + 20)
                {
                    Print("[KOTH_AIManager] Trying near faction base");
                    spawnPos = factionBase + (direction * 100);
                    spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]);
                    
                    if (!GetGame().SurfaceIsSea(spawnPos[0], spawnPos[2]))
                    {
                        Print("[KOTH_AIManager] Found land near base");
                        foundLand = true;
                        break;
                    }
                }
                
                spawnPos = aoCenter - (direction * reducedDistance);
                spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]);
                
                if (!GetGame().SurfaceIsSea(spawnPos[0], spawnPos[2]))
                {
                    Print("[KOTH_AIManager] Found land at attempt " + attempt);
                    foundLand = true;
                    break;
                }
            }
            
            // FINAL FALLBACK: Use faction base if all attempts failed
            if (!foundLand)
            {
                Print("[KOTH_AIManager] CRITICAL: All spawn attempts in water! Using faction base as last resort.");
                spawnPos = factionBase;
                spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]) + 2.0;
            }
        }
        
        Print("[KOTH_AIManager] " + faction + " spawn calculated: " + spawnPos.ToString());
        Print("[KOTH_AIManager] Distance from AO: " + vector.Distance(spawnPos, aoCenter) + "m");
        
        return spawnPos;
    }

    void OnReinforcementArrived(string faction)
    {
        if (faction == "East" && m_EastReinforcements > 0)
            m_EastReinforcements--;
        else if (faction == "West" && m_WestReinforcements > 0)
            m_WestReinforcements--;
    }

    int GetReinforcementCount(string faction)
    {
        if (faction == "East")
            return m_EastReinforcements;
        if (faction == "West")
            return m_WestReinforcements;
        return 0;
    }


}