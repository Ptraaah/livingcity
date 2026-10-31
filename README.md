# LivingCity

**A city that lives without players.**

Every small roleplay server knows the feeling: a beautiful map, a finished
gamemode — and three players online at 3 AM. The world feels dead, so the
three players leave too.

LivingCity fixes the *feeling* at its root. It is a virtual-citizen economy
engine for SA-MP / open.mp: up to 200 simulated citizens (Los Santos, San
Fierro, Las Venturas) who hold jobs, earn hourly wages, spend money on food,
fuel, shopping and rent, pay city tax into a treasury, receive stipends when
they are broke, and push a dynamic market index that moves your prices
up and down — every minute, whether anyone is online or not.

It is distilled from the city economy of **AMERTA Roleplay**, a live
Indonesian roleplay server whose city runs on exactly this design, and
released free for every server that ever felt empty.

## Proof it runs

Compile-clean with pawncc 3.10 (zero errors, zero warnings). Booted on a
real `samp03svr`:

```
[12:06:35] [LivingCity] 200 citizens moved in. Treasury $25000. Market index 1.000.
```

## Install

1. Copy `livingcity.inc` into your `pawno/include` (or `qawno/include`).
2. In your gamemode:

```pawn
#include <livingcity>

public OnGameModeInit()
{
    LC_Init();
    // ...
}

public OnGameModeExit()
{
    LC_Exit();
    // ...
}
```

3. Done. Your city now has an economy. A ready-to-run demo lives in
   `examples/livingcity_demo.pwn` — log in and type `/citystats`.

## Hook your real economy in

```pawn
// Price anything by the living market index:
new price = LC_CityPrice(500);            // 500 at index 1.00, 350 at 0.70, 750 at 1.50

// Route a player purchase through the city: tax lands in the treasury.
new tax = LC_RegisterPlayerPurchase(price, LC_CITY_LS);   // LS / SF / LV tax rates differ

LC_GetTreasury();                          // city treasury balance
LC_GetMarketIndex();                       // Float, 0.70 .. 1.50
LC_GetCitizenMoney();                      // total money in citizen wallets
LC_PrintStats();                           // one-line economy report to the server log
```

## The model (AMERTA's rules)

- **200 citizens** by default, split roughly LS 120 / SF 45 / LV 35.
- **Finite wallets.** Citizens earn wages per job and spend from what they
  actually have — nobody conjures money.
- **City tax** on every purchase: LS 10% / SF 12% / LV 8%, into one treasury.
- **Stipends:** citizens under the floor get help from the treasury, so the
  poor never flatline and the treasury actually circulates.
- **Market index:** drifts up to ±2% every 6 ticks, clamped to [0.70, 1.50],
  and scales every price you pass through `LC_CityPrice`.

Every knob (`MAX_LC_CITIZENS`, `LC_TICK_MS`, tax rates, spend chance,
stipend floor/amount) is a `#define` you can override before the include.

Indonesian: mesin ekonomi warga virtual untuk server roleplay SA-MP /
open.mp. Warga simulasi bekerja, menerima gaji, belanja, membayar pajak ke
kas kota, dan menggerakkan indeks pasar — kota tetap hidup walaupun tidak
ada pemain online. Diambil dari desain ekonomi kota AMERTA Roleplay.

## Support / Dukung

Free and MIT. If LivingCity made your server feel alive, you can support
the maker: https://trakteer.id/adel_putra2

Gratis dan berlisensi MIT. Kalau LivingCity menghidupkan servermu, dukung
pembuatnya di https://trakteer.id/adel_putra2

---

LivingCity — built by **ptraaa**, creator of AMERTA Roleplay.
*Lived by your own thinking.*
