class KOTH_ShopScopes
{
    static bool IsScope(string className)
    {
        array<string> scopeTypes = {"ReflexOptic", "ACOGOptic", "M4_CarryHandleOptic", "M68Optic", "KazuarOptic", "PUScopeOptic", "HuntingOptic", "PSO1Optic", "KobraOptic", "Crossbow_RedpointOptic", "StarlightOptic", "DHOptic"};
        
        foreach (string scopeType : scopeTypes)
        {
            if (className.IndexOf(scopeType) != -1)
                return true;
        }
        
        return false;
    }
    
    static void EquipScope(PlayerBase player, KOTH_ShopItem item)
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
}