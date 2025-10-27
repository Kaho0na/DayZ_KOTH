class KOTH_ShopPistols
{
    static void ClearPistolMagazines(PlayerBase player)
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

    static void EquipPistol(PlayerBase player, KOTH_ShopItem item)
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

    static bool IsPistol(string className)
    {
        array<string> pistolTypes = {"Glock19", "FNX45", "CZ75", "Deagle", "MKII", "P1", "Engraved1911", "Colt1911", "Magnum", "Pistol_Base"};
        
        foreach (string pistolType : pistolTypes)
        {
            if (className.IndexOf(pistolType) != -1)
                return true;
        }
        
        return false;
    }

    static EntityAI FindHolster(PlayerBase player)
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