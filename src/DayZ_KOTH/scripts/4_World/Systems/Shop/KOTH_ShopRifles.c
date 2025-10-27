class KOTH_ShopRifles
{
    static void ClearRifleMagazines(PlayerBase player)
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

    

}