modded class InGameMenu
{
	private ButtonWidget m_ChangeTeamButton;
	private TextWidget m_ChangeTeamLabel;
	
	// Dialog ID for team change confirmation
	const int IDC_TEAM_CHANGE = 999;

	override Widget Init()
	{
		layoutRoot = super.Init();

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
			
			// Show confirmation dialog
			GetGame().GetUIManager().ShowDialog("Change Team", "Are you sure you want to change your team?\nThis will kill your current character.", IDC_TEAM_CHANGE, DBT_YESNO, DBB_NO, DMT_QUESTION, this);
			
			return true;
		}

		return super.OnClick(w, x, y, button);
	}
	
	// Handle dialog result
	override bool OnModalResult(Widget w, int x, int y, int code, int result)
	{
		if (code == IDC_TEAM_CHANGE)
		{
			if (result == DBB_YES)
			{
				Print("[KOTH] Player confirmed team change!");
				
				// Close the menu first
				Close();
				
				// Send the team change request to server
				KOTH_TeamSelectionModule teamModule;
				CF_Modules<KOTH_TeamSelectionModule>.Get(teamModule);
				if (teamModule)
				{
					auto rpc = teamModule.Expansion_CreateRPC("RPC_RequestTeamChange");
					rpc.Expansion_Send(true);
				}
			}
			else
			{
				Print("[KOTH] Player cancelled team change.");
			}
			
			return true;
		}
		
		return super.OnModalResult(w, x, y, code, result);
	}
}