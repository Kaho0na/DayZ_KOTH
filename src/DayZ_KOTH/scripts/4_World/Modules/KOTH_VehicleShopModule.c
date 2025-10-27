[CF_RegisterModule(KOTH_VehicleShopModule)]
class KOTH_VehicleShopModule : CF_ModuleWorld
{
    static KOTH_VehicleShopModule s_Instance;
    
    ref array<ref KOTH_VehicleShopItem> m_Vehicles;
    ref ScriptInvoker m_VehicleMenuInvoker;
    ref map<string, float> m_PlayerRentalCooldowns;
    private ExpansionMarketModule m_MarketModule;
    private KOTH_PlayerRewardManager m_RewardManager;
    
    void KOTH_VehicleShopModule()
    {
        s_Instance = this;
        m_Vehicles = new array<ref KOTH_VehicleShopItem>();
        m_VehicleMenuInvoker = new ScriptInvoker();
        m_PlayerRentalCooldowns = new map<string, float>();
    }
    
    override void OnInit()
    {
        super.OnInit();
        
        Expansion_EnableRPCManager();
        
        if (GetGame().IsServer())
        {
            LoadVehicleData();
            Expansion_RegisterServerRPC("RPC_RequestVehicleShopOpen");
            Expansion_RegisterServerRPC("RPC_RentVehicle");
            
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(InitializeModules, 2000, false);
        }
        
        Expansion_RegisterClientRPC("RPC_ReceiveVehicleShopData");
        Expansion_RegisterClientRPC("RPC_VehicleShopResult");
    }
    
    void InitializeModules()
    {
        if (!Class.CastTo(m_MarketModule, CF_ModuleCoreManager.Get(ExpansionMarketModule)))
        {
            Error("[KOTH_VehicleShop] Failed to get ExpansionMarketModule!");
        }
        else
        {
            Print("[KOTH_VehicleShop] MarketModule initialized successfully");
        }
        
        CF_Modules<KOTH_PlayerRewardManager>.Get(m_RewardManager);
        if (m_RewardManager)
            Print("[KOTH_VehicleShop] RewardManager initialized successfully");
        else
            Error("[KOTH_VehicleShop] ERROR: Failed to initialize RewardManager!");
    }
    
    static KOTH_VehicleShopModule GetInstance()
    {
        return s_Instance;
    }
    
    ScriptInvoker GetMenuSI()
    {
        return m_VehicleMenuInvoker;
    }
    
    void LoadVehicleData()
    {
        string vehiclePath = "$profile:ExpansionMod/KOTH_Vehicle_Categories/";
        
        Print("[KOTH_VehicleShop] Scanning for vehicle files in: " + vehiclePath);
        
        array<string> files = new array<string>();
        string fileName;
        FileAttr fileAttr;
        FindFileHandle handle = FindFile(vehiclePath + "*.json", fileName, fileAttr, 0);
        
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
        
        Print("[KOTH_VehicleShop] Found " + files.Count() + " JSON files");
        
        foreach (string file : files)
        {
            LoadVehicleFile(vehiclePath + file);
        }
        
        Print("[KOTH_VehicleShop] Vehicle data loaded. Total vehicles: " + m_Vehicles.Count());
    }
    
    void LoadVehicleFile(string path)
    {
        if (!FileExist(path))
        {
            Print("[KOTH_VehicleShop] WARNING: Vehicle file not found: " + path);
            return;
        }
        
        KOTH_VehicleShopItem vehicle = new KOTH_VehicleShopItem();
        JsonFileLoader<KOTH_VehicleShopItem>.JsonLoadFile(path, vehicle);
        
        if (vehicle && vehicle.ClassName != "")
        {
            m_Vehicles.Insert(vehicle);
            Print("[KOTH_VehicleShop] Loaded vehicle: " + vehicle.DisplayName);
        }
    }
    
    KOTH_VehicleShopItem GetVehicle(string className)
    {
        foreach (KOTH_VehicleShopItem veh : m_Vehicles)
        {
            if (veh.ClassName == className)
                return veh;
        }
        return null;
    }
    
    void RPC_RequestVehicleShopOpen(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsServer() || !sender)
            return;
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
            return;
        
