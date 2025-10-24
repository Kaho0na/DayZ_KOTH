// KOTH_ShopMenu.c
// Unified shop menu with embedded controller

class KOTH_ShopMenuController : ExpansionViewController
{
    string PlayerBalance;
    string PlayerLevel;
}

class KOTH_ShopMenu : ExpansionScriptViewMenu
{
    protected KOTH_ShopMenuController m_ShopController;
    protected ButtonWidget m_RiflesTab;
    protected ButtonWidget m_PistolsTab;
    protected ButtonWidget m_ScopesTab;
    protected ButtonWidget m_LaunchersTab;
    protected WrapSpacerWidget m_ItemListWrapper;
    protected ButtonWidget m_CloseButton;
    
    protected string m_CurrentCategory;
    protected ref map<Widget, string> m_ButtonToItemClass;
    
    protected ref array<ref KOTH_ShopCategory> m_Categories;
    protected ref array<string> m_OwnedItems;
    protected ref array<EntityAI> m_PreviewObjects;
    protected int m_PlayerLevel;
    protected int m_PlayerMoney;
    
    void KOTH_ShopMenu()
    {
        Print("[KOTH_ShopMenu] Constructor called - subscribing to module invoker");
        m_ButtonToItemClass = new map<Widget, string>();
        m_CurrentCategory = "";
        m_Categories = new array<ref KOTH_ShopCategory>();
        m_OwnedItems = new array<string>();
        m_PreviewObjects = new array<EntityAI>();
        
        KOTH_ShopModule.GetInstance().GetMenuSI().Insert(OnShopDataReceived);
        Print("[KOTH_ShopMenu] Subscribed to shop module invoker");
    }
    
    override string GetLayoutFile()
    {
        return "DayZ_KOTH/GUI/layouts/shop/KOTH_ShopMenu.layout";
    }
    
    override typename GetControllerType()
    {
        return KOTH_ShopMenuController;
    }
    
