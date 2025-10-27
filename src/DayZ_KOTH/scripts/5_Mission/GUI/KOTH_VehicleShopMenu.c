class KOTH_VehicleShopMenuController : ExpansionViewController
{
    string PlayerBalance;
    string PlayerLevel;
    string RentalCooldown;
}

class KOTH_VehicleShopMenu : ExpansionScriptViewMenu
{
    protected KOTH_VehicleShopMenuController m_VehicleController;
    protected WrapSpacerWidget m_VehicleListWrapper;
    protected ButtonWidget m_CloseButton;
    protected TextWidget m_CooldownText;
    
    protected ref map<Widget, string> m_ButtonToVehicleClass;
    protected ref array<EntityAI> m_PreviewObjects;
    
    protected ref array<ref KOTH_VehicleShopItem> m_Vehicles;
    protected int m_PlayerLevel;
    protected int m_PlayerMoney;
    protected string m_PlayerFaction;
    protected float m_CooldownRemaining;
    
    void KOTH_VehicleShopMenu()
    {
        m_ButtonToVehicleClass = new map<Widget, string>();
        m_Vehicles = new array<ref KOTH_VehicleShopItem>();
        m_PreviewObjects = new array<EntityAI>();
        
        KOTH_VehicleShopModule.GetInstance().GetMenuSI().Insert(OnVehicleShopDataReceived);
    }
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/vehicleshop/KOTH_VehicleShopMenu.layout";
    }
    
    override typename GetControllerType()
    {
        return KOTH_VehicleShopMenuController;
    }
    
    override void OnShow()
    {
        super.OnShow();
        
        m_VehicleController = KOTH_VehicleShopMenuController.Cast(GetController());
        
        Widget root = GetLayoutRoot();
        m_VehicleListWrapper = WrapSpacerWidget.Cast(root.FindAnyWidget("VehicleListWrapper"));
        m_CloseButton = ButtonWidget.Cast(root.FindAnyWidget("CloseButton"));
        m_CooldownText = TextWidget.Cast(root.FindAnyWidget("CooldownText"));
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateCooldownDisplay, 1000, true);
    }
    
    override void OnHide()
    {
        super.OnHide();
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateCooldownDisplay);
        CleanupPreviewObjects();
    }
    
    void CleanupPreviewObjects()
    {
        foreach (EntityAI previewObj : m_PreviewObjects)
        {
            if (previewObj)
                GetGame().ObjectDelete(previewObj);
        }
        m_PreviewObjects.Clear();
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_CloseButton)
        {
            GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
            return true;
        }
        
        if (m_ButtonToVehicleClass.Contains(w))
        {
            HandleVehicleButtonClick(w);
            return true;
        }
        
        return false;
    }
    
    void HandleVehicleButtonClick(Widget button)
    {
        if (!m_ButtonToVehicleClass.Contains(button))
            return;
        
        if (m_CooldownRemaining > 0)
        {
            string timeStr = ((int)m_CooldownRemaining).ToString();
            ExpansionNotification("Cooldown Active", "Please wait " + timeStr + " seconds before renting another vehicle.").Error();
            return;
        }
        
        string vehicleClass = m_ButtonToVehicleClass.Get(button);
        OnRentButtonClick(vehicleClass);
    }
    
    void OnRentButtonClick(string vehicleClass)
    {
        Print("[KOTH_VehicleShopMenu] OnRentButtonClick called for: " + vehicleClass);
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
        {
            Print("[KOTH_VehicleShopMenu] ERROR: Player or identity null");
            return;
        }
        
        auto rpc = KOTH_VehicleShopModule.GetInstance().Expansion_CreateRPC("RPC_RentVehicle");
        rpc.Write(vehicleClass);
        rpc.Expansion_Send(player, true, player.GetIdentity());
        
        Print("[KOTH_VehicleShopMenu] RPC sent to server");
        
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
    }
    
    void OnVehicleShopDataReceived(array<ref KOTH_VehicleShopItem> vehicles, int playerLevel, int atmBalance, string playerFaction, float cooldownRemaining)
    {
        Print("[KOTH_VehicleShopMenu] OnVehicleShopDataReceived called");
        Print("[KOTH_VehicleShopMenu] Vehicles count: " + vehicles.Count());
        
        if (!m_VehicleController)
        {
            Print("[KOTH_VehicleShopMenu] ERROR: Controller not initialized yet!");
            return;
        }
        
        m_Vehicles = vehicles;
        m_PlayerLevel = playerLevel;
        m_PlayerMoney = atmBalance;
        m_PlayerFaction = playerFaction;
        m_CooldownRemaining = cooldownRemaining;
        
        m_VehicleController.PlayerBalance = "Balance: $" + m_PlayerMoney.ToString();
        m_VehicleController.PlayerLevel = "Level: " + m_PlayerLevel.ToString();
        
        if (m_CooldownRemaining > 0)
        {
            string timeStr = ((int)m_CooldownRemaining).ToString();
            m_VehicleController.RentalCooldown = "Cooldown: " + timeStr + "s";
        }
        else
        {
            m_VehicleController.RentalCooldown = "Ready to Rent";
        }
        
        m_VehicleController.NotifyPropertyChanged("PlayerBalance");
        m_VehicleController.NotifyPropertyChanged("PlayerLevel");
        m_VehicleController.NotifyPropertyChanged("RentalCooldown");
        
        PopulateVehicleList();
        Print("[KOTH_VehicleShopMenu] PopulateVehicleList called");
    }
    
    void PopulateVehicleList()
    {
        Print("[KOTH_VehicleShopMenu] === PopulateVehicleList START ===");
        Print("[KOTH_VehicleShopMenu] m_VehicleListWrapper: " + m_VehicleListWrapper);
        Print("[KOTH_VehicleShopMenu] m_Vehicles count: " + m_Vehicles.Count());
        
        if (!m_VehicleListWrapper)
        {
            Print("[KOTH_VehicleShopMenu] ERROR: m_VehicleListWrapper is null!");
            return;
        }
        
        Widget child = m_VehicleListWrapper.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            delete child;
            child = next;
        }
        
        m_ButtonToVehicleClass.Clear();
        
        Print("[KOTH_VehicleShopMenu] Starting vehicle loop...");
        
        foreach (KOTH_VehicleShopItem vehicle : m_Vehicles)
        {
            if (vehicle.Faction != "" && vehicle.Faction != "Both" && vehicle.Faction != m_PlayerFaction)
            {
                Print("[KOTH_VehicleShopMenu] Skipping vehicle (faction locked): " + vehicle.DisplayName + " (Faction: " + vehicle.Faction + ", Player: " + m_PlayerFaction + ")");
                continue;
            }
            
            Print("[KOTH_VehicleShopMenu] Processing vehicle: " + vehicle.DisplayName);
            
            bool isLocked = false;
            bool canAfford = true;
            
            if (m_PlayerLevel < vehicle.RequiredLevel)
                isLocked = true;
            
            if (m_PlayerMoney < vehicle.RentPrice)
                canAfford = false;
            
            Print("[KOTH_VehicleShopMenu] Creating entry widget...");
            Widget entry = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/GUI/layouts/vehicleshop/KOTH_VehicleShopMenuEntry.layout", m_VehicleListWrapper);
            
            if (!entry)
            {
                Print("[KOTH_VehicleShopMenu] ERROR: Failed to create entry widget!");
                continue;
            }
            
            Print("[KOTH_VehicleShopMenu] Entry widget created successfully");
            
            SetupVehiclePreview(entry, vehicle);
            
            TextWidget nameText = TextWidget.Cast(entry.FindAnyWidget("VehicleName"));
            TextWidget priceText = TextWidget.Cast(entry.FindAnyWidget("VehiclePrice"));
            ButtonWidget rentButton = ButtonWidget.Cast(entry.FindAnyWidget("RentButton"));
            TextWidget lockedText = TextWidget.Cast(entry.FindAnyWidget("LockedText"));
            
            Print("[KOTH_VehicleShopMenu] Widgets - Name: " + nameText + ", Price: " + priceText + ", Rent: " + rentButton + ", Locked: " + lockedText);
            
            if (nameText)
                nameText.SetText(vehicle.DisplayName);
            
            if (priceText)
                priceText.SetText("Rent: $" + vehicle.RentPrice.ToString());
            
            if (isLocked)
            {
                if (lockedText)
                {
                    lockedText.SetText("REQUIRE LVL " + vehicle.RequiredLevel.ToString());
                    lockedText.Show(true);
                }
                if (rentButton)
                    rentButton.Show(false);
            }
            else
            {
                if (lockedText)
                    lockedText.Show(false);
                
                if (rentButton)
                {
                    rentButton.Show(true);
                    
                    if (!canAfford || m_CooldownRemaining > 0)
                    {
                        rentButton.Enable(false);
                        rentButton.SetColor(ARGB(150, 128, 128, 128));
                    }
                    else
                    {
                        rentButton.Enable(true);
                        rentButton.SetColor(ARGB(255, 200, 200, 0));
                    }
                    
                    m_ButtonToVehicleClass.Set(rentButton, vehicle.ClassName);
                }
            }
        }
        
        Print("[KOTH_VehicleShopMenu] === PopulateVehicleList END ===");
        Print("[KOTH_VehicleShopMenu] Total entries created: " + m_ButtonToVehicleClass.Count());
    }
    
    void SetupVehiclePreview(Widget entryWidget, KOTH_VehicleShopItem vehicle)
    {
        ItemPreviewWidget itemPreview = ItemPreviewWidget.Cast(entryWidget.FindAnyWidget("VehicleImage"));
        if (!itemPreview)
        {
            Print("[KOTH_VehicleShopMenu] ERROR: VehicleImage widget not found");
            return;
        }
        
        EntityAI previewObj = EntityAI.Cast(GetGame().CreateObjectEx(vehicle.ClassName, vector.Zero, ECE_LOCAL | ECE_NOLIFETIME));
        if (previewObj)
        {
            m_PreviewObjects.Insert(previewObj);
            itemPreview.SetItem(previewObj);
            itemPreview.SetModelOrientation(Vector(0, 0, 0));
            itemPreview.Show(true);
            Print("[KOTH_VehicleShopMenu] Preview created for: " + vehicle.ClassName);
        }
        else
        {
            Print("[KOTH_VehicleShopMenu] ERROR: Failed to create preview object for: " + vehicle.ClassName);
        }
    }
    
    void UpdateCooldownDisplay()
    {
        if (m_CooldownRemaining > 0)
        {
            m_CooldownRemaining -= 1.0;
            
            if (m_CooldownRemaining < 0)
                m_CooldownRemaining = 0;
            
            if (m_CooldownRemaining > 0)
            {
                string timeStr = ((int)m_CooldownRemaining).ToString();
                m_VehicleController.RentalCooldown = "Cooldown: " + timeStr + "s";
            }
            else
            {
                m_VehicleController.RentalCooldown = "Ready to Rent";
                PopulateVehicleList();
            }
            
            m_VehicleController.NotifyPropertyChanged("RentalCooldown");
        }
    }
}