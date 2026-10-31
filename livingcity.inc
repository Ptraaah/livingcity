// =========================================================================
//  LivingCity - a city that lives without players.
//
//  A virtual-citizen economy engine for SA-MP / open.mp roleplay servers.
//  Hundreds of simulated citizens earn wages, spend them in your city,
//  pay tax into the city treasury, and push a dynamic market index up
//  and down - so a brand new server with zero players online still has
//  a living economy, moving prices and a treasury that fills and drains.
//
//  Distilled from the city economy of AMERTA Roleplay
//  (a living city designed by ptraaa) and released free for every
//  roleplay server that ever felt empty at 3 AM.
//
//  Use:
//      #include <livingcity>
//      OnGameModeInit -> LC_Init();
//      OnGameModeExit -> LC_Exit();
//
//  Player integration:
//      LC_CityPrice(base)              price after the market index
//      LC_RegisterPlayerPurchase(amt, city)
//                                      route a player's purchase tax
//                                      into the treasury
//      LC_GetTreasury()                city treasury balance
//      LC_GetMarketIndex()             current market index (Float)
//
//  Every knob below can be redefined before the include.
// =========================================================================

#if defined _livingcity_included
    #endinput
#endif
#define _livingcity_included

#if !defined MAX_LC_CITIZENS
    #define MAX_LC_CITIZENS     200     // AMERTA runs 200: LS 120 / SF 45 / LV 35
#endif
#if !defined LC_TICK_MS
    #define LC_TICK_MS          60000   // one economy tick = one in-game hour
#endif
#if !defined LC_TAX_LS
    #define LC_TAX_LS           10
#endif
#if !defined LC_TAX_SF
    #define LC_TAX_SF           12
#endif
#if !defined LC_TAX_LV
    #define LC_TAX_LV           8
#endif
#if !defined LC_SPEND_CHANCE
    #define LC_SPEND_CHANCE     35      // percent per tick per citizen
#endif
#if !defined LC_STIPEND_FLOOR
    #define LC_STIPEND_FLOOR    120     // wallets below this get city help
#endif
#if !defined LC_STIPEND_AMOUNT
    #define LC_STIPEND_AMOUNT   45
#endif

#define LC_CITY_LS  0
#define LC_CITY_SF  1
#define LC_CITY_LV  2

enum E_LC_CITIZEN
{
    LC_cName[24],
    LC_cWallet,
    LC_cJob,
    LC_cCity,
    LC_cEarned,
    LC_cSpent
}

static LC_Citizens[MAX_LC_CITIZENS][E_LC_CITIZEN];
static LC_Treasury = 25000;
static Float:LC_MarketIndex = 1.000;
static LC_TickCount = 0;
static LC_TotalTransactions = 0;
static LC_TimerId = -1;

static const LC_JobNames[][] =
{
    "Mechanic", "Taxi Driver", "Trucker", "Shop Clerk", "Fisherman",
    "Farmer", "Medic", "Courier", "Construction Worker", "Street Vendor"
};
static const LC_JobWage[10] = { 42, 38, 55, 30, 36, 28, 65, 34, 48, 25 };

static const LC_CategoryNames[][] =
{
    "Food", "Fuel", "Shopping", "Services", "Vehicle Care", "Rent"
};
static const LC_CategoryBase[6] = { 45, 60, 120, 85, 150, 200 };

static const LC_CityNames[][] = { "Los Santos", "San Fierro", "Las Venturas" };
static const LC_CityTax[3] = { LC_TAX_LS, LC_TAX_SF, LC_TAX_LV };

