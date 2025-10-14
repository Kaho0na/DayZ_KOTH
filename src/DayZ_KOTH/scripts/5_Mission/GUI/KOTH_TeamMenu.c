class KOTH_TeamMenu: ExpansionScriptViewMenu
{
    private ref KOTH_TeamSelectionModule m_KOTHTeamSelectionModule;
    private ref KOTHTeamMenuController m_KOTHTeamMenuController;
	private MissionGameplay m_Mission;

    private ButtonWidget East;
	private TextWidget SpawnEast;
	private ButtonWidget West;
	private TextWidget SpawnWest;
    private ImageWidget Background;

    void KOTH_TeamMenu()
	{
        CF_Modules<KOTH_TeamSelectionModule>.Get(m_KOTHTeamSelectionModule);

		Class.CastTo(m_Mission, MissionGameplay.Cast(GetGame().GetMission()));
		Class.CastTo(m_KOTHTeamMenuController, GetController());
		
    }

    void ~KOTH_TeamMenu()
    {
        Clear();
    }

    override string GetLayoutFile() 
    {
        return "DayZ_KOTH/GUI/layouts/team_selection/koth_team_selection.layout";
    }

    override typename GetControllerType() 
    {
        return KOTHTeamMenuController;
    }

    void Clear()
    {
        if (m_KOTHTeamSelectionModule)
            m_KOTHTeamSelectionModule = null;

        if (m_KOTHTeamMenuController)
            m_KOTHTeamMenuController = null;
    }

    	override void OnShow()
	{
		super.OnShow();
		
		GetGame().GetInput().ChangeGameFocus(1);
		SetFocus(GetLayoutRoot());
		PPEffects.SetBlurMenu(0.5);
		m_Mission.GetHud().ShowHud(false);
		m_Mission.GetHud().ShowQuickBar(false);
        Background.LoadImageFile(0, "DayZExpansion/SpawnSelection/GUI/textures/wood_background.edds");

    }

    override void OnHide()
    {
        super.OnHide();
        GetDayZGame().GetExpansionGame().GetExpansionUIManager().CloseMenu();
        GetGame().GetInput().ResetGameFocus();
        PPEffects.SetBlurMenu(0.0);
        GetGame().GetMission().GetHud().ShowHud(true);
        GetGame().GetMission().GetHud().ShowQuickBar(true);
    }


    override bool OnMouseEnter(Widget w, int x, int y)
	{
		switch (w)
		{
			if (w == East)
			{
				East.SetColor(ARGB(255,220,220,220));
				SpawnEast.SetColor(ARGB(255,0,0,0));
			}
			else if (w == West)
			{
				West.SetColor(ARGB(255,220,220,220));
				SpawnWest.SetColor(ARGB(255,0,0,0));
			}
		}
		
		return super.OnMouseEnter(w, x, y);;
	}

    override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		switch (w)
		{
			if (w == East)
			{
				East.SetColor(ARGB(255,0,0,0));
				SpawnEast.SetColor(ARGB(255,220,220,220));
			}
			else if (w == West)
			{
				West.SetColor(ARGB(255,0,0,0));
				SpawnWest.SetColor(ARGB(255,220,220,220));
			}
		}
		
		return super.OnMouseLeave(w, enterW, x, y);
	}

    override bool OnClick(Widget w, int x, int y, int button)
    {
        switch (w)
        {
            case East:
                if (m_KOTHTeamSelectionModule)
                {
                    m_KOTHTeamSelectionModule.SelectTeam(1);
                    Hide();
                }
                return true;

            case West:
                if (m_KOTHTeamSelectionModule)
                {
                    m_KOTHTeamSelectionModule.SelectTeam(2);
                    Hide();
                }
                return true;
        }
        return super.OnClick(w, x, y, button);
    }

}

class KOTHTeamMenuController: ExpansionViewController
{
};
