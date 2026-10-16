# 21 – Konfiguration und Auslieferung

## Lernziel

Du kannst Quellcode, Konfiguration, Buildartefakt und laufende Installation unterscheiden. Du verstehst, welche Einstellungen erst beim Start wirken, warum native Bibliotheken zur Zielplattform passen müssen und wie eine neue Version überprüfbar ausgeliefert und zurückgenommen wird.

Wir verbinden Builds aus [Kapitel 14](14-module-projektstruktur-und-build-systeme.md), native Anbindung aus [Kapitel 16](16-native-anbindung-csharp-cpp.md) und Diagnose aus [Kapitel 19](19-logging-und-diagnose.md).

**Dieses Kapitel dokumentiert und erklärt. Es veröffentlicht kein Image, ändert keine NAS-Konfiguration und führt kein Deployment aus. Die Betriebsanleitung bleibt [Backend auf Synology](../deploy/synology/backend.md).**

## A. Spickzettel

| Begriff | Bedeutung | Pink-Robin-Beispiel |
|---|---|---|
| Quellstand | versionierter Code und Konfiguration im Repository | Commit auf `dev` oder `main` |
| Build | Quellen in auslieferbare Dateien übersetzen | Angular-Bundle, .NET-Publish, native Bibliothek |
| Artefakt | Ergebnis eines Builds | Containerimage oder Publish-Verzeichnis |
| Deployment | geprüftes Artefakt in einer Umgebung bereitstellen | Backendimage auf der NAS starten |
| Laufzeitkonfiguration | beim Start oder nach festgelegtem Reloadvertrag gelesene Einstellungen | erlaubte Origins, Ports, Limits |
| Secret | vertrauliche Einstellung | Registry-Zugang oder späterer Zugriffsschlüssel |
| Rollback | zu einer bekannten geeigneten Version zurückkehren | vorheriges Image plus kompatible Konfiguration |

Ein Commit auf `dev` ist keine Auslieferung. Ein Image in der Registry ist noch kein auf der NAS gestarteter Container.

## B. C# als Anker: Konfiguration aus mehreren Quellen

ASP.NET Core kann unter seiner üblichen Standardkonfiguration unter anderem `appsettings.json`, umgebungsspezifische Dateien, Umgebungsvariablen und Kommandozeilenargumente kombinieren. Später hinzugefügte Provider können frühere Werte überschreiben. Benutzerdefinierter Startcode kann diese Reihenfolge ändern.

Ein **Lernentwurf**, keine vorhandene Pink-Robin-Einstellung:

```json
{
  "Robin": {
    "MaxSessions": 20,
    "QueueCapacity": 32
  }
}
```

