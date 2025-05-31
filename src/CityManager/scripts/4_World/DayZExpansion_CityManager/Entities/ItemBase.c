modded class ItemBase
{
	protected bool m_LiberatedCityCleanup;
	protected ExpansionCityManagerModule cityModule;

    /*override void EEInit()
    {
        super.EEInit();

        if (!GetGame().IsServer())
            return;

        if (GetHierarchyParent())
            return;

        ExpansionCreateLiberatedCityCleanup();  // Will auto-check grid and delete
    }*/


    void ExpansionCreateLiberatedCityCleanup()
    {
        if (IsInherited(ExpansionTemporaryOwnedContainer))
            return;

        if (m_LiberatedCityCleanup)
            return;

        if (GetHierarchyParent())
            return;

        if (!cityModule)
            Class.CastTo(cityModule, CF_ModuleCoreManager.Get(ExpansionCityManagerModule));

        if (!cityModule)
            return;

        if (!cityModule.IsInsideLiberatedCity(GetPosition()))
            return;

        m_LiberatedCityCleanup = true;

        #ifdef EXPANSION_CITY_DEBUG
        EXPrint("[City][LiberatedCleanup] " + ToString() + " at " + GetPosition() + " deleted");
        #endif

        GetGame().ObjectDelete(this);
    }

}
