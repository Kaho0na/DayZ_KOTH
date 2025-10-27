class KOTH_ShopItems
{
    static void EquipItem(PlayerBase player, KOTH_ShopItem item)
    {
        EntityAI createdItem = player.GetInventory().CreateInInventory(item.ClassName);
        if (!createdItem)
        {
            Print("[KOTH_Shop] ERROR: Failed to create item in inventory: " + item.ClassName);
            return;
        }
        
        Print("[KOTH_Shop] Created item in inventory: " + item.ClassName);
    }
}