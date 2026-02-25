/**
 * KOTH_MapMenu.c (WITH LBMASTER SUPPORT)
 *
 * King of the Hill by Kahoona
 * Client-side map with priority zone updates
 * Skips Expansion circle drawing when LBMaster Advanced Groups is active
 * Place in: 5_Mission/GUI/KOTH_MapMenu.c
 */

modded class ExpansionMapMenu extends UIScriptedMenu 
{
    ref array<string> koth_ZoneNames = new array<string>();
    ref array<vector> koth_ZonePositions = new array<vector>();
    ref array<float> koth_ZoneRadii = new array<float>();
    ref array<int> koth_ZoneColors = new array<int>();
    ref array<bool> koth_ZoneDrawCircles = new array<bool>();

    bool koth_StaticZonesDrawn = false;

    override Widget Init() 
    {
        Print("[KOTH_MapMenu] Init called");
        layoutRoot = super.Init();
        
        m_MapWidget = MapWidget.Cast(layoutRoot.FindAnyWidget("Map"));
        if (m_MapWidget)
        {
            Print("[KOTH_MapMenu] MapWidget initialized successfully");
        }
        else
        {
            Print("[KOTH_MapMenu] Failed to initialize MapWidget");
            return layoutRoot;
        }

#ifdef LBmaster_Groups
        Print("[KOTH_MapMenu] LBMaster detected - server handles map markers");
#else
        InitKOTHRPC();
        RequestKOTHZones();
#endif
        return layoutRoot;
    }

    void InitKOTHRPC()
    {
#ifdef LBmaster_Groups
        return;
#endif
        if (!GetGame().IsServer())
        {
            GetRPCManager().AddRPC("KOTH_MapMenu", "ReceiveKOTHZones", this, SingleplayerExecutionType.Client);
            Print("[KOTH_MapMenu] Client-side RPC initialized");
        }
    }

    void RequestKOTHZones()
    {
#ifdef LBmaster_Groups
        return;
#endif
        Print("[KOTH_MapMenu] Sending RequestKOTHZones RPC to server");
        GetRPCManager().SendRPC("KOTH_MapMenu", "RequestKOTHZones", null, true, null);
    }

    void ReceiveKOTHZones(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
#ifdef LBmaster_Groups
        return;
#endif
        if (type != CallType.Client)
            return;

        Param5<array<string>, array<vector>, array<float>, array<int>, array<bool>> data;
        if (!ctx.Read(data))
            return;

        koth_ZoneNames = data.param1;
        koth_ZonePositions = data.param2;
        koth_ZoneRadii = data.param3;
        koth_ZoneColors = data.param4;
        koth_ZoneDrawCircles = data.param5;

        Print("[KOTH_MapMenu] Received configuration data - " + koth_ZoneNames.Count() + " zones");

        RedrawAllZones();
    }

    void RedrawAllZones()
    {
#ifdef LBmaster_Groups
        return;
#endif
        if (!m_MapWidget)
        {
            Print("[KOTH_MapMenu] MapWidget is null, cannot redraw");
            return;
        }

        Print("[KOTH_MapMenu] Clearing all map markers for redraw");
        m_MapWidget.ClearUserMarks();
        
        koth_StaticZonesDrawn = false;
        
        DrawStaticZones();
        
        if (koth_ZoneNames.Count() > 3)
        {
            DrawPriorityZone();
        }
        
        Print("[KOTH_MapMenu] All zones redrawn");
    }

    void DrawStaticZones()
    {
        if (koth_StaticZonesDrawn)
        {
            Print("[KOTH_MapMenu] Static zones already drawn");
            return;
        }

        if (!m_MapWidget)
        {
            Print("[KOTH_MapMenu] MapWidget is null");
            return;
        }

        autoptr array<vector> zoneCirclePoints = new array<vector>();

        int staticZoneCount = 3;
        if (koth_ZoneNames.Count() > 3)
            staticZoneCount = 3;

        for (int i = 0; i < staticZoneCount; i++)
        {
            if (koth_ZoneDrawCircles.Get(i))
            {
                vector zonePos = koth_ZonePositions.Get(i);
                float radius = koth_ZoneRadii.Get(i);
                int color = koth_ZoneColors.Get(i);
                zoneCirclePoints.Clear();
                GetKOTHCirclePoints(radius, zonePos, zoneCirclePoints);
                for (int k = 0; k < zoneCirclePoints.Count(); k++)
                {
                    m_MapWidget.AddUserMark(zoneCirclePoints.Get(k), "", color, "\\dz\\gear\\navigation\\data\\map_bush_ca.paa");
                }
                
                Print("[KOTH_MapMenu] Drew " + koth_ZoneNames.Get(i) + " circle (" + zoneCirclePoints.Count() + " points)");
            }
        }

        koth_StaticZonesDrawn = true;
        Print("[KOTH_MapMenu] Static zones drawn successfully");
    }

    void DrawPriorityZone()
    {
        if (!m_MapWidget)
        {
            Print("[KOTH_MapMenu] MapWidget is null");
            return;
        }

        if (koth_ZoneNames.Count() <= 3)
        {
            Print("[KOTH_MapMenu] No priority zone data available");
            return;
        }

        int priorityIndex = 3;
        
        if (koth_ZoneDrawCircles.Get(priorityIndex))
        {
            vector zonePos = koth_ZonePositions.Get(priorityIndex);
            float radius = koth_ZoneRadii.Get(priorityIndex);
            int color = koth_ZoneColors.Get(priorityIndex);
            
            autoptr array<vector> zoneCirclePoints = new array<vector>();
            GetKOTHCirclePoints(radius, zonePos, zoneCirclePoints);
            
            for (int k = 0; k < zoneCirclePoints.Count(); k++)
            {
                m_MapWidget.AddUserMark(zoneCirclePoints.Get(k), "", color, "\\dz\\gear\\navigation\\data\\map_bush_ca.paa");
            }
            
            Print("[KOTH_MapMenu] Drew Priority Zone at " + zonePos + " with radius " + radius + "m (" + zoneCirclePoints.Count() + " points)");
        }
    }

    void GetKOTHCirclePoints(float radius, vector center, out array<vector> result)
    {
        float circumference = 2 * Math.PI * radius;
        int numPoints = circumference / 20;
        
        if (numPoints < 12)
            numPoints = 12;
        if (numPoints > 200)
            numPoints = 200;
        
        float slice = 2 * Math.PI / numPoints;
        for (int i = 0; i < numPoints; i++)
        {
            float angle = slice * i;
            float newX = center[0] + radius * Math.Cos(angle);
            float newZ = center[2] + radius * Math.Sin(angle);
            vector p = Vector(newX, 0, newZ);
            result.Insert(p);
        }
    }
}