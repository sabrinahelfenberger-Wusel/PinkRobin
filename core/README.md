# Portabler PinkRobin-Core (v0.1)

Implementiert in C++17, mit CMake, nativen Tests und versionierter C-Schnittstelle.
Derselbe fachliche Quellcode kann für den physischen Roboter und die serverseitige
Webdemo gebaut werden. Kein SVG, Netzwerk, Systemtimer oder Hardwarezugriff im Core.

## Verhalten

| Ereignis / Zeit | Ergebnis |
| --- | --- |
| Erstellen oder Neustart | schlafend, geschlossene Augen |
| Nach 4000 ms | aufwachend, neugierige Augen |
| Nach insgesamt 7100 ms | wach und neugierig |
| Herzberührung | wach und glücklich, fröhliche Augen für 6000 ms |
| Erneute Herzberührung | Freude um weitere 6000 ms ab dieser Berührung |
| Kopfkontakt beginnt | wach und glücklich, fröhliche Augen |
| Kopfkontakt dauert 5000 ms | zufrieden, Herzaugen bis zum Ende des Kontakts |
| Kopfkontakt endet oder wird abgebrochen | wieder neugierig |

Kopfbewegungen bzw. wiederholtes Keydown dürfen als wiederholter Kontaktbeginn
ankommen: Sie verlängern die Fünf-Sekunden-Schwelle nicht. Herzberührung beendet
einen laufenden Kopfkontakt. Ein nachfolgendes altes Kopfende hebt diese Herzreaktion
nicht auf. Neustart verwirft alle laufenden Reaktionen.

Mode (schlafend/aufwachend/wach), Stimmung und Gesichtsausdruck sind getrennte Werte.
Die Darstellung zeichnet nur den benannten Ausdruck. Blinzeln, Blickanimationen,
Herzleuchten, reduzierte Bewegung und Nachrichtentexte bleiben beim Ausgabeadapter.

## Bauen und testen

Voraussetzung: CMake 3.20+, C- und C++17-Compiler. Vom Repository aus:

```sh
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build --config Release
ctest --test-dir core/build -C Release --output-on-failure
```

Unter Windows eignet sich Visual Studio mit dem Workload **Desktopentwicklung mit
C++** und CMake-Unterstützung. Für einen eigenen Core-Entwicklungsplatz den Ordner
`core` als CMake-Projekt öffnen. Die bestehende Backend-Solution bleibt unverändert.

### Aufbau der Tests

Die C++-Tests sind nach Verantwortlichkeiten aufgeteilt. Jeder Bereich besitzt ein
kleines `main()` und benannte Szenarien; Fehlermeldungen nennen Szenario und Prüfung.
Gemeinsame Prüf- und Ausgabehilfen stehen in `tests/test_support.hpp`.

| Datei | CTest-Name | Inhalt |
| --- | --- | --- |
| `tests/core_tests.cpp` | `behavior` | Begrüssung, Berührung, Neustart, Isolation und Zeitregeln |
| `tests/weather_tests.cpp` | `weather` | Prognosevalidierung, Gültigkeit, Reaktionen und Cooldown |
| `tests/message_tests.cpp` | `message` | Nachrichtenannahme, Reihenfolge, Ringpuffer und Überfüllung |
| `tests/world_model_tests.cpp` | `world_model` | Wetterwissen, Metadaten, Ablauf und Core-Anbindung |
| `tests/c_api_tests.c` | `c_abi` | Native C-Schnittstelle |

Alle Bereiche laufen weiterhin mit dem oben gezeigten CTest-Befehl, auch in CI.
Ein einzelner Bereich lässt sich gezielt ausführen:

```sh
ctest --test-dir core/build -C Release -R "^world_model$" --output-on-failure
```

Die Konsolendemo zeigt ohne Wartezeiten einen reproduzierbaren Ablauf:

```sh
./core/build/robin_demo
```

Unter dem Visual-Studio-Generator liegt sie unter `core/build/Release/robin_demo.exe`.
Ergebnisse: statische Bibliothek `pinkrobin_behavior` und native Shared Library
`pinkrobin_core` (.dll, .so oder .dylib, je nach Plattform).
**Core CI** prüft Release-Builds auf Windows, Linux und macOS sowie einen Linux-Build
mit Address-/UndefinedBehavior-Sanitizern. Die Tests bleiben auch in Release aktiv.

