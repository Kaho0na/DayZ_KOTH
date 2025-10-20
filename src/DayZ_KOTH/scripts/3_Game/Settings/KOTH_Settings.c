/**
 * KOTH_Settings.c (COMPLETE CONFIGURATION WITH ROUND END)
 *
 * King of the Hill by Kahoona
 * Configurable XP and money rewards with global multipliers + round end bonuses
 * Place in: 3_Game/Settings/KOTH_Settings.c
 */

class KOTH_SettingsBase: ExpansionSettingBase
{
    string ModeName = "King of the Hill";

    int ScoreLimit = 100;
    int CaptureTickSeconds = 5;
    int MinPlayersToInfluence = 1;
    int PointsPerTickPerPlayer = 1;
    float NeutralizeSpeedMultiplier = 1.0;

    bool EnablePriorityZoneMovement = true;
    float PriorityZoneMovementInterval = 120.0;
    float PriorityZoneBonusMultiplier = 2.0;

    int XPPerKill = 100;
    int XPPerRevive = 50;
    int XPPerAssist = 25;
    int XPPerCapture = 250;
    int XPCaptureInterval = 10;
    int FriendlyFirePenaltyXP = 150;
    int SuicidePenaltyXP = 100;
    int HeadShotBonusXP = 50;

    bool KillStreakBonusXPEnabled = true;
    ref array<int> KillStreakThresholds = { 5, 10, 15 };
    ref array<int> KillStreakBonusXP = { 100, 200, 300 };

    float GlobalXPMultiplier = 1.0;

    int MoneyPerKill = 100;
    int MoneyPerRevive = 50;
    int MoneyPerAssist = 25;
    int MoneyPerCapture = 300;
    int TeamKillMoneyPenalty = 200;
    int SuicideMoneyPenalty = 100;
    int HeadShotMoneyBonus = 50;

    bool KillStreakMoneyBonusEnabled = true;
    ref array<int> KillStreakMoneyThresholds = { 5, 10, 15 };
    ref array<int> KillStreakBonusMoney = { 100, 200, 300 };

    float GlobalMoneyMultiplier = 1.0;

    float EndScreenDisplaySeconds = 30;
    float VoteTimeSeconds = 20;
    int MaxPlayerLevel = 99;

    int MVPBonusXP = 500;
    int MVPBonusMoney = 500;
    int SharpshooterBonusXP = 400;
    int SharpshooterBonusMoney = 400;
    int MedicBonusXP = 300;
    int MedicBonusMoney = 300;

    float WinningTeamXPMultiplier = 1.5;
    float WinningTeamMoneyMultiplier = 1.5;
    float LosingTeamXPMultiplier = 0.5;
    float LosingTeamMoneyMultiplier = 0.5;

    int MaxTeamImbalance = 3;
    
    bool EnableZoneRotation = false;
    float ZoneRotationInterval = 1800.0;
    int ZoneSelectionMode = 0;
    bool NotifyPlayersOnZoneChange = true;
    int ZoneChangeWarningTime = 60;
    bool AllowAdminZoneChange = true;
}

class KOTH_Settings: KOTH_SettingsBase
{
    static const int VERSION = 2;
    
