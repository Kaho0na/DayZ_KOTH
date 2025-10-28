[CF_RegisterModule(KOTH_ClothingShopModule)]
class KOTH_ClothingShopModule : CF_ModuleWorld
{
    static KOTH_ClothingShopModule s_Instance;
    
    ref array<ref KOTH_ClothingShopLoadout> m_Loadouts;
    ref ScriptInvoker m_ClothingMenuInvoker;
    ref map<string, float> m_PlayerEquipCooldowns;
    
    void KOTH_ClothingShopModule()
    {
        s_Instance = this;
        m_Loadouts = new array<ref KOTH_ClothingShopLoadout>();
        m_ClothingMenuInvoker = new ScriptInvoker();
        m_PlayerEquipCooldowns = new map<string, float>();
    }
    
    static KOTH_ClothingShopModule GetInstance()
    {
        return s_Instance;
    }
    
    ScriptInvoker GetMenuSI()
    {
        return m_ClothingMenuInvoker;
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        
        if (GetGame().IsServer())
        {
            LoadClothingData();
            Expansion_RegisterServerRPC("RPC_RequestClothingShopOpen");
            Expansion_RegisterServerRPC("RPC_EquipLoadout");
        }
        
        Expansion_RegisterClientRPC("RPC_ReceiveClothingShopData");
        Expansion_RegisterClientRPC("RPC_ClothingEquipResult");
    }
    
    void LoadClothingData()
    {
        string clothingPath = "$profile:ExpansionMod/KOTH_Clothes/";
        
        Print("[KOTH_ClothingShop] Scanning for loadout files in: " + clothingPath);
        
        array<string> files = new array<string>();
        string fileName;
        FileAttr fileAttr;
        FindFileHandle handle = FindFile(clothingPath + "*.json", fileName, fileAttr, 0);
        
        if (handle)
        {
            if (fileName.Length() > 0 && !(fileAttr & FileAttr.DIRECTORY))
            {
                files.Insert(fileName);
            }
            
            while (FindNextFile(handle, fileName, fileAttr))
            {
                if (fileName.Length() > 0 && !(fileAttr & FileAttr.DIRECTORY))
                {
                    files.Insert(fileName);
                }
            }
            
            CloseFindFile(handle);
        }
        
        Print("[KOTH_ClothingShop] Found " + files.Count() + " JSON files");
        
        foreach (string file : files)
        {
            LoadLoadoutFile(clothingPath + file);
        }
        
        Print("[KOTH_ClothingShop] Loadout data loaded. Total loadouts: " + m_Loadouts.Count());
    }
    
    void LoadLoadoutFile(string filePath)
    {
        KOTH_ClothingShopLoadout loadout = new KOTH_ClothingShopLoadout();
        
        if (loadout.LoadFromFile(filePath))
        {
            m_Loadouts.Insert(loadout);
            Print("[KOTH_ClothingShop] Loaded loadout: " + loadout.DisplayName + " from " + filePath);
        }
        else
        {
            Print("[KOTH_ClothingShop] ERROR: Failed to load loadout from " + filePath);
        }
    }
    
    KOTH_ClothingShopLoadout GetLoadout(string displayName)
    {
        foreach (KOTH_ClothingShopLoadout loadout : m_Loadouts)
        {
            if (loadout.DisplayName == displayName)
                return loadout;
        }
        return null;
    }
    
    void RPC_RequestClothingShopOpen(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer() || !sender)
            return;
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
            return;
        