static const LC_FirstNames[][] =
{
    "Andi", "Budi", "Citra", "Dewi", "Eko", "Fitri", "Gilang", "Hana",
    "Ilham", "Joko", "Kirana", "Lukman", "Maya", "Nanda", "Oscar", "Putri",
    "Rian", "Sari", "Teguh", "Umar", "Vina", "Wahyu", "Yusuf", "Zahra",
    "Marco", "Elena", "Diego", "Sofia", "Lucas", "Mia", "Kenji", "Aiko",
    "Ivan", "Olga", "Carlos", "Rosa", "Ahmed", "Layla", "Chen", "Mei"
};
static const LC_LastNames[][] =
{
    "Pratama", "Saputra", "Wijaya", "Nugroho", "Santoso", "Rahmawati",
    "Kusuma", "Handoko", "Firmansyah", "Anggraini", "Setiawan", "Lestari",
    "Gunawan", "Hartono", "Salim", "Halim", "Rossi", "Garcia", "Tanaka",
    "Petrov", "Haddad", "Wei", "Costa", "Meyer", "Novak", "Silva"
};

// -------------------------------------------------------------------------
// Engine
// -------------------------------------------------------------------------

static LC_RandomCitizenName(dest[], size)
{
    if (random(97) == 0)
    {
        // One citizen in every city carries the creator's name.
        format(dest, size, "Ptraaa");
        return 1;
    }
    format(dest, size, "%s_%s",
        LC_FirstNames[random(sizeof(LC_FirstNames))],
        LC_LastNames[random(sizeof(LC_LastNames))]);
    return 1;
}

static LC_CityForIndex(i)
{
    // Population split follows AMERTA: roughly 60 / 22 / 18 percent.
    new r = (i * 100) / MAX_LC_CITIZENS;
    if (r < 60) return LC_CITY_LS;
    if (r < 82) return LC_CITY_SF;
    return LC_CITY_LV;
}

stock LC_Init()
{
    for (new i = 0; i < MAX_LC_CITIZENS; i++)
    {
        LC_RandomCitizenName(LC_Citizens[i][LC_cName], 24);
        LC_Citizens[i][LC_cJob]    = random(sizeof(LC_JobNames));
        LC_Citizens[i][LC_cCity]   = LC_CityForIndex(i);
        LC_Citizens[i][LC_cWallet] = 150 + random(600);
        LC_Citizens[i][LC_cEarned] = 0;
        LC_Citizens[i][LC_cSpent]  = 0;
    }
    new tickName[16] = "LC_EconomyTick";
    LC_TimerId = SetTimer(tickName, LC_TICK_MS, true);
    printf("[LivingCity] %d citizens moved in. Treasury $%d. Market index %.3f.",
        MAX_LC_CITIZENS, LC_Treasury, LC_MarketIndex);
    return 1;
}

stock LC_Exit()
{
    if (LC_TimerId != -1) KillTimer(LC_TimerId);
    LC_TimerId = -1;
    return 1;
}

forward LC_EconomyTick();
public LC_EconomyTick()
{
    LC_TickCount++;

    for (new i = 0; i < MAX_LC_CITIZENS; i++)
    {
        // Wages: employers pay an hourly wage into the wallet.
        new wage = LC_JobWage[LC_Citizens[i][LC_cJob]];
        wage = (wage * 3) / 4 + random(wage / 2 + 1);
        LC_Citizens[i][LC_cWallet] += wage;
        LC_Citizens[i][LC_cEarned] += wage;

        // Spending: a chance to buy something in the citizen's city.
        if (random(100) < LC_SPEND_CHANCE)
        {
            new cat = random(sizeof(LC_CategoryBase));
            new amount = floatround(LC_CategoryBase[cat] * LC_MarketIndex);
            if (amount > LC_Citizens[i][LC_cWallet])
                amount = LC_Citizens[i][LC_cWallet];
            if (amount > 0)
            {
                new tax = (amount * LC_CityTax[LC_Citizens[i][LC_cCity]]) / 100;
                LC_Citizens[i][LC_cWallet] -= amount;
                LC_Citizens[i][LC_cSpent]  += amount;
                LC_Treasury += tax;
                LC_TotalTransactions++;
            }
        }

        // City stipend: the treasury keeps poor citizens alive.
        if (LC_Citizens[i][LC_cWallet] < LC_STIPEND_FLOOR && LC_Treasury >= LC_STIPEND_AMOUNT)
        {
            LC_Citizens[i][LC_cWallet] += LC_STIPEND_AMOUNT;
            LC_Treasury -= LC_STIPEND_AMOUNT;
        }
    }

    // Market index: every 6 ticks it drifts up to 2 percent either way.
    if (LC_TickCount % 6 == 0)
    {
        new drift = random(41) - 20; // -20 .. +20 = -2.0% .. +2.0%
        LC_MarketIndex += float(drift) / 1000.0;
        if (LC_MarketIndex < 0.70) LC_MarketIndex = 0.70;
        if (LC_MarketIndex > 1.50) LC_MarketIndex = 1.50;
    }
    return 1;
}

