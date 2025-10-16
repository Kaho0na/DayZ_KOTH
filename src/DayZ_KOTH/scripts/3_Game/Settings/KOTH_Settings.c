/**
 * KOTH_Settings.c (UPDATED)
 *
 * King of the Hill by Kahoona
 * Updated with priority zone movement settings
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 */

class KOTH_SettingsBase: ExpansionSettingBase
{
    // ────────────── META ──────────────
    string ModeName = "King of the Hill";

    // ────────────── CORE GAMEPLAY ──────────────
    int ScoreLimit = 100;
    int CaptureTickSeconds = 1;
    int MinPlayersToInfluence = 1;
    float PointsPerTickPerPlayer = 0.25;
    float NeutralizeSpeedMultiplier = 1.0;

    // ────────────── PRIORITY ZONE SETTINGS ──────────────
    bool EnablePriorityZoneMovement = true;
    float PriorityZoneMovementInterval = 30.0; // Seconds between moves
    float PriorityZoneAngleIncrement = 15.0; // Degrees per move
    bool PriorityZoneClockwise = true; // Movement direction
    float PriorityZoneBonusMultiplier = 2.0; // Points multiplier in priority zone

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
    
    // ────────────── ZONE ROTATION SETTINGS ──────────────
    bool EnableZoneRotation = false;
    float ZoneRotationInterval = 1800.0;
    int ZoneSelectionMode = 0;
    bool NotifyPlayersOnZoneChange = true;
    int ZoneChangeWarningTime = 60;
    bool AllowAdminZoneChange = true;
}

class KOTH_Settings: KOTH_SettingsBase
{
    [NonSerialized()]
    private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
    {
        KOTH_Settings s = new KOTH_Settings;

        ctx.Read(s.ModeName);
        ctx.Read(s.ScoreLimit);
        ctx.Read(s.CaptureTickSeconds);
        ctx.Read(s.MinPlayersToInfluence);
        ctx.Read(s.PointsPerTickPerPlayer);
        ctx.Read(s.NeutralizeSpeedMultiplier);
        
        // Priority zone settings
        ctx.Read(s.EnablePriorityZoneMovement);
        ctx.Read(s.PriorityZoneMovementInterval);
        ctx.Read(s.PriorityZoneAngleIncrement);
        ctx.Read(s.PriorityZoneClockwise);
        ctx.Read(s.PriorityZoneBonusMultiplier);
        
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
        ctx.Read(s.EnableZoneRotation);
        ctx.Read(s.ZoneRotationInterval);
        ctx.Read(s.ZoneSelectionMode);
        ctx.Read(s.NotifyPlayersOnZoneChange);
        ctx.Read(s.ZoneChangeWarningTime);
        ctx.Read(s.AllowAdminZoneChange);

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
        
        // Priority zone settings
        ctx.Write(EnablePriorityZoneMovement);
        ctx.Write(PriorityZoneMovementInterval);
        ctx.Write(PriorityZoneAngleIncrement);
        ctx.Write(PriorityZoneClockwise);
        ctx.Write(PriorityZoneBonusMultiplier);
        
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
        PriorityZoneAngleIncrement = s.PriorityZoneAngleIncrement;
        PriorityZoneClockwise = s.PriorityZoneClockwise;
        PriorityZoneBonusMultiplier = s.PriorityZoneBonusMultiplier;
        
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
        ModeName = "King of the Hill";
        ScoreLimit = 100;
        CaptureTickSeconds = 1;
        MinPlayersToInfluence = 1;
        PointsPerTickPerPlayer = 0.25;
        NeutralizeSpeedMultiplier = 1.0;
        
        EnablePriorityZoneMovement = true;
        PriorityZoneMovementInterval = 30.0;
        PriorityZoneAngleIncrement = 15.0;
        PriorityZoneClockwise = true;
        PriorityZoneBonusMultiplier = 2.0;
        
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