        RequestClothingShopOpen(player, sender);
    }
    
    void RequestClothingShopOpen(PlayerBase player, PlayerIdentity identity)
    {
        if (!GetGame().IsServer() || !player || !identity)
            return;
        
        Print("[KOTH_ClothingShop] Request shop open for: " + identity.GetName());
        
        string uid = identity.GetId();
        KOTH_Players playerData = KOTH_Players.Load(uid);
        
        if (!playerData)
        {
            Print("[KOTH_ClothingShop] ERROR: Player data not found for " + identity.GetName());
            return;
        }
        
        int playerLevel = playerData.CurrentLevel;
        string playerFaction = player.GetKOTHTeam();
        
        float cooldownRemaining = GetEquipCooldown(uid);
        
        auto rpc = Expansion_CreateRPC("RPC_ReceiveClothingShopData");
        rpc.Write(playerLevel);
        rpc.Write(playerFaction);
        rpc.Write(cooldownRemaining);
        
        rpc.Write(m_Loadouts.Count());
        foreach (KOTH_ClothingShopLoadout loadout : m_Loadouts)
        {
            loadout.OnSend(rpc);
        }
        
        rpc.Expansion_Send(true, identity);
        
        Print("[KOTH_ClothingShop] Sent shop data to client");
    }
    
    void RPC_ReceiveClothingShopData(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        int playerLevel;
        if (!ctx.Read(playerLevel)) return;
        
        string playerFaction;
        if (!ctx.Read(playerFaction)) return;
        
        float cooldownRemaining;
        if (!ctx.Read(cooldownRemaining)) return;
        
        int loadoutCount;
        if (!ctx.Read(loadoutCount)) return;
        
        array<ref KOTH_ClothingShopLoadout> loadouts = new array<ref KOTH_ClothingShopLoadout>();
        for (int i = 0; i < loadoutCount; i++)
        {
            KOTH_ClothingShopLoadout loadout = new KOTH_ClothingShopLoadout();
            if (!loadout.OnReceive(ctx)) return;
            loadouts.Insert(loadout);
        }
        
        Print("[KOTH_ClothingShop] CLIENT: Opening menu...");
        if (!GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("KOTH_ClothingShopMenu"))
        {
            Print("[KOTH_ClothingShop] ERROR: Failed to create menu on client!");
            return;
        }
        
        Print("[KOTH_ClothingShop] CLIENT: Invoking menu with " + loadouts.Count() + " loadouts");
        m_ClothingMenuInvoker.Invoke(loadouts, playerLevel, playerFaction, cooldownRemaining);
        Print("[KOTH_ClothingShop] CLIENT: Menu invoker called");
    }
    
    void RPC_EquipLoadout(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_ClothingShop] RPC_EquipLoadout received on server!");
        
        if (!GetGame().IsServer() || !sender)
        {
            Print("[KOTH_ClothingShop] ERROR: Not server or sender null");
            return;
        }
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
        {
            Print("[KOTH_ClothingShop] ERROR: Player cast failed");
            return;
        }
        
        string loadoutName;
        if (!ctx.Read(loadoutName))
        {
            Print("[KOTH_ClothingShop] ERROR: Failed to read loadout name from RPC");
            return;
        }
        
        Print("[KOTH_ClothingShop] Loadout: " + loadoutName + ", Player: " + sender.GetName());
        ProcessLoadoutEquip(player, sender, loadoutName);
    }
    
    void ProcessLoadoutEquip(PlayerBase player, PlayerIdentity identity, string loadoutName)
    {
        Print("[KOTH_ClothingShop] === ProcessLoadoutEquip START ===");
        Print("[KOTH_ClothingShop] Player: " + identity.GetName() + ", Loadout: " + loadoutName);
        
        string uid = identity.GetId();
        
        float cooldown = GetEquipCooldown(uid);
        Print("[KOTH_ClothingShop] Cooldown check: " + cooldown.ToString());
        
        if (cooldown > 0)
        {
            string timeStr = ((int)cooldown).ToString();
            ExpansionNotification("Clothing Shop", "Please wait " + timeStr + " seconds before changing loadout.").Error(identity);
            return;
        }
        
        KOTH_ClothingShopLoadout loadout = GetLoadout(loadoutName);
        if (!loadout)
        {
            Print("[KOTH_ClothingShop] ERROR: Loadout not found: " + loadoutName);
            ExpansionNotification("Clothing Shop", "Loadout not found!").Error(identity);
            return;
        }
        
        KOTH_Players playerData = KOTH_Players.Load(uid);
        if (!playerData)
        {
            Print("[KOTH_ClothingShop] ERROR: Player data not found");
            ExpansionNotification("Clothing Shop", "Player data not found!").Error(identity);
            return;
        }
        
        if (playerData.CurrentLevel < loadout.RequiredLevel)
        {
            ExpansionNotification("Clothing Shop", "You need level " + loadout.RequiredLevel.ToString() + " to equip this loadout!").Error(identity);
            return;
        }
        
        string playerFaction = player.GetKOTHTeam();
        if (loadout.Faction != "" && loadout.Faction != "Both" && loadout.Faction != playerFaction)
        {
            ExpansionNotification("Clothing Shop", "This loadout is for " + loadout.Faction + " faction only!").Error(identity);
            return;
        }
        
        Print("[KOTH_ClothingShop] Validation passed, applying loadout...");
        
        array<EntityAI> savedItems = new array<EntityAI>();
        CollectPlayerInventory(player, savedItems);
        
        ClearPlayerClothing(player);
        
        ApplyLoadout(player, loadout);
        
        RestoreInventory(player, savedItems);
        
        SetEquipCooldown(uid, 5.0);
        
        auto resultRpc = Expansion_CreateRPC("RPC_ClothingEquipResult");
        resultRpc.Write(true);
        resultRpc.Write(loadout.DisplayName);
        resultRpc.Expansion_Send(true, identity);
        
        ExpansionNotification("Clothing Shop", "Equipped " + loadout.DisplayName + "!").Success(identity);
        Print("[KOTH_ClothingShop] === ProcessLoadoutEquip END ===");
    }
    
    void CollectPlayerInventory(PlayerBase player, array<EntityAI> outItems)
    {
        Print("[KOTH_ClothingShop] Collecting player inventory...");
        
        int attCount = player.GetInventory().AttachmentCount();
        for (int i = 0; i < attCount; i++)
        {
            EntityAI attachment = player.GetInventory().GetAttachmentFromIndex(i);
            if (attachment)
            {
                CollectItemsFromContainer(attachment, outItems);
            }
        }
        
        Print("[KOTH_ClothingShop] Collected " + outItems.Count() + " items");
    }
    
    void CollectItemsFromContainer(EntityAI container, array<EntityAI> outItems)
    {
        if (!container || !container.GetInventory())
            return;
        
        CargoBase cargo = container.GetInventory().GetCargo();
        if (cargo)
        {
            int itemCount = cargo.GetItemCount();
            for (int i = 0; i < itemCount; i++)
            {
                EntityAI cargoItem = cargo.GetItem(i);
                if (cargoItem)
                {
                    outItems.Insert(cargoItem);
                    CollectItemsFromContainer(cargoItem, outItems);
                }
            }
        }
        
        int attCount = container.GetInventory().AttachmentCount();
        for (int j = 0; j < attCount; j++)
        {
            EntityAI containerAtt = container.GetInventory().GetAttachmentFromIndex(j);
            if (containerAtt)
            {
                outItems.Insert(containerAtt);
                CollectItemsFromContainer(containerAtt, outItems);
            }
        }
    }
    
    void ClearPlayerClothing(PlayerBase player)
    {
        Print("[KOTH_ClothingShop] Clearing player clothing...");
        
        array<string> clothingSlots = new array<string>();
        clothingSlots.Insert("Headgear");
        clothingSlots.Insert("Mask");
        clothingSlots.Insert("Eyewear");
        clothingSlots.Insert("Gloves");
        clothingSlots.Insert("Body");
        clothingSlots.Insert("Vest");
        clothingSlots.Insert("Back");
        clothingSlots.Insert("Legs");
        clothingSlots.Insert("Feet");
        
        foreach (string slotName : clothingSlots)
        {
            int slotId = InventorySlots.GetSlotIdFromString(slotName);
            EntityAI clothingItem = player.GetInventory().FindAttachment(slotId);
            if (clothingItem)
            {
                player.GetInventory().LocalDestroyEntity(clothingItem);
            }
        }
        
        Print("[KOTH_ClothingShop] Clothing cleared");
    }
    
    void ApplyLoadout(PlayerBase player, KOTH_ClothingShopLoadout loadout)
    {
        Print("[KOTH_ClothingShop] Applying loadout: " + loadout.DisplayName);
        
        ExpansionHumanLoadout.Apply(player, loadout.FileName);
        
        Print("[KOTH_ClothingShop] Loadout applied");
    }
    
    void RestoreInventory(PlayerBase player, array<EntityAI> items)
    {
        Print("[KOTH_ClothingShop] Restoring " + items.Count() + " items...");
        
        array<string> itemClassNames = new array<string>();
        foreach (EntityAI savedItem : items)
        {
            if (savedItem && !savedItem.IsRuined())
            {
                itemClassNames.Insert(savedItem.GetType());
                GetGame().ObjectDelete(savedItem);
            }
        }
        
        foreach (string className : itemClassNames)
        {
            EntityAI newItem = player.GetInventory().CreateInInventory(className);
            if (newItem)
            {
                Print("[KOTH_ClothingShop] Restored item: " + className);
            }
            else
            {
                vector dropPos = player.GetPosition();
                EntityAI droppedItem = EntityAI.Cast(GetGame().CreateObjectEx(className, dropPos, ECE_PLACE_ON_SURFACE));
                if (droppedItem)
                {
                    Print("[KOTH_ClothingShop] Dropped item at feet: " + className);
                }
            }
        }
        
        Print("[KOTH_ClothingShop] Inventory restore complete");
    }
    
    void SetEquipCooldown(string uid, float seconds)
    {
        m_PlayerEquipCooldowns.Set(uid, GetGame().GetTime() + (seconds * 1000));
    }
    
    float GetEquipCooldown(string uid)
    {
        if (!m_PlayerEquipCooldowns.Contains(uid))
            return 0;
        
        float cooldownEnd = m_PlayerEquipCooldowns.Get(uid);
        float currentTime = GetGame().GetTime();
        
        if (currentTime >= cooldownEnd)
        {
            m_PlayerEquipCooldowns.Remove(uid);
            return 0;
        }
        
        return (cooldownEnd - currentTime) / 1000.0;
    }
    
    void RPC_ClothingEquipResult(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        bool success;
        if (!ctx.Read(success)) return;
        
        string loadoutName;
        if (!ctx.Read(loadoutName)) return;
        
        if (success)
        {
            GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
        }
    }
}

