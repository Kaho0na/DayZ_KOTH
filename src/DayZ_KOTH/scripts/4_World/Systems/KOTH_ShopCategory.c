// KOTH_ShopCategory.c
// Container class for a category of shop items

class KOTH_ShopCategory
{
    string Category;
    ref array<ref KOTH_ShopItem> Items;
    
    void KOTH_ShopCategory()
    {
        Items = new array<ref KOTH_ShopItem>();
    }
    
    KOTH_ShopItem GetItem(string className)
    {
        foreach (KOTH_ShopItem item : Items)
        {
            if (item.ClassName == className)
                return item;
        }
        return null;
    }
}