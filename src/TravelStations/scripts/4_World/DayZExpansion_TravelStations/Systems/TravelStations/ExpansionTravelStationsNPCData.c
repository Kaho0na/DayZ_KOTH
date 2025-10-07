enum ExpansionTravelStationsNPCType
{
	NORMAL,
	OBJECT,
	AI
};

class ExpansionTravelStationsNPCData
{

	static ref ExpansionTravelStationsNPCData s_Instance;
	protected ref map<int, ref ExpansionTravelStationsNPCData> m_QuestsNPCs = new map<int, ref ExpansionTravelStationsNPCData>();

    int CityID;
    string StationName;
    vector Position;
	int StationType;
	string StationManagerImage;
	int StationID;

    string ClassName;
	vector Orientation;
	string NPCName = "Unknown";
	string DefaultNPCText = "What do you want? Leave me alone!";

	#ifdef EXPANSIONMODAI
	int NPCEmoteID = EmoteConstants.ID_EMOTE_WATCHING;
	bool NPCEmoteIsStatic;
	#endif
	string NPCLoadoutFile;
	int NPCInteractionEmoteID = EmoteConstants.ID_EMOTE_GREETING;
	int NPCQuestCancelEmoteID = EmoteConstants.ID_EMOTE_SHRUG;
	int NPCQuestStartEmoteID = EmoteConstants.ID_EMOTE_NOD;
	int NPCQuestCompleteEmoteID = EmoteConstants.ID_EMOTE_CLAP;
	
	#ifdef EXPANSIONMODAI
	string NPCFaction = "InvincibleObservers";
	#endif
	int NPCType = ExpansionTravelStationsNPCType.NORMAL;

    [NonSerialized()]
	static string FILENAME = "NONE";
	
	bool Active = true;

    void ExpansionTravelStationsNPCData()
	{
		s_Instance = this;
		m_QuestsNPCs = new map<int, ref ExpansionTravelStationsNPCData>();
	}

	static ExpansionTravelStationsNPCData GetInstance()
	{
		if (!s_Instance)
			s_Instance = new ExpansionTravelStationsNPCData();
		return s_Instance;
	}

    void SetCityID(int id)
	{
		CityID = id;
	}

	int GetCityID()
	{
		return CityID;
	}

	void SetStationID(int id)
	{
		StationID = id;
	}

	int GetStationID()
	{
		return StationID;
	}

    void SetStationName(string className)
	{
		StationName = className;
	}
	
	string GetStationName()
	{
		return StationName;
	}

    void SetPosition(vector pos)
	{
		Position = pos;
	}

	vector GetPosition()
	{
		return Position;
	}

	void SetStationType(int type)
	{
		StationType = type;
	}

	int GetStationType()
	{
		return StationType;
	}
	void SetStationManagerImage(string filename)
	{
		StationManagerImage = filename;
	}

	string GetStationManagerImage()
	{
		return StationManagerImage;
	}

    void SetClassName(string className)
	{
		ClassName = className;
	}
	
	string GetClassName()
	{
		return ClassName;
	}

	void SetNPCType(ExpansionTravelStationsNPCType type)
	{
		NPCType = type;
	}

	ExpansionTravelStationsNPCType GetNPCType()
	{
		return NPCType;
	}

    void SetOrientation(vector orientation)
	{
		Orientation = orientation;
	}

	vector GetOrientation()
	{
		return Orientation;
	}

    void SetNPCName(string name)
	{
		NPCName = name;
	}

	string GetNPCName()
	{
		return NPCName;
	}

    void SetDefaultNPCText(string text)
	{
		DefaultNPCText = text;
	}

	string GetDefaultNPCText()
	{
		return DefaultNPCText;
	}

    void SetLoadoutFile(string fileName)
	{
		NPCLoadoutFile = fileName;
	}

	string GetLoadoutFile()
	{
		return NPCLoadoutFile;
	}

#ifdef EXPANSIONMODAI
	void SetIsEmoteStatic(bool state)
	{
		NPCEmoteIsStatic = state;
	}