class KOTH_ClothingShopLoadout
{
    string DisplayName;
    string Faction;
    int RequiredLevel;
    string FileName;
    
    bool LoadFromFile(string filePath)
    {
        if (!FileExist(filePath))
        {
            Print("[KOTH_ClothingShop] File not found: " + filePath);
            return false;
        }
        
        KOTH_ClothingShopLoadoutConfig config = new KOTH_ClothingShopLoadoutConfig();
        JsonFileLoader<KOTH_ClothingShopLoadoutConfig>.JsonLoadFile(filePath, config);
        
        DisplayName = config.DisplayName;
        Faction = config.Faction;
        RequiredLevel = config.RequiredLevel;
        
        FileName = filePath;
        FileName.Replace("$profile:ExpansionMod/KOTH_Clothes/", "");
        FileName.Replace(".json", "");
        
        return true;
    }
    
    void OnSend(ParamsWriteContext ctx)
    {
        ctx.Write(DisplayName);
        ctx.Write(Faction);
        ctx.Write(RequiredLevel);
        ctx.Write(FileName);
    }
    
    bool OnReceive(ParamsReadContext ctx)
    {
        if (!ctx.Read(DisplayName)) return false;
        if (!ctx.Read(Faction)) return false;
        if (!ctx.Read(RequiredLevel)) return false;
        if (!ctx.Read(FileName)) return false;
        
        return true;
    }
}

class KOTH_ClothingShopLoadoutConfig
{
    string DisplayName;
    string Faction;
    int RequiredLevel;
}