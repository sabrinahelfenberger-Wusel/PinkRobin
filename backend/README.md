# Robin-Backend

ASP.NET-Core-Grundgerüst auf .NET 10 LTS für die spätere Verbindung zwischen
Angular und dem nativen C++-Core. Es enthält bereits eine ausführbare API,
getrennte Gesundheitsprüfungen, typisierte Statusdaten und eine austauschbare
Core-Schnittstelle. Das Backend lädt den C++-Core jetzt direkt als native Bibliothek
über dessen C ABI. Die sichtbare Demo verwendet vorerst weiterhin das Angular-Verhalten.

## Lokal starten

Voraussetzungen: .NET 10 SDK, CMake ab 3.20 und ein C++17-Compiler.
Unter Windows die Visual-Studio-Workload **Desktopentwicklung mit C++** inklusive
MSVC und CMake installieren. Unter macOS werden die Xcode Command Line Tools benötigt,
unter Linux beispielsweise `cmake` und `build-essential`. SDK und Compiler müssen
dieselbe Architektur verwenden (Windows normalerweise x64). Im Repository:

```sh
cd backend
dotnet build PinkRobin.Backend.slnx
dotnet run --project src/PinkRobin.Api --launch-profile PinkRobin.Api
```

Die API läuft unter http://localhost:5080. In Visual Studio mit .NET-10-Unterstützung
die zentrale `PinkRobin.slnx` im Repository-Hauptverzeichnis öffnen; die beiden Startprofile sind unten beschrieben. Die Datei
`PinkRobin.Api.http` enthält Beispielanfragen.

| URL | Bedeutung |
| --- | --- |
| `/health/live` | API läuft (200) |
| `/health/ready` | Core geladen und geprüft (200), bei Ladefehler 503 |
| `/api/v1/status` | API-Version, Core-Verbindung und verfügbare Funktionen |

Es gibt noch keine Steuerung, Demo-Sitzungen, Datenbank oder SignalR-Hubs. Der
Status meldet diese Funktionen ausdrücklich als nicht verfügbar. `IRobinCoreGateway`
erzeugt getrennte native Instanzen über C ABI/PInvoke. Keine zweite
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
docker build -t pinkrobin-backend:local -f backend/Dockerfile .
docker run --rm -p 127.0.0.1:5080:8080 pinkrobin-backend:local
```

Diese Befehle gelten vom Repository-Hauptverzeichnis aus. Das Runtime-Image läuft
als unprivilegierter Benutzer. Seine Gesundheitsprüfung prüft nur Liveness;
Readiness prüft, ob die native Bibliothek mit passender ABI geladen werden konnte.

Die vorhandene NAS hat laut Protokoll Linux 3.10.108. .NET-10-Container teilen den
Host-Kernel; ein erfolgreicher CI-Test auf Ubuntu bestätigt nicht die Kompatibilität
mit dieser Synology. Der erste Runtime-Test auf der NAS war erfolgreich; nach
Image-/Runtimewechseln bleibt ein erneuter NAS-Test sinnvoll. Synology DSM ist keine von Microsoft aufgeführte .NET-Plattform.
Das Frontend-NGINX leitet `/api/` über das gemeinsame Docker-Netz zum Backend
weiter. Dafür muss die neue Version von `update-web.sh` auf die NAS kopiert werden.
`/hubs` bleibt bis zur SignalR-Implementierung bei 404.

## Nächste Schritte

1. Native Core-Anbindung auf Windows und NAS prüfen; Details siehe unten.
2. Begrenzte, getrennte Demo-Sitzungen sowie versionierte Ereignis-/Zustandsverträge implementieren.
3. SignalR mit geprüfter Sitzungszuordnung und Wiederverbindung ergänzen.
4. Synology-Kompatibilität prüfen, internes Netzwerk und `/api`-/`/hubs`-Routing einrichten.
5. Angular kontrolliert an die API anbinden; Verhalten und fachliche Timer in den Core verlagern.

[API-Vertrag](../docs/api/README.md), [Technologiekonzept](../docs/Technologiekonzept.md),
[.NET-Support](https://dotnet.microsoft.com/en-us/platform/support/policy/dotnet-core),
[Unterstützte .NET-10-Systeme](https://github.com/dotnet/core/blob/main/release-notes/10.0/supported-os.md).

## In Visual Studio mit dem grünen Startknopf

1. Aktuellen dev-Stand pullen und `PinkRobin.slnx` im Repository-Hauptverzeichnis öffnen.
2. Falls nötig, `PinkRobin.Api` als Startprojekt festlegen.
3. Neben dem grünen Startknopf das Profil **PinkRobin Demo** auswählen.
4. Starten (F5). Backend startet auf Port 5080, Angular auf Port 4200.
   Der Browser öffnet zunächst den lokalen Backend-Einstieg und wechselt nach
   dem Angular-Start zu `http://localhost:4200/#/virtual-robin`.

