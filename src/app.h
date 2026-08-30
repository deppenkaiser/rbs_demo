#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <sm/sm.h>

#include <rbs/rbs.h>
#include <rbs/rbs_sm.h>

/* ---------------------------------------------------------------------------
 * App-Datenmodell: die deklarativen Programmstrukturen des rbs-Programms.
 *
 * Dieses Modul bündelt alles, was die App *ausmacht* — die Tokens (Fakten)
 * und Values (Memory), ihre Namen, sowie die Regel-/Effekt-/Slot-Tabellen.
 * Die Handler (Lebenszyklus + externe Faktenquellen) werden nur als
 * Prototypen deklariert und sind in app.c implementiert.
 *
 * Die Tabellen sind per `static` inkludiert: jede Übersetzungseinheit, die
 * app.h einbindet, erhält ihre eigene Kopie der (konstanten) Programme.
 * ------------------------------------------------------------------------ */

/* Token = Faktenslot. Positive Formen sind der aktive Zustand, negierte Formen
 * (`N_X`) die „nicht (mehr) aktive" Form. ZERO beendet Term-/Fakten-Arrays. */
typedef enum token
{
	N_PAY = -7,
	N_ADULT = -6,
	N_UMBRELLA = -5,
	N_WET = -4,
	N_CLOUDY = -3,
	N_RAIN = -2,
	ZERO = 0,
	RAIN = 2,
	CLOUDY = 3,
	WET = 4,
	UMBRELLA = 5,
	ADULT = 6,
	PAY = 7,
	TOKEN_COUNT
}* token_t;

/* Namen je Magnitude - 1 (z. B. WET=4 -> token_names[3]). Erster Slot (Idx 0,
 * Magnitude 1) ist unbenutzt; so viele Eintraege wie TOKEN_COUNT decken
 * Magnituden 1..TOKEN_COUNT ab und halten den Zugriff in rbs.c im Rahmen. */
static const char* token_names[] =
{
	"",         /* Idx 0: Magnitude 1 (unbenutzt) */
	"RAIN",     /* Idx 1: RAIN    = 2 */
	"CLOUDY",   /* Idx 2: CLOUDY  = 3 */
	"WET",      /* Idx 3: WET     = 4 */
	"UMBRELLA", /* Idx 4: UMBRELLA = 5 */
	"ADULT",    /* Idx 5: ADULT   = 6 */
	"PAY",      /* Idx 6: PAY     = 7 */
	"",         /* Idx 7: Magnitude 8 */
	""          /* Idx 8: Magnitude 9 */
};

/* Value = Memory-Slot (kontinuierlicher Wertebereich der Simulation). */
typedef enum value
{
	AGE,
	MONEY,
	VALUE_COUNT
}* value_t;

/* --- Handler-Prototypen (Implementierung in app.c) --- */

/* Externe Faktenquelle (z. B. Wettersensor): meldet das Ende des Regens. */
bool app_handle_wet(sm_state_t next_state, void* user_data);
/* App-Ende: keine weiteren externen Ereignisse mehr -> FSM beenden. */
bool app_handle_adult(sm_state_t next_state, void* user_data);
/* Schritt-Callback: liefert je Schritt eine Zeile (Step-Grenze/Status). */
void app_on_step(rbs_sm_t fsm, uint32_t tick);

/* Der Konstruktor/Destruktor der App-FSM ist der sm-Lebenszyklus-Callback
 * (sm_on_start/sm_on_stop), deklariert in <sm/sm.h> und in app.c als
 * `callback` ueberschrieben. Dort laeuft er im Worker-Thread ganz am Anfang
 * (z. B. Speicherverwaltung/Ressourcen) bzw. ganz am Ende. */

/* --- Regeln --- */

/* Regen oder bewoelkt -> nass; sonst trocken (else negiert WET). */
static struct rbs_term app_if_weather[] =
{
	{ .comparison = false, .fact_enum = RAIN },
	{ .comparison = false, .fact_enum = CLOUDY },
	{ .comparison = false, .fact_enum = ZERO }
};
static enum token app_then_weather[] = { WET, ZERO };
static enum token app_else_weather[] = { N_WET, ZERO };

/* Nass -> Schirm noetig; sonst (trocken) kein Schirm (else negiert UMBRELLA). */
static struct rbs_term app_if_wet[] =
{
	{ .comparison = false, .fact_enum = WET },
	{ .comparison = false, .fact_enum = ZERO }
};
static enum token app_then_wet[] = { UMBRELLA, ZERO };
static enum token app_else_wet[] = { N_UMBRELLA, ZERO };

/* Erwachsen -> bezahlt. */
static struct rbs_term app_if_adult[] =
{
	{ .comparison = true, .value_enum = AGE, .op = GT, .operand = 18 },
	{ .comparison = false, .fact_enum = ZERO }
};
static enum token app_then_adult[] = { ADULT, PAY, ZERO };

static struct rbs_rule app_rules[] =
{
	{ app_if_weather, app_then_weather, app_else_weather },
	{ app_if_wet, app_then_wet, app_else_wet },
	{ app_if_adult, app_then_adult, NULL }
};

/* --- Effekte --- */

/* Bezahlter Betrag wird von MONEY abgezogen, solange PAY aktiv ist. */
static struct rbs_effect app_effects[] =
{
	{ .trigger_fact_enum = PAY, .value_enum = MONEY, .op = SUB, .operand = 10 }
};

/* --- Zustands-Slots --- */

static struct rbs_sm_slot app_slots[] =
{
	{ .fact = WET,   .handler = app_handle_wet },
	{ .fact = ADULT, .handler = app_handle_adult },
};