	bool IsEmoteStatic()
	{
		return NPCEmoteIsStatic;
	}

	void SetEmoteID(int emoteID)
	{
		NPCEmoteID = emoteID;
	}

	int GetEmoteID()
	{
		return NPCEmoteID;
	}
	
	void SetFaction(string faction)
	{
		NPCFaction = faction;
	}
	
	string GetFaction()
	{
		return NPCFaction;
	}
#endif

    bool IsActive()
    {
        return Active;
    }

    ExpansionTravelStationsNPCBase SpawnNPC()
	{
		int flags = ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE;

		if (GetGame().IsKindOf(ClassName, "DZ_LightAI"))  //! Only true for animals and Zs
			flags |= ECE_INITAI;

		Object obj = ExpansionGame.CreateObjectExSafe(ClassName, Position, flags);
 		ExpansionTravelStationsNPCBase questNPC;
		if (!ExpansionTravelStationsNPCBase.CastTo(questNPC, obj))
	    {
			Error("ExpansionTravelStationsNPCDataBase::SpawnNPC - Used unsupported object " + ClassName + " as quest NPC in config. Only objects based on ExpansionTravelStationsNPCBase class are allowed!");
			GetGame().ObjectDelete(obj);
	        return null;
	    }

		if (Position)
	        questNPC.SetPosition(Position);

	    if (Orientation)
	        questNPC.SetOrientation(Orientation);

		questNPC.Update();
		if (!questNPC.m_Expansion_NetsyncData)
			questNPC.m_Expansion_NetsyncData = new ExpansionNetsyncData(questNPC);
	#ifdef DIAG_DEVELOPER
		NPCName = NPCName + " | ID: " + ID;
	#endif
		questNPC.m_Expansion_NetsyncData.Set(0, NPCName);
		ExpansionHumanLoadout.Apply(questNPC, GetLoadoutFile(), false);
		
	#ifdef EXPANSIONMODAI
		eAIGroup aiGroup = questNPC.GetGroup();
		if (NPCFaction != string.Empty)
		{
			eAIFaction faction = eAIFaction.Create(NPCFaction);
			if (faction && aiGroup.GetFaction().Type() != faction.Type())
				aiGroup.SetFaction(faction);
		}
	#endif

		return questNPC;
	}

	#ifdef EXPANSIONMODAI
	ExpansionTravelStationsNPCAIBase SpawnNPCAI()
	{
		Position = ExpansionAIPatrol.GetPlacementPosition(Position);
		Object obj = ExpansionGame.CreateObjectSafe(ClassName, Position);
		if (!obj){
			Print("IS NOT AN OBJECT");
			return NULL;}

		ExpansionTravelStationsNPCAIBase questNPC = ExpansionTravelStationsNPCAIBase.Cast(obj);
		if (!questNPC)
		{
			Print("IS NOT A QUEST");
			GetGame().ObjectDelete(obj);
			return NULL;
		}

		questNPC.SetPosition(Position);
		questNPC.SetOrientation(Orientation);
		questNPC.Update();
	#ifdef DIAG_DEVELOPER
		NPCName = NPCName + " | ID: " + ID;
	#endif
		if (!questNPC.m_Expansion_NetsyncData)
			questNPC.m_Expansion_NetsyncData = new ExpansionNetsyncData(questNPC);
		questNPC.m_Expansion_NetsyncData.Set(0, NPCName);
		ExpansionHumanLoadout.Apply(questNPC, NPCLoadoutFile, false);
		questNPC.Expansion_SetCanBeLooted(false);
		questNPC.eAI_SetUnlimitedReload(true);
		questNPC.eAI_SetAccuracy(1.0, 1.0);
		questNPC.eAI_SetThreatDistanceLimit(800);

		eAIGroup aiGroup = questNPC.GetGroup();

		if (NPCFaction != string.Empty)
		{
			eAIFaction faction = eAIFaction.Create(NPCFaction);
			if (faction && aiGroup.GetFaction().Type() != faction.Type())
				aiGroup.SetFaction(faction);
		}
		
		aiGroup.SetFormation(new eAIFormationColumn(aiGroup));
		aiGroup.SetWaypointBehaviour(eAIWaypointBehavior.ALTERNATE);
	
	
		return questNPC;
	}
#endif