Die entsprechenden Umgebungsvariablennamen lauten unter dem üblichen Environment-Provider `Robin__MaxSessions` und `Robin__QueueCapacity`. Der doppelte Unterstrich wird zum Hierarchietrenner `:`. [Microsoft: ASP.NET-Core-Konfiguration](https://learn.microsoft.com/en-us/aspnet/core/fundamentals/configuration/)

Typisierte Optionen machen erwartete Werte sichtbar. Dieser Ausschnitt gehört in ein entsprechend eingerichtetes ASP.NET-Core-Projekt; die Registrierung steht vor `builder.Build()`:

```csharp
public sealed class RobinLimits
{
    public int MaxSessions { get; set; }
    public int QueueCapacity { get; set; }
}

// Ausschnitt aus Program.cs mit vorhandenem WebApplicationBuilder builder:
// builder.Services.AddOptions<RobinLimits>()
//     .Bind(builder.Configuration.GetSection("Robin"))
//     .Validate(value => value.MaxSessions is >= 1 and <= 100,
//         "MaxSessions muss zwischen 1 und 100 liegen")
//     .Validate(value => value.QueueCapacity is >= 1 and <= 1024,
//         "QueueCapacity muss zwischen 1 und 1024 liegen")
//     .ValidateOnStart();
```

Die auskommentierte Registrierung ist ein Einbauausschnitt, kein eigenständiges Programm. Die Grenzwerte sind ausdrücklich Beispiele. Fehlende Werte werden hier zu `0` und fallen bei der Validierung auf. Ein ungültiges Kapazitätslimit sollte nicht erst mitten in einer laufenden Sitzung entdeckt werden.

**Achtung, C#-Gehirn:** Ein Konfigurationswert ist zunächst Eingabe, kein geprüfter Vertrag. Dass sich Text zu `int` binden lässt, bedeutet noch nicht, dass der Wert für Speicher und Last geeignet ist.

## C. Umgebung, Launchprofil und Laufzeit sind verschieden

Ein Visual-Studio-Launchprofil erleichtert den lokalen Start. Es ist keine allgemeine Produktionskonfiguration. Das vorhandene [Backend](../backend/README.md) besitzt einen Entwicklungsstart mit Angular und einen reinen API-Start. Production und Container starten dadurch nicht automatisch einen Angular-Entwicklungsserver.

`Development` und `Production` bezeichnen Umgebungskonventionen und aktivieren je nach Anwendung unterschiedliche Einstellungen. Sie ersetzen keine Zugriffsprüfung. Ein Produktionsserver sollte nicht allein zur Fehlersuche pauschal in einen Entwicklungsmodus versetzt werden; gezielte Diagnose verwenden.

Bei der üblichen Konfiguration überschreiben beispielsweise Umgebungsvariablen gleichnamige Werte aus JSON-Dateien. Wenn verschiedene Environment-Variablen oder Startargumente widersprechen, den tatsächlich verwendeten Host- und Startweg prüfen, statt auf einen Dateinamen zu vertrauen.

Nicht jede Einstellung lässt sich sinnvoll live neu laden. Eine Logstufe kann unter passenden Providern dynamisch sein; eine andere Queue-Kapazität erfordert eventuell neue Ressourcen. Für jede reloadbare Einstellung festlegen, wann sie gilt und wie laufende Sitzungen behandelt werden. Geänderte Container-Umgebungsvariablen erfordern normalerweise die Neuerstellung des Containers; ein blosser Neustart ersetzt seine gespeicherte Konfiguration nicht.

## D. TypeScript: Browserkonfiguration ist öffentlich

Bei einem Frontend können Werte zur Buildzeit in JavaScript eingebaut werden. Diese Werte ändern sich nicht dadurch, dass später eine Umgebungsvariable im NGINX-Container gesetzt wird. Laufzeitkonfiguration muss ausdrücklich durch eine geladene Datei oder einen Endpunkt vorgesehen sein.

Für Pink Robin sind relative `/api/`-Pfade unter derselben Domain vorgesehen. Dann kann der Reverse Proxy das Backendziel bestimmen, ohne dessen internen Containerhostnamen in den Browsercode einzubauen. Ein Browser kann einen Namen wie `pinkrobin-backend` aus einem privaten Docker-Netz nicht automatisch auflösen.

Ein kleiner Lernhelfer prüft einen bewusst engen öffentlichen Konfigurationsvertrag:

```typescript
type PublicConfig = { apiBase: "/api/"; showDiagnostics: boolean };

function parsePublicConfig(value: unknown): PublicConfig | undefined {
    if (typeof value !== "object" || value === null || Array.isArray(value))
        return undefined;
    const object = value as Record<string, unknown>;
    if (Object.keys(object).length !== 2
        || !Object.prototype.hasOwnProperty.call(object, "apiBase")
        || !Object.prototype.hasOwnProperty.call(object, "showDiagnostics")
        || object.apiBase !== "/api/"
        || typeof object.showDiagnostics !== "boolean")
        return undefined;
    return { apiBase: object.apiBase, showDiagnostics: object.showDiagnostics };
}
```

Das Beispiel implementiert weder das Laden der Konfiguration noch einen echten Diagnosemodus. Alles, was der Browser erhält oder im Bundle enthält, muss als öffentlich lesbar betrachtet werden. Ein versteckter Menüpunkt und `showDiagnostics: false` sind keine Zugriffssicherung.

Tokens und andere Secrets gehören nicht in ein Frontend-Bundle. Für Serverkonfiguration ebenfalls keine echten Zugangsdaten in Git, Images oder Diagnoselogs schreiben. Umgebungsvariablen sind eine mögliche Übergabeform, aber kein automatischer Geheimnisschutz gegen Personen mit Zugriff auf den Host oder die Containerverwaltung.

## E. Native Bibliotheken: mehr als die richtige Dateiendung

Der C++-Core wird für eine konkrete Zielumgebung gebaut. Bei der späteren C#-Anbindung müssen zusammenpassen:

- Betriebssystem und CPU-Architektur des tatsächlich laufenden Prozesses.
- Native ABI und erwartete Exporte.
- Verwendete C-/C++-Laufzeit und andere native Abhängigkeiten.
- Ladepfad und Bereitstellung der Bibliothek.

Ein besonders wichtiger Bezug zum aktuellen [Backend-Dockerfile](../backend/Dockerfile): Es verwendet Alpine-basierte .NET-Images. Alpine verwendet musl; eine für eine übliche glibc-Distribution gebaute `.so` ist deshalb nicht automatisch kompatibel. Die passende native Build- und Testumgebung muss Teil der Integration sein.

Ein Runtime Identifier wie `linux-x64` oder eine musl-spezifische Variante beschreibt eine Zielplattform für .NET-Publish und Paketauflösung. Er kompiliert eine beliebige eigene C++-Bibliothek nicht von selbst für diese Plattform. Auch `--self-contained true` nimmt nicht automatisch alle Betriebssystem- und nativen Voraussetzungen weg. [Microsoft: .NET veröffentlichen](https://learn.microsoft.com/en-us/dotnet/core/deploying/)

Keine fremden nativen Bibliotheken einfach umbenennen, um eine Lademeldung zu umgehen. Der reale P/Invoke-Test aus Kapitel 16 muss im Zielartefakt laufen, nicht nur gegen eine zufällig lokal vorhandene Datei.

## F. Container: gleiches Image, passende Umgebung

Ein Containerimage bündelt Anwendung und Teile ihrer Umgebung. Es liefert keinen eigenen Hostkernel. Laut [Synology-Anleitung](../deploy/synology/backend.md) besitzt die vorhandene NAS einen älteren Kernel; ein erfolgreicher Ubuntu-CI-Lauf bestätigt deshalb nicht automatisch jede künftige Runtimeversion auf diesem Host.

Das bestehende Dockerfile trennt SDK-Build und Runtime-Image. Im Runtime-Image läuft das Backend als unprivilegierter Benutzer. Der Containerport ist `8080`; die dokumentierte lokale Diagnosezuordnung ist `127.0.0.1:5080:8080`.

Diese drei Ports dürfen nicht durcheinandergeraten:

| Ort | Dokumentierter Zweck |
|---|---|
| `8080` im Backendcontainer | HTTP des Backendprozesses |
| `127.0.0.1:5080` auf der NAS | lokale Backenddiagnose |
| öffentlich HTTPS `443` | Zugriff über den vorgesehenen Proxyweg |

`EXPOSE` in einem Dockerfile veröffentlicht allein keinen Hostport. Die Portzuordnung und Netzwerke gehören zur Containerkonfiguration. CORS macht einen internen Port weder öffentlich erreichbar noch ersetzt es die Proxyroute.

## G. Reproduzierbarkeit und Artefaktidentität

Ein Git-Commit beschreibt Quellen. Zusätzlich können SDK-Versionen, Paketauflösung, Compiler, Buildflags und Basisimages das Ergebnis beeinflussen. Für einen nachvollziehbaren Build diese Eingaben festhalten.

Ein beweglicher Tag wie `main` kann später auf ein anderes Image zeigen. Ein Image-Digest identifiziert Inhalt genauer. Auch ein Commit-bezogener Tag ist als Registryname grundsätzlich überschreibbar, sofern die Registry das zulässt; seine Namenskonvention allein ist keine kryptografische Garantie.

Basisimages lassen sich per Digest fixieren. Das erleichtert die Nachvollziehbarkeit, benötigt aber einen bewussten Updateprozess, damit gewünschte Fehler- und Sicherheitskorrekturen nachgeführt werden. Ein fixiertes Basisimage macht nicht automatisch alle anderen Buildinputs reproduzierbar. [Docker: Build-Praktiken](https://docs.docker.com/build/building/best-practices/)

Für ein Release die getestete Artefaktidentität dokumentieren und möglichst dasselbe Artefakt weiterreichen, statt auf jeder Maschine unabhängig neu zu bauen. Umgebungswerte werden nach dem vorgesehenen Vertrag separat bereitgestellt.

## H. Pink Robins vorhandener Auslieferungsweg

Nach dem aktuellen [Backend-README](../backend/README.md):

1. Auf `dev` wird geprüft.
2. Auf `main` veröffentlicht Backend CI nach erfolgreichen Prüfungen das Backendimage mit `main`- und Commit-bezogenem Tag.
3. Die Veröffentlichung allein aktualisiert die NAS noch nicht.
4. Die NAS-Anleitung beschreibt den separaten Backendstart und den Proxyweg.

Frontend und Backend besitzen eigene Images. Der vorhandene NAS-Auftrag `update-web.sh` aktualisiert laut Betriebsanleitung das Frontend; er ist keine automatische Backend-Updatefunktion. Eine neue Backendversion nicht versehentlich als bereits installiert melden, nur weil der Workflow grün ist.

Die aktuelle Bereitschaft ist ebenfalls ausdrücklich begrenzt: `/health/live` kann 200 liefern und der Container healthy sein, während `/health/ready` absichtlich 503 meldet, weil die Core-Anbindung noch fehlt. Das ist der dokumentierte Vorbereitungszustand, keine erfolgreiche native Integration.

Für jeden späteren Integrationsschritt genau definieren, was Readiness prüfen soll. Eine laufende API ohne ladbaren Core ist für echte Core-Aufträge nicht bereit. Die Prüfung darf aber nicht selbst ständig fachliche Berührungen oder Neustarts auslösen.

## I. Geordnete Auslieferung und Rückkehr

Eine spätere Backendaktualisierung braucht einen konkreten Ablauf:

1. Quellstand, Imageidentität und benötigte Konfiguration festhalten.
2. Artefakt inklusive nativer Abhängigkeiten prüfen.
3. Neue Version kontrolliert bereitstellen und notwendige Bereitschaft abwarten.
4. Den vorgesehenen Nutzerpfad über den realen Reverse Proxy prüfen.
5. Alte Version erst nach dem vereinbarten Übergang freigeben beziehungsweise beenden.

Das ist ein Planungsmodell, keine Behauptung über bereits vorhandene unterbrechungsfreie Updates. Falls der Server künftig Sitzungen nur im Speicher hält, kann ein Prozesswechsel diese verlieren. Wiederverbindung und Neuinitialisierung müssen das berücksichtigen; ein Containerneustart stellt keine alten Core-Instanzen wieder her.

Für einen Rollback die vorherige Artefaktidentität und passende Konfiguration behalten. Falls später persistente Daten oder Protokolländerungen hinzukommen, muss die ältere Version damit umgehen können. „Altes Image starten“ macht eine nicht rückwärtskompatible Datenänderung nicht rückgängig.

Shutdown gehört zum Deployment: neue Arbeit stoppen, laufende Verarbeitung geordnet beenden, native Handles freigeben und den Prozess innerhalb des vereinbarten Zeitbudgets schliessen. Ein harter Abbruch garantiert keine vollständige Freigabe- oder Diagnoseausgabe.

## J. Was vor einer Freigabe tatsächlich prüfen?

| Prüfung | Aussage und Grenze |
|---|---|
| Build und Unit-Tests | Quellen bauen, geprüfte lokale Regeln stimmen |
| Container-/Publish-Test | Artefakt startet in dieser Testumgebung |
| Native Integration | echte Bibliothek, Architektur, Layout und Aufrufe passen |
| Readiness | vereinbarte Voraussetzungen sind erfüllt |
| Proxy-/Browsertest | reale Route und Clientkommunikation funktionieren |
| Versionserkennung | laufende Instanz entspricht dem erwarteten Artefakt |
| Rückkehrtest | vorherige Version und Konfiguration sind tatsächlich nutzbar |

Konfiguration in Diagnosen nur in einer dafür freigegebenen Form anzeigen. Hilfreich sind Buildrevision, Umgebung und verfügbare Fähigkeiten; vollständige Umgebungsvariablen oder Secretwerte gehören nicht in eine öffentliche Statusantwort.

**Achtung, C#-Gehirn:** Publish-Erfolg, Containerstart, grüner Healthstatus und fachliche Bereitschaft sind vier unterschiedliche Nachweise.

## K. Miniübung

1. Du änderst eine in das Frontend eingebundene API-Adresse nur als NGINX-Umgebungsvariable. Warum bleibt das Browserbundle unverändert?
2. Warum kann eine glibc-`.so` im Alpine-Image trotz richtiger Architektur scheitern?
3. Backend CI hat ein neues Image veröffentlicht. Warum muss auf der NAS noch nicht die neue Version laufen?
4. Warum reicht der Tag `main` nicht zur exakten Dokumentation eines früheren Releases?
5. Welche drei Dinge sollten für einen späteren Rollback mindestens bekannt sein?

<details>
<summary>Lösung anzeigen</summary>

1. Buildzeitwerte sind bereits in den ausgelieferten Dateien enthalten. Eine Laufzeitkonfiguration müsste ausdrücklich geladen und verwendet werden.
2. Die native Laufzeitumgebung unterscheidet sich: musl und glibc sind nicht beliebig austauschbar.
3. Registryveröffentlichung und Deployment sind getrennt; das Backend wird derzeit nicht automatisch durch den Frontend-Updateauftrag ersetzt.
4. Ein beweglicher Tag kann inzwischen ein anderes Image bezeichnen. Die konkrete Artefaktidentität festhalten.
5. Vorheriges Artefakt, dazu passende Konfiguration und Kompatibilität mit dem inzwischen vorhandenen Zustand beziehungsweise Daten-/Protokollstand.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Quellen, Build, Artefakt und laufende Installation getrennt benennen.
- Konfiguration beim vorgesehenen Zeitpunkt prüfen; Secrets nicht ins Browserbundle oder Repository legen.
- Native Abhängigkeiten müssen zu Prozess, ABI und Laufzeitplattform passen.
- Container teilen den Hostkernel und lösen nicht jede Plattformfrage.
- Die getestete Artefaktidentität dokumentieren und Updates kontrolliert nachführen.
- Bereitschaft über den tatsächlichen Nutzerpfad prüfen, nicht nur über Prozessstart.
- Geordneter Shutdown und ein kompatibler Rollback gehören zur Auslieferung.

[Zurück zu Kapitel 20](20-performance-und-ressourcenbudgets.md) · [Inhaltsverzeichnis](README.md)