    override void OnShow()
    {
        super.OnShow();
        
        m_ShopController = KOTH_ShopMenuController.Cast(GetController());
        
        m_RiflesTab = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("RiflesTab"));
        m_PistolsTab = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("PistolsTab"));
        m_ScopesTab = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("ScopesTab"));
        m_LaunchersTab = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("LaunchersTab"));
        m_ItemListWrapper = WrapSpacerWidget.Cast(GetLayoutRoot().FindAnyWidget("ItemListWrapper"));
        m_CloseButton = ButtonWidget.Cast(GetLayoutRoot().FindAnyWidget("CloseButton"));
    }
    
    override void OnHide()
    {
        super.OnHide();
        
        foreach (EntityAI previewObj : m_PreviewObjects)
        {
            if (previewObj)
                GetGame().ObjectDelete(previewObj);
        }
        m_PreviewObjects.Clear();
    }
    
    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (w == m_RiflesTab)
        {
            ShowCategory("Rifles");
            return true;
        }
        
        if (w == m_PistolsTab)
        {
            ShowCategory("Pistols");
            return true;
        }
        
        if (w == m_ScopesTab)
        {
            ShowCategory("Scopes");
            return true;
        }
        
        if (w == m_LaunchersTab)
        {
            ShowCategory("Launchers");
            return true;
        }
        
        if (w == m_CloseButton)
        {
            Hide();
            return true;
        }
        
        if (m_ButtonToItemClass.Contains(w))
        {
            string itemClass = m_ButtonToItemClass.Get(w);
            string buttonName = w.GetName();
            
            if (buttonName.IndexOf("RentButton") != -1)
            {
                OnRentButtonClick(itemClass);
                return true;
            }
            
            if (buttonName.IndexOf("BuyButton") != -1)
            {
                OnBuyButtonClick(itemClass);
                return true;
            }
        }
        
        return false;
    }
    
    void OnShopDataReceived(array<ref KOTH_ShopCategory> categories, array<string> ownedItems, int level, int money)
    {
        Print("[KOTH_ShopMenu] OnShopDataReceived called with " + categories.Count() + " categories");
        
        if (!m_ShopController)
        {
            Print("[KOTH_ShopMenu] ERROR: Controller is null!");
            return;
        }
        
        Print("[KOTH_ShopMenu] Setting shop data...");
        m_Categories = categories;
        m_OwnedItems = ownedItems;
        m_PlayerLevel = level;
        m_PlayerMoney = money;
        
        UpdateBalance(money);
        UpdateLevel(level);
        
        Print("[KOTH_ShopMenu] Showing Rifles category...");
        ShowCategory("Rifles");
        Print("[KOTH_ShopMenu] Menu setup complete!");
    }
    
    void UpdateBalance(int balance)
    {
        m_PlayerMoney = balance;
        m_ShopController.PlayerBalance = "$" + balance.ToString();
        m_ShopController.NotifyPropertyChanged("PlayerBalance");
    }
    
    void UpdateLevel(int level)
    {
        m_PlayerLevel = level;
        m_ShopController.PlayerLevel = "Level: " + level.ToString();
        m_ShopController.NotifyPropertyChanged("PlayerLevel");
    }
    
    bool PlayerOwnsItem(string className)
    {
        return m_OwnedItems.Find(className) != -1;
    }
    
    bool PlayerCanAfford(int price)
    {
        return m_PlayerMoney >= price;
    }
    
    bool PlayerMeetsLevel(int required)
    {
        return m_PlayerLevel >= required;
    }
    
    KOTH_ShopCategory GetCategoryByName(string categoryName)
    {
        foreach (KOTH_ShopCategory cat : m_Categories)
        {
            if (cat.Category == categoryName)
                return cat;
        }
        return null;
    }
    
    void ShowCategory(string categoryName)
    {
        m_CurrentCategory = categoryName;
        
        SetTabActive(categoryName);
        
        KOTH_ShopCategory shopCategory = GetCategoryByName(categoryName);
        if (!shopCategory)
        {
            Print("[KOTH_ShopMenu] Category not found: " + categoryName);
            return;
        }
        
        PopulateItemList(shopCategory);
    }
    
    void SetTabActive(string categoryName)
    {
        if (m_RiflesTab)
            m_RiflesTab.SetColor(ARGB(255, 100, 100, 100));
        if (m_PistolsTab)
            m_PistolsTab.SetColor(ARGB(255, 100, 100, 100));
        if (m_ScopesTab)
            m_ScopesTab.SetColor(ARGB(255, 100, 100, 100));
        if (m_LaunchersTab)
            m_LaunchersTab.SetColor(ARGB(255, 100, 100, 100));
        
        ButtonWidget activeTab;
        if (categoryName == "Rifles")
            activeTab = m_RiflesTab;
        else if (categoryName == "Pistols")
            activeTab = m_PistolsTab;
        else if (categoryName == "Scopes")
            activeTab = m_ScopesTab;
        else if (categoryName == "Launchers")
            activeTab = m_LaunchersTab;
        
        if (activeTab)
            activeTab.SetColor(ARGB(255, 0, 200, 200));
    }
    
    void PopulateItemList(KOTH_ShopCategory shopCategory)
    {
        Widget child = m_ItemListWrapper.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            child.Unlink();
            child = next;
        }
        
        foreach (EntityAI previewObj : m_PreviewObjects)
        {
            if (previewObj)
                GetGame().ObjectDelete(previewObj);
        }
        m_PreviewObjects.Clear();
        
        m_ButtonToItemClass.Clear();
        
        foreach (KOTH_ShopItem item : shopCategory.Items)
        {
            Widget entry = CreateItemEntry(item);
            if (entry)
                m_ItemListWrapper.AddChild(entry);
        }
    }
    
    Widget CreateItemEntry(KOTH_ShopItem item)
    {
        Widget entry = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/GUI/layouts/shop/KOTH_ShopItemEntry.layout", m_ItemListWrapper);
        if (!entry)
        {
            Print("[KOTH_ShopMenu] Failed to create item entry widget");
            return null;
        }
        
        ItemPreviewWidget itemPreview = ItemPreviewWidget.Cast(entry.FindAnyWidget("ItemImage"));
        if (itemPreview)
        {
            EntityAI previewObj = EntityAI.Cast(GetGame().CreateObjectEx(item.ClassName, vector.Zero, ECE_PLACE_ON_SURFACE));
            if (previewObj)
            {
                m_PreviewObjects.Insert(previewObj);
                itemPreview.SetItem(previewObj);
                itemPreview.SetModelOrientation(vector.Zero);
            }
        }
        
        TextWidget itemName = TextWidget.Cast(entry.FindAnyWidget("ItemName"));
        if (itemName)
            itemName.SetText(item.DisplayName);
        
        TextWidget itemAmmo = TextWidget.Cast(entry.FindAnyWidget("ItemAmmo"));
        if (itemAmmo)
            itemAmmo.SetText(item.AmmoType);
        
        ButtonWidget rentButton = ButtonWidget.Cast(entry.FindAnyWidget("RentButton"));
        ButtonWidget buyButton = ButtonWidget.Cast(entry.FindAnyWidget("BuyButton"));
        
        bool itemOwned = PlayerOwnsItem(item.ClassName);
        bool meetsLevel = PlayerMeetsLevel(item.RequiredLevel);
        bool canAffordRent = PlayerCanAfford(item.RentPrice);
        bool canAffordBuy = PlayerCanAfford(item.BuyPrice);
        
        Widget ownedBadgeWidget = entry.FindAnyWidget("OwnedBadge");
        if (ownedBadgeWidget)
            ownedBadgeWidget.Show(itemOwned);
        
        TextWidget levelReq = TextWidget.Cast(entry.FindAnyWidget("LevelRequirement"));
        if (levelReq && !meetsLevel)
        {
            levelReq.SetText("REQUIRE LVL " + item.RequiredLevel.ToString());
            levelReq.Show(true);
        }
        else if (levelReq)
        {
            levelReq.Show(false);
        }
        
        if (rentButton)
        {
            rentButton.SetText("RENT\n$" + item.RentPrice.ToString());
            
            if (!meetsLevel)
            {
                rentButton.Enable(false);
                rentButton.SetColor(ARGB(255, 100, 100, 100));
            }
            else if (!canAffordRent)
            {
                rentButton.SetColor(ARGB(255, 150, 150, 0));
            }
            else
            {
                rentButton.Enable(true);
                rentButton.SetColor(ARGB(255, 200, 200, 0));
            }
            
            m_ButtonToItemClass.Set(rentButton, item.ClassName);
        }
        
        if (buyButton)
        {
            if (itemOwned)
            {
                buyButton.SetText("EQUIP\nFREE");
                buyButton.Enable(true);
                buyButton.SetColor(ARGB(255, 0, 200, 0));
            }
            else
            {
                buyButton.SetText("BUY\n$" + item.BuyPrice.ToString());
                
                if (!meetsLevel)
                {
                    buyButton.Enable(false);
                    buyButton.SetColor(ARGB(255, 100, 100, 100));
                }
                else if (!canAffordBuy)
                {
                    buyButton.SetColor(ARGB(255, 0, 150, 0));
                }
                else
                {
                    buyButton.Enable(true);
                    buyButton.SetColor(ARGB(255, 0, 200, 0));
                }
            }
            
            m_ButtonToItemClass.Set(buyButton, item.ClassName);
        }
        
        return entry;
    }
    
    void OnRentButtonClick(string itemClass)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        
        auto rpc = KOTH_ShopModule.GetInstance().Expansion_CreateRPC("RPC_RentItem");
        rpc.Write(itemClass);
        rpc.Expansion_Send(player, true, player.GetIdentity());
        
        Hide();
    }
    
    void OnBuyButtonClick(string itemClass)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        
        auto rpc = KOTH_ShopModule.GetInstance().Expansion_CreateRPC("RPC_BuyItem");
        rpc.Write(itemClass);
        rpc.Expansion_Send(player, true, player.GetIdentity());
        
        Hide();
    }
}