/**
 * KOTH_PriorityZoneManager.c
 *
 * King of the Hill by Kahoona
 * Manages dynamic moving priority zone that circles around the capture zone
 * Uses dual-zone system to prevent player exit/enter notifications during movement
 * Place in: 4_World/Systems/KOTH_PriorityZoneManager.c
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 */

class KOTH_PriorityZoneManager
{
    private static KOTH_PriArea s_ActivePriorityZone;
    private static KOTH_PriArea s_NextPriorityZone;
    private static vector s_CaptureZoneCenter;
    private static float s_PriorityRadius;
    private static float s_TrajectoryRadius; // Half of capture zone radius
    private static float s_CurrentAngle = 0.0;
    private static bool s_IsActive = false;
    
    // Movement settings
    private static float s_MovementInterval = 30000; // Move every 30 seconds (in milliseconds)
    private static float s_AngleIncrement = 15.0; // Degrees to move each time
    private static bool s_Clockwise = true; // Direction of movement
    
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
        s_TrajectoryRadius = captureRadius / 2.0;
        s_CurrentAngle = Math.RandomFloat(0, 360); // Start at random angle
        
        Print("[KOTH_PriorityZoneManager] Capture Zone Center: " + captureCenter);
        Print("[KOTH_PriorityZoneManager] Capture Zone Radius: " + captureRadius + "m");
        Print("[KOTH_PriorityZoneManager] Priority Zone Radius: " + priorityRadius + "m");
        Print("[KOTH_PriorityZoneManager] Trajectory Radius: " + s_TrajectoryRadius + "m");
        Print("[KOTH_PriorityZoneManager] Starting Angle: " + s_CurrentAngle + "°");
        
        // Create initial priority zone at starting position
        CreateInitialPriorityZone();
        