        ExpansionTravelStationsStaticObject SpawnObject()
        {
            Object obj = ExpansionGame.CreateObjectExSafe(ClassName, Position, ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE);
            ExpansionTravelStationsStaticObject questObject;
            if (!ExpansionTravelStationsStaticObject.CastTo(questObject, obj))
            {
                Error("ExpansionTravelStationsNPCDataBase::SpawnNPC - Used unsupported object " + ClassName + " as AI quest NPC in config. Only objects based on ExpansionTravelStationsStaticObject class are allowed!");
                GetGame().ObjectDelete(obj);
                return null;
            }

            if (Position)
                questObject.SetPosition(Position);

            if (Orientation)
                questObject.SetOrientation(Orientation);

        #ifdef DIAG_DEVELOPER
            NPCName = NPCName + " | ID: " + ID;
        #endif
            questObject.m_Expansion_NetsyncData.Set(0, NPCName);

            return questObject;
        }

    static ExpansionTravelStationsNPCData Load(string fileName)
	{
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";
		Print("[ExpansionTravelStationsNPCData] Load existing configuration file:" + fileName);
		ExpansionTravelStationsNPCData npcConfig;
        ExpansionJsonFileParser<ExpansionTravelStationsNPCData>.Load(EXPANSION_TRAVELSTATIONS_FOLDER + fileName, npcConfig);
		npcConfig.FILENAME = fileName;
		Print("[ExpansionTravelStationsNPCData] ExpansionTravelStationsNPCData:" + npcConfig);
		return npcConfig;
	}

    void Save(string fileName)
	{
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";

		ExpansionJsonFileParser<ExpansionTravelStationsNPCData>.Save(EXPANSION_TRAVELSTATIONS_FOLDER + fileName, this);
	}

    void CopyConfig(ExpansionTravelStationsNPCData npcDataBase)
	{
		CityID = npcDataBase.CityID;
		StationID = npcDataBase.StationID;
        StationName = npcDataBase.StationName;
		StationType = npcDataBase.StationType;
		ClassName = npcDataBase.ClassName;
		Position = npcDataBase.Position;
		Orientation = npcDataBase.Orientation;
		NPCName = npcDataBase.NPCName;
		DefaultNPCText = npcDataBase.DefaultNPCText;

	#ifdef EXPANSIONMODAI
		NPCEmoteID = npcDataBase.NPCEmoteID;
		NPCEmoteIsStatic = npcDataBase.NPCEmoteIsStatic;
	#endif
		NPCLoadoutFile = npcDataBase.NPCLoadoutFile;
		
		NPCInteractionEmoteID = npcDataBase.NPCInteractionEmoteID;
		NPCQuestCancelEmoteID = npcDataBase.NPCQuestCancelEmoteID;
		NPCQuestStartEmoteID = npcDataBase.NPCQuestStartEmoteID;
		NPCQuestCompleteEmoteID = npcDataBase.NPCQuestCompleteEmoteID;

	#ifdef EXPANSIONMODAI
		NPCFaction = npcDataBase.NPCFaction;
	#endif
		NPCType = npcDataBase.NPCType;
	}

    void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(CityID);
		ctx.Write(StationID);
        ctx.Write(StationName);
		ctx.Write(StationType);
		ctx.Write(StationManagerImage);
		ctx.Write(NPCName);
	}

    bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(CityID))
			return false;

		if (!ctx.Read(StationID))
			return false;
        
        if (!ctx.Read(StationName))
            return false;

		if (!ctx.Read(StationType))
		return false;
		
		if (!ctx.Read(StationManagerImage))
		return false;

		if (!ctx.Read(NPCName))
		return false;
		
		return true;
	}

};