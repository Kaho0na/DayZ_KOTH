/**
 * KOTH_LBMasterMapMarkers.c
 *
 * King of the Hill by Kahoona
 * Server-side LBMaster Advanced Groups marker integration
 * Only active when LBmaster_Groups mod is loaded
 * Place in: 4_World/Helpers/KOTH_LBMasterMapMarkers.c
 */

class KOTH_LBMasterMapMarkers
{
    private static ref KOTH_LBMasterMapMarkers s_Instance;

#ifdef LBmaster_Groups
    private LBServerMarker m_EastBaseMarker;
    private LBServerMarker m_WestBaseMarker;
    private LBServerMarker m_AOZoneMarker;
    private LBServerMarker m_PriorityZoneMarker;
#endif

    private bool m_MarkersCreated = false;

    private ref array<string> m_PendingNames;
    private ref array<vector> m_PendingPositions;
    private ref array<float> m_PendingRadii;
    private ref array<int> m_PendingColors;
    private ref array<bool> m_PendingDrawCircles;
    private int m_RetryCount = 0;
    static const int MAX_RETRIES = 5;
    static const int RETRY_DELAY_MS = 3000;

    static KOTH_LBMasterMapMarkers GetInstance()
    {
        if (!s_Instance)
        {
            s_Instance = new KOTH_LBMasterMapMarkers();
        }
        return s_Instance;
    }

    static bool IsLBMasterAvailable()
    {
#ifdef LBmaster_Groups
        return true;
#else
        return false;
#endif
    }

    void CreateOrUpdateMarkers(array<string> names, array<vector> positions, array<float> radii, array<int> colors, array<bool> drawCircles)
    {
        if (!GetGame().IsServer())
            return;

#ifdef LBmaster_Groups
        CleanupMarkers();

        if (!names || names.Count() == 0)
            return;

        m_PendingNames = names;
        m_PendingPositions = positions;
        m_PendingRadii = radii;
        m_PendingColors = colors;
        m_PendingDrawCircles = drawCircles;
        m_RetryCount = 0;

        TryCreateMarkers();
#endif
    }

    private void TryCreateMarkers()
    {
#ifdef LBmaster_Groups
        if (!GetGame().IsServer())
            return;

        if (!m_PendingNames || m_PendingNames.Count() == 0)
            return;

        Print("[KOTH_LBMaster] Attempt " + (m_RetryCount + 1) + "/" + MAX_RETRIES + " - Creating markers for " + m_PendingNames.Count() + " zones");

        if (!LBStaticMarkerManager.Get)
        {
            Print("[KOTH_LBMaster] ERROR: LBStaticMarkerManager.Get is NULL");
            ScheduleRetry();
            return;
        }

        bool allCreated = true;

        // East Base
        if (m_PendingNames.Count() >= 1 && m_PendingDrawCircles.Get(0))
        {
            int eastColor = m_PendingColors.Get(0);
            m_EastBaseMarker = LBStaticMarkerManager.Get.AddTempServerMarker(m_PendingNames.Get(0), m_PendingPositions.Get(0), "LBmaster_Groups\\gui\\icons\\safezone.paa", eastColor, true, true, true, true);
            if (m_EastBaseMarker)
            {
                int eastA, eastR, eastG, eastB;
                InvertColor(eastColor, eastA, eastR, eastG, eastB);
                m_EastBaseMarker.SetRadius(m_PendingRadii.Get(0), eastA, eastR, eastG, eastB, false);
                Print("[KOTH_LBMaster] East Base marker created: " + m_PendingNames.Get(0));
            }
            else
            {
                Print("[KOTH_LBMaster] FAILED East Base marker");
                allCreated = false;
            }
        }

        // West Base
        if (m_PendingNames.Count() >= 2 && m_PendingDrawCircles.Get(1))
        {
            int westColor = m_PendingColors.Get(1);
            m_WestBaseMarker = LBStaticMarkerManager.Get.AddTempServerMarker(m_PendingNames.Get(1), m_PendingPositions.Get(1), "LBmaster_Groups\\gui\\icons\\safezone.paa", westColor, true, true, true, true);
            if (m_WestBaseMarker)
            {
                int westA, westR, westG, westB;
                InvertColor(westColor, westA, westR, westG, westB);
                m_WestBaseMarker.SetRadius(m_PendingRadii.Get(1), westA, westR, westG, westB, false);
                Print("[KOTH_LBMaster] West Base marker created: " + m_PendingNames.Get(1));
            }
            else
            {
                Print("[KOTH_LBMaster] FAILED West Base marker");
                allCreated = false;
            }
        }

        // AO Zone
        if (m_PendingNames.Count() >= 3 && m_PendingDrawCircles.Get(2))
        {
            int aoColor = m_PendingColors.Get(2);
            m_AOZoneMarker = LBStaticMarkerManager.Get.AddTempServerMarker(m_PendingNames.Get(2), m_PendingPositions.Get(2), "LBmaster_Groups\\gui\\icons\\flag.paa", aoColor, true, true, true, true);
            if (m_AOZoneMarker)
            {
                int aoA, aoR, aoG, aoB;
                InvertColor(aoColor, aoA, aoR, aoG, aoB);
                m_AOZoneMarker.SetRadius(m_PendingRadii.Get(2), aoA, aoR, aoG, aoB, true);
                Print("[KOTH_LBMaster] AO Zone marker created: " + m_PendingNames.Get(2));
            }
            else
            {
                Print("[KOTH_LBMaster] FAILED AO Zone marker");
                allCreated = false;
            }
        }

        // Priority Zone
        if (m_PendingNames.Count() >= 4 && m_PendingDrawCircles.Get(3))
        {
            int priColor = m_PendingColors.Get(3);
            m_PriorityZoneMarker = LBStaticMarkerManager.Get.AddTempServerMarker(m_PendingNames.Get(3), m_PendingPositions.Get(3), "LBmaster_Groups\\gui\\icons\\star.paa", priColor, true, true, true, true);
            if (m_PriorityZoneMarker)
            {
                int priA, priR, priG, priB;
                InvertColor(priColor, priA, priR, priG, priB);
                m_PriorityZoneMarker.SetRadius(m_PendingRadii.Get(3), priA, priR, priG, priB, true);
                Print("[KOTH_LBMaster] Priority Zone marker created: " + m_PendingNames.Get(3));
            }
            else
            {
                Print("[KOTH_LBMaster] FAILED Priority Zone marker");
                allCreated = false;
            }
        }

        if (allCreated)
        {
            m_MarkersCreated = true;
            m_PendingNames = null;
            Print("[KOTH_LBMaster] All markers created successfully");
        }
        else
        {
            CleanupMarkers();
            ScheduleRetry();
        }
#endif
    }

