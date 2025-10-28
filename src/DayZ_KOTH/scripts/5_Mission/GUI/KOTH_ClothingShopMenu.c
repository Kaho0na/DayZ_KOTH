class KOTH_ClothingShopMenuController : ExpansionViewController
{
    string PlayerLevel;
    string EquipCooldown;
}

class KOTH_ClothingShopMenu : ExpansionScriptViewMenu
{
    protected KOTH_ClothingShopMenuController m_ClothingController;
    protected WrapSpacerWidget m_LoadoutListWrapper;
    protected ButtonWidget m_CloseButton;
    
    protected ref map<Widget, string> m_ButtonToLoadoutName;
    protected ref array<EntityAI> m_PreviewObjects;
    
    protected ref array<ref KOTH_ClothingShopLoadout> m_Loadouts;
    protected int m_PlayerLevel;
    protected string m_PlayerFaction;
    protected float m_CooldownRemaining;
    
    void KOTH_ClothingShopMenu()
    {
        m_ButtonToLoadoutName = new map<Widget, string>();
        m_Loadouts = new array<ref KOTH_ClothingShopLoadout>();
        m_PreviewObjects = new array<EntityAI>();
        
        KOTH_ClothingShopModule.GetInstance().GetMenuSI().Insert(OnClothingShopDataReceived);
    }
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/clothingshop/KOTH_ClothingShopMenu.layout";
    }
    
    override typename GetControllerType()
    {
        return KOTH_ClothingShopMenuController;
    }
    
    override void OnShow()
    {
        super.OnShow();
        
        m_ClothingController = KOTH_ClothingShopMenuController.Cast(GetController());
        
        Widget root = GetLayoutRoot();
        m_LoadoutListWrapper = WrapSpacerWidget.Cast(root.FindAnyWidget("LoadoutListWrapper"));
        m_CloseButton = ButtonWidget.Cast(root.FindAnyWidget("CloseButton"));
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(UpdateCooldownDisplay, 1000, true);
    }
    
    override void OnHide()
    {
        super.OnHide();
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(UpdateCooldownDisplay);
        
        if (m_PreviewObjects && m_PreviewObjects.Count() > 0)
        {
            foreach (EntityAI previewObj : m_PreviewObjects)
            {
                if (previewObj)
                    GetGame().ObjectDelete(previewObj);
            }
            m_PreviewObjects.Clear();
        }
    }
    
    void CleanupPreviewObjects()
    {
        if (!m_PreviewObjects)
            return;
        
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
        
        if (m_ButtonToLoadoutName.Contains(w))
        {
            HandleLoadoutButtonClick(w);
            return true;
        }
        
        return false;
    }
    
    void HandleLoadoutButtonClick(Widget button)
    {
        if (!m_ButtonToLoadoutName.Contains(button))
            return;
        
        if (m_CooldownRemaining > 0)
        {
            string timeStr = ((int)m_CooldownRemaining).ToString();
            ExpansionNotification("Cooldown Active", "Please wait " + timeStr + " seconds before changing loadout.").Error();
            return;
        }
        
        string loadoutName = m_ButtonToLoadoutName.Get(button);
        OnEquipButtonClick(loadoutName);
    }
    
    void OnEquipButtonClick(string loadoutName)
    {
        Print("[KOTH_ClothingShopMenu] OnEquipButtonClick called for: " + loadoutName);
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
        {
            Print("[KOTH_ClothingShopMenu] ERROR: Player or identity null");
            return;
        }
        
        auto rpc = KOTH_ClothingShopModule.GetInstance().Expansion_CreateRPC("RPC_EquipLoadout");
        rpc.Write(loadoutName);
        rpc.Expansion_Send(player, true, player.GetIdentity());
        
        Print("[KOTH_ClothingShopMenu] RPC sent to server");
    }
    
    void OnClothingShopDataReceived(array<ref KOTH_ClothingShopLoadout> loadouts, int playerLevel, string playerFaction, float cooldownRemaining)
    {
        Print("[KOTH_ClothingShopMenu] OnClothingShopDataReceived called");
        Print("[KOTH_ClothingShopMenu] Loadouts count: " + loadouts.Count());
        
        if (!m_ClothingController)
        {
            Print("[KOTH_ClothingShopMenu] ERROR: Controller not initialized yet!");
            return;
        }
        
        m_Loadouts = loadouts;
        m_PlayerLevel = playerLevel;
        m_PlayerFaction = playerFaction;
        m_CooldownRemaining = cooldownRemaining;
        
        m_ClothingController.PlayerLevel = "Level: " + m_PlayerLevel.ToString();
        
        if (m_CooldownRemaining > 0)
        {
            string timeStr = ((int)m_CooldownRemaining).ToString();
            m_ClothingController.EquipCooldown = "Cooldown: " + timeStr + "s";
        }
        else
        {
            m_ClothingController.EquipCooldown = "Ready to Equip";
        }
        
        m_ClothingController.NotifyPropertyChanged("PlayerLevel");
        m_ClothingController.NotifyPropertyChanged("EquipCooldown");
        
        PopulateLoadoutList();
        Print("[KOTH_ClothingShopMenu] PopulateLoadoutList called");
    }
    
    void PopulateLoadoutList()
    {
        Print("[KOTH_ClothingShopMenu] === PopulateLoadoutList START ===");
        Print("[KOTH_ClothingShopMenu] m_LoadoutListWrapper: " + m_LoadoutListWrapper);
        Print("[KOTH_ClothingShopMenu] m_Loadouts count: " + m_Loadouts.Count());
        
        if (!m_LoadoutListWrapper)
        {
            Print("[KOTH_ClothingShopMenu] ERROR: m_LoadoutListWrapper is null!");
            return;
        }
        
        CleanupPreviewObjects();
        
        Widget child = m_LoadoutListWrapper.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            delete child;
            child = next;
        }
        
        m_ButtonToLoadoutName.Clear();
        
        Print("[KOTH_ClothingShopMenu] Starting loadout loop...");
        
        foreach (KOTH_ClothingShopLoadout loadout : m_Loadouts)
        {
            if (loadout.Faction != "" && loadout.Faction != "Both" && loadout.Faction != m_PlayerFaction)
            {
                Print("[KOTH_ClothingShopMenu] Skipping loadout (faction locked): " + loadout.DisplayName + " (Faction: " + loadout.Faction + ", Player: " + m_PlayerFaction + ")");
                continue;
            }
            
            Print("[KOTH_ClothingShopMenu] Processing loadout: " + loadout.DisplayName);
            
            bool isLocked = false;
            
            if (m_PlayerLevel < loadout.RequiredLevel)
                isLocked = true;
            
            Print("[KOTH_ClothingShopMenu] Creating entry widget...");
            Widget entry = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/GUI/layouts/clothingshop/KOTH_ClothingShopMenuEntry.layout", m_LoadoutListWrapper);
            
            if (!entry)
            {
                Print("[KOTH_ClothingShopMenu] ERROR: Failed to create entry widget!");
                continue;
            }
            
            Print("[KOTH_ClothingShopMenu] Entry widget created successfully");
            
            SetupLoadoutPreview(entry, loadout);
            
            TextWidget nameText = TextWidget.Cast(entry.FindAnyWidget("LoadoutName"));
            ButtonWidget equipButton = ButtonWidget.Cast(entry.FindAnyWidget("EquipButton"));
            TextWidget lockedText = TextWidget.Cast(entry.FindAnyWidget("LockedText"));
            
            Print("[KOTH_ClothingShopMenu] Widgets - Name: " + nameText + ", Equip: " + equipButton + ", Locked: " + lockedText);
            
            if (nameText)
                nameText.SetText(loadout.DisplayName);
            
            if (isLocked)
            {
                if (lockedText)
                {
                    lockedText.SetText("REQUIRE LVL " + loadout.RequiredLevel.ToString());
                    lockedText.Show(true);
                }
                if (equipButton)
                    equipButton.Show(false);
            }
            else
            {
                if (lockedText)
                    lockedText.Show(false);
                
                if (equipButton)
                {
                    equipButton.Show(true);
                    
                    if (m_CooldownRemaining > 0)
                    {
                        equipButton.Enable(false);
                        equipButton.SetColor(ARGB(150, 128, 128, 128));
                    }
                    else
                    {
                        equipButton.Enable(true);
                        equipButton.SetColor(ARGB(255, 200, 200, 0));
                    }
                    
                    m_ButtonToLoadoutName.Set(equipButton, loadout.DisplayName);
                }
            }
        }
        
        Print("[KOTH_ClothingShopMenu] === PopulateLoadoutList END ===");
        Print("[KOTH_ClothingShopMenu] Total entries created: " + m_ButtonToLoadoutName.Count());
    }
    
    void SetupLoadoutPreview(Widget entryWidget, KOTH_ClothingShopLoadout loadout)
    {
        PlayerPreviewWidget playerPreview = PlayerPreviewWidget.Cast(entryWidget.FindAnyWidget("LoadoutPreview"));
        if (!playerPreview)
        {
            Print("[KOTH_ClothingShopMenu] ERROR: LoadoutPreview widget not found");
            return;
        }
        
        DayZPlayerImplement previewPlayer = DayZPlayerImplement.Cast(GetGame().CreateObjectEx("SurvivorMale", "0 0 0", ECE_CREATEPHYSICS | ECE_NOLIFETIME));
        if (!previewPlayer)
        {
            Print("[KOTH_ClothingShopMenu] ERROR: Failed to create preview player");
            return;
        }
        
        m_PreviewObjects.Insert(previewPlayer);
        
        bool loadoutApplied = ExpansionHumanLoadout.Apply(previewPlayer, loadout.FileName);
        Print("[KOTH_ClothingShopMenu] Loadout applied: " + loadoutApplied + " for: " + loadout.FileName);
        
        playerPreview.SetPlayer(previewPlayer);
        playerPreview.SetModelOrientation(vector.Zero);
        playerPreview.Show(true);
        
        Print("[KOTH_ClothingShopMenu] Preview setup complete for: " + loadout.DisplayName);
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
                m_ClothingController.EquipCooldown = "Cooldown: " + timeStr + "s";
            }
            else
            {
                m_ClothingController.EquipCooldown = "Ready to Equip";
                PopulateLoadoutList();
            }
            
            m_ClothingController.NotifyPropertyChanged("EquipCooldown");
        }
    }
}