/**
 * ExpansionSettings.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
 */

 modded class ExpansionSettings
 {
     static ref ScriptInvoker SI_DayZ_KOTH = new ScriptInvoker();
 
     override void Init()
     {
 #ifdef EXPANSIONTRACE
         auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "Init");
 #endif
        super.Init();
        Init(DayZ_KOTH_Settings, true);
     }
 
     void RPC_DayZ_KOTH_Settings(PlayerIdentity sender, Object target, ParamsReadContext ctx)
     {
         Receive(DayZ_KOTH_Settings, ctx);
     }
 
     DayZ_KOTH_Settings GetDayZ_KOTH(bool checkLoaded = true)
     {
         return DayZ_KOTH_Settings.Cast(Get(DayZ_KOTH_Settings, checkLoaded));
     }
 };
 