# MeteoSchweiz-Wetter für Robin

`GET /api/weather` liefert eine Stundenprognose für den serverseitig konfigurierten Ort.
Die Werte sind Prognosen, keine aktuellen Messungen. Die vorhandenen `/api/v1`- und
Health-Endpunkte bleiben bestehen. Wetter ist unabhängig von der nativen Core-Anbindung.

## Konfiguration und Start

In `src/PinkRobin.Api/appsettings.json` steht als Beispiel die PLZ `8001`.
Für den eigenen Standort `Weather:PostalCode` ändern oder vor dem Start setzen:

```sh
Weather__PostalCode=8001 dotnet run --project backend/src/PinkRobin.Api --no-launch-profile --urls http://127.0.0.1:5080
curl -i http://127.0.0.1:5080/api/weather
```

Ohne Queryparameter gilt die konfigurierte Standard-PLZ. Angular kann mit
`GET /api/weather?postalCode=3011` eine andere Schweizer PLZ anfordern.
Ungültige Formate liefern 400, unbekannte oder mehrdeutige PLZ 503.
`Weather:PointId` präzisiert bei Bedarf nur die konfigurierte Standard-PLZ.
Bei der echten Quelle erfolgt die Zuordnung über die Punktmetadaten mit `point_type_id=2`.

## Echte Prognosen oder künstliches Testwetter

Beide Wetter-Endpoints verwenden `IWeatherSource`. Die Registrierung in `Program.cs`
wählt die Implementierung einmal beim Backend-Start:

| `Weather:Provider` | Implementierung | Umgebung |
|---|---|---|
| `MeteoSwiss` (Standard) | `WeatherService` mit Stunden-Cache | Alle |
| `Test` | `TestWeatherSource`, ohne Netzwerkzugriff | Nur `Development` |

In `appsettings.Development.json` beispielsweise:

```jsonc
"Weather": {
  "Provider": "Test",
  // "Provider": "MeteoSwiss",
  "TestScenario": "Storm"
}
```

Nur eine Provider-Zeile aktiv lassen. ASP.NET Core akzeptiert hier Kommentare.
Nach Änderungen das Backend neu starten und Angular neu laden, damit eine neue
Sitzung ohne alten Cooldown entsteht. In der Startausgabe muss für die Testquelle
`Hosting environment: Development` stehen. Außerhalb von Development führt
`Provider: Test` zum Startabbruch; es gibt keinen stillen Rückfall auf eine andere Quelle.

| `Weather:TestScenario` | Niederschlag (mm/h) | Wind (km/h) | Böen (km/h) | Core-Auswahl |
|---|---:|---:|---:|---|
| `Dry` (Standard) | 0 | 5 | 10 | Keine |
| `Rain` | 2 | 10 | 20 | Regenmantel |
| `Storm` | 12 | 30 | 70 | Octopus |

Alle Testszenarien liefern 8,5 °C und eine ab dem Abruf eine Stunde gültige Prognose.
Die Testquelle verwendet die angefragte oder konfigurierte PLZ ohne geografische
Zuordnung; sie ersetzt bewusst die MeteoSchweiz-Abfrage. Jeder Abruf erzeugt neue
Zeitstempel, keinen MeteoSchweiz-Cacheeintrag.

Die Rückgabe trägt `source: "Test"` und `pointId: "test-point"`. Angular kennzeichnet
sie als „Künstliche Testdaten“. Auswahl, Bereitschaft, Wartezeit, Dauer und Cooldown
bleiben dieselbe Core-Logik wie bei echten Prognosen. Es werden keine Outfits direkt
vom Testprovider angefordert und keine Core-Regeln zum Testen umgeschrieben.

### Manueller Kurztest

1. Mit `Provider: Test` nacheinander `Dry`, `Rain` und `Storm` konfigurieren.
2. Jeweils Backend neu starten, Angular neu laden und die Quellenanzeige kontrollieren.
3. Bei verbundenem, bereitem Robin die Auswahl gemäß Tabelle prüfen. Eine gestartete
   Verwandlung endet nach zehn Sekunden.
