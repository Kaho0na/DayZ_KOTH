/**
 * KOTH_MapMenu.c
 *
 * King of the Hill by Kahoona
 * Client-side map circle drawing - following NBZ pattern exactly
 * Place in: 5_Mission/GUI/KOTH_MapMenu.c
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 */

modded class ExpansionMapMenu extends UIScriptedMenu 
{
    ref array<string> koth_ZoneNames = new array<string>();
    ref array<vector> koth_ZonePositions = new array<vector>();
    ref array<float> koth_ZoneRadii = new array<float>();
    ref array<int> koth_ZoneColors = new array<int>();
    ref array<bool> koth_ZoneDrawCircles = new array<bool>();

    bool koth_ZonesDrawn = false;

    override Widget Init() 
    {
        Print("[KOTH_MapMenu] Init called");
        Widget layoutRoot = super.Init();
        
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

        InitKOTHRPC();
        RequestKOTHZones();
        return layoutRoot;
    }

    void InitKOTHRPC()
    {
        if (!GetGame().IsServer())
        {
            GetRPCManager().AddRPC("KOTH_MapMenu", "ReceiveKOTHZones", this, SingleplayerExecutionType.Client);
            Print("[KOTH_MapMenu] Client-side RPC initialized");
        }
    }

    void RequestKOTHZones()
    {
        Print("[KOTH_MapMenu] Sending RequestKOTHZones RPC to server");
        GetRPCManager().SendRPC("KOTH_MapMenu", "RequestKOTHZones", null, true, null);
    }

    void ReceiveKOTHZones(CallType type, ParamsReadContext ctx, PlayerIdentity sender, Object target)
    {
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

        DrawKOTHZones();
    }

    void DrawKOTHZones()
    {
        if (koth_ZonesDrawn)
        {
            Print("[KOTH_MapMenu] Zones have already been drawn. Skipping redraw");
            return;
        }

        if (!m_MapWidget)
        {
            Print("[KOTH_MapMenu] MapWidget is null");
            return;
        }

        autoptr array<vector> zoneCirclePoints = new array<vector>();

        for (int i = 0; i < koth_ZonePositions.Count(); i++)
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

        koth_ZonesDrawn = true;
        Print("[KOTH_MapMenu] All KOTH zones drawn successfully");
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