    private void ScheduleRetry()
    {
        m_RetryCount++;
        if (m_RetryCount < MAX_RETRIES)
        {
            Print("[KOTH_LBMaster] Scheduling retry in " + RETRY_DELAY_MS + "ms...");
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TryCreateMarkers, RETRY_DELAY_MS, false);
        }
        else
        {
            Print("[KOTH_LBMaster] ERROR: Max retries reached. LBMaster markers not available. Falling back to Expansion map markers.");
            m_PendingNames = null;
        }
    }

    void UpdatePriorityZone(string name, vector position, float radius, int color)
    {
        if (!GetGame().IsServer())
            return;

#ifdef LBmaster_Groups
        if (m_PriorityZoneMarker)
        {
            m_PriorityZoneMarker.SetPosition(position);
            m_PriorityZoneMarker.SetName(name);
            int priA, priR, priG, priB;
            InvertColor(color, priA, priR, priG, priB);
            m_PriorityZoneMarker.SetRadius(radius, priA, priR, priG, priB, true);
            Print("[KOTH_LBMaster] Priority zone marker updated - Pos: " + position + " Radius: " + radius);
        }
        else
        {
            m_PriorityZoneMarker = LBStaticMarkerManager.Get.AddTempServerMarker(name, position, "LBmaster_Groups\\gui\\icons\\star.paa", color, true, true, true, true);
            if (m_PriorityZoneMarker)
            {
                InvertColor(color, priA, priR, priG, priB);
                m_PriorityZoneMarker.SetRadius(radius, priA, priR, priG, priB, true);
                Print("[KOTH_LBMaster] Priority zone marker created fresh - Pos: " + position);
            }
        }
#endif
    }

    void CleanupMarkers()
    {
        if (!GetGame().IsServer())
            return;

#ifdef LBmaster_Groups
        if (m_EastBaseMarker)
        {
            LBStaticMarkerManager.Get.RemoveServerMarker(m_EastBaseMarker);
            m_EastBaseMarker = null;
        }
        if (m_WestBaseMarker)
        {
            LBStaticMarkerManager.Get.RemoveServerMarker(m_WestBaseMarker);
            m_WestBaseMarker = null;
        }
        if (m_AOZoneMarker)
        {
            LBStaticMarkerManager.Get.RemoveServerMarker(m_AOZoneMarker);
            m_AOZoneMarker = null;
        }
        if (m_PriorityZoneMarker)
        {
            LBStaticMarkerManager.Get.RemoveServerMarker(m_PriorityZoneMarker);
            m_PriorityZoneMarker = null;
        }
        m_MarkersCreated = false;
        Print("[KOTH_LBMaster] All markers cleaned up");
#endif
    }

    bool HasMarkers()
    {
        return m_MarkersCreated;
    }

    private void InvertColor(int argbColor, out int a, out int r, out int g, out int b)
    {
        a = (argbColor >> 24) & 0xFF;
        r = (argbColor >> 16) & 0xFF;
        g = (argbColor >> 8) & 0xFF;
        b = argbColor & 0xFF;
    }
}