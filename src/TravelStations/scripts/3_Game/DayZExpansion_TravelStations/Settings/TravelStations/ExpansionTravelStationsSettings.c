/**
 * ExpansionTravelStationsSettings.c
 *
 * DayZ Expansion Mod - City Loyalty by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionTravelStationsSettingsBase: ExpansionSettingBase
{
    bool TravelStationsEnabled = true;
	float TravelCostPerMeter = 0.5;
	bool RequireLiberationToTravel = true;
	bool ShowStationsOnMap = true;
	bool BlackoutEffectEnabled = true;
	int CooldownSeconds = 180;
	

}

class ExpansionTravelStationsSettings: ExpansionTravelStationsSettingsBase
{
    static const int VERSION = 1;

	[NonSerialized()]
	private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
	{
		//TraderPrint("OnRecieve - Start");
		
		ExpansionTravelStationsSettings s = new ExpansionTravelStationsSettings;

		ctx.Read(s.TravelStationsEnabled);
		ctx.Read(s.TravelCostPerMeter);
		ctx.Read(s.RequireLiberationToTravel);
		ctx.Read(s.ShowStationsOnMap);
		ctx.Read(s.BlackoutEffectEnabled);
		ctx.Read(s.CooldownSeconds);
	
		CopyInternal(s);
		
		m_IsLoaded = true;
		
		ExpansionSettings.SI_TravelStations.Invoke();

		return true;
	}

    override void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(TravelStationsEnabled);
		ctx.Write(TravelCostPerMeter);
		ctx.Write(RequireLiberationToTravel);
		ctx.Write(ShowStationsOnMap);
		ctx.Write(BlackoutEffectEnabled);
		ctx.Write(CooldownSeconds);

	}

    	// ------------------------------------------------------------
	override int Send(PlayerIdentity identity)
	{
		
		if (!IsMissionHost())
		{
			return 0;
		}
		
		auto rpc = CreateRPC();
		OnSend(rpc);
		rpc.Expansion_Send(true, identity);
		
		return 0;
	}

    override bool Copy(ExpansionSettingBase setting)
	{
		ExpansionTravelStationsSettings s;
		if (!Class.CastTo(s, setting))
			return false;

		CopyInternal(s);
		return true;
	}

    protected void CopyInternal( ExpansionTravelStationsSettings s )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
	#endif


		ExpansionTravelStationsSettings sb = s;
		CopyInternal( sb );
	}

    private void CopyInternal( ExpansionTravelStationsSettingsBase s)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
#endif

        TravelStationsEnabled = s.TravelStationsEnabled;
		TravelCostPerMeter = s.TravelCostPerMeter;
		RequireLiberationToTravel = s.RequireLiberationToTravel;
		ShowStationsOnMap = s.ShowStationsOnMap;
		BlackoutEffectEnabled = s.BlackoutEffectEnabled;
		CooldownSeconds = s.CooldownSeconds;


	}

	override bool IsLoaded()
	{
		return m_IsLoaded;
	}
	
	override void Unload()
	{
		m_IsLoaded = false;
	}

	override bool OnLoad()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnLoad");
	#endif

		m_IsLoaded = true;

		bool save;

		bool TravelStationsSettingsExist = FileExist(EXPANSION_TRAVELSTATIONS_SETTINGS);

		if (TravelStationsSettingsExist)
		{
			CF_Log.Info("[ExpansionTravelStationsSettings] Load existing setting file:" + EXPANSION_TRAVELSTATIONS_SETTINGS);

			ExpansionTravelStationsSettings settingsDefault = new ExpansionTravelStationsSettings;
			settingsDefault.Defaults();

			ExpansionTravelStationsSettingsBase settingsBase;

			JsonFileLoader<ExpansionTravelStationsSettingsBase>.JsonLoadFile(EXPANSION_TRAVELSTATIONS_SETTINGS, settingsBase);
			JsonFileLoader<ExpansionTravelStationsSettings>.JsonLoadFile(EXPANSION_TRAVELSTATIONS_SETTINGS, this);
			
		}
		else
		{
			CF_Log.Info("[ExpansionTravelStationsSettings] No existing setting file:" + EXPANSION_TRAVELSTATIONS_SETTINGS + ". Creating defaults!");

			Defaults();
			save = true;
		}

		if (save)
		{
			Save();
		}

		return TravelStationsSettingsExist;
	}

	override bool OnSave()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnSave");
	#endif

		JsonFileLoader<ExpansionTravelStationsSettings>.JsonSaveFile( EXPANSION_TRAVELSTATIONS_SETTINGS, this );

		return true;
	}

	override void Update( ExpansionSettingBase setting )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Update").Add(setting);
	#endif

		super.Update( setting );

		ExpansionSettings.SI_TravelStations.Invoke();
	}
	
    override void Defaults()
	{
		m_Version = VERSION;
		TravelStationsEnabled = true;
		TravelCostPerMeter = 0.5;
		RequireLiberationToTravel = true;
		ShowStationsOnMap = true;
		BlackoutEffectEnabled = true;
		CooldownSeconds = 180;
	}

    override string SettingName()
	{
		return "TravelStations Settings";
	}
}