        RequestVehicleShopOpen(player, sender);
    }

    void RequestVehicleShopOpen(PlayerBase player, PlayerIdentity identity)
    {
        if (!GetGame().IsServer() || !player || !identity)
            return;
        
        Print("[KOTH_VehicleShop] Request shop open for: " + identity.GetName());
        
        string uid = identity.GetId();
        KOTH_Players playerData = KOTH_Players.Load(uid);
        
        if (!playerData)
        {
            Print("[KOTH_VehicleShop] ERROR: Player data not found for " + identity.GetName());
            return;
        }

        int playerBalance = playerData.TotalMoneyinBank;
        int playerLevel = playerData.CurrentLevel;      
        string playerFaction = player.GetKOTHTeam();
        
        float cooldownRemaining = GetRentalCooldown(uid);
        
        auto rpc = Expansion_CreateRPC("RPC_ReceiveVehicleShopData");
        rpc.Write(playerBalance);
        rpc.Write(playerLevel);
        rpc.Write(playerFaction);
        rpc.Write(cooldownRemaining);
        
        rpc.Write(m_Vehicles.Count());
        foreach (KOTH_VehicleShopItem veh : m_Vehicles)
        {
            veh.OnSend(rpc);
        }
        
        rpc.Expansion_Send(true, identity);
        
        Print("[KOTH_VehicleShop] Sent shop data to client");
    }
    
    void RPC_ReceiveVehicleShopData(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        int playerBalance;
        if (!ctx.Read(playerBalance)) return;
        
        int playerLevel;
        if (!ctx.Read(playerLevel)) return;
        
        string playerFaction;
        if (!ctx.Read(playerFaction)) return;
        
        float cooldownRemaining;
        if (!ctx.Read(cooldownRemaining)) return;
        
        int vehicleCount;
        if (!ctx.Read(vehicleCount)) return;
        
        array<ref KOTH_VehicleShopItem> vehicles = new array<ref KOTH_VehicleShopItem>();
        for (int i = 0; i < vehicleCount; i++)
        {
            KOTH_VehicleShopItem veh = new KOTH_VehicleShopItem();
            if (!veh.OnReceive(ctx)) return;
            vehicles.Insert(veh);
        }
        
        Print("[KOTH_VehicleShop] CLIENT: Opening menu...");
        if (!GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("KOTH_VehicleShopMenu"))
        {
            Print("[KOTH_VehicleShop] ERROR: Failed to create menu on client!");
            return;
        }
        
        Print("[KOTH_VehicleShop] CLIENT: Invoking menu with " + vehicles.Count() + " vehicles");
        m_VehicleMenuInvoker.Invoke(vehicles, playerLevel, playerBalance, playerFaction, cooldownRemaining);
        Print("[KOTH_VehicleShop] CLIENT: Menu invoker called");
    }
    
    void RPC_RentVehicle(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[KOTH_VehicleShop] RPC_RentVehicle received on server!");
        
        if (!GetGame().IsServer() || !sender)
        {
            Print("[KOTH_VehicleShop] ERROR: Not server or sender null");
            return;
        }
        
        PlayerBase player = PlayerBase.Cast(target);
        if (!player)
        {
            Print("[KOTH_VehicleShop] ERROR: Player cast failed");
            return;
        }
        
        string vehicleClass;
        if (!ctx.Read(vehicleClass))
        {
            Print("[KOTH_VehicleShop] ERROR: Failed to read vehicle class from RPC");
            return;
        }
        
        Print("[KOTH_VehicleShop] Vehicle class: " + vehicleClass + ", Player: " + sender.GetName());
        ProcessVehicleRental(player, sender, vehicleClass);
    }
    
    void ProcessVehicleRental(PlayerBase player, PlayerIdentity identity, string vehicleClass)
    {
        Print("[KOTH_VehicleShop] === ProcessVehicleRental START ===");
        Print("[KOTH_VehicleShop] Player: " + identity.GetName() + ", Vehicle: " + vehicleClass);
        
        string uid = identity.GetId();
        
        float cooldown = GetRentalCooldown(uid);
        Print("[KOTH_VehicleShop] Cooldown check: " + cooldown.ToString());
        
        if (cooldown > 0)
        {
            string timeStr = ((int)cooldown).ToString();
            ExpansionNotification("Vehicle Rental", "Please wait " + timeStr + " seconds before renting another vehicle.").Error(identity);
            return;
        }
        
        KOTH_VehicleShopItem vehicle = GetVehicle(vehicleClass);
        if (!vehicle)
        {
            Print("[KOTH_VehicleShop] ERROR: Vehicle not found: " + vehicleClass);
            ExpansionNotification("Vehicle Rental", "Vehicle not found in shop!").Error(identity);
            return;
        }
        
        Print("[KOTH_VehicleShop] Vehicle found: " + vehicle.DisplayName);
        
        if (!ValidateRental(player, identity, vehicle))
        {
            Print("[KOTH_VehicleShop] Validation failed");
            return;
        }
        
        Print("[KOTH_VehicleShop] Validation passed, deducting money");
        
        if (m_MarketModule)
        {
            ExpansionMarketATM_Data atmData = m_MarketModule.GetPlayerATMData(uid);
            if (atmData)
            {
                atmData.RemoveMoney(vehicle.RentPrice);
                atmData.Save();
                Print("[KOTH_VehicleShop] Money deducted: $" + vehicle.RentPrice.ToString());
            }
        }
        KOTH_Players playerData = KOTH_Players.Load(uid);
        playerData.TotalMoneyinBank = playerData.TotalMoneyinBank - vehicle.RentPrice;
        playerData.Save();
        
        if (m_RewardManager)
        {
            m_RewardManager.SyncPlayerStatsToClient(identity, playerData);
        }

        vector spawnPos = FindVehicleSpawnPosition(player);
        
        SpawnVehicleForPlayer(player, vehicle, spawnPos);
        
        SetRentalCooldown(uid, 30.0);
        
        ExpansionNotification("Vehicle Rented", "You rented a " + vehicle.DisplayName + " for $" + vehicle.RentPrice.ToString()).Success(identity);
    }
    
    bool ValidateRental(PlayerBase player, PlayerIdentity identity, KOTH_VehicleShopItem vehicle)
    {
        string uid = identity.GetId();
        
        KOTH_Players playerData = KOTH_Players.Load(uid);
        if (playerData)
        {
            int playerLevel = playerData.CurrentLevel;
            int playerBalance = playerData.TotalMoneyinBank;
        }
        
        if (playerLevel < vehicle.RequiredLevel)
        {
            ExpansionNotification("Level Required", "You need level " + vehicle.RequiredLevel.ToString() + " to rent this vehicle.").Error(identity);
            return false;
        }
        
        string playerFaction = player.GetKOTHTeam();
        
        if (vehicle.Faction != "" && vehicle.Faction != "Both" && vehicle.Faction != playerFaction)
        {
            ExpansionNotification("Faction Locked", "This vehicle is locked to " + vehicle.Faction + " faction.").Error(identity);
            return false;
        }
        
        if (playerBalance < vehicle.RentPrice)
        {
            ExpansionNotification("Insufficient Funds", "You need $" + vehicle.RentPrice.ToString() + " to rent this vehicle.").Error(identity);
            return false;
        }
        
        return true;
    }
    
    vector FindVehicleSpawnPosition(PlayerBase player)
    {
        vector playerPos = player.GetPosition();
        KOTH_ZoneManager zoneManager = KOTH_ZoneManager.GetInstance();
        
        if (!zoneManager)
        {
            Print("[KOTH_VehicleShop] ERROR: ZoneManager not found!");
            return playerPos;
        }
        
        KOTH_ZoneData activeZone = zoneManager.GetActiveZone();
        if (!activeZone)
        {
            Print("[KOTH_VehicleShop] ERROR: No active zone!");
            return playerPos;
        }
        
        string playerTeam = player.GetKOTHTeam();
        vector baseCenter;
        
        if (playerTeam == "West")
            baseCenter = activeZone.GetWestSpawnBuilding();
        else if (playerTeam == "East")
            baseCenter = activeZone.GetEastSpawnBuilding();
        else
        {
            Print("[KOTH_VehicleShop] ERROR: Player has no team!");
            return playerPos;
        }
        
        KOTH_Settings settings = GetExpansionSettings().GetDayZ_KOTH();
        float spawnRadius = 20.0;
        if (settings)
            spawnRadius = settings.VehicleSpawnCheckRadius;
        
        for (int attempts = 0; attempts < 10; attempts++)
        {
            float angle = Math.RandomFloat(0, 360);
            float distance = Math.RandomFloat(10, spawnRadius);
            
            float offsetX = Math.Sin(angle * Math.DEG2RAD) * distance;
            float offsetZ = Math.Cos(angle * Math.DEG2RAD) * distance;
            
            float spawnX = baseCenter[0] + offsetX;
            float spawnZ = baseCenter[2] + offsetZ;
            float spawnY = GetGame().SurfaceY(spawnX, spawnZ);
            
            vector testPos = Vector(spawnX, spawnY, spawnZ);
            
            if (!IsPositionBlocked(testPos))
            {
                Print("[KOTH_VehicleShop] Found clear spawn position at: " + testPos.ToString());
                return testPos;
            }
        }
        
        Print("[KOTH_VehicleShop] WARNING: Could not find clear position, using fallback");
        float fallbackX = baseCenter[0] + 15;
        float fallbackZ = baseCenter[2] + 15;
        float fallbackY = GetGame().SurfaceY(fallbackX, fallbackZ);
        return Vector(fallbackX, fallbackY, fallbackZ);
    }
    
    bool IsPositionBlocked(vector pos)
    {
        array<Object> nearbyObjects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        
        GetGame().GetObjectsAtPosition(pos, 5.0, nearbyObjects, proxyCargos);
        
        foreach (Object obj : nearbyObjects)
        {
            if (Car.Cast(obj))
                return true;
        }
        
        return false;
    }
    
    void SpawnVehicleForPlayer(PlayerBase player, KOTH_VehicleShopItem vehicle, vector spawnPos)
    {
        Print("[KOTH_VehicleShop] === SpawnVehicleForPlayer START ===");
        Print("[KOTH_VehicleShop] Vehicle class: " + vehicle.ClassName);
        Print("[KOTH_VehicleShop] Spawn position: " + spawnPos.ToString());
        
        vector spawnOrientation = Vector(Math.RandomFloat(0, 360), 0, 0);
        
        int remainingAmount = 1;
        EntityAI parent = NULL;
        bool attachmentNotAttached = false;
        
        Print("[KOTH_VehicleShop] Calling ExpansionItemSpawnHelper.SpawnVehicle...");
        Object spawnedObj = ExpansionItemSpawnHelper.SpawnVehicle(vehicle.ClassName, NULL, parent, spawnPos, spawnOrientation, remainingAmount, NULL, -1, "", attachmentNotAttached);
        
        if (!spawnedObj)
        {
            Print("[KOTH_VehicleShop] ERROR: Failed to spawn vehicle: " + vehicle.ClassName);
            return;
        }
        
        Print("[KOTH_VehicleShop] Vehicle spawned successfully!");
        
        Car spawnedVehicle = Car.Cast(spawnedObj);
        if (!spawnedVehicle)
        {
            Print("[KOTH_VehicleShop] ERROR: Could not cast to Car");
            return;
        }
        
        Print("[KOTH_VehicleShop] Adding attachments...");
        foreach (string attachment : vehicle.Attachments)
        {
            EntityAI att = spawnedVehicle.GetInventory().CreateInInventory(attachment);
            if (att)
            {
                att.SetHealth("", "", 1000);
                Print("[KOTH_VehicleShop] Added attachment: " + attachment);
            }
        }
        
        spawnedVehicle.Fill(CarFluid.FUEL, spawnedVehicle.GetFluidCapacity(CarFluid.FUEL));
        spawnedVehicle.Fill(CarFluid.OIL, spawnedVehicle.GetFluidCapacity(CarFluid.OIL));
        spawnedVehicle.Fill(CarFluid.BRAKE, spawnedVehicle.GetFluidCapacity(CarFluid.BRAKE));
        spawnedVehicle.Fill(CarFluid.COOLANT, spawnedVehicle.GetFluidCapacity(CarFluid.COOLANT));
        
        Print("[KOTH_VehicleShop] Vehicle spawned successfully: " + vehicle.DisplayName);
    }
    
    void SetRentalCooldown(string uid, float seconds)
    {
        m_PlayerRentalCooldowns.Set(uid, GetGame().GetTime() + (seconds * 1000));
    }
    
    float GetRentalCooldown(string uid)
    {
        if (!m_PlayerRentalCooldowns.Contains(uid))
            return 0;
        
        float cooldownEnd = m_PlayerRentalCooldowns.Get(uid);
        float currentTime = GetGame().GetTime();
        
        if (currentTime >= cooldownEnd)
        {
            m_PlayerRentalCooldowns.Remove(uid);
            return 0;
        }
        
        return (cooldownEnd - currentTime) / 1000.0;
    }
    
    void RPC_VehicleShopResult(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (GetGame().IsServer())
            return;
        
        bool success;
        if (!ctx.Read(success)) return;
        
        string message;
        if (!ctx.Read(message)) return;
        
        if (success)
        {
            ExpansionNotification("Vehicle Shop", message).Success();
        }
        else
        {
            ExpansionNotification("Vehicle Shop", message).Error();
        }
    }
}

