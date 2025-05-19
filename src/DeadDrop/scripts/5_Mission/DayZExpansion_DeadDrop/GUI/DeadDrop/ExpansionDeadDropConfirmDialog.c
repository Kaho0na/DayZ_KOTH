class ExpansionDeadDropConfirmDialog: ExpansionDialogBase
{
	protected ref ExpansionDialogContent_Text m_Text;
	protected ref ExpansionDeadDropConfirmDialogButton_Accept m_AcceptButton;
	protected ref ExpansionDeadDropConfirmDialogButton_Cancel m_CancelButton;

	protected ref ExpansionDeadDropMenu m_Menu;
	protected int m_Cost;


	void ExpansionDeadDropConfirmDialog(ScriptView parentView)
	{
		m_ParentView = parentView;
		m_Menu = ExpansionDeadDropMenu.Cast(parentView);
		m_Cost = GetExpansionSettings().GetDeadDrop().RecoveryCost;
        GetLayoutRoot().SetColor(ARGB(225, 0, 0, 0));

		m_Text = new ExpansionDialogContent_Text(this);
		AddContent(m_Text);
		m_Text.SetText(string.Format("Are you sure you want to recover this loot for $%2?", m_Cost));
		m_Text.Show();

        if (!m_AcceptButton)
		{
		    m_AcceptButton = new ExpansionDeadDropConfirmDialogButton_Accept(this);
		    AddButton(m_AcceptButton);
            m_AcceptButton.Show();
		}

        if (!m_CancelButton)
		{
		    m_CancelButton = new ExpansionDeadDropConfirmDialogButton_Cancel(this);
		    AddButton(m_CancelButton);
            m_CancelButton.Show();
		}
	}

	override string GetDialogTitle()
	{
		return "Confirm Recovery";
	}

	ExpansionDeadDropMenu GetMenu()
	{
		return m_Menu;
	}
}

class ExpansionDeadDropConfirmDialogButton_Accept: ExpansionDialogButton_Text
{
    protected ref ExpansionDeadDropConfirmDialog m_TransferDialog;

	void ExpansionDeadDropConfirmDialogButton_Accept(ExpansionDialogBase dialog)
	{
		m_Dialog = dialog;

        if (!m_TransferDialog)
        m_TransferDialog = ExpansionDeadDropConfirmDialog.Cast(m_Dialog);


		SetButtonText("CONFIRM");  // Make sure this is set
		SetTextColor(ARGB(255, 200, 255, 200));  // Light green
	}
	
	override void OnButtonClick()
	{
		ExpansionDeadDropConfirmDialog confirmDialog = ExpansionDeadDropConfirmDialog.Cast(m_Dialog);
		confirmDialog.GetMenu().OnConfirmRecovery();
		confirmDialog.Hide();
		confirmDialog.Destroy();
	}
}


class ExpansionDeadDropConfirmDialogButton_Cancel: ExpansionDialogButton_Text
{
    protected ref ExpansionDeadDropConfirmDialog m_TransferDialog;

	void ExpansionDeadDropConfirmDialogButton_Cancel(ExpansionDialogBase dialog)
	{
		m_Dialog = dialog;

        if (!m_TransferDialog)
        m_TransferDialog = ExpansionDeadDropConfirmDialog.Cast(m_Dialog);

        SetButtonText("CANCEL");
        SetTextColor(ARGB(255, 255, 200, 200));  // Light red
        
	}

	override void OnButtonClick()
	{
		m_Dialog.Hide();
		m_Dialog.Destroy();
	}
}

