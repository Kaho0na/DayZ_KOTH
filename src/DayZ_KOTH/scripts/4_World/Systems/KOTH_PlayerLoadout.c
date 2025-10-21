/**
 * KOTH_PlayerLoadout.c
 *
 * King of the Hill by Kahoona
 * Handles player loadouts based on team selection
 * (PlayerBase modding moved to KOTH_PlayerBase.c)
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 */

class KOTH_PlayerLoadout
{
    static void SetPlayerLoadout(PlayerBase player, string team)
    {
        if (!player)
            return;
        
        ClearPlayerInventory(player);
        
        SetPlayerStats(player);
        eAIGroup playerGroup = player.GetGroup();
        if (!playerGroup)
            playerGroup = eAIGroup.GetGroupByLeader(player);
        eAIFaction m_WestFaction = eAIFaction.Create("West");
        eAIFaction m_EastFaction = eAIFaction.Create("East");

        if (team == "East")
        {
            SetEastLoadout(player);
            // Set Expansion AI faction
            if (playerGroup)
            {
                if (m_EastFaction)
                {
                    playerGroup.SetFaction(m_EastFaction);
                    Print("[KOTH_PlayerLoadout] Set player to East faction");
                }
            }
        }
        else if (team == "West")
        {
            SetWestLoadout(player);
            // Set Expansion AI faction
            if (playerGroup)
            {

                if (m_WestFaction)
                {
                    playerGroup.SetFaction(m_WestFaction);
                    Print("[KOTH_PlayerLoadout] Set player to West faction");
                }
            }
        }
        
        Print("[KOTH_PlayerLoadout] Applied " + team + " loadout to player: " + player.GetIdentity().GetName());
    }
    
    static void ClearPlayerInventory(PlayerBase player)
    {
        array<EntityAI> itemsToDelete = new array<EntityAI>();
        
        int attCount = player.GetInventory().AttachmentCount();
        for (int att = 0; att < attCount; att++)
        {
            EntityAI attachment = player.GetInventory().GetAttachmentFromIndex(att);
            if (attachment)
            {
                itemsToDelete.Insert(attachment);
            }
        }
        
        EntityAI handsItem = player.GetHumanInventory().GetEntityInHands();
        if (handsItem)
        {
            itemsToDelete.Insert(handsItem);
        }
        
        foreach (EntityAI item : itemsToDelete)
        {
            player.GetInventory().LocalDestroyEntity(item);
        }
    }
    
    static void SetPlayerStats(PlayerBase player)
    {
        player.SetHealth("", "", player.GetMaxHealth("", ""));
        
        player.SetHealth("", "Blood", player.GetMaxHealth("", "Blood"));
        
        player.SetHealth("", "Shock", player.GetMaxHealth("", "Shock"));
        
        player.m_AgentPool.RemoveAllAgents();
        
        player.GetStatWater().Set(player.GetStatWater().GetMax());
        player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
        
        player.GetStatHeatComfort().Set(0);
        
        player.m_BleedingManagerServer.RemoveAllSources();
        
        if (player.IsUnconscious())
        {
            player.SetHealth("", "Shock", 100);
            DayZPlayerSyncJunctures.SendPlayerUnconsciousness(player, false);
        }
    }
    