## Zeit, Instanzen und Lebensdauer

Der Adapter übergibt monotone Millisekunden an `advance_to`/`dispatch`. Es wird keine
Wanduhr gelesen; die Tests brauchen weder sleeps noch echte Timer. Fortschreiten der
Zeit muss regelmässig vom Server-/Firmwareadapter ausgelöst werden. Ein Snapshot
allein löst keinen Zeitfortschritt aus. Grössere Zeitsprünge berücksichtigen alle
fälligen Zustandswechsel. Rückwärtszeit wird ohne Zustandsänderung abgelehnt.
Der Adapter muss einen eventuellen 32-Bit-Hardwaretimer-Überlauf erweitern.

Jede Demo-Sitzung bzw. jeder physische Robin besitzt eine eigene Instanz. Keine
globalen Zustände. Der Adapter serialisiert Zugriffe auf dieselbe Instanz und
vernichtet sie erst nach Abschluss aller Zugriffe. Der C++-Verhaltensteil benötigt
keine dynamischen Allokationen; der C-Wrapper allokiert genau einen Handle beim
Erstellen und gibt ihn beim Zerstören frei.

## C-Schnittstelle / spätere C#-Anbindung

Öffentlicher Header: `include/pinkrobin/c_api.h`. ABI-Version **1**:

- `robin_core_create(now_ms)` / `robin_core_destroy(handle)` verwalten den Handle.
- `robin_core_dispatch(handle, event, now_ms)` verarbeitet ein bekanntes Ereignis.
- `robin_core_advance(handle, now_ms)` führt fällige Zeitübergänge aus.
- `robin_core_get_snapshot(handle, output, output_size)` kopiert den Zustand.
- `robin_core_abi_version()` prüft die unterstützte ABI vor der Nutzung.

Rückgabewerte: 0 erfolgreich, 1 ungültiges Argument, 2 rückwärts laufende Zeit.
Erstellen liefert bei Allokationsfehler NULL; Destroy(NULL) ist erlaubt. Ungültige
Ereignisse werden vor jeder Zeit-/Zustandsänderung abgelehnt. Der Aufrufer darf keine
zerstörten oder fremden Handles verwenden. Keine C++-Exceptions verlassen die ABI.

Snapshot v1 umfasst **40 Bytes**: sechs uint32_t-Felder (ABI-Version, Mode, Mood,
Expression, head_contact 0/1, reserviert 0), dann uint64_t time_ms und revision.
Enum-Werte stehen explizit im C-Header. Revision steigt bei fachlichen Änderungen,
nicht bei jedem Tick. Zeitübergänge erfolgen vor dem Ereignis desselben Zeitpunkts.
C# kann diese API mit Cdecl, `SafeHandle` und sequenziellem Struct-Layout anbinden;
Strings, C++-Enums und C++-bool werden nicht über die ABI übertragen.

## Was noch folgt

Der Core ist eine erste Verhaltensimplementierung, noch keine Firmware oder komplette
Auftrags-/Sicherheitssteuerung. Er enthält keine Motorbefehle, Pairingrechte,
Demo-Sitzungsverwaltung, Duplikatkennungen, Transportprotokolle oder Authentifizierung.
Diese Grenzen bleiben in Backend-/Hardwareadaptern bzw. folgenden Core-Modulen.
Ereignis-Zeitstempel müssen vom vertrauenswürdigen Adapter stammen, nicht vom Browser.

Das Backend meldet weiterhin `core.connected=false`; die native Bibliothek ist noch
nicht in dessen Image eingebunden. Angular verwendet weiterhin seine Demo-Logik.
Als nächster Schritt: C#-SafeHandle-Anbindung, pro Sitzung ein Core, serverseitige
Zeit und danach geprüfte Ereignis-/Zustandsübermittlung zu Angular. Kein gemeinsamer
Singleton-Verhaltenszustand für alle Besucher und keine Auslieferung als WebAssembly.
Die existierende Live-Demo bleibt funktionsfähig.
