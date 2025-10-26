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
        
        Print("[KOTH_Shop] SERVER: Building shop data RPC...");
        
        auto rpc = Expansion_CreateRPC("RPC_ReceiveShopData");
        rpc.Write(playerData.CurrentLevel);
        rpc.Write(playerData.TotalMoneyinBank);
        
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
                
                category.Items.Insert(item);
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
            GetGame().ObjectDelete(testItem);
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
        
        playerData.TotalMoneyinBank = playerData.TotalMoneyinBank - item.RentPrice;
        playerData.Save();
        
        if (IsScope(itemClass))
        {
            EquipScope(player, item);
        }
        else if (IsItemCategory(itemClass))
        {
            if (!HasInventorySpaceForItem(player, itemClass))
            {
                ExpansionNotification("Shop", "No space in inventory").Error(ident);
                playerData.TotalMoneyinBank = playerData.TotalMoneyinBank + item.RentPrice;
                playerData.Save();
                return;
            }
            EquipItem(player, item);
        }
        else if (IsPistol(itemClass))
        {
            ClearPistolMagazines(player);
            EquipPistol(player, item);
            GiveStandardLoadout(player, item);
        }
        else
        {
            ClearRifleMagazines(player);
            EquipPrimaryWeapon(player, item);
            GiveStandardLoadout(player, item);
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
                    return;
                }
            }
            
            playerData.TotalMoneyinBank = playerData.TotalMoneyinBank - item.BuyPrice;
            playerData.OwnedWeapons.Insert(itemClass);
            playerData.Save();
            
            if (IsItemCategory(itemClass))
            {
                EquipItem(player, item);
            }
            else if (IsScope(itemClass))
            {
                EquipScope(player, item);
            }
            else if (IsPistol(itemClass))
            {
                ClearPistolMagazines(player);
                EquipPistol(player, item);
                GiveStandardLoadout(player, item);
            }
            else
            {
                ClearRifleMagazines(player);
                EquipPrimaryWeapon(player, item);
                GiveStandardLoadout(player, item);
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
                EquipItem(player, item);
            }
            else if (IsScope(itemClass))
            {
                EquipScope(player, item);
            }
            else if (IsPistol(itemClass))
            {
                ClearPistolMagazines(player);
                EquipPistol(player, item);
                GiveStandardLoadout(player, item);
            }
            else
            {
                ClearRifleMagazines(player);
                EquipPrimaryWeapon(player, item);
                GiveStandardLoadout(player, item);
            }
            
            SendShopResult(player, true, "Equipped " + item.DisplayName, itemClass, false);
        }
    }
    
    void EquipItem(PlayerBase player, KOTH_ShopItem item)
    {
        EntityAI createdItem = player.GetInventory().CreateInInventory(item.ClassName);
        if (!createdItem)
        {
            Print("[KOTH_Shop] ERROR: Failed to create item in inventory: " + item.ClassName);
            return;
        }
        
        Print("[KOTH_Shop] Created item in inventory: " + item.ClassName);
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
    
    void EquipScope(PlayerBase player, KOTH_ShopItem item)
    {
        EntityAI weapon = player.GetHumanInventory().GetEntityInHands();
        
        if (!weapon || !weapon.IsWeapon())
        {
            ExpansionNotification("Shop", "You need a weapon in your hands to equip a scope").Error(player.GetIdentity());
            return;
        }
        
        array<EntityAI> attachments = new array<EntityAI>();
        weapon.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, attachments);
        
        foreach (EntityAI att : attachments)
        {
            if (IsScope(att.GetType()))
            {
                GetGame().ObjectDelete(att);
                Print("[KOTH_Shop] Removed old scope: " + att.GetType());
                break;
            }
        }
        
        EntityAI newScope = ExpansionItemSpawnHelper.SpawnAttachment(item.ClassName, weapon);
        
        if (!newScope)
        {
            ExpansionNotification("Shop", "Cannot attach this scope to your current weapon").Error(player.GetIdentity());
            Print("[KOTH_Shop] Failed to attach scope: " + item.ClassName);
        }
        else
        {
            Print("[KOTH_Shop] Successfully attached scope: " + item.ClassName);
            
            int batterySlot = InventorySlots.GetSlotIdFromString("BatteryD");
            if (batterySlot != -1)
            {
                EntityAI existingBattery = newScope.GetInventory().FindAttachment(batterySlot);
                if (!existingBattery)
                {
                    EntityAI battery = newScope.GetInventory().CreateAttachment("Battery9V");
                    if (battery)
                    {
                        Print("[KOTH_Shop] Added 9V battery to scope");
                    }
                }
            }
        }
    }
    
    void ClearPistolMagazines(PlayerBase player)
    {
        array<string> pistolMagTypes = {"Mag_Glock_15Rnd", "Mag_FNX45_15Rnd", "Mag_CZ75_15Rnd", "Mag_1911_7Rnd", "Mag_Deagle_9Rnd", "Mag_MKII_10Rnd", "Mag_P1_8Rnd", "Ammo_9x19", "Ammo_45ACP", "Ammo_357"};
        
        array<EntityAI> itemsToDelete = new array<EntityAI>();
        array<EntityAI> allItems = new array<EntityAI>();
        
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, allItems);
        
        foreach (EntityAI item : allItems)
        {
            if (item.IsMagazine() || item.IsAmmoPile())
            {
                string itemType = item.GetType();
                foreach (string pistolMagType : pistolMagTypes)
                {
                    if (itemType.IndexOf(pistolMagType) != -1)
                    {
                        itemsToDelete.Insert(item);
                        break;
                    }
                }
            }
        }
        
        foreach (EntityAI itemToDelete : itemsToDelete)
        {
            GetGame().ObjectDelete(itemToDelete);
        }
        
        Print("[KOTH_Shop] Cleared " + itemsToDelete.Count() + " pistol magazines and ammo");
    }
    
    void ClearRifleMagazines(PlayerBase player)
    {
        array<string> pistolMagTypes = {"Mag_Glock_15Rnd", "Mag_FNX45_15Rnd", "Mag_CZ75_15Rnd", "Mag_1911_7Rnd", "Mag_Deagle_9Rnd", "Mag_MKII_10Rnd", "Mag_P1_8Rnd", "Ammo_9x19", "Ammo_45ACP", "Ammo_357"};
        
        array<EntityAI> itemsToDelete = new array<EntityAI>();
        array<EntityAI> allItems = new array<EntityAI>();
        
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, allItems);
        
        foreach (EntityAI item : allItems)
        {
            if (item.IsMagazine() || item.IsAmmoPile())
            {
                string itemType = item.GetType();
                bool isPistolMag = false;
                
                foreach (string pistolMagType : pistolMagTypes)
                {
                    if (itemType.IndexOf(pistolMagType) != -1)
                    {
                        isPistolMag = true;
                        break;
                    }
                }
                
                if (!isPistolMag)
                {
                    itemsToDelete.Insert(item);
                }
            }
        }
        
        foreach (EntityAI itemToDelete : itemsToDelete)
        {
            GetGame().ObjectDelete(itemToDelete);
        }
        
        Print("[KOTH_Shop] Cleared " + itemsToDelete.Count() + " rifle magazines and ammo (preserved pistol mags)");
    }
    
    bool IsPistol(string className)
    {
        array<string> pistolTypes = {"Glock19", "FNX45", "CZ75", "Deagle", "MKII", "P1", "Engraved1911", "Colt1911", "Magnum", "Pistol_Base"};
        
        foreach (string pistolType : pistolTypes)
        {
            if (className.IndexOf(pistolType) != -1)
                return true;
        }
        
        return false;
    }
    
    void EquipPistol(PlayerBase player, KOTH_ShopItem item)
    {
        EntityAI holster = FindHolster(player);
        
        if (holster)
        {
            EntityAI oldPistol = holster.GetInventory().FindAttachment(InventorySlots.GetSlotIdFromString("Pistol"));
            if (oldPistol)
            {
                GetGame().ObjectDelete(oldPistol);
                Print("[KOTH_Shop] Deleted old pistol from holster");
            }
            
            EntityAI newPistol = holster.GetInventory().CreateAttachment(item.ClassName);
            if (newPistol)
            {
                if (item.MagazineClass != "")
                {
                    EntityAI mag = newPistol.GetInventory().CreateAttachment(item.MagazineClass);
                    if (mag)
                    {
                        Magazine magCast = Magazine.Cast(mag);
                        if (magCast)
                            magCast.ServerSetAmmoMax();
                    }
                }
                
                foreach (string attachment : item.DefaultAttachments)
                {
                    newPistol.GetInventory().CreateAttachment(attachment);
                }
                
                Print("[KOTH_Shop] Equipped pistol in holster: " + item.ClassName);
            }
        }
        else
        {
            Print("[KOTH_Shop] WARNING: No holster found, placing pistol in hands");
            EquipPrimaryWeapon(player, item);
        }
    }
    
    void EquipPrimaryWeapon(PlayerBase player, KOTH_ShopItem item)
    {
        EntityAI currentWeapon = player.GetHumanInventory().GetEntityInHands();
        if (currentWeapon)
        {
            GetGame().ObjectDelete(currentWeapon);
        }
        
        EntityAI weapon = player.GetHumanInventory().CreateInHands(item.ClassName);
        if (!weapon)
        {
            Print("[KOTH_Shop] ERROR: Failed to create weapon: " + item.ClassName);
            return;
        }
        
        if (item.MagazineClass != "")
        {
            EntityAI mag = weapon.GetInventory().CreateAttachment(item.MagazineClass);
            if (mag)
            {
                Magazine magCast = Magazine.Cast(mag);
                if (magCast)
                    magCast.ServerSetAmmoMax();
            }
        }
        
        foreach (string attachment : item.DefaultAttachments)
        {
            weapon.GetInventory().CreateAttachment(attachment);
        }
    }
    
    EntityAI FindHolster(PlayerBase player)
    {
        array<string> holsterTypes = {"PlateCarrierHolster_Green", "PlateCarrierHolster_Camo", "PlateCarrierHolster_Black", "ChestHolster"};
        
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        
        foreach (EntityAI item : items)
        {
            foreach (string holsterType : holsterTypes)
            {
                if (item.GetType() == holsterType)
                {
                    return item;
                }
            }
        }
        
        return null;
    }
    
    void ClearAllAmmoAndMagazines(PlayerBase player)
    {
        array<EntityAI> itemsToDelete = new array<EntityAI>();
        array<EntityAI> allItems = new array<EntityAI>();
        
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, allItems);
        
        foreach (EntityAI item : allItems)
        {
            if (item.IsMagazine() || item.IsAmmoPile())
            {
                itemsToDelete.Insert(item);
            }
        }
        
        foreach (EntityAI itemToDelete : itemsToDelete)
        {
            GetGame().ObjectDelete(itemToDelete);
        }
        
        Print("[KOTH_Shop] Cleared " + itemsToDelete.Count() + " magazines and ammo piles");
    }
    
    void GiveStandardLoadout(PlayerBase player, KOTH_ShopItem item)
    {
        if (item.MagazineClass == "")
        {
            Print("[KOTH_Shop] No magazine class defined for " + item.ClassName);
            return;
        }
        
        int magsGiven = 0;
        int magsToGive = 3;
        
        for (int i = 0; i < magsToGive; i++)
        {
            EntityAI mag = player.GetInventory().CreateInInventory(item.MagazineClass);
            if (mag)
            {
                Magazine magCast = Magazine.Cast(mag);
                if (magCast)
                {
                    magCast.ServerSetAmmoMax();
                    magsGiven++;
                }
            }
            else
            {
                Print("[KOTH_Shop] WARNING: Failed to create magazine in inventory, trying ground drop");
                
                vector dropPos = player.GetPosition() + (player.GetDirection() * 0.5);
                EntityAI groundMag = GetGame().CreateObjectEx(item.MagazineClass, dropPos, ECE_PLACE_ON_SURFACE);
                if (groundMag)
                {
                    Magazine groundMagCast = Magazine.Cast(groundMag);
                    if (groundMagCast)
                    {
                        groundMagCast.ServerSetAmmoMax();
                        magsGiven++;
                        ExpansionNotification("Shop", "Inventory full - magazine dropped").Info(player.GetIdentity());
                    }
                }
            }
        }
        
        Print("[KOTH_Shop] Gave " + magsGiven + " magazines for " + item.ClassName);
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
                m_ItemEquippedInvoker.Invoke(itemClass);
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