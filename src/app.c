#include <stdio.h>

#include <api/api.h>
#include <sm/sm.h>
#include <logging/logging.h>

#include "app.h"

/* Externe Faktenquelle (z. B. Wettersensor / Systemzeit): meldet das Ende
 * des Regens, indem RAIN und CLOUDY negiert werden. Das ist bewusster
 * externer Input — kein Konsum; die Regeln ziehen daraus selbst nach. */
bool app_handle_wet(sm_state_t next_state, void* user_data)
{
	rbs_sm_t fsm = (rbs_sm_t) user_data;

	rbs_set_fact_named(fsm->rbs, rbs_invert_token(RAIN));
	rbs_set_fact_named(fsm->rbs, rbs_invert_token(CLOUDY));

	return rbs_sm_advance(fsm, next_state);
}

/* App-Ende: keine weiteren externen Ereignisse mehr -> FSM beenden. */
bool app_handle_adult(sm_state_t next_state, void* user_data)
{
	rbs_sm_t fsm = (rbs_sm_t) user_data;
	(void) fsm;
	(void) next_state;

	return false;
}

/* Schritt-Callback: liefert je Schritt eine Zeile (Step-Grenze/Status)
 * gefolgt von einer Trennlinie. */
void app_on_step(rbs_sm_t fsm, uint32_t tick)
{
	rbs_t rbs = fsm->rbs;
	char buf[128];
	int pos = snprintf(buf, sizeof(buf), "step %u | UMBRELLA:", (unsigned) tick);
	pos += snprintf(buf + pos, sizeof(buf) - pos, " %s | MONEY:",
	                rbs_is_fact(rbs->facts, rbs->token_count, UMBRELLA) ? "true" : "false");
	snprintf(buf + pos, sizeof(buf) - pos, " %.0f", rbs->memory[MONEY]);
	logging_log_message(buf);
	logging_log_message("----------------------------");
}

/* Konstruktor (sm-Lebenszyklus-Callback): laeuft ganz am Anfang des
 * Worker-Threads und uebernimmt die Speicherverwaltung sowie den Aufbau der
 * initialen Welt — hier: rbs-Buffer anlegen + initialisieren, Welt-Konstanten
 * (AGE/MONEY) und die Ausgangslage (Regen + Bewoelkung) setzen. Danach
 * uebernimmt die Regel-Engine. */
callback void sm_on_start(sm_core_t core)
{
	rbs_sm_t fsm = (rbs_sm_t) core->user_data;
	struct rbs* rbs = fsm->rbs;

	/* Speicherverwaltung: Fakten-/Memory-Buffer anlegen + initialisieren. */
	rbs->facts = rbs_create_facts_buffer(rbs->token_count);
	rbs->memory = rbs_create_memory_buffer(rbs->value_count);
	rbs_initialize_facts(rbs->facts, rbs->token_count);
	rbs_initialize_memory(rbs->memory, rbs->value_count);

	/* Welt-Konstanten der Sim-Welt. */
	rbs->memory[AGE] = 20;
	rbs->memory[MONEY] = 100;

	/* Initiale Welt: es regnet und ist bewoelkt. Jedes Setzen wird von
	 * rbs_set_fact_named (mit Namen + Vorzeichen) geloggt. */
	rbs_set_fact_named(rbs, RAIN);
	rbs_set_fact_named(rbs, CLOUDY);
}

/* Destruktor (sm-Lebenszyklus-Callback): laeuft ganz am Ende des
 * Worker-Threads, nachdem die Schleife terminiert ist. Hier z. B. die
 * Endfakten bilanzieren und Ressourcen/Speicher freigeben. */
callback void sm_on_stop(sm_core_t core)
{
	rbs_sm_t fsm = (rbs_sm_t) core->user_data;

	logging_log_message(rbs_is_fact(fsm->rbs->facts, fsm->rbs->token_count, UMBRELLA) ?
	                    "app: UMBRELLA ist gesetzt (fsm beendet)" :
	                    "app: UMBRELLA NICHT gesetzt");

	/* Speicherverwaltung: die im Konstruktor/main angelegten Buffer wieder
	 * freigeben (setzt fsm->rbs->facts bzw. ->memory auf NULL). */
	rbs_destroy_facts_buffer(&fsm->rbs->facts);
	rbs_destroy_memory_buffer(&fsm->rbs->memory);
}
