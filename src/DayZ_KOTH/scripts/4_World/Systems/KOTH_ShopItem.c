// KOTH_ShopItem.c
// Data class representing a single item in the shop

class KOTH_ShopItem
{
    string ClassName;
    string DisplayName;
    string AmmoType;
    int RentPrice;
    int BuyPrice;
    int RequiredLevel;
    string MagazineClass;
    ref array<string> DefaultAttachments;
    
    void KOTH_ShopItem()
    {
        DefaultAttachments = new array<string>();
    }
}