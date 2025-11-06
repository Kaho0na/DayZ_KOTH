// KOTH_ShopModule.c - OPTIMIZED
// Server/Client bridge for shop system using Expansion RPC pattern

[CF_RegisterModule(KOTH_ShopModule)]
class KOTH_ShopModule : CF_ModuleWorld
{
    static KOTH_ShopModule s_Instance;
    
    ref map<string, ref KOTH_ShopCategory> m_Categories;
    ref ScriptInvoker m_ShopMenuInvoker;
    ref ScriptInvoker m_ItemPurchasedInvoker;
    ref ScriptInvoker m_ItemEquippedInvoker;
    private ExpansionMarketModule m_MarketModule;
    private KOTH_PlayerRewardManager m_RewardManager;
    
    void KOTH_ShopModule()
    {
        s_Instance = this;
        m_Categories = new map<string, ref KOTH_ShopCategory>();
        m_ShopMenuInvoker = new ScriptInvoker();
        m_ItemPurchasedInvoker = new ScriptInvoker();
        m_ItemEquippedInvoker = new ScriptInvoker();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        
        if (GetGame().IsServer())
        {
            LoadShopData();
            Expansion_RegisterServerRPC("RPC_RequestShopOpen");
            Expansion_RegisterServerRPC("RPC_RentItem");
            Expansion_RegisterServerRPC("RPC_BuyItem");
            
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeModules, 2000, false);
        }
        
