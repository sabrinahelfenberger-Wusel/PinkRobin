# Nachrichtenvertrag des Cores

## Aufbau

Eine Nachricht besteht aus Metadaten und einem typisierten Inhalt.

- `MessageMetadata`: Herkunft der Nachricht.
- `MessagePayload`: entweder `InteractionReceived` oder `WeatherUpdated`.
- `Message`: verbindet Metadaten und Inhalt.

Neue Nachrichtenarten können als eigene Strukturen in `MessagePayload`
ergänzt werden. Ihre Verarbeitung wird in `Core::receive()` hinzugefügt.

## Quellen

| Quelle | Wert | Bedeutung |
| --- | --- | --- |
| web | 0 | Eingabe über die Weboberfläche |
| companion | 1 | Eingabe über die Companion-App |
| device | 2 | Eingabe vom Gerät |
| backend | 3 | Vom Backend erzeugtes Ereignis, etwa Kontakt-Timeout |

Die Werte stimmen in C++, C und C# überein.
Eine Quelle ist eine Herkunftsangabe, kein Berechtigungsnachweis.

## Verarbeitung

```text
Adapter → C-ABI → Message → Core::receive()
                              ├── Interaktion → dispatch()
                              └── Wetter → validieren, vormerken, advance_to()
```

`receive()` prüft zunächst Quelle und Zeit.
Ungültige Quellen und rückwärts laufende Zeit werden abgelehnt.

Interaktionen werden durch die bestehende Ereignisverarbeitung geprüft
und ausgeführt. Wetter wird vor einer Zustandsänderung gegen die
übergebene Zeit validiert.

## Zeitregeln

`now_ms` bezeichnet die aktuelle monotone Core-Zeit.
Alle Aufrufer einer Core-Instanz müssen dieselbe Zeitbasis verwenden.

Die Gültigkeitszeiten einer Wetterprognose verwenden ebenfalls diese
Zeitbasis. UTC wird ausserhalb des Cores umgerechnet.

Eine Prognose ist ab `valid_from_ms` gültig und ab
`valid_until_ms` bereits abgelaufen.

Der Core benötigt weder eine Systemuhr noch eigene Threads.
Adapter müssen den Zugriff auf eine Instanz serialisieren.

## Native Schnittstelle

Neue Aufrufer verwenden:

- `robin_core_receive_interaction()`
- `robin_core_receive_weather()`

Beide erhalten Quelle und Verarbeitungszeit ausdrücklich.

Die bisherigen Funktionen bleiben erhalten:

- `robin_core_dispatch()` verwendet die Quelle `web`.
- `robin_core_post_weather()` merkt Wetter nur vor; die Verarbeitung
  erfolgt beim nächsten `advance_to()`.

C++-Typen wie `std::variant` bleiben innerhalb von C++.
Die C-ABI verwendet feste Zahlentypen und einfache Strukturen.

## Umfang und nächste Schritte

Dieser Schritt führt den gemeinsamen Nachrichtenvertrag ein.
Die Herkunft wird geprüft, aber noch nicht dauerhaft gespeichert.

Noch nicht enthalten sind:

- ein gemeinsamer begrenzter Nachrichtenpuffer,
- Annahmeergebnisse für Ersetzung und Überfüllung,
- allgemeine Prioritäts- und Verwerfungsregeln.

Diese Verarbeitung wird in Issue #16 ergänzt.
Das WorldModel folgt in Issue #7.

## Prüfung

Im Repository-Hauptordner:

```powershell
cmake --build core/build --config Release
ctest --test-dir core/build -C Release --output-on-failure
dotnet run --project backend/tests/Interop -c Release
```