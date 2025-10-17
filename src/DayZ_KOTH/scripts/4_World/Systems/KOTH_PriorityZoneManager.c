/**
 * KOTH_PriorityZoneManager.c (RANDOM TELEPORT)
 *
 * King of the Hill by Kahoona
 * Priority zone teleports to random positions on the circle
 * Place in: 4_World/Systems/KOTH_PriorityZoneManager.c
 */

class KOTH_PriorityZoneManager
{
    private static KOTH_PriArea s_ActivePriorityZone;
    private static vector s_CaptureZoneCenter;
    private static float s_PriorityRadiusMax;
    private static float s_PriorityRadiusMin;
    private static float s_CurrentPriorityRadius;
    private static float s_TrajectoryRadius;
    private static float s_CurrentAngle = 0.0;
    private static bool s_IsActive = false;
    
    private static bool s_EnableMovement = true;
    private static float s_MovementInterval = 30.0;
    private static float s_BonusMultiplier = 2.0;
    
    private static vector s_CurrentPosition;
    
    static void Initialize(vector captureCenter, float captureRadius, float priorityRadius)
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] ═══════════════════════════════════════");
        Print("[KOTH_PriorityZoneManager] Initializing Random Priority Zone");
        
        s_CaptureZoneCenter = captureCenter;
        s_PriorityRadiusMax = priorityRadius;
        s_PriorityRadiusMin = priorityRadius * 0.5;
        s_CurrentPriorityRadius = Math.RandomFloat(s_PriorityRadiusMin, s_PriorityRadiusMax);
        s_TrajectoryRadius = captureRadius - s_PriorityRadiusMax - 20.0;
        
        LoadSettings();
        
        s_CurrentAngle = Math.RandomFloat(0, 360);
        s_CurrentPosition = CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
        
        Print("[KOTH_PriorityZoneManager] Capture Zone Center: " + captureCenter);
        Print("[KOTH_PriorityZoneManager] Trajectory Radius: " + s_TrajectoryRadius + "m");
        Print("[KOTH_PriorityZoneManager] Priority Radius Range: " + s_PriorityRadiusMin + "m - " + s_PriorityRadiusMax + "m");
        Print("[KOTH_PriorityZoneManager] Starting Radius: " + s_CurrentPriorityRadius + "m");
        Print("[KOTH_PriorityZoneManager] Starting Angle: " + s_CurrentAngle + "°");
        Print("[KOTH_PriorityZoneManager] Movement Interval: " + s_MovementInterval + "s");
        Print("[KOTH_PriorityZoneManager] Bonus Multiplier: " + s_BonusMultiplier + "x");
        
        CreatePriorityZone();
        
        if (s_EnableMovement)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TeleportToRandomPosition, s_MovementInterval * 1000, true);
        }
        
        s_IsActive = true;
        
        Print("[KOTH_PriorityZoneManager] ═══════════════════════════════════════");
    }
    
    static void LoadSettings()
    {
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (settings)
        {
            s_EnableMovement = settings.EnablePriorityZoneMovement;
            s_MovementInterval = settings.PriorityZoneMovementInterval;
            s_BonusMultiplier = settings.PriorityZoneBonusMultiplier;
            
            Print("[KOTH_PriorityZoneManager] Settings loaded from JSON");
        }
        else
        {
            Print("[KOTH_PriorityZoneManager] WARNING: Could not load settings, using defaults");
        }
    }
    
    static void CreatePriorityZone()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] Creating priority zone at: " + s_CurrentPosition + " with radius: " + s_CurrentPriorityRadius);
        
        if (Class.CastTo(s_ActivePriorityZone, GetGame().CreateObjectEx("KOTH_PriArea", s_CurrentPosition, ECE_NONE)))
        {
            s_ActivePriorityZone.KOTH_Init(s_CurrentPosition, s_CurrentPriorityRadius);
            Print("[KOTH_PriorityZoneManager] Priority zone created successfully");
        }
        else
        {
            Error("[KOTH_PriorityZoneManager] Failed to create priority zone!");
        }
    }
    
    static void TeleportToRandomPosition()
    {
        if (!s_EnableMovement || !s_IsActive || !s_ActivePriorityZone)
            return;
        
        s_CurrentAngle = Math.RandomFloat(0, 360);
        s_CurrentPriorityRadius = Math.RandomFloat(s_PriorityRadiusMin, s_PriorityRadiusMax);
        s_CurrentPosition = CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
        
        s_ActivePriorityZone.SetPosition(s_CurrentPosition);
        
        if (s_ActivePriorityZone.GetTrigger())
        {
            s_ActivePriorityZone.GetTrigger().SetPosition(s_CurrentPosition);
            s_ActivePriorityZone.GetTrigger().SetCollisionCylinder(s_CurrentPriorityRadius, 200);
        }
        
        Print("[KOTH_PriorityZoneManager] Teleported to angle " + s_CurrentAngle + "° | Radius: " + s_CurrentPriorityRadius + "m | Position: " + s_CurrentPosition);
        
        BroadcastPriorityZoneUpdate();
        NotifyPlayersZoneChanged();
    }
    
    static void BroadcastPriorityZoneUpdate()
    {
        if (!GetGame().IsServer())
            return;
        
        KOTH_ZoneManager zoneManager = KOTH_ZoneManager.GetInstance();
        if (zoneManager)
        {
            zoneManager.SyncPriorityZoneToAllClients(s_CurrentPosition, s_CurrentPriorityRadius);
        }
    }
    
    static void NotifyPlayersZoneChanged()
    {
        if (!GetGame().IsServer())
            return;
        
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player && player.GetIdentity())
            {
                ExpansionNotification("New Orders", "Capture the new mission area").Info();
            }
        }
        
        Print("[KOTH_PriorityZoneManager] Notified all players of zone change");
    }
    
    static vector CalculatePositionOnCircle(vector center, float radius, float angleDegrees)
    {
        float angleRadians = angleDegrees * Math.DEG2RAD;
        
        float offsetX = Math.Cos(angleRadians) * radius;
        float offsetZ = Math.Sin(angleRadians) * radius;
        
        vector newPos = center;
        newPos[0] = newPos[0] + offsetX;
        newPos[2] = newPos[2] + offsetZ;
        
        return newPos;
    }
    
    static void SetMovementInterval(float seconds)
    {
        s_MovementInterval = seconds;
        
        if (s_IsActive && s_EnableMovement)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(TeleportToRandomPosition);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TeleportToRandomPosition, s_MovementInterval * 1000, true);
        }
        
        Print("[KOTH_PriorityZoneManager] Movement interval set to " + seconds + " seconds");
    }
    
    static void Cleanup()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] Cleaning up priority zones");
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(TeleportToRandomPosition);
        
        if (s_ActivePriorityZone)
        {
            GetGame().ObjectDelete(s_ActivePriorityZone);
            s_ActivePriorityZone = null;
        }
        
        s_IsActive = false;
        
        Print("[KOTH_PriorityZoneManager] Priority zone cleanup complete");
    }
    
    static vector GetCurrentPosition()
    {
        return s_CurrentPosition;
    }
    
    static float GetCurrentAngle()
    {
        return s_CurrentAngle;
    }
    
    static float GetCurrentRadius()
    {
        return s_CurrentPriorityRadius;
    }
    
    static bool IsActive()
    {
        return s_IsActive;
    }
    
    static float GetBonusMultiplier()
    {
        return s_BonusMultiplier;
    }
    
    static KOTH_PriArea GetActivePriorityZone()
    {
        return s_ActivePriorityZone;
    }
}