        Expansion_RegisterClientRPC("RPC_ReceiveShopData");
        Expansion_RegisterClientRPC("RPC_ShopResult");
    }
    
    void InitializeModules()
    {
        if (!Class.CastTo(m_MarketModule, CF_ModuleCoreManager.Get(ExpansionMarketModule)))
        {
            Error("[KOTH_Shop] Failed to get ExpansionMarketModule!");
        }
        else
        {
            Print("[KOTH_Shop] MarketModule initialized successfully");
        }
        
        CF_Modules<KOTH_PlayerRewardManager>.Get(m_RewardManager);
        if (m_RewardManager)
            Print("[KOTH_Shop] RewardManager initialized successfully");
        else
            Error("[KOTH_Shop] ERROR: Failed to initialize RewardManager!");
    }
    
    static KOTH_ShopModule GetInstance()
    {
        return s_Instance;
    }
    
    ScriptInvoker GetMenuSI()
    {
        return m_ShopMenuInvoker;
    }
    
    ScriptInvoker GetItemPurchasedSI()
    {
        return m_ItemPurchasedInvoker;
    }
    
    ScriptInvoker GetItemEquippedSI()
    {
        return m_ItemEquippedInvoker;
    }
    
    void LoadShopData()
    {
        LoadCategory("Rifles");
        LoadCategory("Pistols");
        LoadCategory("Scopes");
        LoadCategory("Items");
        
        Print("[KOTH_Shop] Shop data loaded. Total categories: " + m_Categories.Count());
    }
    
    void LoadCategory(string categoryName)
    {
        string path = "$profile:ExpansionMod/KOTH_Shop_Categories/" + categoryName + ".json";
        
        if (!FileExist(path))
        {
            Print("[KOTH_Shop] WARNING: Category file not found: " + path);
            return;
        }
        
        KOTH_ShopCategory shopCategory = new KOTH_ShopCategory();
        JsonFileLoader<KOTH_ShopCategory>.JsonLoadFile(path, shopCategory);
        
        if (shopCategory && shopCategory.Items.Count() > 0)
        {
            m_Categories.Set(categoryName, shopCategory);
            Print("[KOTH_Shop] Loaded " + categoryName + ": " + shopCategory.Items.Count() + " items");
        }
    }
    
    KOTH_ShopItem GetItem(string className)
    {
        foreach (string catName, KOTH_ShopCategory cat : m_Categories)
        {
            foreach (KOTH_ShopItem item : cat.Items)
            {
                if (item.ClassName == className)
                    return item;
            }
        }
        return null;
    }
    
    KOTH_ShopCategory GetCategory(string categoryName)
    {
        return m_Categories.Get(categoryName);
    }
    
    protected void RPC_RequestShopOpen(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_Shop] SERVER: RPC_RequestShopOpen received");
        
        if (!GetGame().IsServer())
            return;
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
        {
            Print("[KOTH_Shop] ERROR: Player cast failed");
            return;
        }
        
        Print("[KOTH_Shop] SERVER: Calling RequestShopOpen for " + sender.GetName());
        RequestShopOpen(player, sender);
    }

    void RequestShopOpen(PlayerBase player, PlayerIdentity ident)
    {
        Print("[KOTH_Shop] SERVER: RequestShopOpen called for " + ident.GetName());
        
        string uid = ident.GetId();
        KOTH_Players playerData = KOTH_Players.Load(uid);
        
        if (!playerData)
        {
            Print("[KOTH_Shop] ERROR: Player data not found for " + ident.GetName());
            return;
        }
        
        string playerFaction = player.GetKOTHTeam();
        
        if (playerFaction != "East" && playerFaction != "West")
        {
            Print("[KOTH_Shop] ERROR: Player has no valid team! Team: " + playerFaction);
            ExpansionNotification("Shop Error", "You must select a team first").Error(ident);
            return;
        }
        
        Print("[KOTH_Shop] SERVER: Building shop data RPC... Player faction: " + playerFaction);
        
        auto rpc = Expansion_CreateRPC("RPC_ReceiveShopData");
        rpc.Write(playerData.CurrentLevel);
        rpc.Write(playerData.TotalMoneyinBank);
        rpc.Write(playerFaction);
        
        rpc.Write(playerData.OwnedWeapons.Count());
        foreach (string ownedItem : playerData.OwnedWeapons)
        {
            rpc.Write(ownedItem);
        }
        
        rpc.Write(m_Categories.Count());
        foreach (string catName, KOTH_ShopCategory cat : m_Categories)
        {
            rpc.Write(catName);
            rpc.Write(cat.Items.Count());
            
            foreach (KOTH_ShopItem item : cat.Items)
            {
                rpc.Write(item.ClassName);
                rpc.Write(item.DisplayName);
                rpc.Write(item.AmmoType);
                rpc.Write(item.RentPrice);
                rpc.Write(item.BuyPrice);
                rpc.Write(item.RequiredLevel);
                rpc.Write(item.MagazineClass);
                rpc.Write(item.Faction);
                
                rpc.Write(item.DefaultAttachments.Count());
                foreach (string att : item.DefaultAttachments)
                {
                    rpc.Write(att);
                }
            }
        }
        
        Print("[KOTH_Shop] SERVER: Sending RPC_ReceiveShopData to client...");
        rpc.Expansion_Send(player, true, ident);
        Print("[KOTH_Shop] SERVER: RPC sent successfully");
    }
    
    protected void RPC_RentItem(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer())
            return;
        
        string itemClass;
        if (!ctx.Read(itemClass))
            return;
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
            return;
        
        RentItem(player, itemClass);
    }
    
    protected void RPC_BuyItem(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer())
            return;
        
        string itemClass;
        if (!ctx.Read(itemClass))
            return;
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
            return;
        
        BuyItem(player, itemClass);
    }
    

    
    protected void RPC_ReceiveShopData(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_Shop] CLIENT: RPC_ReceiveShopData received");
        
        if (GetGame().IsServer())
            return;
        
        int playerLevel;
        if (!ctx.Read(playerLevel))
            return;
        
        int playerMoney;
        if (!ctx.Read(playerMoney))
            return;
        
        string playerFaction;
        if (!ctx.Read(playerFaction))
            return;
        
        Print("[KOTH_Shop] CLIENT: Player faction is " + playerFaction);
        
        array<string> ownedItems = new array<string>();
        int ownedCount;
        if (!ctx.Read(ownedCount))
            return;
        
        for (int i = 0; i < ownedCount; i++)
        {
            string ownedWeapon;
            if (!ctx.Read(ownedWeapon))
                return;
            ownedItems.Insert(ownedWeapon);
        }
        
        array<ref KOTH_ShopCategory> categories = new array<ref KOTH_ShopCategory>();
        int catCount;
        if (!ctx.Read(catCount))
            return;
        
        for (int j = 0; j < catCount; j++)
        {
            string categoryName;
            if (!ctx.Read(categoryName))
                return;
            
            int itemCount;
            if (!ctx.Read(itemCount))
                return;
            
            KOTH_ShopCategory category = new KOTH_ShopCategory();
            category.Category = categoryName;
            category.Items = new array<ref KOTH_ShopItem>();
            
            for (int k = 0; k < itemCount; k++)
            {
                KOTH_ShopItem item = new KOTH_ShopItem();
                
                if (!ctx.Read(item.ClassName))
                    return;
                if (!ctx.Read(item.DisplayName))
                    return;
                if (!ctx.Read(item.AmmoType))
                    return;
                if (!ctx.Read(item.RentPrice))
                    return;
                if (!ctx.Read(item.BuyPrice))
                    return;
                if (!ctx.Read(item.RequiredLevel))
                    return;
                if (!ctx.Read(item.MagazineClass))
                    return;
                if (!ctx.Read(item.Faction))
                    return;
                
                int attCount;
                if (!ctx.Read(attCount))
                    return;
                
                item.DefaultAttachments = new array<string>();
                for (int l = 0; l < attCount; l++)
                {
                    string att;
                    if (!ctx.Read(att))
                        return;
                    item.DefaultAttachments.Insert(att);
                }
                
                if (item.Faction == "Both" || item.Faction == playerFaction)
                {
                    category.Items.Insert(item);
                    Print("[KOTH_Shop] CLIENT: Added item " + item.DisplayName + " (Faction: " + item.Faction + ")");
                }
                else
                {
                    Print("[KOTH_Shop] CLIENT: Filtered out item " + item.DisplayName + " (Faction: " + item.Faction + ", Player: " + playerFaction + ")");
                }
            }
            
            categories.Insert(category);
        }
        
        Print("[KOTH_Shop] CLIENT: Opening menu...");
        if (!GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("KOTH_ShopMenu"))
        {
            Print("[KOTH_Shop] ERROR: Failed to create menu!");
            return;
        }
        
        Print("[KOTH_Shop] CLIENT: Invoking menu with " + categories.Count() + " categories");
        m_ShopMenuInvoker.Invoke(categories, ownedItems, playerLevel, playerMoney);
        Print("[KOTH_Shop] CLIENT: Menu invoker called");
    }
    
    bool IsItemCategory(string itemClass)
    {
        KOTH_ShopCategory itemCat = GetCategory("Items");
        if (!itemCat)
            return false;
        
        foreach (KOTH_ShopItem item : itemCat.Items)
        {
            if (item.ClassName == itemClass)
                return true;
        }
        
        return false;
    }
    
    bool HasInventorySpaceForItem(PlayerBase player, string itemClass)
    {
        Print("[KOTH_Shop] Checking inventory space for: " + itemClass);
        
        EntityAI testItem = player.GetInventory().CreateInInventory(itemClass);
        if (testItem)
        {
            Print("[KOTH_Shop] Test item created successfully - has space!");
            testItem.DeleteSafe();
            return true;
        }
        
        Print("[KOTH_Shop] Test item creation failed - no space!");
        return false;
    }
    
    void RentItem(PlayerBase player, string itemClass)
    {
        if (!player)
            return;
        
        PlayerIdentity ident = player.GetIdentity();
        string uid = ident.GetId();
        
        KOTH_Players playerData = KOTH_Players.Load(uid);
        if (!playerData)
        {
            ExpansionNotification("Shop Error", "Player data not found").Error(ident);
            return;
        }
        
        ExpansionMarketATM_Data atmData;
        if (m_MarketModule)
        {
            atmData = m_MarketModule.GetPlayerATMData(uid);
        }
        
        KOTH_ShopItem item = GetItem(itemClass);
        if (!item)
        {
            ExpansionNotification("Shop Error", "Item not found in shop").Error(ident);
            return;
        }
        
        if (playerData.CurrentLevel < item.RequiredLevel)
        {
            ExpansionNotification("Shop", "You need level " + item.RequiredLevel.ToString() + " to rent this item").Error(ident);
            return;
        }
        
        if (playerData.TotalMoneyinBank < item.RentPrice)
        {
            ExpansionNotification("Shop", "Not enough money. Need $" + item.RentPrice.ToString()).Error(ident);
            return;
        }
        
        if (m_RewardManager)
        {
            m_RewardManager.SyncPlayerStatsToClient(ident, playerData);
            m_RewardManager.RemovePlayerMoney(player, item.RentPrice, "Item Rented");
        }
        
        if (KOTH_ShopScopes.IsScope(itemClass))
        {
            KOTH_ShopScopes.EquipScope(player, item);
        }
        else if (IsItemCategory(itemClass))
        {
            if (!HasInventorySpaceForItem(player, itemClass))
            {
                ExpansionNotification("Shop", "No space in inventory").Error(ident);
                
                if (m_RewardManager)
                {
                    m_RewardManager.SyncPlayerStatsToClient(ident, playerData);
                    m_RewardManager.AddPlayerMoney(player, item.RentPrice, "Refund Rented");
                }
                return;
            }
            KOTH_ShopItems.EquipItem(player, item);
        }
        else if (KOTH_ShopPistols.IsPistol(itemClass))
        {
            KOTH_ShopPistols.ClearPistolMagazines(player);
            KOTH_ShopPistols.EquipPistol(player, item);
            KOTH_ShopRifles.GiveStandardLoadout(player, item);
        }
        else
        {
            KOTH_ShopRifles.ClearRifleMagazines(player);
            KOTH_ShopRifles.EquipPrimaryWeapon(player, item);
            KOTH_ShopRifles.GiveStandardLoadout(player, item);
        }
        
        SendShopResult(player, true, "Rented " + item.DisplayName + " for $" + item.RentPrice.ToString(), itemClass, false);
    }
    
    void BuyItem(PlayerBase player, string itemClass)
    {
        if (!player)
            return;
        
        PlayerIdentity ident = player.GetIdentity();
        string uid = ident.GetId();
        
        KOTH_Players playerData = KOTH_Players.Load(uid);
        if (!playerData)
        {
            ExpansionNotification("Shop Error", "Player data not found").Error(ident);
            return;
        }
        
        ExpansionMarketATM_Data atmData;
        if (m_MarketModule)
        {
            atmData = m_MarketModule.GetPlayerATMData(uid);
        }
        
        KOTH_ShopItem item = GetItem(itemClass);
        if (!item)
        {
            ExpansionNotification("Shop Error", "Item not found in shop").Error(ident);
            return;
        }
        
        if (playerData.CurrentLevel < item.RequiredLevel)
        {
            ExpansionNotification("Shop", "You need level " + item.RequiredLevel.ToString() + " to buy this item").Error(ident);
            return;
        }
        
        bool alreadyOwned = playerData.OwnedWeapons.Find(itemClass) != -1;
        
        if (!alreadyOwned)
        {
            if (playerData.TotalMoneyinBank < item.BuyPrice)
            {
                ExpansionNotification("Shop", "Not enough money. Need $" + item.BuyPrice.ToString()).Error(ident);
                return;
            }
            
            if (IsItemCategory(itemClass))
            {
                if (!HasInventorySpaceForItem(player, itemClass))
                {
                    ExpansionNotification("Shop", "No space in inventory").Error(ident);
                    
                    if (m_RewardManager)
                    {
                        m_RewardManager.SyncPlayerStatsToClient(ident, playerData);
                        m_RewardManager.AddPlayerMoney(player, item.RentPrice, "Refund Bought");
                    }
                    return;
                }
            }
            
            playerData.OwnedWeapons.Insert(itemClass);
            playerData.Save();
            
            if (m_RewardManager)
            {
                m_RewardManager.SyncPlayerStatsToClient(ident, playerData);
                m_RewardManager.RemovePlayerMoney(player, item.BuyPrice, "Item Bought");
            }
            
            if (IsItemCategory(itemClass))
            {
                KOTH_ShopItems.EquipItem(player, item);
            }
            else if (KOTH_ShopScopes.IsScope(itemClass))
            {
                KOTH_ShopScopes.EquipScope(player, item);
            }
            else if (KOTH_ShopPistols.IsPistol(itemClass))
            {
                KOTH_ShopPistols.ClearPistolMagazines(player);
                KOTH_ShopPistols.EquipPistol(player, item);
                KOTH_ShopRifles.GiveStandardLoadout(player, item);
            }
            else
            {
                KOTH_ShopRifles.ClearRifleMagazines(player);
                KOTH_ShopRifles.EquipPrimaryWeapon(player, item);
                KOTH_ShopRifles.GiveStandardLoadout(player, item);
            }
            
            SendShopResult(player, true, "Purchased " + item.DisplayName + " for $" + item.BuyPrice.ToString(), itemClass, true);
        }
        else
        {
            if (IsItemCategory(itemClass))
            {
                if (!HasInventorySpaceForItem(player, itemClass))
                {
                    ExpansionNotification("Shop", "No space in inventory").Error(ident);
                    return;
                }
            }
            
            if (IsItemCategory(itemClass))
            {
                KOTH_ShopItems.EquipItem(player, item);
            }
            else if (KOTH_ShopScopes.IsScope(itemClass))
            {
                KOTH_ShopScopes.EquipScope(player, item);
            }
            else if (KOTH_ShopPistols.IsPistol(itemClass))
            {
                KOTH_ShopPistols.ClearPistolMagazines(player);
                KOTH_ShopPistols.EquipPistol(player, item);
                KOTH_ShopRifles.GiveStandardLoadout(player, item);
            }
            else
            {
                KOTH_ShopRifles.ClearRifleMagazines(player);
                KOTH_ShopRifles.EquipPrimaryWeapon(player, item);
                KOTH_ShopRifles.GiveStandardLoadout(player, item);
            }
            
            SendShopResult(player, true, "Equipped " + item.DisplayName, itemClass, false);
        }
    }
    
    protected void RPC_ShopResult(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        bool success;
        if (!ctx.Read(success))
            return;
        
        string message;
        if (!ctx.Read(message))
            return;
        
        string itemClass;
        if (!ctx.Read(itemClass))
            return;
        
        bool wasPurchase;
        if (!ctx.Read(wasPurchase))
            return;
        
        int newBalance;
        if (!ctx.Read(newBalance))
            return;
        
        if (success)
        {
            ExpansionNotification("Shop", message).Success();
            
            if (wasPurchase)
            {
                m_ItemPurchasedInvoker.Invoke(itemClass, newBalance);
            }
            else if (itemClass != "")
            {
                m_ItemEquippedInvoker.Invoke(itemClass, newBalance);
            }
        }
        else
        {
            ExpansionNotification("Shop", message).Error();
        }
    }
    
    void SendShopResult(PlayerBase player, bool success, string message, string itemClass = "", bool wasPurchase = false)
    {
        int newBalance = 0;
        if (success)
        {
            string uid = player.GetIdentity().GetId();
            KOTH_Players playerData = KOTH_Players.Load(uid);
            if (playerData)
                newBalance = playerData.TotalMoneyinBank;
        }
        
        auto rpc = Expansion_CreateRPC("RPC_ShopResult");
        rpc.Write(success);
        rpc.Write(message);
        rpc.Write(itemClass);
        rpc.Write(wasPurchase);
        rpc.Write(newBalance);
        rpc.Expansion_Send(player, true, player.GetIdentity());
    }
}