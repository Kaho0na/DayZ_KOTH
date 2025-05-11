class ExpansionTravelStationsConfirmDialog: ExpansionDialogBase
{
	protected ref ExpansionDialogContent_Text m_Text;
	protected ref ExpansionTravelStationsConfirmDialogButton_Accept m_AcceptButton;
	protected ref ExpansionTravelStationsConfirmDialogButton_Cancel m_CancelButton;

	protected ref ExpansionTravelStationsMenu m_Menu;
	protected int m_Cost;
	protected string m_StationName;

	void ExpansionTravelStationsConfirmDialog(ScriptView parentView, string stationName, int cost)
	{
		m_ParentView = parentView;
		m_Menu = ExpansionTravelStationsMenu.Cast(parentView);
		m_Cost = cost;
		m_StationName = stationName;
        GetLayoutRoot().SetColor(ARGB(225, 0, 0, 0));

		m_Text = new ExpansionDialogContent_Text(this);
		AddContent(m_Text);
		m_Text.SetText(string.Format("Are you sure you want to travel to %1 for $%2?", m_StationName, m_Cost));
		m_Text.Show();

        if (!m_AcceptButton)
		{
		    m_AcceptButton = new ExpansionTravelStationsConfirmDialogButton_Accept(this);
		    AddButton(m_AcceptButton);
            m_AcceptButton.Show();
		}

        if (!m_CancelButton)
		{
		    m_CancelButton = new ExpansionTravelStationsConfirmDialogButton_Cancel(this);
		    AddButton(m_CancelButton);
            m_CancelButton.Show();
		}
	}

	override string GetDialogTitle()
	{
		return "Confirm Travel";
	}

	ExpansionTravelStationsMenu GetMenu()
	{
		return m_Menu;
	}
}

class ExpansionTravelStationsConfirmDialogButton_Accept: ExpansionDialogButton_Text
{
    protected ref ExpansionTravelStationsConfirmDialog m_TransferDialog;

	void ExpansionTravelStationsConfirmDialogButton_Accept(ExpansionDialogBase dialog)
	{
		m_Dialog = dialog;

        if (!m_TransferDialog)
        m_TransferDialog = ExpansionTravelStationsConfirmDialog.Cast(m_Dialog);


		SetButtonText("CONFIRM");  // Make sure this is set
		SetTextColor(ARGB(255, 200, 255, 200));  // Light green
	}
	
	override void OnButtonClick()
	{
		ExpansionTravelStationsConfirmDialog confirmDialog = ExpansionTravelStationsConfirmDialog.Cast(m_Dialog);
		confirmDialog.GetMenu().OnConfirmTravel();
		confirmDialog.Hide();
		confirmDialog.Destroy();
	}
}


class ExpansionTravelStationsConfirmDialogButton_Cancel: ExpansionDialogButton_Text
{
    protected ref ExpansionTravelStationsConfirmDialog m_TransferDialog;

	void ExpansionTravelStationsConfirmDialogButton_Cancel(ExpansionDialogBase dialog)
	{
		m_Dialog = dialog;

        if (!m_TransferDialog)
        m_TransferDialog = ExpansionTravelStationsConfirmDialog.Cast(m_Dialog);

        SetButtonText("CANCEL");
        SetTextColor(ARGB(255, 255, 200, 200));  // Light red
        
	}

	override void OnButtonClick()
	{
		m_Dialog.Hide();
		m_Dialog.Destroy();
	}
}

