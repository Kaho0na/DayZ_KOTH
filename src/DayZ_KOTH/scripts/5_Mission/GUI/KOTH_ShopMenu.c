// KOTH_ShopMenu.c - OPTIMIZED
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
    protected ButtonWidget m_ItemsTab;
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
        m_ButtonToItemClass = new map<Widget, string>();
        m_CurrentCategory = "";
        m_Categories = new array<ref KOTH_ShopCategory>();
        m_OwnedItems = new array<string>();
        m_PreviewObjects = new array<EntityAI>();
        
        KOTH_ShopModule.GetInstance().GetMenuSI().Insert(OnShopDataReceived);
        KOTH_ShopModule.GetInstance().GetItemPurchasedSI().Insert(OnItemPurchased);
        KOTH_ShopModule.GetInstance().GetItemEquippedSI().Insert(OnItemEquipped);
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
        
        Widget root = GetLayoutRoot();
        m_RiflesTab = ButtonWidget.Cast(root.FindAnyWidget("RiflesTab"));
        m_PistolsTab = ButtonWidget.Cast(root.FindAnyWidget("PistolsTab"));
        m_ScopesTab = ButtonWidget.Cast(root.FindAnyWidget("ScopesTab"));
        m_ItemsTab = ButtonWidget.Cast(root.FindAnyWidget("ItemsTab"));
        m_ItemListWrapper = WrapSpacerWidget.Cast(root.FindAnyWidget("ItemListWrapper"));
        m_CloseButton = ButtonWidget.Cast(root.FindAnyWidget("CloseButton"));
    }
    
    override void OnHide()
    {
        super.OnHide();
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
        Print("[KOTH_ShopMenu] OnClick called on widget: " + w.GetName());
        
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
        
        if (w == m_ItemsTab)
        {
            ShowCategory("Items");
            return true;
        }
        
        if (w == m_CloseButton)
        {
            GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
            return true;
        }
        
        if (m_ButtonToItemClass.Contains(w))
        {
            Print("[KOTH_ShopMenu] Item button clicked!");
            HandleItemButtonClick(w);
            return true;
        }
        
        Print("[KOTH_ShopMenu] Widget not handled: " + w.GetName());
        return false;
    }
    
    void HandleItemButtonClick(Widget button)
    {
        Print("[KOTH_ShopMenu] HandleItemButtonClick called");
        
        if (!m_ButtonToItemClass.Contains(button))
        {
            Print("[KOTH_ShopMenu] ERROR: Button not found in map!");
            return;
        }
        
        string itemClass = m_ButtonToItemClass.Get(button);
        string buttonName = button.GetName();
        
        Print("[KOTH_ShopMenu] Button: " + buttonName + " | Item: " + itemClass);
        
        if (buttonName.IndexOf("RentButton") != -1)
        {
            Print("[KOTH_ShopMenu] Calling OnRentButtonClick");
            OnRentButtonClick(itemClass);
        }
        else if (buttonName.IndexOf("BuyButton") != -1)
        {
            Print("[KOTH_ShopMenu] Calling OnBuyButtonClick");
            OnBuyButtonClick(itemClass);
        }
    }
    
    void OnShopDataReceived(array<ref KOTH_ShopCategory> categories, array<string> ownedItems, int level, int money)
    {
        Print("[KOTH_ShopMenu] OnShopDataReceived called with " + categories.Count() + " categories");
        
        if (!m_ShopController)
        {
            Print("[KOTH_ShopMenu] ERROR: Controller not initialized yet!");
            return;
        }
        
        m_Categories = categories;
        m_OwnedItems = ownedItems;
        m_PlayerLevel = level;
        m_PlayerMoney = money;
        
        UpdateBalance(money);
        UpdateLevel(level);
        ShowCategory("Rifles");
    }
    
    void OnItemPurchased(string itemClass, int newBalance)
    {
        Print("[KOTH_ShopMenu] OnItemPurchased: " + itemClass + " | New Balance: " + newBalance);
        
        if (m_OwnedItems.Find(itemClass) == -1)
        {
            m_OwnedItems.Insert(itemClass);
        }
        
        UpdateBalance(newBalance);
        
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshCurrentCategory, 100, false);
    }
    
    void OnItemEquipped(string itemClass)
    {
        Print("[KOTH_ShopMenu] OnItemEquipped: " + itemClass);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(RefreshCurrentCategory, 100, false);
    }
    
    void RefreshCurrentCategory()
    {
        if (m_CurrentCategory != "")
        {
            Print("[KOTH_ShopMenu] Refreshing category: " + m_CurrentCategory);
            ShowCategory(m_CurrentCategory);
        }
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
    
    bool PlayerHasItemInInventory(string className)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return false;
        
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        
        foreach (EntityAI item : items)
        {
            if (item.GetType() == className)
                return true;
        }
        
        EntityAI handsItem = player.GetHumanInventory().GetEntityInHands();
        if (handsItem && handsItem.GetType() == className)
            return true;
        
        return false;
    }
    
    void ShowCategory(string categoryName)
    {
        Print("[KOTH_ShopMenu] ShowCategory: " + categoryName);
        m_CurrentCategory = categoryName;
        
        KOTH_ShopCategory category = null;
        foreach (KOTH_ShopCategory cat : m_Categories)
        {
            if (cat.Category == categoryName)
            {
                category = cat;
                break;
            }
        }
        
        if (!category)
        {
            Print("[KOTH_ShopMenu] ERROR: Category not found: " + categoryName);
            return;
        }
        
        ClearItemList();
        
        foreach (KOTH_ShopItem shopItem : category.Items)
        {
            if (categoryName == "Scopes")
            {
                array<string> compatibleScopes = GetCompatibleScopes();
                bool isCompatible = false;
                
                foreach (string compatScope : compatibleScopes)
                {
                    if (compatScope == shopItem.ClassName)
                    {
                        isCompatible = true;
                        break;
                    }
                }
                
                if (!isCompatible)
                {
                    Print("[KOTH_ShopMenu] Skipping incompatible scope: " + shopItem.ClassName);
                    continue;
                }
            }
            
            CreateItemEntry(shopItem);
        }
    }
    
    array<string> GetCompatibleScopes()
    {
        array<string> compatibleScopes = new array<string>();
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return compatibleScopes;
        
        EntityAI weaponInHands = player.GetHumanInventory().GetEntityInHands();
        if (!weaponInHands || !weaponInHands.IsWeapon())
            return compatibleScopes;
        
        Print("[KOTH_ShopMenu] Checking scope compatibility for weapon: " + weaponInHands.GetType());
        
        KOTH_ShopCategory scopeCategory = null;
        foreach (KOTH_ShopCategory cat : m_Categories)
        {
            if (cat.Category == "Scopes")
            {
                scopeCategory = cat;
                break;
            }
        }
        
        if (!scopeCategory)
            return compatibleScopes;
        
        foreach (KOTH_ShopItem scopeItem : scopeCategory.Items)
        {
            if (CanAttachScope(weaponInHands, scopeItem.ClassName))
            {
                compatibleScopes.Insert(scopeItem.ClassName);
            }
        }
        
        Print("[KOTH_ShopMenu] Found " + compatibleScopes.Count() + " compatible scopes");
        return compatibleScopes;
    }
    
    bool CanAttachScope(EntityAI weapon, string scopeClassName)
    {
        InventoryLocation existingScopeLoc = new InventoryLocation();
        EntityAI existingScope = null;
        
        array<EntityAI> attachments = new array<EntityAI>();
        weapon.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, attachments);
        
        foreach (EntityAI att : attachments)
        {
            if (IsScope(att.GetType()))
            {
                existingScope = att;
                att.GetInventory().GetCurrentInventoryLocation(existingScopeLoc);
                GameInventory.LocationRemoveEntity(existingScopeLoc);
                break;
            }
        }
        
        EntityAI testScope = ExpansionItemSpawnHelper.SpawnAttachment(scopeClassName, weapon);
        bool canAttach = false;
        
        if (testScope)
        {
            GetGame().ObjectDelete(testScope);
            canAttach = true;
        }
        
        if (existingScope && existingScopeLoc)
        {
            GameInventory.LocationAddEntity(existingScopeLoc);
        }
        
        return canAttach;
    }
    
    bool IsScope(string className)
    {
        array<string> scopeTypes = {"ReflexOptic", "ACOGOptic", "M4_CarryHandleOptic", "M68Optic", "KazuarOptic", "PUScopeOptic", "HuntingOptic", "PSO1Optic", "KobraOptic", "Crossbow_RedpointOptic", "StarlightOptic", "DHOptic"};
        
        foreach (string scopeType : scopeTypes)
        {
            if (className.IndexOf(scopeType) != -1)
                return true;
        }
        
        return false;
    }
    
    bool IsScopeAttachedToWeapon(string scopeClassName)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return false;
        
        EntityAI weapon = player.GetHumanInventory().GetEntityInHands();
        if (!weapon || !weapon.IsWeapon())
            return false;
        
        array<EntityAI> attachments = new array<EntityAI>();
        weapon.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, attachments);
        
        foreach (EntityAI att : attachments)
        {
            if (att.GetType() == scopeClassName)
                return true;
        }
        
        return false;
    }
    
    void ClearItemList()
    {
        Widget child = m_ItemListWrapper.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            child.Unlink();
            child = next;
        }
        
        CleanupPreviewObjects();
        m_ButtonToItemClass.Clear();
    }
    
    Widget CreateItemEntry(KOTH_ShopItem shopItem)
    {
        Widget entryWidget = GetGame().GetWorkspace().CreateWidgets("DayZ_KOTH/GUI/layouts/shop/KOTH_ShopItemEntry.layout", m_ItemListWrapper);
        if (!entryWidget)
            return null;
        
        SetupItemPreview(entryWidget, shopItem);
        SetupItemInfo(entryWidget, shopItem);
        SetupItemButtons(entryWidget, shopItem);
        
        return entryWidget;
    }
    
    void SetupItemPreview(Widget entryWidget, KOTH_ShopItem shopItem)
    {
        ItemPreviewWidget itemPreview = ItemPreviewWidget.Cast(entryWidget.FindAnyWidget("ItemImage"));
        if (!itemPreview)
            return;
        
        EntityAI previewObj = EntityAI.Cast(GetGame().CreateObjectEx(shopItem.ClassName, vector.Zero, ECE_PLACE_ON_SURFACE));
        if (previewObj)
        {
            m_PreviewObjects.Insert(previewObj);
            itemPreview.SetItem(previewObj);
            itemPreview.SetModelOrientation(vector.Zero);
        }
    }
    
    void SetupItemInfo(Widget entryWidget, KOTH_ShopItem shopItem)
    {
        TextWidget itemName = TextWidget.Cast(entryWidget.FindAnyWidget("ItemName"));
        if (itemName)
            itemName.SetText(shopItem.DisplayName);
        
        TextWidget itemAmmo = TextWidget.Cast(entryWidget.FindAnyWidget("ItemAmmo"));
        if (itemAmmo)
            itemAmmo.SetText(shopItem.AmmoType);
        
        bool isItemOwned = PlayerOwnsItem(shopItem.ClassName);
        bool playerHasLevel = PlayerMeetsLevel(shopItem.RequiredLevel);
        
        Widget ownedBadge = entryWidget.FindAnyWidget("OwnedBadge");
        if (ownedBadge)
            ownedBadge.Show(isItemOwned);
        
        TextWidget levelReq = TextWidget.Cast(entryWidget.FindAnyWidget("LevelRequirement"));
        if (levelReq)
        {
            if (!playerHasLevel)
            {
                levelReq.SetText("REQUIRE LVL " + shopItem.RequiredLevel.ToString());
                levelReq.Show(true);
            }
            else
            {
                levelReq.Show(false);
            }
        }
    }
    
    void SetupItemButtons(Widget entryWidget, KOTH_ShopItem shopItem)
    {
        ButtonWidget rentButton = ButtonWidget.Cast(entryWidget.FindAnyWidget("RentButton"));
        ButtonWidget buyButton = ButtonWidget.Cast(entryWidget.FindAnyWidget("BuyButton"));
        
        bool isItemOwned = PlayerOwnsItem(shopItem.ClassName);
        bool hasInInventory = PlayerHasItemInInventory(shopItem.ClassName);
        bool playerHasLevel = PlayerMeetsLevel(shopItem.RequiredLevel);
        bool canAffordRent = PlayerCanAfford(shopItem.RentPrice);
        bool canAffordBuy = PlayerCanAfford(shopItem.BuyPrice);
        
        bool isScopeItem = IsScope(shopItem.ClassName);
        bool isScopeAttached = false;
        
        if (isScopeItem)
        {
            isScopeAttached = IsScopeAttachedToWeapon(shopItem.ClassName);
        }
        
        if (rentButton)
        {
            SetupRentButton(rentButton, shopItem, isItemOwned, hasInInventory, playerHasLevel, canAffordRent, isScopeItem, isScopeAttached);
            m_ButtonToItemClass.Set(rentButton, shopItem.ClassName);
        }
        
        if (buyButton)
        {
            SetupBuyButton(buyButton, shopItem, isItemOwned, hasInInventory, playerHasLevel, canAffordBuy, isScopeItem, isScopeAttached);
            m_ButtonToItemClass.Set(buyButton, shopItem.ClassName);
        }
    }
    
    void SetupRentButton(ButtonWidget rentBtn, KOTH_ShopItem shopItem, bool isOwned, bool hasInInventory, bool hasLevel, bool hasRentMoney, bool isScope, bool isScopeAttached)
    {
        if (isScope && isScopeAttached)
        {
            rentBtn.SetText("ATTACHED");
            rentBtn.Enable(false);
            rentBtn.SetColor(ARGB(255, 100, 100, 100));
            return;
        }
        
        if (isOwned)
        {
            rentBtn.SetText("OWNED");
            rentBtn.Enable(false);
            rentBtn.SetColor(ARGB(255, 100, 100, 100));
        }
        else if (hasInInventory)
        {
            rentBtn.SetText("IN INVENTORY");
            rentBtn.Enable(false);
            rentBtn.SetColor(ARGB(255, 100, 100, 100));
        }
        else
        {
            rentBtn.SetText("RENT\n$" + shopItem.RentPrice.ToString());
            
            if (!hasLevel)
            {
                rentBtn.Enable(false);
                rentBtn.SetColor(ARGB(255, 100, 100, 100));
            }
            else if (!hasRentMoney)
            {
                rentBtn.Enable(true);
                rentBtn.SetColor(ARGB(255, 150, 150, 0));
            }
            else
            {
                rentBtn.Enable(true);
                rentBtn.SetColor(ARGB(255, 200, 200, 0));
            }
        }
    }
    
    void SetupBuyButton(ButtonWidget buyBtn, KOTH_ShopItem shopItem, bool isOwned, bool hasInInventory, bool hasLevel, bool hasBuyMoney, bool isScope, bool isScopeAttached)
    {
        if (isScope && isScopeAttached)
        {
            buyBtn.SetText("ATTACHED");
            buyBtn.Enable(false);
            buyBtn.SetColor(ARGB(255, 100, 100, 100));
            return;
        }
        
        if (isOwned)
        {
            if (hasInInventory)
            {
                buyBtn.SetText("IN INVENTORY");
                buyBtn.Enable(false);
                buyBtn.SetColor(ARGB(255, 100, 100, 100));
            }
            else
            {
                buyBtn.SetText("EQUIP\nFREE");
                buyBtn.Enable(true);
                buyBtn.SetColor(ARGB(255, 0, 200, 0));
            }
        }
        else
        {
            buyBtn.SetText("BUY\n$" + shopItem.BuyPrice.ToString());
            
            if (!hasLevel)
            {
                buyBtn.Enable(false);
                buyBtn.SetColor(ARGB(255, 100, 100, 100));
            }
            else if (!hasBuyMoney)
            {
                buyBtn.Enable(true);
                buyBtn.SetColor(ARGB(255, 0, 150, 0));
            }
            else
            {
                buyBtn.Enable(true);
                buyBtn.SetColor(ARGB(255, 0, 200, 0));
            }
        }
    }
    
    void OnRentButtonClick(string itemClass)
    {
        Print("[KOTH_ShopMenu] OnRentButtonClick: " + itemClass);
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
        {
            Print("[KOTH_ShopMenu] ERROR: Player not found!");
            return;
        }
        
        auto rpc = KOTH_ShopModule.GetInstance().Expansion_CreateRPC("RPC_RentItem");
        rpc.Write(itemClass);
        rpc.Expansion_Send(player, true, player.GetIdentity());
        Print("[KOTH_ShopMenu] RPC sent for rent");
    }
    
    void OnBuyButtonClick(string itemClass)
    {
        Print("[KOTH_ShopMenu] OnBuyButtonClick: " + itemClass);
        
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
        {
            Print("[KOTH_ShopMenu] ERROR: Player not found!");
            return;
        }
        
        auto rpc = KOTH_ShopModule.GetInstance().Expansion_CreateRPC("RPC_BuyItem");
        rpc.Write(itemClass);
        rpc.Expansion_Send(player, true, player.GetIdentity());
        Print("[KOTH_ShopMenu] RPC sent for buy");
    }
}