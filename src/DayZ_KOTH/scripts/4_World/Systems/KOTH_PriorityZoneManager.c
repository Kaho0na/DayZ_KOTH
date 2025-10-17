/**
 * KOTH_PriorityZoneManager.c
 *
 * King of the Hill by Kahoona
 * Manages dynamic moving priority zone with SMOOTH circular movement
 * Place in: 4_World/Systems/KOTH_PriorityZoneManager.c
 */

class KOTH_PriorityZoneManager
{
    private static KOTH_PriArea s_ActivePriorityZone;
    private static vector s_CaptureZoneCenter;
    private static float s_PriorityRadius;
    private static float s_TrajectoryRadius;
    private static float s_CurrentAngle = 0.0;
    private static bool s_IsActive = false;
    
    // Movement settings (loaded from KOTH_Settings)
    private static bool s_EnableMovement = true;
    private static float s_MovementInterval = 30.0;
    private static float s_AngleIncrement = 15.0;
    private static bool s_Clockwise = true;
    private static float s_BonusMultiplier = 2.0;
    
    // Smooth movement state
    private static vector s_CurrentPosition;
    private static vector s_TargetPosition;
    private static bool s_IsMoving = false;
    private static float s_MoveProgress = 0.0;
    
    //! ═══════════════════════════════════════════════════════════════
    //! INITIALIZATION
    //! ═══════════════════════════════════════════════════════════════
    
