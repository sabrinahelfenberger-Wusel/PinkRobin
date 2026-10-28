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

## Nachrichtenannahme und Verarbeitung

Annahme und Ausführung sind getrennt:

```text
Core::receive()
    ├── Quelle, Zeit und Inhalt prüfen
    ├── Interaktion → Ringpuffer mit acht Plätzen
    └── Wetter → einzelnen Speicherplatz belegen oder ersetzen

Core::advance_to()
    ├── fällige Interaktionen in Eingangsreihenfolge verarbeiten
    ├── dabei ihre jeweiligen Empfangszeitpunkte berücksichtigen
    └── Zustand und Verhalten bis zur angeforderten Zeit fortschreiben
```

`receive()` verändert den sichtbaren Zustand noch nicht.
Der Aufrufer muss die Verarbeitung mit `advance_to()` anstossen.

### Annahmeergebnisse

| Ergebnis | Bedeutung |
| --- | --- |
| accepted | Nachricht angenommen |
| replaced | Wartende Wetterprognose ersetzt |
| invalid_argument | Ungültige Quelle oder ungültiger Inhalt |
| time_reversed | Empfangszeit liegt vor der Core-Zeit oder der zuletzt angenommenen Nachricht |
| full | Interaktionspuffer voll; Nachricht nicht angenommen |

Abgelehnte Nachrichten verändern weder den Pufferinhalt noch den
zuletzt angenommenen Empfangszeitpunkt.

### Speicherregeln

Interaktionen behalten ihre Eingangsreihenfolge.
Gleiche Empfangszeitpunkte sind erlaubt.

Wetter belegt einen eigenen Speicherplatz. Eine neue gültige Prognose
ersetzt die wartende. Eine ungültige Prognose ersetzt nichts.
Wetter wird frühestens ab seinem Empfangszeitpunkt übernommen;
seine Gültigkeit wird zusätzlich geprüft.

Die Kapazitäten sind fest begrenzt. Der Core benötigt dafür keine
dynamische Speicherallokation.

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

Die C-ABI unterscheidet zwischen synchroner Übergabe und reiner Annahme.

### Annehmen und sofort verarbeiten

- `robin_core_receive_interaction()`
- `robin_core_receive_weather()`

Beide erhalten Quelle und Empfangszeit ausdrücklich.
Sie rufen zunächst `receive()` und bei erfolgreicher Annahme
anschliessend `advance_to()` auf.

### Nur annehmen

`robin_core_post_interaction()` reiht eine Interaktion ein.
Die Verarbeitung erfolgt durch einen späteren Aufruf von
`robin_core_advance()`.

### Rückgabewerte

Die C-ABI übersetzt die Annahmeergebnisse ausdrücklich in API-Codes.
`accepted` und `replaced` werden beide als `ROBIN_OK` zurückgegeben.
`full` wird als `ROBIN_QUEUE_FULL` mit dem Wert 4 gemeldet.

Die Zahlenwerte von `MessageAcceptance` dürfen nicht direkt als
API-Rückgabecodes interpretiert werden.

### C#-Adapter

`NativeRobinCore.PostInteraction()` nimmt nur an und meldet einen
vollen Puffer mit `CoreMessageQueueFullException`.

`NativeRobinCore.Dispatch()` nimmt an und verarbeitet sofort.
Bei vollem Puffer verarbeitet es zuerst die wartenden Ereignisse
bis zur übergebenen Zeit und versucht die Eingabe genau einmal erneut.
Weitere Fehler werden an den Aufrufer weitergegeben.

### Bestehende Funktionen

- `robin_core_dispatch()` verwendet die Quelle `web` und verarbeitet synchron.
- `robin_core_post_weather()` merkt Wetter weiterhin nur vor.

C++-Typen wie `std::variant` bleiben innerhalb von C++.
Die C-ABI verwendet feste Zahlentypen und einfache Strukturen.
## Umfang und nächste Schritte

Nachrichtenvertrag und begrenzte Verarbeitung sind implementiert:

- typisierte Nachrichten mit Herkunft,
- acht Interaktionsplätze als Ringpuffer,
- ein ersetzbarer Wetterplatz,
- getrennte Annahme und Verarbeitung,
- Prüfung von Empfangszeit und Gültigkeit,
- ausdrückliche Rückmeldung bei vollem Puffer.

Die Herkunft bleibt im Interaktionspuffer bis zur Verarbeitung erhalten.
Eine dauerhafte Speicherung von Herkunft und Beobachtungen ist noch
nicht umgesetzt.

Das Wetter-WorldModel ist in [WorldModel.md](WorldModel.md)
beschrieben. Weitere Wahrnehmungsarten bleiben spätere Erweiterungen.
Allgemeine Aktionsprioritäten und Unterbrechungsregeln gehören zur
späteren Verhaltens- und Aktionskoordination in Issue #8.

Der Core verwendet keine eigene Uhr und keine eigenen Threads.
Adapter müssen Verarbeitung und Zeitfortschritt anstossen.
## Prüfung

Im Repository-Hauptordner:

```powershell
cmake --build core/build --config Release
ctest --test-dir core/build -C Release --output-on-failure
dotnet run --project backend/tests/Interop -c Release
```