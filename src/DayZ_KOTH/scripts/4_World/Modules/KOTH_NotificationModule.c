[CF_RegisterModule(KOTH_NotificationModule)]
class KOTH_NotificationModule: CF_ModuleWorld
{
    protected Widget m_Container;
    protected ref array<ref KOTH_CompactNotification> m_ActiveNotifications;
    protected const int MAX_NOTIFICATIONS = 5;
    protected const float NOTIFICATION_SPACING = 28.0;

    void KOTH_NotificationModule()
    {
        m_ActiveNotifications = new array<ref KOTH_CompactNotification>;
    }

    override void OnInit()
    {
        super.OnInit();
        
        Print("[KOTH_NotificationModule] OnInit - Server: " + GetGame().IsServer() + " Client: " + GetGame().IsClient());
        
        EnableMissionStart();
        Expansion_EnableRPCManager();
        Expansion_RegisterClientRPC("RPC_CompactNotification");
    }

    override void OnMissionStart(Class sender, CF_EventArgs args)
    {
        super.OnMissionStart(sender, args);
        
        Print("[KOTH_NotificationModule] OnMissionStart - Server: " + GetGame().IsServer());
        
        if (GetGame().IsServer())
        {
            Print("[KOTH_NotificationModule] Server-side - skipping widget creation");
            return;
        }
        
        Print("[KOTH_NotificationModule] Client - Initializing container");
        InitializeContainer();
    }

    static KOTH_NotificationModule GetInstance()
    {
        KOTH_NotificationModule instance;
        CF_Modules<KOTH_NotificationModule>.Get(instance);
        return instance;
    }

    static void ShowNotification(string message, int color = 0xFFFFFFFF, float displayTime = 3.0, PlayerIdentity sendTo = NULL)
    {
        ShowNotificationAdvanced(message, "", 0xFFFFFFFF, "", 0xFFFFFFFF, color, displayTime, sendTo);
    }

    static void ShowNotificationAdvanced(string message, string money, int moneyColor, string xp, int xpColor, int accentColor, float displayTime = 3.0, PlayerIdentity sendTo = NULL)
    {
        Print("[KOTH_NotificationModule] ShowNotificationAdvanced called - Server: " + IsMissionHost());
        
        if (IsMissionHost())
        {
            Print("[KOTH_NotificationModule] Sending RPC to clients: " + message);
            auto instance = GetInstance();
            if (!instance)
            {
                Print("[KOTH_NotificationModule] ERROR: GetInstance returned NULL!");
                return;
            }
            auto rpc = instance.Expansion_CreateRPC("RPC_CompactNotification");
            rpc.Write(message);
            rpc.Write(money);
            rpc.Write(moneyColor);
            rpc.Write(xp);
            rpc.Write(xpColor);
            rpc.Write(accentColor);
            rpc.Write(displayTime);
            rpc.Expansion_Send(true, sendTo);
            Print("[KOTH_NotificationModule] RPC sent successfully");
        }
        else
        {
            Print("[KOTH_NotificationModule] Client-side direct call: " + message);
            GetInstance().CreateNotificationAdvanced(message, money, moneyColor, xp, xpColor, accentColor, displayTime);
        }
    }

    void RPC_CompactNotification(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_NotificationModule] RPC_CompactNotification received on client");
        string message;
        if (!ctx.Read(message))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read message");
            return;
        }
        string money;
        if (!ctx.Read(money))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read money");
            return;
        }
        int moneyColor;
        if (!ctx.Read(moneyColor))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read moneyColor");
            return;
        }
        string xp;
        if (!ctx.Read(xp))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read xp");
            return;
        }
        int xpColor;
        if (!ctx.Read(xpColor))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read xpColor");
            return;
        }
        int accentColor;
        if (!ctx.Read(accentColor))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read accentColor");
            return;
        }
        float displayTime;
        if (!ctx.Read(displayTime))
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to read displayTime");
            return;
        }
        Print("[KOTH_NotificationModule] RPC data read successfully: " + message);
        CreateNotificationAdvanced(message, money, moneyColor, xp, xpColor, accentColor, displayTime);
    }

    void CreateNotification(string message, int color = 0xFFFFFFFF, float displayTime = 3.0)
    {
        CreateNotificationAdvanced(message, "", 0xFFFFFFFF, "", 0xFFFFFFFF, color, displayTime);
    }

    void CreateNotificationAdvanced(string message, string money, int moneyColor, string xp, int xpColor, int accentColor, float displayTime = 3.0)
    {
        if (!m_Container)
        {
            Print("[KOTH_NotificationModule] Container not initialized, attempting lazy initialization");
            InitializeContainer();
        }
        
        if (!m_Container)
        {
            Print("[KOTH_NotificationModule] ERROR: Container still not initialized after lazy init!");
            return;
        }
        
        Print("[KOTH_NotificationModule] Showing: " + message);
        if (m_ActiveNotifications.Count() >= MAX_NOTIFICATIONS)
        {
            if (m_ActiveNotifications[0])
            {
                m_ActiveNotifications[0].ForceRemove();
            }
            m_ActiveNotifications.Remove(0);
        }
        KOTH_CompactNotification notification = new KOTH_CompactNotification(m_Container, message, money, moneyColor, xp, xpColor, accentColor, displayTime);
        m_ActiveNotifications.Insert(notification);
        UpdatePositions();
    }
    
    void InitializeContainer()
    {
        Print("[KOTH_NotificationModule] InitializeContainer - Creating widgets now");
        
        Widget rootWidget = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/gui/layouts/compact_notification_container.layout");
        if (!rootWidget)
        {
            Print("[KOTH_NotificationModule] ERROR: Failed to create container root widget!");
            return;
        }
        Print("[KOTH_NotificationModule] Root widget created: " + rootWidget);
        
        m_Container = rootWidget.FindAnyWidget("CompactNotificationContainer");
        if (!m_Container)
        {
            Print("[KOTH_NotificationModule] ERROR: CompactNotificationContainer widget not found!");
            return;
        }
        Print("[KOTH_NotificationModule] Container initialized successfully: " + m_Container);
    }

    void UpdatePositions()
    {
        int count = m_ActiveNotifications.Count();
        int i;
        for (i = 0; i < count; i++)
        {
            if (m_ActiveNotifications[i] && m_ActiveNotifications[i].m_Root)
            {
                float yPos = (count - 1 - i) * NOTIFICATION_SPACING;
                m_ActiveNotifications[i].m_Root.SetPos(0, yPos);
            }
        }
    }

    void RemoveNotification(KOTH_CompactNotification notification)
    {
        int idx = m_ActiveNotifications.Find(notification);
        if (idx > -1)
        {
            m_ActiveNotifications.Remove(idx);
            UpdatePositions();
        }
    }
}

