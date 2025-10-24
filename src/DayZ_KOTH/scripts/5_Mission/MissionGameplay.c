modded class MissionGameplay
{
    override void OnKeyPress(int key)
    {
        super.OnKeyPress(key);
        
        if (key == KeyCode.KC_K)
        {
            Print("[MissionGameplay] K key pressed - Opening KOTH Shop Menu");
            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player && player.GetIdentity())
            {
                Print("[KOTH_Shop] Sending RPC to request shop open");
                auto rpc = KOTH_ShopModule.GetInstance().Expansion_CreateRPC("RPC_RequestShopOpen");
                rpc.Expansion_Send(player, true, player.GetIdentity());
            }
        }
    }
}