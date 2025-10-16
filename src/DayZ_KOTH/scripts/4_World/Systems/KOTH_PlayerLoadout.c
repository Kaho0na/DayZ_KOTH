/**
 * KOTH_PlayerLoadout.c
 *
 * King of the Hill by Kahoona
 * Handles player loadouts based on team selection
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 */

// ═══════════════════════════════════════════════════════════════
// MODDED PLAYERBASE CLASS - MUST BE DEFINED FIRST
// ═══════════════════════════════════════════════════════════════

modded class PlayerBase
{
    private EntityAI m_KOTHArmband;
    private string m_KOTHTeam = "";
    
    void SetKOTHArmband(EntityAI armband)
    {
        m_KOTHArmband = armband;
    }
    
    EntityAI GetKOTHArmband()
    {
        return m_KOTHArmband;
    }
    
    void SetKOTHTeam(string team)
    {
        m_KOTHTeam = team;
    }
    
    string GetKOTHTeam()
    {
        return m_KOTHTeam;
    }
    
    // Prevent armband from being dropped
    override bool CanDropEntity(notnull EntityAI item)
    {
        if (item && (item.IsKindOf("Armband_Red") || item.IsKindOf("Armband_Blue")))
        {
            if (m_KOTHTeam == "East" || m_KOTHTeam == "West")
            {
                return false;
            }
        }
        
        return super.CanDropEntity(item);
    }
}

// ═══════════════════════════════════════════════════════════════
// KOTH PLAYER LOADOUT CLASS
// ═══════════════════════════════════════════════════════════════

