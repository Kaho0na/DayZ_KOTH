/**
 * KOTH_Armbands.c
 * 
 * Modded armband classes that can't be removed when on a KOTH team
 * Place in: 4_World/Entities/KOTH_Armbands.c
 */

modded class Armband_Red
{
    override bool CanPutInCargo(EntityAI parent)
    {
        // Check if the player wearing this is on a KOTH team
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (player && player.GetKOTHTeam() == "East")
        {
            return false; // Can't put in cargo if on East team
        }
        
        return super.CanPutInCargo(parent);
    }
    
    override bool CanRemoveFromCargo(EntityAI parent)
    {
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (player && player.GetKOTHTeam() == "East")
        {
            return false;
        }
        
        return super.CanRemoveFromCargo(parent);
    }
    
    override bool CanPutIntoHands(EntityAI parent)
    {
        PlayerBase player = PlayerBase.Cast(parent);
        if (player && player.GetKOTHTeam() == "East")
        {
            return false; // Can't take in hands if on East team
        }
        
        return super.CanPutIntoHands(parent);
    }
    
    override bool CanReleaseCargo(EntityAI cargo)
    {
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (player && player.GetKOTHTeam() == "East")
        {
            return false;
        }
        
        return super.CanReleaseCargo(cargo);
    }
}

modded class Armband_Blue
{
    override bool CanPutInCargo(EntityAI parent)
    {
        // Check if the player wearing this is on a KOTH team
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (player && player.GetKOTHTeam() == "West")
        {
            return false; // Can't put in cargo if on West team
        }
        
        return super.CanPutInCargo(parent);
    }
    
    override bool CanRemoveFromCargo(EntityAI parent)
    {
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (player && player.GetKOTHTeam() == "West")
        {
            return false;
        }
        
        return super.CanRemoveFromCargo(parent);
    }
    
    override bool CanPutIntoHands(EntityAI parent)
    {
        PlayerBase player = PlayerBase.Cast(parent);
        if (player && player.GetKOTHTeam() == "West")
        {
            return false; // Can't take in hands if on West team
        }
        
        return super.CanPutIntoHands(parent);
    }
    
    override bool CanReleaseCargo(EntityAI cargo)
    {
        PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
        if (player && player.GetKOTHTeam() == "West")
        {
            return false;
        }
        
        return super.CanReleaseCargo(cargo);
    }
}