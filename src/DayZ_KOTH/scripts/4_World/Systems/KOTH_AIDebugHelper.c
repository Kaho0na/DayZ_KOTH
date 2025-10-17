/**
 * KOTH_AIDebugHelper.c
 *
 * Debug helper to understand Expansion AI structure
 * Place in: 4_World/Systems/KOTH_AIDebugHelper.c
 * 
 * Call this from MissionServer.OnInit() to test AI detection
 */

class KOTH_AIDebugHelper
{
    static void DebugAllEntitiesInGame()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
        Print("[KOTH_AIDebug] Starting AI Entity Detection");
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
        
        array<Object> allObjects = new array<Object>();
        GetGame().GetObjectsAtPosition(GetGame().GetPlayer().GetPosition(), 5000, allObjects, null);
        
        int aiCount = 0;
        int playerCount = 0;
        
        for (int i = 0; i < allObjects.Count(); i++)
        {
            Object obj = allObjects.Get(i);
            
            if (!obj || !obj.IsAlive())
                continue;
            
            PlayerBase player;
            eAIBase ai;
            DayZPlayerImplement dzPlayer;
            
            if (Class.CastTo(player, obj))
            {
                playerCount++;
                Print("[KOTH_AIDebug] Found PlayerBase: " + player.GetType());
                
                if (player.GetIdentity())
                {
                    Print("[KOTH_AIDebug]   - Has Identity: " + player.GetIdentity().GetName());
                    Print("[KOTH_AIDebug]   - KOTH Team: " + player.GetKOTHTeam());
                }
                else
                {
                    Print("[KOTH_AIDebug]   - NO Identity (might be AI)");
                }
            }
            
            if (Class.CastTo(ai, obj))
            {
                aiCount++;
                Print("[KOTH_AIDebug] Found eAIBase: " + ai.GetType());
                Print("[KOTH_AIDebug]   - Display Name: " + ai.GetDisplayName());
                Print("[KOTH_AIDebug]   - Is Alive: " + ai.IsAlive());
                
                eAIGroup group = ai.GetGroup();
                if (group)
                {
                    Print("[KOTH_AIDebug]   - Has Group: YES");
                    
                    eAIFaction faction = group.GetFaction();
                    if (faction)
                    {
                        Print("[KOTH_AIDebug]   - Faction Name: " + faction.GetName());
                        Print("[KOTH_AIDebug]   - Faction Type: " + faction.Type());
                    }
                    else
                    {
                        Print("[KOTH_AIDebug]   - NO FACTION!");
                    }
                }
                else
                {
                    Print("[KOTH_AIDebug]   - NO GROUP!");
                }
            }
            
            if (Class.CastTo(dzPlayer, obj))
            {
                Print("[KOTH_AIDebug] Found DayZPlayerImplement: " + dzPlayer.GetType());
                
                if (!dzPlayer.GetIdentity())
                {
                    Print("[KOTH_AIDebug]   - NO Identity (AI candidate)");
                }
            }
        }
        
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
        Print("[KOTH_AIDebug] Summary:");
        Print("[KOTH_AIDebug] - PlayerBase entities: " + playerCount);
        Print("[KOTH_AIDebug] - eAIBase entities: " + aiCount);
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
    }
    
    static void DebugTriggerInsiders(Object trigger)
    {
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
        Print("[KOTH_AIDebug] Checking Trigger Insiders");
        
        Trigger trig = Trigger.Cast(trigger);
        if (!trig)
        {
            Print("[KOTH_AIDebug] ERROR: Not a valid trigger!");
            return;
        }
        
        int insiderCount = trig.GetInsiders().Count();
        Print("[KOTH_AIDebug] Total insiders: " + insiderCount);
        
        for (int i = 0; i < insiderCount; i++)
        {
            Object insider = trig.GetInsiders().Get(i).GetObject();
            if (!insider)
                continue;
            
            Print("[KOTH_AIDebug] Insider " + i + ": " + insider.GetType());
            Print("[KOTH_AIDebug]   - Is Alive: " + insider.IsAlive());
            
            PlayerBase player;
            eAIBase ai;
            
            if (Class.CastTo(player, insider))
            {
                Print("[KOTH_AIDebug]   - Cast to PlayerBase: SUCCESS");
                if (player.GetIdentity())
                {
                    Print("[KOTH_AIDebug]   - Has Identity (Real Player)");
                }
                else
                {
                    Print("[KOTH_AIDebug]   - NO Identity (AI?)");
                }
            }
            
            if (Class.CastTo(ai, insider))
            {
                Print("[KOTH_AIDebug]   - Cast to eAIBase: SUCCESS");
                
                eAIGroup group = ai.GetGroup();
                if (group)
                {
                    eAIFaction faction = group.GetFaction();
                    if (faction)
                    {
                        Print("[KOTH_AIDebug]   - Faction: " + faction.GetName());
                    }
                }
            }
        }
        
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
    }
    
    static void TestAIInZone()
    {
        if (!GetGame().IsServer())
            return;
        
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
        Print("[KOTH_AIDebug] Testing AI in Zone");
        
        KOTH_Area mainZone = KOTH_Area.GetInstance();
        if (!mainZone)
        {
            Print("[KOTH_AIDebug] ERROR: No main zone instance!");
            return;
        }
        
        KOTH_AreaTrigger trigger = mainZone.GetMainTrigger();
        if (!trigger)
        {
            Print("[KOTH_AIDebug] ERROR: No trigger instance!");
            return;
        }
        
        array<PlayerBase> players = trigger.GetPlayersInside();
        array<PlayerBase> aiEntities = trigger.GetAIInside();
        
        Print("[KOTH_AIDebug] Players in zone: " + players.Count());
        Print("[KOTH_AIDebug] AI in zone: " + aiEntities.Count());
        
        int eastPlayers = trigger.GetTeamPlayerCount("East");
        int westPlayers = trigger.GetTeamPlayerCount("West");
        int eastAI = trigger.GetTeamAICount("East");
        int westAI = trigger.GetTeamAICount("West");
        
        Print("[KOTH_AIDebug] East Players: " + eastPlayers);
        Print("[KOTH_AIDebug] West Players: " + westPlayers);
        Print("[KOTH_AIDebug] East AI: " + eastAI);
        Print("[KOTH_AIDebug] West AI: " + westAI);
        
        Print("[KOTH_AIDebug] ═══════════════════════════════════════");
    }
}