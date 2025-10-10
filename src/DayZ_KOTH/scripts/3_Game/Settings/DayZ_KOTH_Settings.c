/**
 * DayZ_KOTH_Settings.c
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

class DayZ_KOTH_SettingsBase: ExpansionSettingBase
{
    // ────────────── META ──────────────
    string ModeName = "King of the Hill";


    // ────────────── CORE GAMEPLAY ──────────────
    int ScoreLimit = 100;
    int CaptureTickSeconds = 1;
    int MinPlayersToInfluence = 1;
    float PointsPerTickPerPlayer = 0.25;
    float NeutralizeSpeedMultiplier = 1.0;

    // ────────────── EXPERIENCE SYSTEM ──────────────
    int XPPerKill = 100;
    int XPPerRevive = 50;
    int XPPerAssist = 25;
    int XPPerCapture = 250;
    int FriendlyFirePenaltyXP = -150;
    int SuicidePenaltyXP = -100;
    int HeadShotBonusXP = 50;

    bool KillStreakBonusXPEnabled = true;
    ref array<int> KillStreakThresholds = { 5, 10, 15 };
    ref array<int> KillStreakBonusXP = { 100, 200, 300 };

    float GlobalXPMultiplier = 1.0;

    // ────────────── MONEY SYSTEM ──────────────
    int MoneyPerKill = 100;
    int MoneyPerRevive = 50;
    int MoneyPerAssist = 25;
    int MoneyPerCapture = 300;
    int TeamKillMoneyPenalty = -200;
    int SuicideMoneyPenalty = -100;
    int HeadShotMoneyBonus = 50;

    bool KillStreakMoneyBonusEnabled = true;
    ref array<int> KillStreakMoneyThresholds = { 5, 10, 15 };
    ref array<int> KillStreakBonusMoney = { 100, 200, 300 };

    float GlobalMoneyMultiplier = 1.0;

    // ────────────── BALANCE / FAIRNESS ──────────────
    int MaxTeamImbalance = 3;
}

class DayZ_KOTH_Settings: DayZ_KOTH_SettingsBase
{

	[NonSerialized()]
	private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
	{
		
		DayZ_KOTH_Settings s = new DayZ_KOTH_Settings;

		ctx.Read(s.ModeName);
        ctx.Read(s.ScoreLimit);
        ctx.Read(s.CaptureTickSeconds);
        ctx.Read(s.MinPlayersToInfluence);
        ctx.Read(s.PointsPerTickPerPlayer);
        ctx.Read(s.NeutralizeSpeedMultiplier);
        ctx.Read(s.XPPerKill);
        ctx.Read(s.XPPerRevive);
        ctx.Read(s.XPPerAssist);
        ctx.Read(s.XPPerCapture);
        ctx.Read(s.FriendlyFirePenaltyXP);
        ctx.Read(s.SuicidePenaltyXP);
        ctx.Read(s.HeadShotBonusXP);
        ctx.Read(s.KillStreakBonusXPEnabled);
        ctx.Read(s.KillStreakThresholds);
        ctx.Read(s.KillStreakBonusXP);
        ctx.Read(s.GlobalXPMultiplier);
        ctx.Read(s.MoneyPerKill);
        ctx.Read(s.MoneyPerRevive);
        ctx.Read(s.MoneyPerAssist);
        ctx.Read(s.MoneyPerCapture);
        ctx.Read(s.TeamKillMoneyPenalty);
        ctx.Read(s.SuicideMoneyPenalty);
        ctx.Read(s.HeadShotMoneyBonus);
        ctx.Read(s.KillStreakMoneyBonusEnabled);
        ctx.Read(s.KillStreakMoneyThresholds);
        ctx.Read(s.KillStreakBonusMoney);
        ctx.Read(s.GlobalMoneyMultiplier);
        ctx.Read(s.MaxTeamImbalance);
	
		CopyInternal(s);
		
		m_IsLoaded = true;
		
		ExpansionSettings.SI_DayZ_KOTH.Invoke();

		return true;
	}

    override void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(ModeName);
        ctx.Write(ScoreLimit);
        ctx.Write(CaptureTickSeconds);
        ctx.Write(MinPlayersToInfluence);
        ctx.Write(PointsPerTickPerPlayer);
        ctx.Write(NeutralizeSpeedMultiplier);
        ctx.Write(XPPerKill);
        ctx.Write(XPPerRevive);
        ctx.Write(XPPerAssist);
        ctx.Write(XPPerCapture);
        ctx.Write(FriendlyFirePenaltyXP);
        ctx.Write(SuicidePenaltyXP);
        ctx.Write(HeadShotBonusXP);
        ctx.Write(KillStreakBonusXPEnabled);
        ctx.Write(KillStreakThresholds);
        ctx.Write(KillStreakBonusXP);
        ctx.Write(GlobalXPMultiplier);
        ctx.Write(MoneyPerKill);
        ctx.Write(MoneyPerRevive);
        ctx.Write(MoneyPerAssist);
        ctx.Write(MoneyPerCapture);
        ctx.Write(TeamKillMoneyPenalty);
        ctx.Write(SuicideMoneyPenalty);
        ctx.Write(HeadShotMoneyBonus);
        ctx.Write(KillStreakMoneyBonusEnabled);
        ctx.Write(KillStreakMoneyThresholds);
        ctx.Write(KillStreakBonusMoney);
        ctx.Write(GlobalMoneyMultiplier);
        ctx.Write(MaxTeamImbalance);

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
		DayZ_KOTH_Settings s;
		if (!Class.CastTo(s, setting))
			return false;

		CopyInternal(s);
		return true;
	}

    protected void CopyInternal( DayZ_KOTH_Settings s )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
	#endif


		DayZ_KOTH_Settings sb = s;
		CopyInternal( sb );
	}

    private void CopyInternal( DayZ_KOTH_SettingsBase s)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
#endif

        ModeName = s.ModeName;
        ScoreLimit = s.ScoreLimit;
        CaptureTickSeconds = s.CaptureTickSeconds;
        MinPlayersToInfluence = s.MinPlayersToInfluence;
        PointsPerTickPerPlayer = s.PointsPerTickPerPlayer;
        NeutralizeSpeedMultiplier = s.NeutralizeSpeedMultiplier;
        XPPerKill = s.XPPerKill;
        XPPerRevive = s.XPPerRevive;
        XPPerAssist = s.XPPerAssist;
        XPPerCapture = s.XPPerCapture;
        FriendlyFirePenaltyXP = s.FriendlyFirePenaltyXP;
        SuicidePenaltyXP = s.SuicidePenaltyXP;
        HeadShotBonusXP = s.HeadShotBonusXP;
        KillStreakBonusXPEnabled = s.KillStreakBonusXPEnabled;
        KillStreakThresholds = s.KillStreakThresholds;
        KillStreakBonusXP = s.KillStreakBonusXP;
        GlobalXPMultiplier = s.GlobalXPMultiplier;
        MoneyPerKill = s.MoneyPerKill;
        MoneyPerRevive = s.MoneyPerRevive;
        MoneyPerAssist = s.MoneyPerAssist;
        MoneyPerCapture = s.MoneyPerCapture;
        TeamKillMoneyPenalty = s.TeamKillMoneyPenalty;
        SuicideMoneyPenalty = s.SuicideMoneyPenalty;
        HeadShotMoneyBonus = s.HeadShotMoneyBonus;
        KillStreakMoneyBonusEnabled = s.KillStreakMoneyBonusEnabled;
        KillStreakMoneyThresholds = s.KillStreakMoneyThresholds;
        KillStreakBonusMoney = s.KillStreakBonusMoney;
        GlobalMoneyMultiplier = s.GlobalMoneyMultiplier;
        MaxTeamImbalance = s.MaxTeamImbalance;
        
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

		bool DayZ_KOTH_SettingsExist = FileExist(EXPANSION_DAYZ_KOTH_SETTINGS);

		if (DayZ_KOTH_SettingsExist)
		{
			CF_Log.Info("[DayZ_KOTH_Settings] Load existing setting file:" + EXPANSION_DAYZ_KOTH_SETTINGS);

			DayZ_KOTH_Settings settingsDefault = new DayZ_KOTH_Settings;
			settingsDefault.Defaults();

			DayZ_KOTH_SettingsBase settingsBase;

			JsonFileLoader<DayZ_KOTH_SettingsBase>.JsonLoadFile(EXPANSION_DAYZ_KOTH_SETTINGS, settingsBase);
			JsonFileLoader<DayZ_KOTH_Settings>.JsonLoadFile(EXPANSION_DAYZ_KOTH_SETTINGS, this);
			
		}
		else
		{
			CF_Log.Info("[DayZ_KOTH_Settings] No existing setting file:" + EXPANSION_DAYZ_KOTH_SETTINGS + ". Creating defaults!");

			Defaults();
			save = true;
		}

		if (save)
		{
			Save();
		}

		return DayZ_KOTH_SettingsExist;
	}

	override bool OnSave()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnSave");
	#endif

		JsonFileLoader<DayZ_KOTH_Settings>.JsonSaveFile( EXPANSION_DAYZ_KOTH_SETTINGS, this );

		return true;
	}

	override void Update( ExpansionSettingBase setting )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Update").Add(setting);
	#endif

		super.Update( setting );

		ExpansionSettings.SI_DayZ_KOTH.Invoke();
	}
	
    override void Defaults()
	{
        ModeName = "King of the Hill";
        ScoreLimit = 100;
        CaptureTickSeconds = 1;
        MinPlayersToInfluence = 1;
        PointsPerTickPerPlayer = 0.25;
        NeutralizeSpeedMultiplier = 1.0;
        XPPerKill = 100;
        XPPerRevive = 50;
        XPPerAssist = 25;
        XPPerCapture = 250;
        FriendlyFirePenaltyXP = -150;
        SuicidePenaltyXP = -100;
        HeadShotBonusXP = 50;
        KillStreakBonusXPEnabled = true;
        KillStreakThresholds = { 5, 10, 15 };
        KillStreakBonusXP = { 100, 200, 300 };
        GlobalXPMultiplier = 1.0;
        MoneyPerKill = 100;
        MoneyPerRevive = 50;
        MoneyPerAssist = 25;
        MoneyPerCapture = 300;
        TeamKillMoneyPenalty = -200;
        SuicideMoneyPenalty = -100;
        HeadShotMoneyBonus = 50;
        KillStreakMoneyBonusEnabled = true;
        KillStreakMoneyThresholds = { 5, 10, 15 };
        KillStreakBonusMoney = { 100, 200, 300 };
        GlobalMoneyMultiplier = 1.0;
        MaxTeamImbalance = 3;
	}

    override string SettingName()
	{
		return "DayZ King Of The Hill Settings";
	}
}