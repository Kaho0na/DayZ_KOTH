modded class InGameMenu
{
	private ButtonWidget m_ChangeTeamButton;
	private TextWidget m_ChangeTeamLabel;

	override Widget Init()
	{
		Widget layoutRoot = super.Init();

		//! Find the existing Feedback button and relabel it
		m_ChangeTeamButton = ButtonWidget.Cast(layoutRoot.FindAnyWidget("feedbackbtn"));
		if (m_ChangeTeamButton)
		{
			m_ChangeTeamLabel = TextWidget.Cast(m_ChangeTeamButton.FindAnyWidget("feedbackbtn_label"));
			if (m_ChangeTeamLabel)
			{
				m_ChangeTeamLabel.SetText("CHANGE TEAM");
			}

			Print("[KOTH] Feedback button converted to Change Team button!");
		}
		else
		{
			Print("[KOTH] ERROR: Could not find feedbackbtn in InGameMenu layout!");
		}

		return layoutRoot;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		//! Detect click on our repurposed button
		if (w == m_ChangeTeamButton)
		{
			Print("[KOTH] Change Team button clicked!");
			Close();
			KOTH_TeamSelectionModule teamModule;
			CF_Modules<KOTH_TeamSelectionModule>.Get(teamModule);
            if (teamModule)
            {
                auto rpc = teamModule.Expansion_CreateRPC("RPC_RequestTeamChange");
                rpc.Expansion_Send(true);
            }
			return true;
		}

		return super.OnClick(w, x, y, button);
	}
}
