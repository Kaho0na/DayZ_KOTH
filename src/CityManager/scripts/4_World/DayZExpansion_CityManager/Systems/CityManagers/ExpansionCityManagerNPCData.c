enum ExpansionCityManagerNPCType
{
	NORMAL,
	OBJECT,
	AI
};

class ExpansionCityManagerNPCData
{

	static ref ExpansionCityManagerNPCData s_Instance;
	protected ref map<int, ref ExpansionCityManagerNPCData> m_QuestsNPCs = new map<int, ref ExpansionCityManagerNPCData>();

    int CityID;
    string CityName;
    vector Position;
    int CityRadius;
    int CityCurrentLoyalty;
    int CityMaxReachLoyalty;
    bool CityLiberated;
	int CityIncome;
	bool CityTrain;
	bool CityBus;
	int CityTraders;
	string CityManagerImage;

    string ClassName
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
	int NPCType = ExpansionQuestNPCType.NORMAL;

    [NonSerialized()]
	static string FILENAME = "NONE";
	
	bool Active = true;

    void ExpansionCityManagerNPCData()
	{
		s_Instance = this;
		m_QuestsNPCs = new map<int, ref ExpansionCityManagerNPCData>();
	}

	static ExpansionCityManagerNPCData GetInstance()
	{
		if (!s_Instance)
			s_Instance = new ExpansionCityManagerNPCData();
		return s_Instance;
	}

    void SetID(int id)
	{
		CityID = id;
	}

	int GetID()
	{
		return CityID;
	}

    void SetCityName(string className)
	{
		CityName = className;
	}
	
	string GetCityName()
	{
		return CityName;
	}

    void SetPosition(vector pos)
	{
		Position = pos;
	}

	vector GetPosition()
	{
		return Position;
	}

    void SetCityRadius(int radius)
	{
		CityRadius = radius;
	}

	int GetCityRadius()
	{
		return CityRadius;
	}

    void SetCityCurrentLoyalty(int loyalty)
	{
		CityCurrentLoyalty = loyalty;
	}

	int GetCityCurrentLoyalty()
	{
		return CityCurrentLoyalty;
	}

    void SetCityMaxReachLoyalty(int maxloyalty)
	{
		CityMaxReachLoyalty = maxloyalty;
	}

	int GetCityMaxReachLoyalty()
	{
		return CityMaxReachLoyalty;
	}
    
    void SetCityLiberated(bool liberated)
	{
		CityLiberated = liberated;
	}

	bool isCityLiberated()
	{
		return CityLiberated;
	}

    void SetCityTrain(bool train)
	{
		CityTrain = train;
	}

	bool cityHasTrain()
	{
		return CityTrain;
	}

	void SetCityBus(bool bus)
	{
		CityBus = bus;
	}

	bool cityHasBus()
	{
		return CityBus;
	}

	void SetCityIncome(int income)
	{
		CityIncome = income;
	}

	int GetCityIncome()
	{
		return CityIncome;
	}

	void SetCityTraders(int traders)
	{
		CityTraders = traders;
	}

	int GetCityTraders()
	{
		return CityTraders;
	}

	void SetCityManagerImage(string filename)
	{
		CityManagerImage = filename;
	}

	string GetCityManagerImage()
	{
		return CityManagerImage;
	}

    void SetClassName(string className)
	{
		ClassName = className;
	}
	
	string GetClassName()
	{
		return ClassName;
	}

	void SetNPCType(ExpansionQuestNPCType type)
	{
		NPCType = type;
	}

	ExpansionQuestNPCType GetNPCType()
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