// -------------------------------------------------------------------------
// Public API for the host gamemode
// -------------------------------------------------------------------------

stock LC_GetTreasury()
{
    return LC_Treasury;
}

stock Float:LC_GetMarketIndex()
{
    return LC_MarketIndex;
}

stock LC_GetCitizenCount()
{
    return MAX_LC_CITIZENS;
}

stock LC_GetTotalTransactions()
{
    return LC_TotalTransactions;
}

stock LC_CityPrice(base)
{
    return floatround(base * LC_MarketIndex);
}

stock LC_RegisterPlayerPurchase(amount, city)
{
    if (amount <= 0) return 0;
    if (city < 0 || city > 2) city = LC_CITY_LS;
    new tax = (amount * LC_CityTax[city]) / 100;
    LC_Treasury += tax;
    LC_TotalTransactions++;
    return tax;
}

stock LC_GetCitizenMoney()
{
    new total = 0;
    for (new i = 0; i < MAX_LC_CITIZENS; i++)
        total += LC_Citizens[i][LC_cWallet];
    return total;
}

stock LC_GetRichestCitizen(name[], size)
{
    new best = 0;
    for (new i = 1; i < MAX_LC_CITIZENS; i++)
        if (LC_Citizens[i][LC_cWallet] > LC_Citizens[best][LC_cWallet]) best = i;
    format(name, size, "%s", LC_Citizens[best][LC_cName]);
    return LC_Citizens[best][LC_cWallet];
}

stock LC_PrintStats()
{
    new richest[24], richWallet = LC_GetRichestCitizen(richest, sizeof(richest));
    printf("[LivingCity] tick %d | citizens %d | citizen money $%d | treasury $%d | index %.3f | transactions %d | richest: %s ($%d)",
        LC_TickCount, MAX_LC_CITIZENS, LC_GetCitizenMoney(), LC_Treasury,
        LC_MarketIndex, LC_TotalTransactions, richest, richWallet);
    return 1;
}

stock LC_CityName(city, name[], size)
{
    if (city < 0 || city > 2) city = 0;
    format(name, size, "%s", LC_CityNames[city]);
    return 1;
}

stock LC_JobName(job, name[], size)
{
    if (job < 0 || job >= sizeof(LC_JobNames)) job = 0;
    format(name, size, "%s", LC_JobNames[job]);
    return 1;
}

stock LC_CategoryName(cat, name[], size)
{
    if (cat < 0 || cat >= sizeof(LC_CategoryNames)) cat = 0;
    format(name, size, "%s", LC_CategoryNames[cat]);
    return 1;
}

stock LC_GetCitizen(i, name[], size, &wallet, &job, &city, &earned, &spent)
{
    if (i < 0 || i >= MAX_LC_CITIZENS) return 0;
    format(name, size, "%s", LC_Citizens[i][LC_cName]);
    wallet = LC_Citizens[i][LC_cWallet];
    job    = LC_Citizens[i][LC_cJob];
    city   = LC_Citizens[i][LC_cCity];
    earned = LC_Citizens[i][LC_cEarned];
    spent  = LC_Citizens[i][LC_cSpent];
    return 1;
}