class KOTH_VehicleShopItem
{
    string ClassName;
    string DisplayName;
    int RentPrice;
    string Faction;
    int RequiredLevel;
    int MaxCount;
    ref TStringArray Attachments;
    
    void KOTH_VehicleShopItem()
    {
        Attachments = new TStringArray();
    }
    
    void OnSend(ParamsWriteContext ctx)
    {
        ctx.Write(ClassName);
        ctx.Write(DisplayName);
        ctx.Write(RentPrice);
        ctx.Write(Faction);
        ctx.Write(RequiredLevel);
        ctx.Write(MaxCount);
        ctx.Write(Attachments.Count());
        foreach (string att : Attachments)
        {
            ctx.Write(att);
        }
    }
    
    bool OnReceive(ParamsReadContext ctx)
    {
        if (!ctx.Read(ClassName)) return false;
        if (!ctx.Read(DisplayName)) return false;
        if (!ctx.Read(RentPrice)) return false;
        if (!ctx.Read(Faction)) return false;
        if (!ctx.Read(RequiredLevel)) return false;
        if (!ctx.Read(MaxCount)) return false;
        
        int attCount;
        if (!ctx.Read(attCount)) return false;
        
        Attachments.Clear();
        for (int i = 0; i < attCount; i++)
        {
            string att;
            if (!ctx.Read(att)) return false;
            Attachments.Insert(att);
        }
        
        return true;
    }
}