    ExpansionCityManagerNPCBase SpawnNPC()
	{
		int flags = ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE;

		if (GetGame().IsKindOf(ClassName, "DZ_LightAI"))  //! Only true for animals and Zs
			flags |= ECE_INITAI;

		Object obj = ExpansionGame.CreateObjectExSafe(ClassName, Position, flags);
 		ExpansionCityManagerNPCBase questNPC;
		if (!ExpansionCityManagerNPCBase.CastTo(questNPC, obj))
	    {
			Error("ExpansionCityManagerNPCDataBase::SpawnNPC - Used unsupported object " + ClassName + " as quest NPC in config. Only objects based on ExpansionCityManagerNPCBase class are allowed!");
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
	ExpansionCityManagerNPCAIBase SpawnNPCAI()
	{
		Print("SpawnNPCAI in Data has been called!")
		Position = ExpansionAIPatrol.GetPlacementPosition(Position);
		Print("City Manager NPC Position = " + Position)
		Object obj = ExpansionGame.CreateObjectSafe(ClassName, Position);
		if (!obj){
			Print("IS NOT AN OBJECT");
			return NULL;}

		ExpansionCityManagerNPCAIBase questNPC = ExpansionCityManagerNPCAIBase.Cast(obj);
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

        ExpansionCityManagerStaticObject SpawnObject()
        {
            Object obj = ExpansionGame.CreateObjectExSafe(ClassName, Position, ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE);
            ExpansionCityManagerStaticObject questObject;
            if (!ExpansionCityManagerStaticObject.CastTo(questObject, obj))
            {
                Error("ExpansionCityManagerNPCDataBase::SpawnNPC - Used unsupported object " + ClassName + " as AI quest NPC in config. Only objects based on ExpansionCityManagerStaticObject class are allowed!");
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

    static ExpansionCityManagerNPCData Load(string fileName)
	{
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";
		Print("[ExpansionCityManagerNPCData] Load existing configuration file:" + fileName);
		ExpansionCityManagerNPCData npcConfig;
        ExpansionJsonFileParser<ExpansionCityManagerNPCData>.Load(EXPANSION_CITYMANAGER_FOLDER + fileName, npcConfig);
		Print("[ExpansionCityManagerNPCData] ExpansionCityManagerNPCData:" + npcConfig);
		return npcConfig;
	}

    void Save(string fileName)
	{
		if (!ExpansionString.EndsWithIgnoreCase(fileName, ".json"))
			fileName += ".json";

		ExpansionJsonFileParser<ExpansionCityManagerNPCData>.Save(EXPANSION_CITYMANAGER_FOLDER + fileName, this);
	}

    void CopyConfig(ExpansionCityManagerNPCData npcDataBase)
	{
		CityID = npcDataBase.CityID;
        CityName = npcDataBase.CityName;
        CityRadius = npcDataBase.CityRadius;
        CityCurrentLoyalty = npcDataBase.CityCurrentLoyalty;
        CityMaxReachLoyalty = npcDataBase.CityMaxReachLoyalty;
        CityLiberated = npcDataBase.CityLiberated;
		CityIncome = npcDataBase.CityIncome;
		CityTrain = npcDataBase.CityTrain;
		CityBus = npcDataBase.CityBus;
		CityTraders = npcDataBase.CityTraders;

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
        ctx.Write(CityName);
        ctx.Write(CityRadius);
        ctx.Write(CityCurrentLoyalty);
        ctx.Write(CityMaxReachLoyalty);
        ctx.Write(CityLiberated);
		ctx.Write(CityIncome);
		ctx.Write(CityTrain);
		ctx.Write(CityBus);
		ctx.Write(CityTraders);
		ctx.Write(CityManagerImage);
		ctx.Write(NPCName);
	}

    bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(CityID))
			return false;
        
        if (!ctx.Read(CityName))
            return false;

        if (!ctx.Read(CityRadius))
            return false;

        if (!ctx.Read(CityCurrentLoyalty))
            return false;

        if (!ctx.Read(CityMaxReachLoyalty))
            return false;

        if (!ctx.Read(CityLiberated))
            return false;
		
		if (!ctx.Read(CityIncome))
			return false;

		if (!ctx.Read(CityTrain))
		return false;

		if (!ctx.Read(CityBus))
		return false;
		
		if (!ctx.Read(CityTraders))
		return false;
		
		if (!ctx.Read(CityManagerImage))
		return false;

		if (!ctx.Read(NPCName))
		return false;
		
		return true;
	}

};