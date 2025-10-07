/**
 * ExpansionDeadDropSettings.c
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
class ExpansionDeadDropSettings: ExpansionSettingBase
{
	static const int VERSION = 1;

	bool DeadDropEnabled;
	int NPCID;
	int RecoveryCost;
	int TimeToDeletePlayerAfterDeath;
	int TimeToDeleteDeadDrop;

	string DeadDropImage;

	vector Position;
	string ClassName;
	vector Orientation;
	string NPCName = "Title is different here";
	string DefaultNPCText;
	int NPCEmoteID;
	bool NPCEmoteIsStatic;
	string NPCLoadoutFile;
	int NPCInteractionEmoteID;
	int NPCQuestCancelEmoteID;
	int NPCQuestStartEmoteID;
	int NPCQuestCompleteEmoteID;
	string NPCFaction;
	int NPCType;
	bool Active;

	[NonSerialized()]
	private bool m_IsLoaded;

    override bool OnRecieve(ParamsReadContext ctx)
	{

	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "OnRecieve").Add(ctx);
	#endif
		ctx.Read(DeadDropEnabled);
		ctx.Read(NPCID);
		ctx.Read(RecoveryCost);
		ctx.Read(TimeToDeletePlayerAfterDeath);
		ctx.Read(TimeToDeleteDeadDrop);
		ctx.Read(DeadDropImage);
		ctx.Read(Position);
		ctx.Read(ClassName);
		ctx.Read(Orientation);
		ctx.Read(NPCName);
		ctx.Read(DefaultNPCText);
		ctx.Read(NPCEmoteID);
		ctx.Read(NPCEmoteIsStatic);
		ctx.Read(NPCLoadoutFile);
		ctx.Read(NPCInteractionEmoteID);
		ctx.Read(NPCQuestCancelEmoteID);
		ctx.Read(NPCQuestStartEmoteID);
		ctx.Read(NPCQuestCompleteEmoteID);
		ctx.Read(NPCFaction);
		ctx.Read(NPCType);
		ctx.Read(Active);
		
		m_IsLoaded = true;
		
		ExpansionSettings.SI_DeadDrop.Invoke();

		return true;
	}

    override void OnSend(ParamsWriteContext ctx)
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "OnSend").Add(ctx);
	#endif

		ctx.Write(DeadDropEnabled);
		ctx.Write(NPCID);
		ctx.Write(RecoveryCost);
		ctx.Write(TimeToDeletePlayerAfterDeath);
		ctx.Write(TimeToDeleteDeadDrop);
		ctx.Write(DeadDropImage);
		ctx.Write(Position);
		ctx.Write(ClassName);
		ctx.Write(Orientation);
		ctx.Write(NPCName);
		ctx.Write(DefaultNPCText);
		ctx.Write(NPCEmoteID);
		ctx.Write(NPCEmoteIsStatic);
		ctx.Write(NPCLoadoutFile);
		ctx.Write(NPCInteractionEmoteID);
		ctx.Write(NPCQuestCancelEmoteID);
		ctx.Write(NPCQuestStartEmoteID);
		ctx.Write(NPCQuestCompleteEmoteID);
		ctx.Write(NPCFaction);
		ctx.Write(NPCType);
		ctx.Write(Active);

	}

	override int Send( PlayerIdentity identity )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Send").Add(identity);
	#endif

		if ( !IsMissionHost() )
		{
			return 0;
		}

		auto rpc = CreateRPC();
		OnSend( rpc );
		rpc.Expansion_Send(true, identity);

		return 0;
	}

	override bool Copy( ExpansionSettingBase setting )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Copy").Add(setting);
	#endif

		ExpansionDeadDropSettings s;
		if ( !Class.CastTo( s, setting ) )
			return false;

		CopyInternal( s );
		return true;
	}

    private void CopyInternal(ExpansionDeadDropSettings s)
	{
#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "CopyInternal").Add(s);
#endif

        DeadDropEnabled = s.DeadDropEnabled;
		NPCID = s.NPCID;
		RecoveryCost = s.RecoveryCost;
		TimeToDeletePlayerAfterDeath = s.TimeToDeletePlayerAfterDeath;
		TimeToDeleteDeadDrop = s.TimeToDeleteDeadDrop;
		DeadDropImage = s.DeadDropImage;
		Position = s.Position;
		ClassName = s.ClassName;
		Orientation = s.Orientation;
		NPCName = s.NPCName;
		DefaultNPCText = s.DefaultNPCText;
		NPCEmoteID = s.NPCEmoteID;
		NPCEmoteIsStatic = s.NPCEmoteIsStatic;
		NPCLoadoutFile = s.NPCLoadoutFile;
		NPCInteractionEmoteID = s.NPCInteractionEmoteID;
		NPCQuestCancelEmoteID = s.NPCQuestCancelEmoteID;
		NPCQuestStartEmoteID = s.NPCQuestStartEmoteID;
		NPCQuestCompleteEmoteID = s.NPCQuestCompleteEmoteID;
		NPCFaction = s.NPCFaction;
		NPCType = s.NPCType;
		Active = s.Active;

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

		bool DeadDropSettingsExist = FileExist(EXPANSION_DEADDROP_SETTINGS);

		if (DeadDropSettingsExist)
		{
			JsonFileLoader<ExpansionDeadDropSettings>.JsonLoadFile(EXPANSION_DEADDROP_SETTINGS, this);
			if (m_Version < VERSION)
			{
				EXPrint("[ExpansionDeadDropSetting] Load - Converting v" + m_Version + " \"" + EXPANSION_DEADDROP_SETTINGS + "\" to v" + VERSION);

				ExpansionDeadDropSettings defaultSettings = new ExpansionDeadDropSettings;
				defaultSettings.Defaults();
				
				m_Version = VERSION;
				save = true;
			}
		}
		else
		{
			CF_Log.Info("[ExpansionDeadDropSettings] No existing setting file:" + EXPANSION_DEADDROP_SETTINGS + ". Creating defaults!");
			Defaults();
			save = true;
		}

		if (save)
		{
			Save();
		}

		return DeadDropSettingsExist;
	}

	override bool OnSave()
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.SETTINGS, this, "OnSave");
	#endif

		JsonFileLoader<ExpansionDeadDropSettings>.JsonSaveFile( EXPANSION_DEADDROP_SETTINGS, this );

		return true;
	}

	override void Update( ExpansionSettingBase setting )
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_1(ExpansionTracing.SETTINGS, this, "Update").Add(setting);
	#endif

		super.Update( setting );

		ExpansionSettings.SI_DeadDrop.Invoke();
	}
	
	override void Defaults()
	{
		m_Version = VERSION;
		DeadDropEnabled = true;
		NPCID = 999;
		RecoveryCost = 1000;
		TimeToDeletePlayerAfterDeath = 10;
		TimeToDeleteDeadDrop = 3600;
		DeadDropImage = "DeadDrop/GUI/characters/thebaronFinal.paa";
		Position = "14832.373047 72.013893 14564.759766";
		ClassName = "ExpansionDeadDropNPCAIMirek";
		Orientation = "-180 0 0";
		NPCName = "DeadDrop Title Here";
		DefaultNPCText = "Text Goes Here";
		NPCEmoteID = 46;
		NPCEmoteIsStatic = false;
		NPCLoadoutFile = "PlayerMaleSuitLoadout";
		NPCInteractionEmoteID = 1;
		NPCQuestCancelEmoteID = 60;
		NPCQuestStartEmoteID = 58;
		NPCQuestCompleteEmoteID = 39;
		NPCFaction = "InvincibleObservers";
		NPCType = 2;
		Active = true;
	}


    override string SettingName()
	{
		return "DeadDrop Settings";
	}
}