    static void Initialize(vector captureCenter, float captureRadius, float priorityRadius)
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] ═══════════════════════════════════════");
        Print("[KOTH_PriorityZoneManager] Initializing Dynamic Priority Zone");
        
        s_CaptureZoneCenter = captureCenter;
        s_PriorityRadius = priorityRadius;
        s_TrajectoryRadius = captureRadius - priorityRadius - 20.0; // Stay inside with buffer
        s_CurrentAngle = Math.RandomFloat(0, 360);
        
        // Load settings from JSON
        LoadSettings();
        
        Print("[KOTH_PriorityZoneManager] Capture Zone Center: " + captureCenter);
        Print("[KOTH_PriorityZoneManager] Trajectory Radius: " + s_TrajectoryRadius + "m");
        Print("[KOTH_PriorityZoneManager] Starting Angle: " + s_CurrentAngle + "°");
        Print("[KOTH_PriorityZoneManager] Movement Enabled: " + s_EnableMovement);
        Print("[KOTH_PriorityZoneManager] Movement Interval: " + s_MovementInterval + "s");
        Print("[KOTH_PriorityZoneManager] Angle Increment: " + s_AngleIncrement + "°");
        
        // Calculate initial position
        s_CurrentPosition = CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
        s_TargetPosition = s_CurrentPosition;
        
        // Create the single priority zone
        CreatePriorityZone();
        
        if (s_EnableMovement)
        {
            // Start angle update timer (every interval, calculate new target)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateTargetPosition, s_MovementInterval * 1000, true);
            
            // Start smooth movement updater (60 FPS)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateSmoothMovement, 16, true);
        }
        
        s_IsActive = true;
        
        Print("[KOTH_PriorityZoneManager] ═══════════════════════════════════════");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! LOAD SETTINGS FROM JSON
    //! ═══════════════════════════════════════════════════════════════
    
    static void LoadSettings()
    {
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        if (settings)
        {
            s_EnableMovement = settings.EnablePriorityZoneMovement;
            s_MovementInterval = settings.PriorityZoneMovementInterval;
            s_AngleIncrement = settings.PriorityZoneAngleIncrement;
            s_Clockwise = settings.PriorityZoneClockwise;
            s_BonusMultiplier = settings.PriorityZoneBonusMultiplier;
            
            Print("[KOTH_PriorityZoneManager] Settings loaded from JSON");
        }
        else
        {
            Print("[KOTH_PriorityZoneManager] WARNING: Could not load settings, using defaults");
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CREATE PRIORITY ZONE
    //! ═══════════════════════════════════════════════════════════════
    
    static void CreatePriorityZone()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] Creating priority zone at: " + s_CurrentPosition);
        
        if (Class.CastTo(s_ActivePriorityZone, GetGame().CreateObjectEx("KOTH_PriArea", s_CurrentPosition, ECE_NONE)))
        {
            s_ActivePriorityZone.KOTH_Init(s_CurrentPosition, s_PriorityRadius);
            Print("[KOTH_PriorityZoneManager] Priority zone created successfully");
            
            // Create initial marker
            KOTH_MarkerSystem.UpdatePriorityMarker(s_CurrentPosition, s_PriorityRadius);
        }
        else
        {
            Error("[KOTH_PriorityZoneManager] Failed to create priority zone!");
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! SMOOTH MOVEMENT SYSTEM
    //! ═══════════════════════════════════════════════════════════════
    
    static void UpdateTargetPosition()
    {
        if (!s_EnableMovement || !s_IsActive)
            return;
        
        // Calculate next angle
        if (s_Clockwise)
            s_CurrentAngle = s_CurrentAngle + s_AngleIncrement;
        else
            s_CurrentAngle = s_CurrentAngle - s_AngleIncrement;
        
        // Normalize angle to 0-360
        while (s_CurrentAngle >= 360.0)
            s_CurrentAngle = s_CurrentAngle - 360.0;
        while (s_CurrentAngle < 0.0)
            s_CurrentAngle = s_CurrentAngle + 360.0;
        
        // Calculate new target position
        s_TargetPosition = CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
        
        // Start smooth movement
        s_IsMoving = true;
        s_MoveProgress = 0.0;
        
        Print("[KOTH_PriorityZoneManager] New target: Angle " + s_CurrentAngle + "° | Position: " + s_TargetPosition);
    }
    
    static void UpdateSmoothMovement()
    {
        if (!s_IsActive || !s_ActivePriorityZone)
            return;
        
        if (!s_IsMoving)
            return;
        
        // Calculate progress per frame (60 FPS over interval)
        float progressPerFrame = 1.0 / (s_MovementInterval * 60.0);
        s_MoveProgress = s_MoveProgress + progressPerFrame;
        
        if (s_MoveProgress >= 1.0)
        {
            // Reached target
            s_CurrentPosition = s_TargetPosition;
            s_IsMoving = false;
            s_MoveProgress = 1.0;
        }
        else
        {
            // Lerp between current and target
            s_CurrentPosition = vector.Lerp(s_CurrentPosition, s_TargetPosition, s_MoveProgress);
        }
        
        // Update zone position
        s_ActivePriorityZone.SetPosition(s_CurrentPosition);
        
        // Update trigger position
        if (s_ActivePriorityZone.GetTrigger())
        {
            s_ActivePriorityZone.GetTrigger().SetPosition(s_CurrentPosition);
        }
        
        // Update marker position (every frame for smooth visual)
        KOTH_MarkerSystem.UpdatePriorityMarker(s_CurrentPosition, s_PriorityRadius);
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! POSITION CALCULATION
    //! ═══════════════════════════════════════════════════════════════
    
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
    
    //! ═══════════════════════════════════════════════════════════════
    //! CONFIGURATION
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetMovementInterval(float seconds)
    {
        s_MovementInterval = seconds;
        
        if (s_IsActive && s_EnableMovement)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateTargetPosition);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UpdateTargetPosition, s_MovementInterval * 1000, true);
        }
        
        Print("[KOTH_PriorityZoneManager] Movement interval set to " + seconds + " seconds");
    }
    
    static void SetAngleIncrement(float degrees)
    {
        s_AngleIncrement = degrees;
        Print("[KOTH_PriorityZoneManager] Angle increment set to " + degrees + "°");
    }
    
    static void SetDirection(bool clockwise)
    {
        s_Clockwise = clockwise;
        string dir;
        if (clockwise)
            dir = "clockwise";
        else
            dir = "counter-clockwise";
        Print("[KOTH_PriorityZoneManager] Direction set to " + dir);
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CLEANUP
    //! ═══════════════════════════════════════════════════════════════
    
    static void Cleanup()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] Cleaning up priority zones");
        
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateTargetPosition);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(UpdateSmoothMovement);
        
        if (s_ActivePriorityZone)
        {
            GetGame().ObjectDelete(s_ActivePriorityZone);
            s_ActivePriorityZone = null;
        }
        
        s_IsActive = false;
        
        Print("[KOTH_PriorityZoneManager] Priority zone cleanup complete");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! GETTERS
    //! ═══════════════════════════════════════════════════════════════
    
    static vector GetCurrentPosition()
    {
        return s_CurrentPosition;
    }
    
    static float GetCurrentAngle()
    {
        return s_CurrentAngle;
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