class KOTH_CompactNotification: ScriptedWidgetEventHandler
{
    Widget m_Root;
    protected TextWidget m_Message;
    protected TextWidget m_Money;
    protected TextWidget m_XP;
    protected Widget m_AccentBar;
    protected float m_DisplayTime;
    protected float m_FadeTime;
    protected bool m_IsFading;
    protected bool m_IsRemoving;

    void KOTH_CompactNotification(Widget parent, string message, string money, int moneyColor, string xp, int xpColor, int accentColor, float displayTime = 3.0)
    {
        if (!parent)
        {
            Print("[KOTH_CompactNotification] ERROR: Parent widget is null!");
            return;
        }
        m_Root = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/gui/layouts/compact_notification.layout", parent);
        if (!m_Root)
        {
            Print("[KOTH_CompactNotification] ERROR: Failed to create notification widget!");
            return;
        }
        m_Root.SetHandler(this);
        m_DisplayTime = displayTime;
        m_FadeTime = 0.3;
        m_IsFading = false;
        m_IsRemoving = false;
        m_Message = TextWidget.Cast(m_Root.FindAnyWidget("NotificationMessage"));
        m_Money = TextWidget.Cast(m_Root.FindAnyWidget("NotificationMoney"));
        m_XP = TextWidget.Cast(m_Root.FindAnyWidget("NotificationXP"));
        m_AccentBar = m_Root.FindAnyWidget("AccentBar");
        
        Print("[KOTH_CompactNotification] Money color: " + moneyColor + " XP color: " + xpColor);
        
        if (m_Message)
        {
            m_Message.SetText(message);
        }
        if (m_Money)
        {
            if (money != "")
            {
                m_Money.Show(true);
                Widget.Cast(m_Money).SetColor(moneyColor);
                m_Money.SetText(money);
            }
            else
            {
                m_Money.Show(false);
            }
        }
        if (m_XP)
        {
            if (xp != "")
            {
                m_XP.Show(true);
                Widget.Cast(m_XP).SetColor(xpColor);
                m_XP.SetText(xp);
            }
            else
            {
                m_XP.Show(false);
            }
        }
        if (m_AccentBar)
        {
            m_AccentBar.SetColor(accentColor);
        }
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(StartFadeOut, m_DisplayTime * 1000, false);
    }

    void ~KOTH_CompactNotification()
    {
        if (m_Root)
        {
            m_Root.Unlink();
        }
    }
    
    void ForceRemove()
    {
        if (m_IsRemoving) return;
        m_IsRemoving = true;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(StartFadeOut);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(FadeOutStep);
        if (m_Root)
        {
            m_Root.Unlink();
            m_Root = null;
        }
    }

    void StartFadeOut()
    {
        if (m_IsRemoving) return;
        m_IsFading = true;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(FadeOutStep, 16, true);
    }

    void FadeOutStep()
    {
        if (m_IsRemoving || !m_Root) return;
        float currentAlpha = m_Root.GetAlpha();
        float newAlpha = currentAlpha - (1.0 / (m_FadeTime * 60));
        if (newAlpha <= 0)
        {
            GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(FadeOutStep);
            ForceRemove();
            return;
        }
        m_Root.SetAlpha(newAlpha);
    }
}

static void ShowCompactNotification(string message, int color = 0xFFFFFFFF, float displayTime = 3.0)
{
    KOTH_NotificationModule.ShowNotification(message, color, displayTime);
}

static void SendCompactNotificationToAll(string message, int color = 0xFFFFFFFF, float displayTime = 3.0)
{
    KOTH_NotificationModule.ShowNotification(message, color, displayTime);
}