    static void SetEastLoadout(PlayerBase player)
    {
        EntityAI item;
        EntityAI attachment;
        EntityAI cargo;
        
        EntityAI primaryWeapon;
        EntityAI pistol;
        EntityAI knife;
        EntityAI grenade;
        EntityAI bandage;
        EntityAI epi;
        EntityAI morphine;
        EntityAI saline;
        
        item = player.GetInventory().CreateInInventory("TTSKOPants");
        if (item)
        {
            bandage = item.GetInventory().CreateInInventory("BandageDressing");
            item.GetInventory().CreateInInventory("BandageDressing");
            item.GetInventory().CreateInInventory("BandageDressing");
            epi = item.GetInventory().CreateInInventory("Epinephrine");
            morphine = item.GetInventory().CreateInInventory("Morphine");
            saline = item.GetInventory().CreateInInventory("SalineBagIV");
        }
        
        item = player.GetInventory().CreateInInventory("TTsKOJacket_Camo");
        if (item)
        {
            knife = item.GetInventory().CreateInInventory("CombatKnife");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
        }
        
        player.GetInventory().CreateInInventory("TTSKOBoots");
        
        item = player.GetInventory().CreateInInventory("SmershVest");
        if (item)
        {
            grenade = item.GetInventory().CreateAttachment("RGD5Grenade");
            item.GetInventory().CreateAttachment("RGD5Grenade");
            
            item.GetInventory().CreateInInventory("Mag_AK74_45Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_45Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
        }
        
        player.GetInventory().CreateInInventory("Ssh68Helmet");
        
        item = player.GetInventory().CreateInInventory("MilitaryBelt");
        if (item)
        {
            attachment = item.GetInventory().CreateAttachment("PlateCarrierHolster_Camo");
            if (attachment)
            {
                pistol = attachment.GetInventory().CreateAttachment("CZ75");
                if (pistol)
                {
                    pistol.GetInventory().CreateAttachment("Mag_CZ75_15Rnd");
                }
            }
        }
        
        primaryWeapon = player.GetHumanInventory().CreateInHands("AKS74U");
        if (primaryWeapon)
        {
            primaryWeapon.GetInventory().CreateAttachment("AKS74U_Bttstck");
            primaryWeapon.GetInventory().CreateAttachment("Mag_AK74_45Rnd");
        }
        
        SetPermanentArmband(player, "Armband_Red");
        
        AssignToHotbar(player, primaryWeapon, 0);
        AssignToHotbar(player, pistol, 1);
        AssignToHotbar(player, knife, 2);
        AssignToHotbar(player, grenade, 3);
        AssignToHotbar(player, bandage, 4);
        AssignToHotbar(player, epi, 5);
        AssignToHotbar(player, morphine, 6);
        AssignToHotbar(player, saline, 7);
    }
    
    static void SetWestLoadout(PlayerBase player)
    {
        EntityAI item;
        EntityAI attachment;
        EntityAI cargo;
        
        EntityAI primaryWeapon;
        EntityAI pistol;
        EntityAI knife;
        EntityAI grenade;
        EntityAI bandage;
        EntityAI epi;
        EntityAI morphine;
        EntityAI saline;
        
        item = player.GetInventory().CreateInInventory("BDUPants");
        if (item)
        {
            bandage = item.GetInventory().CreateInInventory("BandageDressing");
            item.GetInventory().CreateInInventory("BandageDressing");
            item.GetInventory().CreateInInventory("BandageDressing");
            item.GetInventory().CreateInInventory("BandageDressing");
            epi = item.GetInventory().CreateInInventory("Epinephrine");
            morphine = item.GetInventory().CreateInInventory("Morphine");
            saline = item.GetInventory().CreateInInventory("SalineBagIV");
        }
        
        item = player.GetInventory().CreateInInventory("BDUJacket");
        if (item)
        {
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
        }
        
        item = player.GetInventory().CreateInInventory("UKAssVest_Camo");
        if (item)
        {
            grenade = item.GetInventory().CreateAttachment("M67Grenade");
            item.GetInventory().CreateAttachment("M67Grenade");
            
            item.GetInventory().CreateInInventory("Mag_CMAG_40Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_40Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
        }
        
        player.GetInventory().CreateInInventory("BallisticHelmet_BDU");
        
        item = player.GetInventory().CreateInInventory("MilitaryBoots_Black");
        if (item)
        {
            knife = item.GetInventory().CreateAttachment("CombatKnife");
        }
        
        item = player.GetInventory().CreateInInventory("MilitaryBelt");
        if (item)
        {
            attachment = item.GetInventory().CreateAttachment("PlateCarrierHolster_Green");
            if (attachment)
            {
                pistol = attachment.GetInventory().CreateAttachment("Glock19");
                if (pistol)
                {
                    pistol.GetInventory().CreateAttachment("Mag_Glock_15Rnd");
                }
            }
        }
        
        primaryWeapon = player.GetHumanInventory().CreateInHands("M16A2");
        if (primaryWeapon)
        {
            primaryWeapon.GetInventory().CreateAttachment("Mag_CMAG_40Rnd_Black");
        }
        
        SetPermanentArmband(player, "Armband_Blue");
        
        AssignToHotbar(player, primaryWeapon, 0);
        AssignToHotbar(player, pistol, 1);
        AssignToHotbar(player, knife, 2);
        AssignToHotbar(player, grenade, 3);
        AssignToHotbar(player, bandage, 4);
        AssignToHotbar(player, epi, 5);
        AssignToHotbar(player, morphine, 6);
        AssignToHotbar(player, saline, 7);
    }
    
    static void SetPermanentArmband(PlayerBase player, string armbandType)
    {
        EntityAI armband = player.GetInventory().CreateInInventory(armbandType);
        
        if (armband)
        {
            armband.SetAllowDamage(false);
            
            player.SetKOTHArmband(armband);
            
            if (armbandType == "Armband_Red")
                player.SetKOTHTeam("East");
            else if (armbandType == "Armband_Blue")
                player.SetKOTHTeam("West");
            
            Print("[KOTH_PlayerLoadout] Applied permanent " + armbandType + " to player");
        }
    }
    
    static void CheckAndRestoreArmband(PlayerBase player, string team)
    {
        EntityAI armband = player.GetKOTHArmband();
        
        if (!armband || !player.GetInventory().HasEntityInInventory(armband))
        {
            string armbandType = "Armband_Red";
            if (team == "West")
                armbandType = "Armband_Blue";
                
            SetPermanentArmband(player, armbandType);
        }
    }
    
    static void AssignToHotbar(PlayerBase player, EntityAI item, int slot)
    {
        if (!player || !item)
            return;
        
        if (slot < 0 || slot > 9)
            return;
        
        player.SetQuickBarEntityShortcut(item, slot);
        
        Print("[KOTH_PlayerLoadout] Assigned " + item.GetType() + " to hotbar slot " + (slot + 1));
    }
}