4. Mit Testquelle `TestScenario: Strom` eintragen: Die Optionsvalidierung muss bereits
   beim Start mit dem Hinweis auf `Dry`, `Rain` oder `Storm` abbrechen. Danach korrigieren.
5. Auf `Provider: MeteoSwiss` wechseln und nach dem Neustart echte Prognosen sowie
   die entsprechende Quellenbeschriftung prüfen.

Die Schreibweise der Werte ist exakt einzuhalten. Das Szenario wird beim Start nur
für `Provider: Test` geprüft; der `switch` der Testquelle weist unbekannte Werte
zusätzlich beim Abruf ab. Die Testquelle bietet derzeit nur die drei gültigen Szenarien,
keine konfigurierbare Einspeisung ungültiger Prognosewerte oder Netzwerkfehler.

## Antwortvertrag

Beispielwerte (keine Live-Daten):

```json
{
  "source": "MeteoSwiss",
  "postalCode": "8001",
  "pointId": "42",
  "forecastRunAt": "2026-10-18T12:00:00+00:00",
  "retrievedAt": "2026-10-18T12:30:00+00:00",
  "validFrom": "2026-10-18T12:00:00+00:00",
  "validTo": "2026-10-18T13:00:00+00:00",
  "temperatureC": 8.5,
  "precipitationMm": 1.2,
  "windSpeedKmh": 15.0,
  "windGustKmh": 25.0
}
```

Das Intervall ist `[validFrom, validTo)`. Temperatur und Wind sind Stundenmittel,
Niederschlag ist die Stundensumme in mm, Böen sind das Stundenmaximum in km/h.
`precipitationMm` beinhaltet den Gesamtniederschlag, nicht nur Regen. Daraus allein
lassen sich weder Schneefall noch Gewitter sicher ableiten.

Angular kann den vorhandenen `/api/**`-Entwicklungsproxy verwenden. Ein passender Vertrag:

```ts
export interface RobinWeather {
  source: 'MeteoSwiss' | 'Test';
  postalCode: string;
  pointId: string;
  forecastRunAt: string;
  retrievedAt: string;
  validFrom: string;
  validTo: string;
  temperatureC: number;
  precipitationMm: number;
  windSpeedKmh: number;
  windGustKmh: number;
}
// this.http.get<RobinWeather>('/api/weather', { params: { postalCode: '3011' } })
```

Die Verhaltensentscheidung liegt im C++-Core. Aktuell gelten feste Schwellen und
ein gemeinsamer Wetter-Cooldown; zusätzliche Zufalls- oder Seltenheitsregeln sind
noch nicht umgesetzt. Der GET-Endpoint startet keine Animation.
Bei 503 Wetterreaktionen auslassen und später erneut abfragen; fehlende Werte
nicht als trockenes Wetter interpretieren. Bei Wiedergabe/Weitergabe sichtbar
„Quelle: MeteoSchweiz“ angeben.

## MeteoSchweiz-Datenabruf und Cache

Der Singleton nutzt `IHttpClientFactory`, einen Refresh-Lock und einen einstündigen
In-Memory-Cache für alle PLZ. Gespeichert werden nur vollständige Werte der nächsten
zwei Intervallenden; damit ist die aktuelle Stunde bis zum nächsten Refresh abgedeckt.
Jeder Request wählt daraus seine PLZ und das gültige Intervall. Ein Ortswechsel lädt
keine landesweiten Dateien erneut. Nach einem Downloadfehler gilt eine Minute
Retry-Pause; eine unbekannte PLZ beeinflusst andere Orte nicht. Ein Neustart leert
den Cache; mehrere Instanzen besitzen getrennte Caches.

STAC-Items werden einschließlich `next`-Links gelesen. Aus den Asset-Namen wird
der neueste gemeinsame Prognoselauf für `tre200h0`, `rre150h0`, `fu3010h0` und
`fu3010h1` gewählt. Damit werden keine unterschiedlichen Prognoseläufe vermischt.
Der Lauf darf höchstens sechs Stunden alt sein (bewusste Freshness-Grenze für Robin).
Die CSV-Dateien werden mit Latin-1, Semikolon und UTC-Zeitformat `yyyyMMddHHmm`
verarbeitet. Ganze Parameterdateien werden gestreamt, nur die benötigten
Stundenwerte der PLZ-Punkte bleiben im Speicher. Ein Refresh lädt dennoch vier landesweite Dateien
mit jeweils bis zu ca. 33 MB; daher bei wenigen Backend-Instanzen bleiben.