    [NonSerialized()]
    private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
    {
        KOTH_Settings s = new KOTH_Settings;

        if (!ctx.Read(s.ModeName)) return false;
        if (!ctx.Read(s.ScoreLimit)) return false;
        if (!ctx.Read(s.CaptureTickSeconds)) return false;
        if (!ctx.Read(s.MinPlayersToInfluence)) return false;
        if (!ctx.Read(s.PointsPerTickPerPlayer)) return false;
        if (!ctx.Read(s.NeutralizeSpeedMultiplier)) return false;
        
        if (!ctx.Read(s.EnablePriorityZoneMovement)) return false;
        if (!ctx.Read(s.PriorityZoneMovementInterval)) return false;
        if (!ctx.Read(s.PriorityZoneBonusMultiplier)) return false;
        
        if (!ctx.Read(s.XPPerKill)) return false;
        if (!ctx.Read(s.XPPerRevive)) return false;
        if (!ctx.Read(s.XPPerAssist)) return false;
        if (!ctx.Read(s.XPPerCapture)) return false;
        if (!ctx.Read(s.XPCaptureInterval)) return false;
        if (!ctx.Read(s.FriendlyFirePenaltyXP)) return false;
        if (!ctx.Read(s.SuicidePenaltyXP)) return false;
        if (!ctx.Read(s.HeadShotBonusXP)) return false;
        if (!ctx.Read(s.KillStreakBonusXPEnabled)) return false;
        if (!ctx.Read(s.KillStreakThresholds)) return false;
        if (!ctx.Read(s.KillStreakBonusXP)) return false;
        if (!ctx.Read(s.GlobalXPMultiplier)) return false;
        if (!ctx.Read(s.MoneyPerKill)) return false;
        if (!ctx.Read(s.MoneyPerRevive)) return false;
        if (!ctx.Read(s.MoneyPerAssist)) return false;
        if (!ctx.Read(s.MoneyPerCapture)) return false;
        if (!ctx.Read(s.TeamKillMoneyPenalty)) return false;
        if (!ctx.Read(s.SuicideMoneyPenalty)) return false;
        if (!ctx.Read(s.HeadShotMoneyBonus)) return false;
        if (!ctx.Read(s.KillStreakMoneyBonusEnabled)) return false;
        if (!ctx.Read(s.KillStreakMoneyThresholds)) return false;
        if (!ctx.Read(s.KillStreakBonusMoney)) return false;
        if (!ctx.Read(s.GlobalMoneyMultiplier)) return false;
        
        if (!ctx.Read(s.EndScreenDisplaySeconds)) return false;
        if (!ctx.Read(s.VoteTimeSeconds)) return false;
        if (!ctx.Read(s.MaxPlayerLevel)) return false;
        
        if (!ctx.Read(s.MVPBonusXP)) return false;
        if (!ctx.Read(s.MVPBonusMoney)) return false;
        if (!ctx.Read(s.SharpshooterBonusXP)) return false;
        if (!ctx.Read(s.SharpshooterBonusMoney)) return false;
        if (!ctx.Read(s.MedicBonusXP)) return false;
        if (!ctx.Read(s.MedicBonusMoney)) return false;
        
        if (!ctx.Read(s.WinningTeamXPMultiplier)) return false;
        if (!ctx.Read(s.WinningTeamMoneyMultiplier)) return false;
        if (!ctx.Read(s.LosingTeamXPMultiplier)) return false;
        if (!ctx.Read(s.LosingTeamMoneyMultiplier)) return false;
        
        if (!ctx.Read(s.MaxTeamImbalance)) return false;
        if (!ctx.Read(s.EnableZoneRotation)) return false;
        if (!ctx.Read(s.ZoneRotationInterval)) return false;
        if (!ctx.Read(s.ZoneSelectionMode)) return false;
        if (!ctx.Read(s.NotifyPlayersOnZoneChange)) return false;
        if (!ctx.Read(s.ZoneChangeWarningTime)) return false;
        if (!ctx.Read(s.AllowAdminZoneChange)) return false;

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
        
        ctx.Write(EnablePriorityZoneMovement);
        ctx.Write(PriorityZoneMovementInterval);
        ctx.Write(PriorityZoneBonusMultiplier);
        
        ctx.Write(XPPerKill);
        ctx.Write(XPPerRevive);
        ctx.Write(XPPerAssist);
        ctx.Write(XPPerCapture);
        ctx.Write(XPCaptureInterval);
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
        
        ctx.Write(EndScreenDisplaySeconds);
        ctx.Write(VoteTimeSeconds);
        ctx.Write(MaxPlayerLevel);
        
        ctx.Write(MVPBonusXP);
        ctx.Write(MVPBonusMoney);
        ctx.Write(SharpshooterBonusXP);
        ctx.Write(SharpshooterBonusMoney);
        ctx.Write(MedicBonusXP);
        ctx.Write(MedicBonusMoney);
        
        ctx.Write(WinningTeamXPMultiplier);
        ctx.Write(WinningTeamMoneyMultiplier);
        ctx.Write(LosingTeamXPMultiplier);
        ctx.Write(LosingTeamMoneyMultiplier);
        
        ctx.Write(MaxTeamImbalance);
        ctx.Write(EnableZoneRotation);
        ctx.Write(ZoneRotationInterval);
        ctx.Write(ZoneSelectionMode);
        ctx.Write(NotifyPlayersOnZoneChange);
        ctx.Write(ZoneChangeWarningTime);
        ctx.Write(AllowAdminZoneChange);
    }

