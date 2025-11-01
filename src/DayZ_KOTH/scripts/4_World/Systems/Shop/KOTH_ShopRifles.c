class KOTH_ShopRifles
{
    static void ClearRifleMagazines(PlayerBase player)
    {
        array<string> pistolMagTypes = {"Mag_Glock_15Rnd", "Mag_FNX45_15Rnd", "Mag_CZ75_15Rnd", "Mag_1911_7Rnd", "Mag_Deagle_9Rnd", "Mag_MKII_10Rnd", "Mag_P1_8Rnd"};
        array<string> rifleAmmoTypes = {"AmmoBox_00buck_10rnd", "AmmoBox_12gaRubberSlug_10Rnd", "AmmoBox_12gaSlug_10Rnd", "AmmoBox_22_50Rnd", "AmmoBox_357_20Rnd", "AmmoBox_380_35rnd", "AmmoBox_45ACP_25rnd", "AmmoBox_308WinTracer_20Rnd", "AmmoBox_308Win_20Rnd", "AmmoBox_545x39Tracer_20Rnd", "AmmoBox_545x39_20Rnd", "AmmoBox_556x45Tracer_20Rnd", "AmmoBox_556x45_20Rnd", "AmmoBox_762x39Tracer_20Rnd", "AmmoBox_762x39_20Rnd", "AmmoBox_762x54Tracer_20Rnd", "AmmoBox_762x54_20Rnd", "AmmoBox_9x19_25rnd", "AmmoBox_9x39AP_20Rnd", "AmmoBox_9x39_20Rnd", "Ammo_12gaPellets", "Ammo_12gaRubberSlug", "Ammo_12gaSlug", "Ammo_22", "Ammo_357", "Ammo_380", "Ammo_45ACP", "Ammo_308Win", "Ammo_308WinTracer", "Ammo_545x39", "Ammo_545x39Tracer", "Ammo_556x45", "Ammo_556x45Tracer", "Ammo_762x39", "Ammo_762x39Tracer", "Ammo_762x54", "Ammo_762x54Tracer", "Ammo_9x19", "Ammo_9x39", "Ammo_9x39AP"};
        array<EntityAI> itemsToDelete = new array<EntityAI>();
        array<EntityAI> allItems = new array<EntityAI>();
        
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, allItems);
        
        foreach (EntityAI item : allItems)
        {
            string itemType = item.GetType();
            bool shouldDelete = false;
            
            if (item.IsMagazine() || item.IsAmmoPile())
            {
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
                    shouldDelete = true;
                }
            }
            
            if (!shouldDelete)
            {
                foreach (string rifleAmmoType : rifleAmmoTypes)
                {
                    if (itemType.IndexOf(rifleAmmoType) != -1)
                    {
                        shouldDelete = true;
                        break;
                    }
                }
            }
            
            if (shouldDelete)
            {
                itemsToDelete.Insert(item);
            }
        }
        
        foreach (EntityAI itemToDelete : itemsToDelete)
        {
            itemToDelete.DeleteSafe();
        }
        
        Print("[KOTH_Shop] Cleared " + itemsToDelete.Count() + " rifle magazines and ammo boxes (preserved pistol mags)");
    }



    static void GiveStandardLoadout(PlayerBase player, KOTH_ShopItem item)
    {
        if (item.MagazineClass == "")
        {
            Print("[KOTH_Shop] No magazine class defined for " + item.ClassName);
            return;
        }
        PlayerIdentity ident = player.GetIdentity();
        int magsGiven = 0;
        int magsToGive = 5;
        
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
                Print("[KOTH_Shop] No space in inventory for more magazines");
                ExpansionNotification("Shop", "No space in inventory").Error(ident);
            }
        }
        
        Print("[KOTH_Shop] Gave " + magsGiven + " magazines for " + item.ClassName);
    }

    static void EquipPrimaryWeapon(PlayerBase player, KOTH_ShopItem item)
    {
        EntityAI currentWeapon = player.GetHumanInventory().GetEntityInHands();
        if (currentWeapon)
        {
            currentWeapon.DeleteSafe();
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

    

}