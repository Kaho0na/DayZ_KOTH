class ExpansionActionOpenFactionMenu: ActionInteractBase
{	
	private ExpansionMarketModule m_MarketModule;

	void ExpansionActionOpenFactionMenu()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ATTACHITEM;
		m_Text = "#use";
	}

	override void CreateConditionComponents()  
	{	
		m_ConditionItem = new CCINone;
		m_ConditionTarget = new CCTCursor;
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		if (!GetExpansionSettings().GetMarket(false).IsLoaded())
			return false;

		if (!GetExpansionSettings().GetMarket().ATMSystemEnabled)
			return false;
		
		if (!CF_Modules<ExpansionMarketModule>.Get(m_MarketModule))
			return false;

		if (!m_MarketModule.CanOpenMenu())
			return false;
		
		if (!GetGame().IsDedicatedServer())
		{
			if ( !target.GetObject() )
				return false;

			m_Text = "Open Resistance Bank";
		}

		return true;
	}

    override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);
		
		m_MarketModule.SendPlayerFactionData(action_data.m_Player.GetIdentity());
	}
}