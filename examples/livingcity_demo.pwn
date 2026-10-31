// LivingCity demo - a bare gamemode that boots the virtual-citizen
// economy and shows it off. Drop livingcity.inc in your pawno/include,
// compile, run, then log in and type /citystats.
//
// Commands:
//   /citystats    population, money in citizen wallets, treasury,
//                 market index, transaction count, richest citizen
//   /citybuy      simulate a $500 player purchase in Los Santos and
//                 watch its tax land in the treasury
//   (one more command exists. It answers to the creator's name.)

#include <a_samp>
#include <livingcity>

main() {}

public OnGameModeInit()
{
    SetGameModeText("LivingCity demo");
    AddPlayerClass(0, 1958.3783, 1343.1572, 15.3746, 269.1425, 0, 0, 0, 0, 0, 0);
    LC_Init();
    return 1;
}

public OnGameModeExit()
{
    LC_Exit();
    return 1;
}

public OnPlayerConnect(playerid)
{
    SendClientMessage(playerid, 0xC9A227FF,
        "Welcome. This city lives on its own - type /citystats and watch it breathe.");
    return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
    if (!strcmp(cmdtext, "/citystats", true))
    {
        new richest[24], line[160];
        new richWallet = LC_GetRichestCitizen(richest, sizeof(richest));
        format(line, sizeof(line), "Citizens: %d | Money in wallets: $%d | Treasury: $%d",
            LC_GetCitizenCount(), LC_GetCitizenMoney(), LC_GetTreasury());
        SendClientMessage(playerid, 0xFFFFFFAA, line);
        format(line, sizeof(line), "Market index: %.3f | Transactions so far: %d | Richest citizen: %s ($%d)",
            LC_GetMarketIndex(), LC_GetTotalTransactions(), richest, richWallet);
        SendClientMessage(playerid, 0xFFFFFFAA, line);
        return 1;
    }
    if (!strcmp(cmdtext, "/citybuy", true))
    {
        new price = LC_CityPrice(500);
        new tax = LC_RegisterPlayerPurchase(price, LC_CITY_LS);
        new line[144];
        format(line, sizeof(line),
            "You 'bought' an item priced $%d after the market index. $%d tax went to the city treasury (now $%d).",
            price, tax, LC_GetTreasury());
        SendClientMessage(playerid, 0xFFFFFFAA, line);
        return 1;
    }
    if (!strcmp(cmdtext, "/ptraaa", true))
    {
        SendClientMessage(playerid, 0xC9A227FF,
            "LivingCity - built by ptraaa, creator of AMERTA Roleplay. Lived by your own thinking.");
        return 1;
    }
    return 0;
}
