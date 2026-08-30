# rbs_demo – Beispiel-App für die rbs-Regel-Engine

`rbs_demo` ist eine kleine Beispiel-Anwendung, die zeigt, wie man mit der
`rbs`-Bibliothek (Rule Based System als Zustandsautomat) plus der
`sm`-State-Machine ein deklaratives Regelprogramm aufbaut und ausführt.

Das Beispiel ist das klassische **Wetter-/Schirm-Programm**:

> Wenn es regnet oder bewölkt ist → ist man nass.
> Wenn man nass ist → braucht man einen Schirm.
> Wenn man erwachsen ist → zahlt man (Effekt: −10 Geld).

Die Demo demonstriert dabei
- die deklarativen Programmstrukturen (Tokens, Regeln, Effekte, Slots) in `app.h`,
- den **sm-Lebenszyklus** (`sm_on_start` = Konstruktor, `sm_on_stop` = Destruktor) für
  Speicherverwaltung und Weltaufbau im Worker-Thread,
- den externen Fakt-Input über Slot-Handler (z. B. „Regen klärt auf").

## Struktur

| Datei | Inhalt |
|---|---|
| `src/app.h` | deklarative Programmstrukturen (Token-/Value-Enums, Namen, Regeln, Effekte, Slots) + Handler-Prototypen |
| `src/app.c` | alle Handler (Slot-Handler, Schritt-Callback) + Konstruktor/Destruktor `sm_on_start`/`sm_on_stop` |
| `src/main.c` | `main()`: rbs-Konfiguration + fsm-Verdrahtung + Start |

## Bauen & Ausführen

Die Demo bindet die `rbs`-Bibliothek per `add_subdirectory` ein (relativer
Pfad `../../libraries/rbs`).

```bash
cmake -S . -B build
cmake --build build
./build/bin/rbs_demo
```

Erwartete Ausgabe (Auszug):

```
app: fsm startet (wetter: es regnet und ist bewoelkt)
step 1 | UMBRELLA: false | MONEY: 100
wetter: regen klaert auf
step 2 | UMBRELLA: true | MONEY: 90
app: UMBRELLA ist gesetzt (fsm beendet)
rbs main end
```

## Abhängigkeiten

- `rbs` – `libraries/rbs` (Regel-Engine; zieht `sm`, `logging`, `api`, `m` transitiv)
