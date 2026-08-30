#include <sm/sm.h>
#include <logging/logging.h>

#include "app.h"

int main()
{
	logging_log_message("rbs main start");

	/* Statische Welt-Konfiguration; facts/memory allokiert der Konstruktor
	 * sm_on_start (Speicherverwaltung im Worker-Thread). */
	struct rbs rbs =
	{
		.token_count = TOKEN_COUNT,
		.value_count = VALUE_COUNT,
		.fact_names = token_names,
		.fact_names_count = sizeof(token_names) / sizeof(token_names[0])
	};

	struct rbs_sm fsm =
	{
		.rbs = &rbs,
		.rules = app_rules,
		.rule_count = sizeof(app_rules) / sizeof(app_rules[0]),
		.effects = app_effects,
		.effect_count = sizeof(app_effects) / sizeof(app_effects[0]),
		.slots = app_slots,
		.slot_count = sizeof(app_slots) / sizeof(app_slots[0]),
		.on_step = app_on_step,
	};

	rbs_sm_run(&fsm);

	/* Ressourcen-Aufbau und -Abbau uebernimmt der Thread-Lebenszyklus:
	 * Buffer im sm_on_start-Konstruktor angelegt, im sm_on_stop-Destruktor
	 * wieder freigegeben. */

	logging_log_message("rbs main end");
	return 0;
}