Timeout des gesamten Refresh: 90 Sekunden. HTTP-/Formatfehler, veraltete Läufe
und nicht abgedeckte Stunden ergeben 503. Es werden keine alten Werte als aktuelle
Wetterlage ausgegeben. Technische Details stehen nur im Backend-Log.

## Prüfen

Ab Repository-Wurzel mit .NET 10:

```sh
dotnet build backend/src/PinkRobin.Api -c Release
dotnet run --project backend/tests/Weather/Weather.csproj -c Release
```

Die unabhängigen Fixture-Tests prüfen parallele Cache-Misses, Ablauf nach einer
Stunde, Stundenwechsel innerhalb des Caches, Punkt-Typ-Trennung, Dezimalwerte,
fehlende/nicht endliche Werte, falschen CSV-Kopf und Upstream-Fehler mit Retry-Pause.
Auch unbekannte PLZ und veraltete Prognoseläufe werden geprüft.
Sie benötigen keine MeteoSchweiz-Verbindung.

Für einen Live-Test wie oben starten und `/api/weather` zweimal abfragen:
`retrievedAt` muss gleich bleiben; die Zahlen müssen endlich und die Zeitintervalle
aktuell sein. Eine nicht existierende PLZ testen: 503 statt erfundener Werte.
Ohne Internet sind Fixture-Tests möglich, der Live-Endpoint liefert 503.

## Offizielle Referenzen

- https://opendatadocs.meteoswiss.ch/e-forecast-data/e4-local-forecast-data
- https://data.geo.admin.ch/api/stac/v1/collections/ch.meteoschweiz.ogd-local-forecasting
- https://data.geo.admin.ch/ch.meteoschweiz.ogd-local-forecasting/ogd-local-forecasting_meta_point.csv
- https://data.geo.admin.ch/ch.meteoschweiz.ogd-local-forecasting/ogd-local-forecasting_meta_parameters.csv

## Verifikation dieser Änderung

Am 18.10.2026 geprüft: Release-Build ohne Warnungen/Fehler, Fixture-Tests und
bestehende Backend-HTTP-Smoke-Tests. Live-Aufruf für PLZ 8001: HTTP 200,
Punkt 800100, Prognoselauf 18:00 UTC, gültiges Intervall 18:00–19:00 UTC.
Ein zweiter Abruf lieferte dieselbe Antwort einschließlich `retrievedAt`.

## Angular-Anbindung

Die Robin-Seite zeigt die Stundenprognose und eine manuelle PLZ-Eingabe.
`WeatherService` stellt `weather`, `state` und `error` als Signals bereit. Er fragt
beim Öffnen zunächst den Standardort und danach jede Minute die gewählte Backend-Quelle ab;
beim Verlassen der Seite werden Abfragen beendet. Bei Ortswechsel wird ein laufender
Abruf abgebrochen. Fehler oder ungültige Antworten entfernen die Wetterdaten.
Die PLZ wird nur im Seitenspeicher gehalten. Es erfolgt keine IP-Ortung.

Die Robin-Seite übermittelt bei gültiger Prognose und bestehender Sitzung die PLZ
an `POST /api/v1/session/weather` mit `X-Robin-Session`. Das Backend beschafft die
Prognose aus der gewählten Quelle (bei MeteoSchweiz aus demselben Cache) und übergibt
sie mit umgerechneter Gültigkeit an den C++-Core.
Der Core entscheidet über Octopus oder Regenmantel und verwaltet Wartezeit, Dauer
und Cooldown. Angular zeichnet den aktiven Zustand aus `GET /api/v1/session`.
Ablaufgrafiken und Zuständigkeiten: [Wetterwissen und Verwandlungen](../docs/architecture/Wetterreaktionen.md).

Frontend-Prüfung: `cd web`, `npm test` und `npm run build`.
Der zusätzliche Parser-Test prüft fehlende Werte,
falsche Datentypen sowie zukünftige und abgelaufene Intervalle.