    override int Send(PlayerIdentity identity)
    {
        if (!IsMissionHost())
            return 0;
        
        auto rpc = CreateRPC();
        OnSend(rpc);
        rpc.Expansion_Send(true, identity);
        
        return 0;
    }

    override bool Copy(ExpansionSettingBase setting)
    {
        KOTH_Settings s;
        if (!Class.CastTo(s, setting))
            return false;

        CopyInternal(s);
        return true;
    }

    protected void CopyInternal(KOTH_Settings s)
    {
        KOTH_SettingsBase sb = s;
        CopyInternal(sb);
    }

    private void CopyInternal(KOTH_SettingsBase s)
    {
        ModeName = s.ModeName;
        ScoreLimit = s.ScoreLimit;
        CaptureTickSeconds = s.CaptureTickSeconds;
        MinPlayersToInfluence = s.MinPlayersToInfluence;
        PointsPerTickPerPlayer = s.PointsPerTickPerPlayer;
        NeutralizeSpeedMultiplier = s.NeutralizeSpeedMultiplier;
        
        EnablePriorityZoneMovement = s.EnablePriorityZoneMovement;
        PriorityZoneMovementInterval = s.PriorityZoneMovementInterval;
        PriorityZoneBonusMultiplier = s.PriorityZoneBonusMultiplier;
        
        XPPerKill = s.XPPerKill;
        XPPerRevive = s.XPPerRevive;
        XPPerAssist = s.XPPerAssist;
        XPPerCapture = s.XPPerCapture;
        XPCaptureInterval = s.XPCaptureInterval;
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
        
        EndScreenDisplaySeconds = s.EndScreenDisplaySeconds;
        VoteTimeSeconds = s.VoteTimeSeconds;
        MaxPlayerLevel = s.MaxPlayerLevel;
        
        MVPBonusXP = s.MVPBonusXP;
        MVPBonusMoney = s.MVPBonusMoney;
        SharpshooterBonusXP = s.SharpshooterBonusXP;
        SharpshooterBonusMoney = s.SharpshooterBonusMoney;
        MedicBonusXP = s.MedicBonusXP;
        MedicBonusMoney = s.MedicBonusMoney;
        
        WinningTeamXPMultiplier = s.WinningTeamXPMultiplier;
        WinningTeamMoneyMultiplier = s.WinningTeamMoneyMultiplier;
        LosingTeamXPMultiplier = s.LosingTeamXPMultiplier;
        LosingTeamMoneyMultiplier = s.LosingTeamMoneyMultiplier;
        
        MaxTeamImbalance = s.MaxTeamImbalance;
        EnableZoneRotation = s.EnableZoneRotation;
        ZoneRotationInterval = s.ZoneRotationInterval;
        ZoneSelectionMode = s.ZoneSelectionMode;
        NotifyPlayersOnZoneChange = s.NotifyPlayersOnZoneChange;
        ZoneChangeWarningTime = s.ZoneChangeWarningTime;
        AllowAdminZoneChange = s.AllowAdminZoneChange;
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
        m_IsLoaded = true;
        bool save;

        bool KOTH_SettingsExist = FileExist(EXPANSION_KOTH_Settings);

        if (KOTH_SettingsExist)
        {
            CF_Log.Info("[KOTH_Settings] Load existing setting file:" + EXPANSION_KOTH_Settings);

            KOTH_Settings settingsDefault = new KOTH_Settings;
            settingsDefault.Defaults();

            KOTH_SettingsBase settingsBase;

            JsonFileLoader<KOTH_SettingsBase>.JsonLoadFile(EXPANSION_KOTH_Settings, settingsBase);
            JsonFileLoader<KOTH_Settings>.JsonLoadFile(EXPANSION_KOTH_Settings, this);
        }
        else
        {
            CF_Log.Info("[KOTH_Settings] No existing setting file:" + EXPANSION_KOTH_Settings + ". Creating defaults!");

            Defaults();
            save = true;
        }

        if (save)
        {
            Save();
        }

        return KOTH_SettingsExist;
    }

