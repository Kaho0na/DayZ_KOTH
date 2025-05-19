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
     static ref ScriptInvoker SI_DeadDrop = new ScriptInvoker();
 
     override void Init()
     {
 #ifdef EXPANSIONTRACE
         auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "Init");
 #endif
 
        super.Init();

        Init(ExpansionDeadDropSettings, true);
     }
 
     void RPC_DeadDropSettings(PlayerIdentity sender, Object target, ParamsReadContext ctx)
     {
         Receive(ExpansionDeadDropSettings, ctx);
     }
 
     ExpansionDeadDropSettings GetDeadDrop(bool checkLoaded = true)
     {
         return ExpansionDeadDropSettings.Cast(Get(ExpansionDeadDropSettings, checkLoaded));
     }
 };
 