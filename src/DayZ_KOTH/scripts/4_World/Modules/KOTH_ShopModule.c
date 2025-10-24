// KOTH_ShopModule.c
// Server/Client bridge for shop system using Expansion RPC pattern

[CF_RegisterModule(KOTH_ShopModule)]
class KOTH_ShopModule : CF_ModuleWorld
{
    static KOTH_ShopModule s_Instance;
    
    ref map<string, ref KOTH_ShopCategory> m_Categories;
    ref ScriptInvoker m_ShopMenuInvoker;
    
    void KOTH_ShopModule()
    {
        s_Instance = this;
        m_Categories = new map<string, ref KOTH_ShopCategory>();
        m_ShopMenuInvoker = new ScriptInvoker();
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
        }
        
        Expansion_RegisterClientRPC("RPC_ReceiveShopData");
        Expansion_RegisterClientRPC("RPC_ShopResult");
    }
    
    static KOTH_ShopModule GetInstance()
    {
        return s_Instance;
    }
    
    ScriptInvoker GetMenuSI()
    {
        return m_ShopMenuInvoker;
    }
    
    void LoadShopData()
    {
        Print("[KOTH_Shop] Loading shop data...");
        
        LoadCategory("Rifles");
        LoadCategory("Pistols");
        LoadCategory("Scopes");
        LoadCategory("Launchers");
        
        Print("[KOTH_Shop] Shop data loaded. Total categories: " + m_Categories.Count());
        
        foreach (string catName, KOTH_ShopCategory cat : m_Categories)
        {
            Print("[KOTH_Shop]   - " + catName + ": " + cat.Items.Count() + " items");
        }
    }
    
    void LoadCategory(string categoryName)
    {
        string path = "$profile:ExpansionMod/KOTH_Shop_Categories/" + categoryName + ".json";
        
        Print("[KOTH_Shop] Attempting to load: " + path);
        
        if (!FileExist(path))
        {
            Print("[KOTH_Shop] WARNING: Category file not found: " + path);
            return;
        }
        
        Print("[KOTH_Shop] File exists, loading JSON...");
        
        KOTH_ShopCategory shopCategory = new KOTH_ShopCategory();
        JsonFileLoader<KOTH_ShopCategory>.JsonLoadFile(path, shopCategory);
        
        if (shopCategory && shopCategory.Items.Count() > 0)
        {
            m_Categories.Set(categoryName, shopCategory);
            Print("[KOTH_Shop] Loaded category: " + categoryName + " (" + shopCategory.Items.Count() + " items)");
        }
        else
        {
            Print("[KOTH_Shop] ERROR: Failed to load category or empty: " + categoryName);
        }
    }
    
    KOTH_ShopItem GetItem(string className)
    {
        foreach (string catName, KOTH_ShopCategory cat : m_Categories)
        {
            KOTH_ShopItem item = cat.GetItem(className);
            if (item)
                return item;
        }
        return null;
    }
    
    KOTH_ShopCategory GetCategory(string categoryName)
    {
        return m_Categories.Get(categoryName);
    }
    
    protected void RPC_RequestShopOpen(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_ShopModule] RPC_RequestShopOpen received on server");
        
        if (!GetGame().IsServer())
        {
            Print("[KOTH_ShopModule] ERROR: RPC_RequestShopOpen called on client!");
            return;
        }
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
        {
            Print("[KOTH_ShopModule] ERROR: Target is not a PlayerBase!");
            return;
        }
        
        Print("[KOTH_ShopModule] Calling RequestShopOpen for player: " + sender.GetName());
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
        Print("[KOTH_ShopModule] RequestShopOpen called for: " + ident.GetName());
        
        string uid = ident.GetId();
        Print("[KOTH_ShopModule] Loading player data for UID: " + uid);
        
        KOTH_Players playerData = KOTH_Players.Load(uid);
        
        if (!playerData)
        {
            Print("[KOTH_Shop] ERROR: Player data not found for " + ident.GetName());
            return;
        }
        
        Print("[KOTH_ShopModule] Player data loaded. Level: " + playerData.CurrentLevel + " Money: " + playerData.TotalMoneyinBank);
        Print("[KOTH_ShopModule] Creating RPC to send shop data...");
        
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
        
        Print("[KOTH_ShopModule] Sending shop data RPC to client...");
        rpc.Expansion_Send(player, true, ident);
        Print("[KOTH_ShopModule] Shop data RPC sent successfully!");
    }
    
    protected void RPC_ReceiveShopData(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_ShopModule] RPC_ReceiveShopData received on client");
        
        if (GetGame().IsServer())
        {
            Print("[KOTH_ShopModule] ERROR: RPC_ReceiveShopData called on server!");
            return;
        }
        
        Print("[KOTH_ShopModule] Reading player level...");
        int playerLevel;
        if (!ctx.Read(playerLevel))
        {
            Print("[KOTH_ShopModule] ERROR: Failed to read playerLevel!");
            return;
        }
        
        Print("[KOTH_ShopModule] Reading player money...");
        int playerMoney;
        if (!ctx.Read(playerMoney))
        {
            Print("[KOTH_ShopModule] ERROR: Failed to read playerMoney!");
            return;
        }
        
        Print("[KOTH_ShopModule] Player Level: " + playerLevel + " Money: " + playerMoney);
        
        Print("[KOTH_ShopModule] Reading owned items...");
        array<string> ownedItems = new array<string>();
        int ownedCount;
        if (!ctx.Read(ownedCount))
        {
            Print("[KOTH_ShopModule] ERROR: Failed to read ownedCount!");
            return;
        }
        
        Print("[KOTH_ShopModule] Owned items count: " + ownedCount);
        for (int i = 0; i < ownedCount; i++)
        {
            string ownedWeapon;
            if (!ctx.Read(ownedWeapon))
            {
                Print("[KOTH_ShopModule] ERROR: Failed to read ownedWeapon at index " + i);
                return;
            }
            ownedItems.Insert(ownedWeapon);
        }
        
        Print("[KOTH_ShopModule] Reading categories...");
        array<ref KOTH_ShopCategory> categories = new array<ref KOTH_ShopCategory>();
        int catCount;
        if (!ctx.Read(catCount))
        {
            Print("[KOTH_ShopModule] ERROR: Failed to read catCount!");
            return;
        }
        
        Print("[KOTH_ShopModule] Categories count: " + catCount);
        
        for (int j = 0; j < catCount; j++)
        {
            string catName;
            if (!ctx.Read(catName))
                return;
            
            KOTH_ShopCategory cat = new KOTH_ShopCategory();
            cat.Category = catName;
            
            int itemCount;
            if (!ctx.Read(itemCount))
                return;
            
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
                
                for (int l = 0; l < attCount; l++)
                {
                    string att;
                    if (!ctx.Read(att))
                        return;
                    item.DefaultAttachments.Insert(att);
                }
                
                cat.Items.Insert(item);
            }
            
            categories.Insert(cat);
        }
        
        Print("[KOTH_ShopModule] All data parsed successfully. Invoking menu with " + categories.Count() + " categories");
        
        Print("[KOTH_ShopModule] Creating/opening shop menu...");
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("KOTH_ShopMenu");
        
        Print("[KOTH_ShopModule] Invoking shop data to menu...");
        m_ShopMenuInvoker.Invoke(categories, ownedItems, playerLevel, playerMoney);
        Print("[KOTH_ShopModule] Menu invoked successfully!");
    }
    
    void RentItem(PlayerBase player, string className)
    {
        KOTH_ShopItem item = GetItem(className);
        if (!item)
        {
            SendShopResult(player, false, "Item not found");
            return;
        }
        
        string uid = player.GetIdentity().GetId();
        KOTH_Players playerData = KOTH_Players.Load(uid);
        
        if (!playerData)
        {
            SendShopResult(player, false, "Player data not found");
            return;
        }
        
        if (playerData.CurrentLevel < item.RequiredLevel)
        {
            SendShopResult(player, false, "Requires Level " + item.RequiredLevel);
            return;
        }
        
        if (playerData.TotalMoneyinBank < item.RentPrice)
        {
            SendShopResult(player, false, "Insufficient funds");
            return;
        }
        
        playerData.TotalMoneyinBank -= item.RentPrice;
        playerData.Save();
        
        EquipWeapon(player, item);
        
        SendShopResult(player, true, "Rented " + item.DisplayName);
    }
    
    void BuyItem(PlayerBase player, string className)
    {
        KOTH_ShopItem item = GetItem(className);
        if (!item)
        {
            SendShopResult(player, false, "Item not found");
            return;
        }
        
        string uid = player.GetIdentity().GetId();
        KOTH_Players playerData = KOTH_Players.Load(uid);
        
        if (!playerData)
        {
            SendShopResult(player, false, "Player data not found");
            return;
        }
        
        if (playerData.HasOwnedItem(className))
        {
            EquipWeapon(player, item);
            SendShopResult(player, true, "Equipped " + item.DisplayName);
            return;
        }
        
        if (playerData.CurrentLevel < item.RequiredLevel)
        {
            SendShopResult(player, false, "Requires Level " + item.RequiredLevel);
            return;
        }
        
        if (playerData.TotalMoneyinBank < item.BuyPrice)
        {
            SendShopResult(player, false, "Insufficient funds");
            return;
        }
        
        playerData.TotalMoneyinBank -= item.BuyPrice;
        playerData.AddOwnedItem(className);
        playerData.Save();
        
        EquipWeapon(player, item);
        
        SendShopResult(player, true, "Purchased " + item.DisplayName);
    }
    
    void EquipWeapon(PlayerBase player, KOTH_ShopItem item)
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
        
        Print("[KOTH_Shop] Equipped " + item.DisplayName + " to " + player.GetIdentity().GetName());
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
        
        if (success)
        {
            ExpansionNotification("Shop", message).Success();
        }
        else
        {
            ExpansionNotification("Shop", message).Error();
        }
    }
    
    void SendShopResult(PlayerBase player, bool success, string message)
    {
        auto rpc = Expansion_CreateRPC("RPC_ShopResult");
        rpc.Write(success);
        rpc.Write(message);
        rpc.Expansion_Send(player, true, player.GetIdentity());
    }
} 