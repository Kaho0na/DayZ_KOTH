/**
 * KOTH_Settings.c (COMPLETE CONFIGURATION WITH ROUND END)
 *
 * King of the Hill by Kahoona
 * Configurable XP and money rewards with global multipliers + round end bonuses
 * Place in: 3_Game/Settings/KOTH_Settings.c
 */

class KOTH_SettingsBase: ExpansionSettingBase
{

    int ScoreLimit = 100;
    int CaptureTickSeconds = 5;
    int MinPlayersToInfluence = 1;
    float PointsPerTickPerPlayer = 1.0;

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
    float GlobalXPMultiplier = 1.0;

    int MoneyPerKill = 100;
    int MoneyPerRevive = 50;
    int MoneyPerAssist = 25;
    int MoneyPerCapture = 300;
    int TeamKillMoneyPenalty = 200;
    int SuicideMoneyPenalty = 100;
    int HeadShotMoneyBonus = 50;
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
    
    bool EnableZoneRotation = false;
    float ZoneRotationInterval = 1800.0;
    int ZoneSelectionMode = 0;
    int VehicleRespawnCheckInterval = 60;
    int VehicleSpawnCheckRadius = 30;
    bool EnableAI = true;
    int FactionMaxAI = 10;
    float AISpawnBuffer = 100.0;
    float AISpawnAngleVariation = 30.0;
    string WestLoadout = "WestLoadout";
    string EastLoadout = "EastLoadout";
    float AIAccuracyMin = 0.3;
    float AIAccuracyMax = 0.6;
    float AIThreatDistance = 500.0;
    string AISpeed = "WALK";
    string AIThreatSpeed = "SPRINT";
    int AIUnlimitedReload = 0;
    int AICanBeLooted = 1;
    string AILootingBehaviour = "WEAPONS | UPGRADE";
}

class KOTH_Settings: KOTH_SettingsBase
{
    static const int VERSION = 2;
    
    [NonSerialized()]
    private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
    {
        KOTH_Settings s = new KOTH_Settings;

        if (!ctx.Read(s.ScoreLimit)) return false;
        if (!ctx.Read(s.CaptureTickSeconds)) return false;
        if (!ctx.Read(s.MinPlayersToInfluence)) return false;
        if (!ctx.Read(s.PointsPerTickPerPlayer)) return false;
        
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
        if (!ctx.Read(s.GlobalXPMultiplier)) return false;
        if (!ctx.Read(s.MoneyPerKill)) return false;
        if (!ctx.Read(s.MoneyPerRevive)) return false;
        if (!ctx.Read(s.MoneyPerAssist)) return false;
        if (!ctx.Read(s.MoneyPerCapture)) return false;
        if (!ctx.Read(s.TeamKillMoneyPenalty)) return false;
        if (!ctx.Read(s.SuicideMoneyPenalty)) return false;
        if (!ctx.Read(s.HeadShotMoneyBonus)) return false;
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
        
        if (!ctx.Read(s.EnableZoneRotation)) return false;
        if (!ctx.Read(s.ZoneRotationInterval)) return false;
        if (!ctx.Read(s.ZoneSelectionMode)) return false;
        if (!ctx.Read(s.VehicleRespawnCheckInterval)) return false;
        if (!ctx.Read(s.VehicleSpawnCheckRadius)) return false;
        if (!ctx.Read(s.EnableAI)) return false;
        if (!ctx.Read(s.FactionMaxAI)) return false;
        if (!ctx.Read(s.AISpawnBuffer)) return false;
        if (!ctx.Read(s.AISpawnAngleVariation)) return false;
        if (!ctx.Read(s.WestLoadout)) return false;
        if (!ctx.Read(s.EastLoadout)) return false;
        if (!ctx.Read(s.AIAccuracyMin)) return false;
        if (!ctx.Read(s.AIAccuracyMax)) return false;
        if (!ctx.Read(s.AIThreatDistance)) return false;
        if (!ctx.Read(s.AISpeed)) return false;
        if (!ctx.Read(s.AIThreatSpeed)) return false;
        if (!ctx.Read(s.AIUnlimitedReload)) return false;
        if (!ctx.Read(s.AICanBeLooted)) return false;
        if (!ctx.Read(s.AILootingBehaviour)) return false;

        CopyInternal(s);
        m_IsLoaded = true;
        ExpansionSettings.SI_DayZ_KOTH.Invoke();

        return true;
    }

