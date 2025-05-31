/**
 * ExpansionFactionBankSettings.c
 *
 * DayZ Expansion Mod - Faction Bank by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionFactionBankSettingsBase: ExpansionSettingBase
{
    bool FactionBankEnabled = true;
    bool FactionBankEnableReputation = true;

}

class ExpansionFactionBankSettings: ExpansionFactionBankSettingsBase
{
    static const int VERSION = 1;
    int FactionBankBalance;
	int SalaryTimerInterval;
	int BaseSalaryAmount;
	int MaxSalaryCap;
	float SalaryMultiplier;


	[NonSerialized()]
	private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
	{
		//TraderPrint("OnRecieve - Start");
		
		ExpansionFactionBankSettings s = new ExpansionFactionBankSettings;

		ctx.Read(s.FactionBankEnabled);
		ctx.Read(s.FactionBankEnableReputation);

		ctx.Read(s.FactionBankBalance);
		ctx.Read(s.SalaryTimerInterval);
		ctx.Read(s.BaseSalaryAmount);
		ctx.Read(s.MaxSalaryCap);
		ctx.Read(s.SalaryMultiplier);

		CopyInternal(s);
		
		m_IsLoaded = true;
		
		ExpansionSettings.SI_FactionBank.Invoke();

		return true;
	}

    override void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(FactionBankEnabled);
		ctx.Write(FactionBankEnableReputation);

		ctx.Write(FactionBankBalance);
		ctx.Write(SalaryTimerInterval);
		ctx.Write(BaseSalaryAmount);
		ctx.Write(MaxSalaryCap);
		ctx.Write(SalaryMultiplier);

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
		ExpansionFactionBankSettings s;
		if (!Class.CastTo(s, setting))
			return false;

		CopyInternal(s);
		return true;
	}

    protected void CopyInternal( ExpansionFactionBankSettings s )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
	#endif
		FactionBankBalance = s.FactionBankBalance;
		SalaryTimerInterval = s.SalaryTimerInterval;
		BaseSalaryAmount = s.BaseSalaryAmount;
		MaxSalaryCap = s.MaxSalaryCap;
		SalaryMultiplier = s.SalaryMultiplier;

		ExpansionFactionBankSettings sb = s;
		CopyInternal( sb );
	}

    private void CopyInternal( ExpansionFactionBankSettingsBase s)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
#endif

        FactionBankEnabled = s.FactionBankEnabled;
        FactionBankEnableReputation = s.FactionBankEnableReputation;

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

		bool factionbankSettingsExist = FileExist(EXPANSION_FACTIONBANK_SETTINGS);

		if (factionbankSettingsExist)
		{
			CF_Log.Info("[ExpansionFactionBankSettings] Load existing setting file:" + EXPANSION_FACTIONBANK_SETTINGS);

			ExpansionFactionBankSettings settingsDefault = new ExpansionFactionBankSettings;
			settingsDefault.Defaults();

			ExpansionFactionBankSettingsBase settingsBase;

			JsonFileLoader<ExpansionFactionBankSettingsBase>.JsonLoadFile(EXPANSION_FACTIONBANK_SETTINGS, settingsBase);
			JsonFileLoader<ExpansionFactionBankSettings>.JsonLoadFile(EXPANSION_FACTIONBANK_SETTINGS, this);
			
		}
		else
		{
			CF_Log.Info("[ExpansionFactionBankSettings] No existing setting file:" + EXPANSION_FACTIONBANK_SETTINGS + ". Creating defaults!");

			Defaults();
			save = true;
		}

		if (save)
		{
			Save();
		}

		return factionbankSettingsExist;
	}

	override bool OnSave()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnSave");
	#endif

		JsonFileLoader<ExpansionFactionBankSettings>.JsonSaveFile( EXPANSION_FACTIONBANK_SETTINGS, this );

		return true;
	}

	override void Update( ExpansionSettingBase setting )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Update").Add(setting);
	#endif

		super.Update( setting );

		ExpansionSettings.SI_FactionBank.Invoke();
	}
	
    override void Defaults()
	{
		m_Version = VERSION;

		FactionBankEnabled = true;
		FactionBankEnableReputation = true;
		FactionBankBalance = 0;
		SalaryTimerInterval = 60; // 1 minute
		BaseSalaryAmount = 100; // Base salary amount;
		MaxSalaryCap = 100000; // Maximum salary cap
		SalaryMultiplier = 1.0; // Multiplier for salary calculation
	}

    override string SettingName()
	{
		return "FactionBank Settings";
	}
}