        // Start movement timer
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MovePriorityZone, s_MovementInterval, true);
        
        s_IsActive = true;
        
        Print("[KOTH_PriorityZoneManager] Priority zone will move every " + (s_MovementInterval / 1000) + " seconds");
        Print("[KOTH_PriorityZoneManager] Movement increment: " + s_AngleIncrement + "° per move");
        Print("[KOTH_PriorityZoneManager] ═══════════════════════════════════════");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CREATE INITIAL PRIORITY ZONE
    //! ═══════════════════════════════════════════════════════════════
    
    static void CreateInitialPriorityZone()
    {
        if (!GetGame().IsServer())
            return;
        
        // Calculate position on trajectory circle
        vector position = CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
        
        Print("[KOTH_PriorityZoneManager] Creating initial priority zone at angle " + s_CurrentAngle + "°");
        Print("[KOTH_PriorityZoneManager] Position: " + position);
        
        // Create priority zone at calculated position
        if (Class.CastTo(s_ActivePriorityZone, GetGame().CreateObjectEx("KOTH_PriArea", position, ECE_NONE)))
        {
            s_ActivePriorityZone.KOTH_Init(position, s_PriorityRadius);
            Print("[KOTH_PriorityZoneManager] Initial priority zone created successfully");
            
            // Update marker position
            KOTH_MarkerSystem.UpdatePriorityMarker(position, s_PriorityRadius);
        }
        else
        {
            Error("[KOTH_PriorityZoneManager] Failed to create initial priority zone!");
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! MOVEMENT LOGIC (DUAL-ZONE SYSTEM)
    //! ═══════════════════════════════════════════════════════════════
    
    static void MovePriorityZone()
    {
        if (!s_IsActive || !GetGame().IsServer())
            return;
        
        // Calculate next angle
        if (s_Clockwise)
            s_CurrentAngle += s_AngleIncrement;
        else
            s_CurrentAngle -= s_AngleIncrement;
        
        // Wrap angle to 0-360 range
        if (s_CurrentAngle >= 360.0)
            s_CurrentAngle -= 360.0;
        else if (s_CurrentAngle < 0.0)
            s_CurrentAngle += 360.0;
        
        Print("[KOTH_PriorityZoneManager] Moving priority zone to angle: " + s_CurrentAngle + "°");
        
        // STEP 1: Create new zone at next position
        vector newPosition = CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
        
        Print("[KOTH_PriorityZoneManager] Creating next priority zone at: " + newPosition);
        
        if (Class.CastTo(s_NextPriorityZone, GetGame().CreateObjectEx("KOTH_PriArea", newPosition, ECE_NONE)))
        {
            s_NextPriorityZone.KOTH_Init(newPosition, s_PriorityRadius);
            Print("[KOTH_PriorityZoneManager] Next priority zone created successfully");
            
            // STEP 2: Update marker to new position
            KOTH_MarkerSystem.UpdatePriorityMarker(newPosition, s_PriorityRadius);
            
            // STEP 3: Delete old zone after a short delay to ensure smooth transition
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DeleteOldPriorityZone, 1000, false);
            
            // Notify players
            NotifyPlayersZoneMoved(newPosition);
        }
        else
        {
            Error("[KOTH_PriorityZoneManager] Failed to create next priority zone!");
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! DELETE OLD ZONE (STEP 3 OF MOVEMENT)
    //! ═══════════════════════════════════════════════════════════════
    
    static void DeleteOldPriorityZone()
    {
        if (!GetGame().IsServer())
            return;
        
        // Delete the old active zone
        if (s_ActivePriorityZone)
        {
            Print("[KOTH_PriorityZoneManager] Deleting old priority zone");
            GetGame().ObjectDelete(s_ActivePriorityZone);
            s_ActivePriorityZone = null;
        }
        
        // Make the next zone the new active zone
        s_ActivePriorityZone = s_NextPriorityZone;
        s_NextPriorityZone = null;
        
        Print("[KOTH_PriorityZoneManager] Zone transition complete");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! POSITION CALCULATION
    //! ═══════════════════════════════════════════════════════════════
    
    static vector CalculatePositionOnCircle(vector center, float radius, float angleDegrees)
    {
        // Convert degrees to radians
        float angleRadians = angleDegrees * Math.DEG2RAD;
        
        // Calculate X and Z offsets using trigonometry
        float offsetX = Math.Cos(angleRadians) * radius;
        float offsetZ = Math.Sin(angleRadians) * radius;
        
        // Create new position (Y stays the same as center)
        vector newPos = Vector(center[0] + offsetX, center[1], center[2] + offsetZ);
        
        return newPos;
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CONFIGURATION
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetMovementInterval(float seconds)
    {
        s_MovementInterval = seconds * 1000;
        
        // Restart timer with new interval
        if (s_IsActive)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(MovePriorityZone);
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(MovePriorityZone, s_MovementInterval, true);
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
    //! NOTIFICATIONS
    //! ═══════════════════════════════════════════════════════════════
    
    static void NotifyPlayersZoneMoved(vector newPosition)
    {
        if (!GetGame().IsServer())
            return;
        
        ref array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        
        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase player = PlayerBase.Cast(players.Get(i));
            if (player)
            {
                // Calculate distance from player to new priority zone
                float distance = vector.Distance(player.GetPosition(), newPosition);
                
                // Only notify if player is in capture zone
                if (distance <= (s_TrajectoryRadius * 2)) // Within capture zone range
                {
                    player.MessageStatus("[KOTH] Priority bonus zone has moved!");
                }
            }
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CLEANUP
    //! ═══════════════════════════════════════════════════════════════
    
    static void Cleanup()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_PriorityZoneManager] Cleaning up priority zones");
        
        // Stop movement timer
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(MovePriorityZone);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(DeleteOldPriorityZone);
        
        // Delete both zones if they exist
        if (s_ActivePriorityZone)
        {
            GetGame().ObjectDelete(s_ActivePriorityZone);
            s_ActivePriorityZone = null;
        }
        
        if (s_NextPriorityZone)
        {
            GetGame().ObjectDelete(s_NextPriorityZone);
            s_NextPriorityZone = null;
        }
        
        s_IsActive = false;
        
        Print("[KOTH_PriorityZoneManager] Priority zone cleanup complete");
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! GETTERS
    //! ═══════════════════════════════════════════════════════════════
    
    static vector GetCurrentPosition()
    {
        return CalculatePositionOnCircle(s_CaptureZoneCenter, s_TrajectoryRadius, s_CurrentAngle);
    }
    
    static float GetCurrentAngle()
    {
        return s_CurrentAngle;
    }
    
    static bool IsActive()
    {
        return s_IsActive;
    }
}