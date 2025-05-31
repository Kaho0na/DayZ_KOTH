/**
 * FactionBankSalary.c
 *
 * DayZ Expansion Mod - Faction Bank by Kahoona
 * Credit to the DayZ Expansion Mod Team
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class FactionBankSalary: MissionServer
{
    private ref ExpansionMarketModule m_MarketModule;
    protected int m_FactionBankBalance;
    protected int m_MaxReputation;
    protected int m_BaseSalaryAmount;
    protected float m_SalaryMultiplier;
    protected int m_MoneyAddTimer;
    protected int maxDepositMoney;


    void FactionBankSalary()
    {
        if (GetGame().IsServer())
        {
            SetupSalarySystem();
        }
    }


    private void SetupSalarySystem()
    {
        m_MarketModule = ExpansionMarketModule.Cast(CF_ModuleCoreManager.Get(ExpansionMarketModule));
        if (!m_MarketModule)
        {
            ExpansionLogATM("Error: MarketModule not found! Salary system will not work.");
            return;
        }

        
        m_MaxReputation = GetExpansionSettings().GetHardline().MaxReputation;
        m_BaseSalaryAmount = GetExpansionSettings().GetFactionBank().BaseSalaryAmount;
        m_SalaryMultiplier = GetExpansionSettings().GetFactionBank().SalaryMultiplier;
        m_MoneyAddTimer = GetExpansionSettings().GetFactionBank().SalaryTimerInterval;
        maxDepositMoney = GetExpansionSettings().GetMarket().MaxDepositMoney;
        StartSalaryTimer();
    }

    private void StartSalaryTimer()
    {
        if (m_MoneyAddTimer <= 0)
            m_MoneyAddTimer = 60; // Default to 60s if config error

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(AddMoneyToActivePlayers, m_MoneyAddTimer * 1000, true);
        ExpansionLogATM("Salary timer started, interval: " + m_MoneyAddTimer.ToString() + " seconds.");
    }

    private void AddMoneyToActivePlayers()
    {
        if (!m_MarketModule)
            return;
        
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);

        foreach (Man player : players)
        {
            PlayerIdentity identity = player.GetIdentity();
            if (!identity)
                continue;

            ExpansionMarketATM_Data playerATMData = m_MarketModule.GetPlayerATMData(identity.GetId());
            if (!playerATMData)
                continue;
            // Get current faction bank balance from settings
            m_FactionBankBalance = GetExpansionSettings().GetFactionBank().FactionBankBalance;
            // Get player reputation
            PlayerBase playerBase = PlayerBase.Cast(player);
            if (!playerBase)
                continue;
            int playerReputation = playerBase.Expansion_GetReputation();

            // calculate salary based on reputation and faction bank balance
            int moneyToAdd = CalculatePlayerSalary(playerReputation);
            int currentATMMoney = playerATMData.GetMoney();

            // check if the ATM balance is full
            int futureDepositMoney = moneyToAdd + currentATMMoney;

            if (futureDepositMoney > maxDepositMoney)
            {
                int possibleDepositAmount = maxDepositMoney - currentATMMoney;
                if (possibleDepositAmount > 0)
                {
                    // if the ATM balance is full, add only the possible amount 
                    playerATMData.AddMoney(possibleDepositAmount);
                    ExpansionNotification(new StringLocaliser("Payment"), new StringLocaliser("You received a payout only %1. Bank account full!", possibleDepositAmount.ToString()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 7, ExpansionNotificationType.TOAST).Create(identity);
                    ExpansionLogATM(string.Format("Player \"%1\" received only %2 salary (bank full)", player.GetName(), possibleDepositAmount));
                
                }
                else
                {
                    // if the ATM balance is full, do not add any money and notify the player
                    ExpansionNotification(new StringLocaliser("Payment"), new StringLocaliser("You received no payout. Bank account full!"), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 7, ExpansionNotificationType.TOAST).Create(identity);
                    ExpansionLogATM(string.Format("Player \"%1\" received no salary (bank full)", player.GetName()));
                }
            }
            else // ATM is not full add the full amount 
            {
                playerATMData.AddMoney(moneyToAdd);
                ExpansionNotification(new StringLocaliser("Payment"), new StringLocaliser("You received a payout of %1.", moneyToAdd.ToString()), ExpansionIcons.GetPath("Exclamationmark"), COLOR_EXPANSION_NOTIFICATION_SUCCSESS, 7, ExpansionNotificationType.TOAST).Create(identity);
                ExpansionLogATM(string.Format("Player \"%1\" received %2 salary", player.GetName(), moneyToAdd));
            }

            playerATMData.Save();
        }
    }

    private int CalculatePlayerSalary(int playerReputation)
    {
        bool useReputation = GetExpansionSettings().GetHardline().UseReputation;
        bool factionEnabledReputation = GetExpansionSettings().GetFactionBank().FactionBankEnableReputation;
        int maxSalaryCap = GetExpansionSettings().GetFactionBank().MaxSalaryCap;

        if (!useReputation || !factionEnabledReputation)
            return m_BaseSalaryAmount;

        if (m_MaxReputation == 0 || playerReputation == 0)
            return m_BaseSalaryAmount;

        float reputationRatio = (float)playerReputation / (float)m_MaxReputation;
        int bonus = m_FactionBankBalance * reputationRatio * m_SalaryMultiplier;
        if (bonus > m_BaseSalaryAmount && bonus <= maxSalaryCap)
            return bonus;
        else if (bonus > maxSalaryCap)
            return maxSalaryCap;
        else
            return m_BaseSalaryAmount;
    }

    private void ExpansionLogATM(string message)
    {
        if (GetExpansionSettings().GetLog().ATM)
            GetExpansionSettings().GetLog().PrintLog("[ATM] " + message);
    }
}