Voraussetzungen: Visual Studio mit .NET-10-Unterstützung und ASP.NET-/Web-Workload,
.NET 10 SDK, Node.js 22.12+ (22er-Reihe) und npm im PATH. Nach Installation von Node
Visual Studio neu starten. Fehlende Angular-Abhängigkeiten installiert der erste
Debug-Build einmalig mit npm ci; weitere Starts installieren sie nicht erneut.
Bei geänderten Frontend-Abhängigkeiten wie gewohnt npm install/ci im web-Ordner.

HTML, CSS und TypeScript sind im Projekt unter **Frontend** verlinkt. Änderungen
speichern: Angular aktualisiert die Browseransicht automatisch. C#-Breakpoints
funktionieren im Backend über Visual Studio. JavaScript-Debugging ist separat
konfigurierbar; die Startfunktion setzt keine .esproj-/JS-SDK-Erweiterung voraus.

Das Profil **PinkRobin.Api** startet nur das Backend. Auf der Kommandozeile:

```sh
dotnet run --project src/PinkRobin.Api --launch-profile "PinkRobin Demo"
```

Dabei gegebenenfalls `http://localhost:5080/` manuell öffnen; automatisches Öffnen
über launchBrowser übernimmt Visual Studio. Ein bereits auf Port 4200 laufender
Angular-Server wird genutzt. Die Stopptaste beendet den von diesem Startprofil
selbst gestarteten Angular-Prozess mit dem Backend; ein vorher manuell gestarteter
Angular-Server bleibt bestehen. Port 5080 muss frei sein.

Nur das Demo-Profil aktiviert den Entwicklungs-SPA-Proxy. Production, Docker und
NAS-Auslieferung starten weiterhin kein Angular und benutzen kein Node/npm.
Das separate Backend-Profil und die vorhandenen Container-Tests bleiben erhalten.
Grundlage: [Microsoft: SPA-Entwicklungsstart](https://learn.microsoft.com/en-us/aspnet/core/client-side/spa/intro).

## Verbindung zwischen Backend und Core

- `Core/NativeRobinCore.cs`: C#-Typen, Cdecl/PInvoke-Aufrufe und ein `SafeHandle`
  für die native Instanz. `Dispose()` gibt sie frei; Aufrufe und Freigabe sind synchronisiert.
- `Core/NativeRobinCoreGateway.cs`: prüft beim ersten Zugriff ABI-Version 1 sowie
  Erstellen/Lesen/Freigeben einer Instanz. Der Status meldet dann
  `connected: true` und `implementation: "native-cabi-v1"`. Bei Ladefehlern steht
  die Ursache im Backend-Protokoll; nach einer Reparatur Backend neu starten.
- `NativeCore.targets`: baut den Core bei Build/F5 mit CMake und kopiert DLL,
  `.so` oder `.dylib` neben die API; `dotnet publish` übernimmt diese Datei ebenfalls.
  Ein separater manueller Core-Build ist für den Backend-Start nicht erforderlich.
- `tests/Interop`: prüft echte C#-zu-C++-Aufrufe, Aufwachen, Berührung, Herzaugen,
  getrennte Instanzen, ungültige Eingaben und Freigabe auf Windows, Linux und macOS.

```sh
dotnet run --project backend/tests/Interop -c Release
```

Dieser Testbefehl gilt im Repository-Hauptverzeichnis. Pro künftiger Sitzung muss
`CreateInstance()` eine eigene Instanz liefern. Der Aufrufer besitzt sie und muss
sie freigeben. Zeitwerte sind monotone Millisekunden relativ zur Sitzung, keine
Kalenderzeit; Events und Fortschalten einer Sitzung müssen in zeitlicher Reihenfolge
verarbeitet werden. Der Core startet keine eigenen Threads oder Timer.

Das Dockerfile baut den Core unter Alpine und liefert ihn gemeinsam mit der API
und `libstdc++` aus. Das Backend benötigt dafür keinen zusätzlichen Port und keinen
separaten Core-Container. Die Bibliothek läuft im Backend-Prozess. Sitzungen,
HTTP-Befehle und SignalR bleiben bis zum nächsten Integrationsschritt deaktiviert.

### CMake wird beim Backend-Build nicht gefunden (Fehler 9009)

Der Build sucht zuerst das mit Visual Studio installierte CMake, auch wenn es
nicht im PATH liegt. Falls nötig nutzt er `vswhere` zur Suche nach der Installation.
Erst danach verwendet er `cmake` aus dem PATH. Im Visual Studio Installer unter
**Ändern → Einzelne Komponenten** die **C++-CMake-Tools für Windows** installieren,
falls sie fehlen. Visual Studio danach neu starten. Ein eigener Pfad lässt sich
mit der MSBuild-Eigenschaft `RobinCMake` angeben.
