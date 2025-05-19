modded class PlayerBase
{
    override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(ExpansionActionOpenDeadDropMenu, InputActionMap);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if (GetGame().IsServer())
		{
			ExpansionDeadDropModule module;
			if (CF_Modules<ExpansionDeadDropModule>.Get(module))
			{
				module.OnPlayerKilled(this, killer);
				Print("[DeadDrop] Player is dead ");

			}
		}
	}

};