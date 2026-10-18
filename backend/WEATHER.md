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

Die Konfiguration gilt pro Backend-Instanz; es gibt bewusst keinen frei wählbaren
PLZ-Queryparameter. Eine vierstellige PLZ wird beim Start validiert. Die Zuordnung
zum Prognosepunkt erfolgt über `meta_point.csv` mit `point_type_id=2`.
Bei mehreren Punkten derselben PLZ zusätzlich `Weather__PointId` setzen; niemals
annehmen, dass PLZ und `point_id` identisch sind. Unbekannte oder mehrdeutige Orte
führen zu 503 und einem erklärenden Logeintrag.

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
  source: 'MeteoSwiss';
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
// this.http.get<RobinWeather>('/api/weather')
```

Die ReactionEngine entscheidet selbst über Schwellen, Seltenheit und Cooldowns.
Der Endpoint startet keine Animation und bietet keinen Animationskatalog.
Bei 503 Wetterreaktionen auslassen und später erneut abfragen; fehlende Werte
nicht als trockenes Wetter interpretieren. Bei Wiedergabe/Weitergabe sichtbar
„Quelle: MeteoSchweiz“ angeben.

## Datenabruf und Cache

Der Singleton nutzt `IHttpClientFactory`, einen Refresh-Lock und einen einstündigen
In-Memory-Cache der Stundenreihe für genau einen Ort. Jeder Request wählt daraus
sein aktuell gültiges Intervall. Gleichzeitige Anfragen verursachen einen Refresh.
Nach einem Fehler gilt eine Minute Retry-Pause. Ein Neustart leert den Cache;
mehrere Instanzen besitzen getrennte Caches.

STAC-Items werden einschließlich `next`-Links gelesen. Aus den Asset-Namen wird
der neueste gemeinsame Prognoselauf für `tre200h0`, `rre150h0`, `fu3010h0` und
`fu3010h1` gewählt. Damit werden keine unterschiedlichen Prognoseläufe vermischt.
Der Lauf darf höchstens sechs Stunden alt sein (bewusste Freshness-Grenze für Robin).
Die CSV-Dateien werden mit Latin-1, Semikolon und UTC-Zeitformat `yyyyMMddHHmm`
verarbeitet. Ganze Parameterdateien werden gestreamt, nur die ausgewählten
Punktwerte bleiben im Speicher. Ein Refresh lädt dennoch vier landesweite Dateien
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