    override bool OnSave()
    {
        Print("[KOTH_Settings] Saving settings to: " + EXPANSION_KOTH_Settings);
        JsonFileLoader<KOTH_Settings>.JsonSaveFile(EXPANSION_KOTH_Settings, this);
        return true;
    }

    override void Update(ExpansionSettingBase setting)
    {
        super.Update(setting);
        ExpansionSettings.SI_DayZ_KOTH.Invoke();
    }
    
    override void Defaults()
    {
        m_Version = VERSION;
        
        ModeName = "King of the Hill";
        ScoreLimit = 100;
        CaptureTickSeconds = 5;
        MinPlayersToInfluence = 1;
        PointsPerTickPerPlayer = 1;
        NeutralizeSpeedMultiplier = 1.0;
        
        EnablePriorityZoneMovement = true;
        PriorityZoneMovementInterval = 120.0;
        PriorityZoneBonusMultiplier = 2.0;
        
        XPPerKill = 100;
        XPPerRevive = 50;
        XPPerAssist = 25;
        XPPerCapture = 250;
        XPCaptureInterval = 10;
        FriendlyFirePenaltyXP = 150;
        SuicidePenaltyXP = 100;
        HeadShotBonusXP = 50;
        KillStreakBonusXPEnabled = true;
        KillStreakThresholds = { 5, 10, 15 };
        KillStreakBonusXP = { 100, 200, 300 };
        GlobalXPMultiplier = 1.0;
        MoneyPerKill = 100;
        MoneyPerRevive = 50;
        MoneyPerAssist = 25;
        MoneyPerCapture = 300;
        TeamKillMoneyPenalty = 200;
        SuicideMoneyPenalty = 100;
        HeadShotMoneyBonus = 50;
        KillStreakMoneyBonusEnabled = true;
        KillStreakMoneyThresholds = { 5, 10, 15 };
        KillStreakBonusMoney = { 100, 200, 300 };
        GlobalMoneyMultiplier = 1.0;
        
        EndScreenDisplaySeconds = 30;
        VoteTimeSeconds = 20;
        MaxPlayerLevel = 99;
        
        MVPBonusXP = 500;
        MVPBonusMoney = 500;
        SharpshooterBonusXP = 400;
        SharpshooterBonusMoney = 400;
        MedicBonusXP = 300;
        MedicBonusMoney = 300;
        
        WinningTeamXPMultiplier = 1.5;
        WinningTeamMoneyMultiplier = 1.5;
        LosingTeamXPMultiplier = 0.5;
        LosingTeamMoneyMultiplier = 0.5;
        
        MaxTeamImbalance = 3;
        EnableZoneRotation = false;
        ZoneRotationInterval = 1800.0;
        ZoneSelectionMode = 0;
        NotifyPlayersOnZoneChange = true;
        ZoneChangeWarningTime = 60;
        AllowAdminZoneChange = true;
    }

    override string SettingName()
    {
        return "DayZ King Of The Hill Settings";
    }
}