    override void OnSend(ParamsWriteContext ctx)
    {
        ctx.Write(ScoreLimit);
        ctx.Write(CaptureTickSeconds);
        ctx.Write(MinPlayersToInfluence);
        ctx.Write(PointsPerTickPerPlayer);
        
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
        ctx.Write(GlobalXPMultiplier);
        ctx.Write(MoneyPerKill);
        ctx.Write(MoneyPerRevive);
        ctx.Write(MoneyPerAssist);
        ctx.Write(MoneyPerCapture);
        ctx.Write(TeamKillMoneyPenalty);
        ctx.Write(SuicideMoneyPenalty);
        ctx.Write(HeadShotMoneyBonus);
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
        
        ctx.Write(EnableZoneRotation);
        ctx.Write(ZoneRotationInterval);
        ctx.Write(ZoneSelectionMode);
        ctx.Write(VehicleRespawnCheckInterval);
        ctx.Write(VehicleSpawnCheckRadius);
        ctx.Write(EnableAI);
        ctx.Write(FactionMaxAI);
        ctx.Write(AISpawnBuffer);
        ctx.Write(AISpawnAngleVariation);
        ctx.Write(WestLoadout);
        ctx.Write(EastLoadout);
        ctx.Write(AIAccuracyMin);
        ctx.Write(AIAccuracyMax);
        ctx.Write(AIThreatDistance);
        ctx.Write(AISpeed);
        ctx.Write(AIThreatSpeed);
        ctx.Write(AIUnlimitedReload);
        ctx.Write(AICanBeLooted);
        ctx.Write(AILootingBehaviour);
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
        ScoreLimit = s.ScoreLimit;
        CaptureTickSeconds = s.CaptureTickSeconds;
        MinPlayersToInfluence = s.MinPlayersToInfluence;
        PointsPerTickPerPlayer = s.PointsPerTickPerPlayer;
        
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
        GlobalXPMultiplier = s.GlobalXPMultiplier;
        MoneyPerKill = s.MoneyPerKill;
        MoneyPerRevive = s.MoneyPerRevive;
        MoneyPerAssist = s.MoneyPerAssist;
        MoneyPerCapture = s.MoneyPerCapture;
        TeamKillMoneyPenalty = s.TeamKillMoneyPenalty;
        SuicideMoneyPenalty = s.SuicideMoneyPenalty;
        HeadShotMoneyBonus = s.HeadShotMoneyBonus;
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
        
        EnableZoneRotation = s.EnableZoneRotation;
        ZoneRotationInterval = s.ZoneRotationInterval;
        ZoneSelectionMode = s.ZoneSelectionMode;
        VehicleRespawnCheckInterval = s.VehicleRespawnCheckInterval;
        VehicleSpawnCheckRadius = s.VehicleSpawnCheckRadius;
        EnableAI = s.EnableAI;
        FactionMaxAI = s.FactionMaxAI;
        AISpawnBuffer = s.AISpawnBuffer;
        AISpawnAngleVariation = s.AISpawnAngleVariation;
        WestLoadout = s.WestLoadout;
        EastLoadout = s.EastLoadout;
        AIAccuracyMin = s.AIAccuracyMin;
        AIAccuracyMax = s.AIAccuracyMax;
        AIThreatDistance = s.AIThreatDistance;
        AISpeed = s.AISpeed;
        AIThreatSpeed = s.AIThreatSpeed;
        AIUnlimitedReload = s.AIUnlimitedReload;
        AICanBeLooted = s.AICanBeLooted;
        AILootingBehaviour = s.AILootingBehaviour;
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
        
        ScoreLimit = 100;
        CaptureTickSeconds = 30;
        MinPlayersToInfluence = 1;
        PointsPerTickPerPlayer = 1.0;
        
        EnablePriorityZoneMovement = true;
        PriorityZoneMovementInterval = 240.0;
        PriorityZoneBonusMultiplier = 2.0;
        
        XPPerKill = 100;
        XPPerRevive = 50;
        XPPerAssist = 25;
        XPPerCapture = 100;
        XPCaptureInterval = 30;
        FriendlyFirePenaltyXP = 150;
        SuicidePenaltyXP = 100;
        HeadShotBonusXP = 50;
        GlobalXPMultiplier = 1.0;
        MoneyPerKill = 100;
        MoneyPerRevive = 50;
        MoneyPerAssist = 25;
        MoneyPerCapture = 100;
        TeamKillMoneyPenalty = 200;
        SuicideMoneyPenalty = 100;
        HeadShotMoneyBonus = 50;
        GlobalMoneyMultiplier = 1.0;
        
        EndScreenDisplaySeconds = 45.0;
        VoteTimeSeconds = 40.0;
        MaxPlayerLevel = 99;
        
        MVPBonusXP = 1000;
        MVPBonusMoney = 1000;
        SharpshooterBonusXP = 800;
        SharpshooterBonusMoney = 800;
        MedicBonusXP = 500;
        MedicBonusMoney = 500;
        
        WinningTeamXPMultiplier = 1.5;
        WinningTeamMoneyMultiplier = 1.5;
        LosingTeamXPMultiplier = 0.5;
        LosingTeamMoneyMultiplier = 0.5;
        
        EnableZoneRotation = false;
        ZoneRotationInterval = 3600.0;
        ZoneSelectionMode = 2;
        VehicleRespawnCheckInterval = 60;
        VehicleSpawnCheckRadius = 30;
        EnableAI = true;
        FactionMaxAI = 5;
        AISpawnBuffer = 100.0;
        AISpawnAngleVariation = 30.0;
        WestLoadout = "WestLoadout";
        EastLoadout = "EastLoadout";
        AIAccuracyMin = 0.3;
        AIAccuracyMax = 0.6;
        AIThreatDistance = 500.0;
        AISpeed = "WALK";
        AIThreatSpeed = "SPRINT";
        AIUnlimitedReload = 0;
        AICanBeLooted = 1;
        AILootingBehaviour = "WEAPONS | UPGRADE";
    }

    override string SettingName()
    {
        return "DayZ King Of The Hill Settings";
    }
}