class KOTH_PlayerLoadout
{
    //! ═══════════════════════════════════════════════════════════════
    //! MAIN LOADOUT FUNCTION
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetPlayerLoadout(PlayerBase player, string team)
    {
        if (!player)
            return;
        
        // Clear existing inventory first
        ClearPlayerInventory(player);
        
        // Set player stats to full
        SetPlayerStats(player);
        
        // Apply team-specific loadout
        if (team == "East")
        {
            SetEastLoadout(player);
        }
        else if (team == "West")
        {
            SetWestLoadout(player);
        }
        
        Print("[KOTH_PlayerLoadout] Applied " + team + " loadout to player: " + player.GetIdentity().GetName());
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! CLEAR INVENTORY
    //! ═══════════════════════════════════════════════════════════════
    
    static void ClearPlayerInventory(PlayerBase player)
    {
        // Remove all items except what player is wearing
        player.RemoveAllItems();
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! SET PLAYER STATS
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetPlayerStats(PlayerBase player)
    {
        // Set health to full
        player.SetHealth("", "", player.GetMaxHealth("", ""));
        
        // Set blood to full
        player.SetHealth("", "Blood", player.GetMaxHealth("", "Blood"));
        
        // IMPORTANT: Set shock to MAX (not zero!) to prevent unconsciousness
        player.SetHealth("", "Shock", player.GetMaxHealth("", "Shock"));
        
        // Remove all diseases
        player.m_AgentPool.RemoveAllAgents();
        
        // Set food and water to full
        player.GetStatWater().Set(player.GetStatWater().GetMax());
        player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
        
        // Set temperature to normal
        player.GetStatHeatComfort().Set(0);
        
        // Remove any injuries
        player.m_BleedingManagerServer.RemoveAllSources();
        
        // Wake up player if unconscious
        if (player.IsUnconscious())
        {
            player.SetHealth("", "Shock", 100);
            DayZPlayerSyncJunctures.SendPlayerUnconsciousness(player, false);
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! EAST TEAM LOADOUT
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetEastLoadout(PlayerBase player)
    {
        EntityAI item;
        EntityAI attachment;
        EntityAI cargo;
        
        // Items to assign to hotbar
        EntityAI primaryWeapon;
        EntityAI pistol;
        EntityAI knife;
        EntityAI grenade;
        EntityAI bandage;
        EntityAI epi;
        EntityAI morphine;
        EntityAI saline;
        
        // ─────────────────────────────────────────────────────
        // CLOTHING
        // ─────────────────────────────────────────────────────
        
        // Pants - TTSKOPants
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
        
        // Jacket - TTsKOJacket_Camo
        item = player.GetInventory().CreateInInventory("TTsKOJacket_Camo");
        if (item)
        {
            knife = item.GetInventory().CreateInInventory("CombatKnife");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_30Rnd");
        }
        
        // Boots - TTSKOBoots
        player.GetInventory().CreateInInventory("TTSKOBoots");
        
        // Vest - SmershVest
        item = player.GetInventory().CreateInInventory("SmershVest");
        if (item)
        {
            // Attach grenades
            grenade = item.GetInventory().CreateAttachment("RGD5Grenade");
            item.GetInventory().CreateAttachment("RGD5Grenade");
            
            // Add magazines to vest
            item.GetInventory().CreateInInventory("Mag_AK74_45Rnd");
            item.GetInventory().CreateInInventory("Mag_AK74_45Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
            item.GetInventory().CreateInInventory("Mag_CZ75_15Rnd");
        }
        
        // Helmet - Ssh68Helmet
        player.GetInventory().CreateInInventory("Ssh68Helmet");
        
        // Belt - MilitaryBelt with holster and pistol
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
        
        // ─────────────────────────────────────────────────────
        // PRIMARY WEAPON - AKS74U
        // ─────────────────────────────────────────────────────
        
        primaryWeapon = player.GetHumanInventory().CreateInHands("AKS74U");
        if (primaryWeapon)
        {
            primaryWeapon.GetInventory().CreateAttachment("AKS74U_Bttstck");
            primaryWeapon.GetInventory().CreateAttachment("Mag_AK74_45Rnd");
        }
        
        // ─────────────────────────────────────────────────────
        // ARMBAND - Red (permanent)
        // ─────────────────────────────────────────────────────
        
        SetPermanentArmband(player, "Armband_Red");
        
        // ─────────────────────────────────────────────────────
        // ASSIGN HOTBAR (QUICKBAR)
        // ─────────────────────────────────────────────────────
        
        AssignToHotbar(player, primaryWeapon, 0);  // Slot 1 - Primary weapon
        AssignToHotbar(player, pistol, 1);          // Slot 2 - Pistol
        AssignToHotbar(player, knife, 2);           // Slot 3 - Knife
        AssignToHotbar(player, grenade, 3);         // Slot 4 - Grenade
        AssignToHotbar(player, bandage, 4);         // Slot 5 - Bandage
        AssignToHotbar(player, epi, 5);             // Slot 6 - Epinephrine
        AssignToHotbar(player, morphine, 6);        // Slot 7 - Morphine
        AssignToHotbar(player, saline, 7);          // Slot 8 - Saline Bag
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! WEST TEAM LOADOUT
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetWestLoadout(PlayerBase player)
    {
        EntityAI item;
        EntityAI attachment;
        EntityAI cargo;
        
        // Items to assign to hotbar
        EntityAI primaryWeapon;
        EntityAI pistol;
        EntityAI knife;
        EntityAI grenade;
        EntityAI bandage;
        EntityAI epi;
        EntityAI morphine;
        EntityAI saline;
        
        // ─────────────────────────────────────────────────────
        // CLOTHING
        // ─────────────────────────────────────────────────────
        
        // Pants - BDUPants
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
        
        // Jacket - BDUJacket
        item = player.GetInventory().CreateInInventory("BDUJacket");
        if (item)
        {
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_30Rnd_Black");
        }
        
        // Vest - USAssVest_Camo
        item = player.GetInventory().CreateInInventory("UKAssVest_Camo");
        if (item)
        {
            // Attach grenades
            grenade = item.GetInventory().CreateAttachment("M67Grenade");
            item.GetInventory().CreateAttachment("M67Grenade");
            
            // Add magazines to vest
            item.GetInventory().CreateInInventory("Mag_CMAG_40Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_CMAG_40Rnd_Black");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
            item.GetInventory().CreateInInventory("Mag_Glock_15Rnd");
        }
        
        // Helmet - BallisticHelmet
        player.GetInventory().CreateInInventory("BallisticHelmet_BDU");
        
        // Boots - MilitaryBoots_Black with knife
        item = player.GetInventory().CreateInInventory("MilitaryBoots_Black");
        if (item)
        {
            knife = item.GetInventory().CreateAttachment("CombatKnife");
        }
        
        // Belt - MilitaryBelt with holster and pistol
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
        
        // ─────────────────────────────────────────────────────
        // PRIMARY WEAPON - M16A2
        // ─────────────────────────────────────────────────────
        
        primaryWeapon = player.GetHumanInventory().CreateInHands("M16A2");
        if (primaryWeapon)
        {
            primaryWeapon.GetInventory().CreateAttachment("Mag_CMAG_40Rnd_Black");
        }
        
        // ─────────────────────────────────────────────────────
        // ARMBAND - Blue (permanent)
        // ─────────────────────────────────────────────────────
        
        SetPermanentArmband(player, "Armband_Blue");
        
        // ─────────────────────────────────────────────────────
        // ASSIGN HOTBAR (QUICKBAR)
        // ─────────────────────────────────────────────────────
        
        AssignToHotbar(player, primaryWeapon, 0);  // Slot 1 - Primary weapon
        AssignToHotbar(player, pistol, 1);          // Slot 2 - Pistol
        AssignToHotbar(player, knife, 2);           // Slot 3 - Knife
        AssignToHotbar(player, grenade, 3);         // Slot 4 - Grenade
        AssignToHotbar(player, bandage, 4);         // Slot 5 - Bandage
        AssignToHotbar(player, epi, 5);             // Slot 6 - Epinephrine
        AssignToHotbar(player, morphine, 6);        // Slot 7 - Morphine
        AssignToHotbar(player, saline, 7);          // Slot 8 - Saline Bag
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! PERMANENT ARMBAND SYSTEM
    //! ═══════════════════════════════════════════════════════════════
    
    static void SetPermanentArmband(PlayerBase player, string armbandType)
    {
        // Create armband
        EntityAI armband = player.GetInventory().CreateInInventory(armbandType);
        
        if (armband)
        {
            // Lock the armband so it can't be removed
            armband.SetAllowDamage(false);
            
            // Store armband reference and team for later checks
            player.SetKOTHArmband(armband);
            
            // Set the player's team
            if (armbandType == "Armband_Red")
                player.SetKOTHTeam("East");
            else if (armbandType == "Armband_Blue")
                player.SetKOTHTeam("West");
            
            Print("[KOTH_PlayerLoadout] Applied permanent " + armbandType + " to player");
        }
    }
    
    //! Check and restore armband if removed (call this periodically)
    static void CheckAndRestoreArmband(PlayerBase player, string team)
    {
        EntityAI armband = player.GetKOTHArmband();
        
        if (!armband || !player.GetInventory().HasEntityInInventory(armband))
        {
            // Armband was removed, restore it
            string armbandType = "Armband_Red";
            if (team == "West")
                armbandType = "Armband_Blue";
                
            SetPermanentArmband(player, armbandType);
        }
    }
    
    //! ═══════════════════════════════════════════════════════════════
    //! HOTBAR ASSIGNMENT
    //! ═══════════════════════════════════════════════════════════════
    
    static void AssignToHotbar(PlayerBase player, EntityAI item, int slot)
    {
        if (!player || !item)
            return;
        
        // Slot index is 0-based (0-9 for slots 1-10)
        if (slot < 0 || slot > 9)
            return;
        
        // Assign item to quickbar slot
        player.SetQuickBarEntityShortcut(item, slot);
        
        Print("[KOTH_PlayerLoadout] Assigned " + item.GetType() + " to hotbar slot " + (slot + 1));
    }
}