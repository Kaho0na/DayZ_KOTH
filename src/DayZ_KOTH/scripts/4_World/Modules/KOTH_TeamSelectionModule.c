/**
 * KOTH_TeamSelectionModule.c
 *
 * King of the Hill by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

[CF_RegisterModule(KOTH_TeamSelectionModule)]
class KOTH_TeamSelectionModule: CF_ModuleWorld
{

    void KOTH_TeamSelectionModule()
    {

    }

    override void OnInit()
    {
        super.OnInit();
        Print("[DayZ_KOTH] KOTH Module initialized");

		EnableClientNew();
		EnableClientReady();
		EnableInvokeConnect();
		EnableMissionFinish();
		EnableMissionStart();
		Expansion_EnableRPCManager();

        Expansion_RegisterClientRPC("RPC_SelectTeamMenu");
		Expansion_RegisterServerRPC("RPC_SelectTeam");
		Expansion_RegisterClientRPC("RPC_CloseTeamMenu");
        Expansion_RegisterServerRPC("RPC_RequestTeamChange");

    }

    void StartTeamSelection(PlayerBase player, PlayerIdentity identity)
    {
        if (!IsMissionHost())
            return;

        if (!player)
        {
            Print("[DayZ_KOTH] ERROR: PlayerBase is NULL in StartTeamSelection");
            return;
        }

        if (!identity)
        {
            Print("[DayZ_KOTH] ERROR: PlayerIdentity is NULL in StartTeamSelection");
            return;
        }

        Print("[DayZ_KOTH] Starting team selection for player: " + identity.GetName());
        string uid = identity.GetId();
        // Open the team selection menu on the client
        auto rpc = Expansion_CreateRPC("RPC_SelectTeamMenu");
        rpc.Write(uid);
        rpc.Expansion_Send(true, identity);
        
    }

    // Called on client
    private void RPC_SelectTeamMenu(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsClient())
            return;

        string uid;
        if (!ctx.Read(uid))
        {
            Print("[DayZ_KOTH] ERROR: Failed to read UID in RPC_SelectTeamMenu");
            return;
        }

        Print("[DayZ_KOTH] Opening team selection menu for UID: " + uid);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Exec_ShowTeamMenu, 1000, true);

    }

    private void Exec_ShowTeamMenu()
    {
        //! Return if game is not yet ready to show spawn menu
        if (GetDayZGame().IsLoading())
            return;

        if (GetGame().GetUIManager().GetMenu())
            return;

        if (GetDayZGame().GetExpansionGame().GetExpansionUIManager().GetMenu()) // ✅ Correct call
            return;

        if (GetDayZGame().GetMissionState() != DayZGame.MISSION_STATE_GAME)
            return;

        if (!GetGame().GetMission().GetHud())
            return;

        // Game ready to show menu
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CreateSVMenu("KOTH_TeamMenu");

        Print("[DayZ_KOTH] Team selection menu successfully opened!");
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Exec_ShowTeamMenu);
    }


    void SelectTeam(int team)
    {
        PlayerIdentity identity = GetGame().GetPlayer().GetIdentity();
        if (!identity)
        {
            Print("[DayZ_KOTH] ERROR: PlayerIdentity is NULL in SelectTeam");
            return;
        }

        string teamName;
        if (team == 1)
            teamName = "East";
        else if (team == 2)
            teamName = "West";
        else
        {
            Print("[DayZ_KOTH] ERROR: Invalid team selected: " + team);
            return;
        }

        Print("[DayZ_KOTH] Player " + identity.GetName() + " selected team: " + teamName);

        // ─────────────────────────────────────────────────────────────
        // CLIENT SIDE: send selection RPC to server
        // ─────────────────────────────────────────────────────────────
        if (!IsMissionHost()) // means this is running on a client
        {
            auto rpc = Expansion_CreateRPC("RPC_SelectTeam");
            rpc.Write(identity.GetId());
            rpc.Write(teamName);
            rpc.Expansion_Send(true);
            return;
        }
    }


    // Called on server
    private void RPC_SelectTeam(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        string uid;
        if (!ctx.Read(uid))
        {
            Print("[DayZ_KOTH] ERROR: Failed to read UID in RPC_SelectTeam");
            return;
        }

        string teamName;
        if (!ctx.Read(teamName))
        {
            Print("[DayZ_KOTH] ERROR: Failed to read team name in RPC_SelectTeam");
            return;
        }

        Exec_SelectTeam(sender, uid, teamName);
    }

    private void Exec_SelectTeam(PlayerIdentity sender, string uid, string teamName)
    {
        string playerUID = sender.GetId();
        PlayerBase player = PlayerBase.GetPlayerByUID(playerUID);
        if (!player)
        {
            Error(ToString() + "::Exec_SelectTeam - ERROR: Player with UID " + playerUID + " does not exist");
            return;
        }

        KOTH_ZoneData zone = KOTH_ZoneData.Load("Chernogorsk");  // TODO: Later make this dynamic per active zone
        if (!zone)
        {
            Error(ToString() + "::Exec_SelectTeam - ERROR: Failed to load zone data!");
            return;
        }

        // ───────────────────────────────────────────────
        //  Determine spawn position based on team
        // ───────────────────────────────────────────────
        vector spawnPos = "0 0 0";

        if (teamName == "East")
        {
            spawnPos = zone.GetEastSpawnBuilding();
        }
        else if (teamName == "West")
        {
            spawnPos = zone.GetWestSpawnBuilding();
        }
        else
        {
            Print("[DayZ_KOTH] ERROR: Invalid team name in Exec_SelectTeam: " + teamName);
            return;
        }

        // ───────────────────────────────────────────────
        //  Teleport player safely
        // ───────────────────────────────────────────────
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KOTH_SpawnUtils.SpawnPlayerAtPosition, 200, false, player, spawnPos);
        Print("[DayZ_KOTH] Spawning player " + sender.GetName() + " at " + teamName + " spawn: " + spawnPos);
        player.SetAllowDamage(true);

        // ───────────────────────────────────────────────
        //  Save player's team selection persistently
        // ───────────────────────────────────────────────
        KOTH_PlayerData playerData = KOTH_PlayerData.Load(uid);
        if (playerData)
        {
            playerData.SetLastTeamSelection(teamName);
            playerData.Save(uid);
            Print("[DayZ_KOTH] Saved team selection (" + teamName + ") for player " + sender.GetName());
        }
        else
        {
            Print("[DayZ_KOTH] WARNING: Could not load player data for UID " + uid);
        }
    }

    // ───────────────────────────────────────────────
    // Client calls this to ask the server to open team menu
    // ───────────────────────────────────────────────
    private void RPC_RequestTeamChange(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        Print("[DayZ_KOTH] RPC_RequestTeamChange received on server!");

        if (!sender)
            return;

        PlayerBase player = PlayerBase.GetPlayerByUID(sender.GetId());
        if (!player)
            return;

        StartTeamSelection(player, sender);
    }


    void CloseTeamMenu()
    {
        if (!GetGame().IsClient())
            return;

        auto rpc = Expansion_CreateRPC("RPC_CloseTeamMenu");
        rpc.Expansion_Send(true);
    }

    // Called on client
    private void RPC_CloseTeamMenu(PlayerIdentity sender, Object target, ParamsReadContext ctx)
    {
        if (!GetGame().IsClient())
            return;

        Print("[DayZ_KOTH] Closing team selection menu on client");
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
    }
}