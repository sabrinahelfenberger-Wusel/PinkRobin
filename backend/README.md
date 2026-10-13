# Robin-Backend

ASP.NET-Core-Grundgerüst auf .NET 10 LTS für die spätere Verbindung zwischen
Angular und dem nativen C++-Core. Es enthält bereits eine ausführbare API,
getrennte Gesundheitsprüfungen, typisierte Statusdaten und eine austauschbare
Core-Schnittstelle. Robins Verhalten läuft aktuell weiterhin in Angular.

## Lokal starten

Voraussetzung: .NET 10 SDK. Im Repository:

```sh
cd backend
dotnet build PinkRobin.Backend.slnx
dotnet run --project src/PinkRobin.Api
```

Die API läuft unter http://localhost:5080. In Visual Studio mit .NET-10-Unterstützung
`PinkRobin.Backend.slnx` öffnen und `PinkRobin.Api` starten. Die Datei
`PinkRobin.Api.http` enthält Beispielanfragen.

| URL | Bedeutung |
| --- | --- |
| `/health/live` | API läuft (200) |
| `/health/ready` | Core bereit; derzeit absichtlich 503 |
| `/api/v1/status` | API-Version, Core-Verbindung und verfügbare Funktionen |

Es gibt noch keine Steuerung, Demo-Sitzungen, Datenbank oder SignalR-Hubs. Der
Status meldet diese Funktionen ausdrücklich als nicht verfügbar. `IRobinCoreGateway`
ist die Grenze zur späteren C-ABI/PInvoke- oder Prozess-Anbindung. Keine zweite
C#-Nachimplementierung von Robins Verhalten. Zeichnung und Augenformen bleiben
bei Angular bzw. dem physischen Display.

## Konfiguration und Tests

Im Development-Profil dürfen `http://localhost:4200` und `http://127.0.0.1:4200`
die Status-API im Browser lesen. In Production ist Cross-Origin-Zugriff ohne
explizite Konfiguration deaktiviert. `Cors:AllowedOrigins` akzeptiert nur konkrete
HTTP(S)-Origins; keine Wildcards. Für die spätere Auslieferung unter derselben Domain
ist CORS nicht nötig. TLS soll am Synology-Reverse-Proxy enden; der lokale API-Port
ist nur für Entwicklung bzw. das interne Deployment vorgesehen.

Bei laufender Development-API:

```sh
python3 tests/smoke.py --development
```

Der eigene GitHub-Workflow **Backend CI** baut das Projekt, testet echte HTTP-Antworten
und baut und prüft das Produktionsimage. Auf main veröffentlicht er genau dieses
geprüfte Image als `ghcr.io/sabrinahelfenberger-wusel/pinkrobin-backend:main` sowie
`sha-<Commit-ID>`. Auf dev wird nur geprüft. Der bestehende Frontend-Workflow bleibt
unabhängig. Die Installation auf der NAS erfolgt zunächst manuell; Anleitung:
[Backend auf Synology](../deploy/synology/backend.md).

## Docker und Synology

```sh
docker build -t pinkrobin-backend:local backend
docker run --rm -p 127.0.0.1:5080:8080 pinkrobin-backend:local
```

Diese Befehle gelten vom Repository-Hauptverzeichnis aus. Das Runtime-Image läuft
als unprivilegierter Benutzer. Seine Gesundheitsprüfung prüft nur Liveness;
Readiness bleibt bis zur echten Core-Anbindung 503.

Die vorhandene NAS hat laut Protokoll Linux 3.10.108. .NET-10-Container teilen den
Host-Kernel; ein erfolgreicher CI-Test auf Ubuntu bestätigt nicht die Kompatibilität
mit dieser Synology. Der erste Runtime-Test auf der NAS war erfolgreich; nach
Image-/Runtimewechseln bleibt ein erneuter NAS-Test sinnvoll. Synology DSM ist keine von Microsoft aufgeführte .NET-Plattform.
Das Frontend-NGINX leitet `/api/` über das gemeinsame Docker-Netz zum Backend
weiter. Dafür muss die neue Version von `update-web.sh` auf die NAS kopiert werden.
`/hubs` bleibt bis zur SignalR-Implementierung bei 404.

## Nächste Schritte

1. Native Core-Anbindung festlegen und `/health/ready` an ihre echte Bereitschaft koppeln.
2. Begrenzte, getrennte Demo-Sitzungen sowie versionierte Ereignis-/Zustandsverträge implementieren.
3. SignalR mit geprüfter Sitzungszuordnung und Wiederverbindung ergänzen.
4. Synology-Kompatibilität prüfen, internes Netzwerk und `/api`-/`/hubs`-Routing einrichten.
5. Angular kontrolliert an die API anbinden; Verhalten und fachliche Timer in den Core verlagern.

[API-Vertrag](../docs/api/README.md), [Technologiekonzept](../docs/Technologiekonzept.md),
[.NET-Support](https://dotnet.microsoft.com/en-us/platform/support/policy/dotnet-core),
[Unterstützte .NET-10-Systeme](https://github.com/dotnet/core/blob/main/release-notes/10.0/supported-os.md).
