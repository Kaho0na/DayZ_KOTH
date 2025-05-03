/**
 * ExpansionCityManagerSettings.c
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

class ExpansionCityManagerSettingsBase: ExpansionSettingBase
{
    bool CityManagerEnabled = true;

}

class ExpansionCityManagerSettings: ExpansionCityManagerSettingsBase
{
    static const int VERSION = 1;

	[NonSerialized()]
	private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
	{
		//TraderPrint("OnRecieve - Start");
		
		ExpansionCityManagerSettings s = new ExpansionCityManagerSettings;

		ctx.Read(s.CityManagerEnabled);
	
		CopyInternal(s);
		
		m_IsLoaded = true;
		
		ExpansionSettings.SI_CityManager.Invoke();

		return true;
	}

    override void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(CityManagerEnabled);

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
		ExpansionCityManagerSettings s;
		if (!Class.CastTo(s, setting))
			return false;

		CopyInternal(s);
		return true;
	}

    protected void CopyInternal( ExpansionCityManagerSettings s )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
	#endif


		ExpansionCityManagerSettings sb = s;
		CopyInternal( sb );
	}

    private void CopyInternal( ExpansionCityManagerSettingsBase s)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
#endif

        CityManagerEnabled = s.CityManagerEnabled;


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

		bool CityManagerSettingsExist = FileExist(EXPANSION_CITYMANAGER_SETTINGS);

		if (CityManagerSettingsExist)
		{
			CF_Log.Info("[ExpansionCityManagerSettings] Load existing setting file:" + EXPANSION_CITYMANAGER_SETTINGS);

			ExpansionCityManagerSettings settingsDefault = new ExpansionCityManagerSettings;
			settingsDefault.Defaults();

			ExpansionCityManagerSettingsBase settingsBase;

			JsonFileLoader<ExpansionCityManagerSettingsBase>.JsonLoadFile(EXPANSION_CITYMANAGER_SETTINGS, settingsBase);
			JsonFileLoader<ExpansionCityManagerSettings>.JsonLoadFile(EXPANSION_CITYMANAGER_SETTINGS, this);
			
		}
		else
		{
			CF_Log.Info("[ExpansionCityManagerSettings] No existing setting file:" + EXPANSION_CITYMANAGER_SETTINGS + ". Creating defaults!");

			Defaults();
			save = true;
		}

		if (save)
		{
			Save();
		}

		return CityManagerSettingsExist;
	}

	override bool OnSave()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnSave");
	#endif

		JsonFileLoader<ExpansionCityManagerSettings>.JsonSaveFile( EXPANSION_CITYMANAGER_SETTINGS, this );

		return true;
	}

	override void Update( ExpansionSettingBase setting )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Update").Add(setting);
	#endif

		super.Update( setting );

		ExpansionSettings.SI_CityManager.Invoke();
	}
	
    override void Defaults()
	{
		m_Version = VERSION;

		CityManagerEnabled = true;

	}

    override string SettingName()
	{
		return "CityManager Settings";
	}
}