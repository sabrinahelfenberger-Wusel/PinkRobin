# 17 – HTTP, WebSockets und Verbindungsmanagement

## Lernziel

Du kannst HTTP-Anfragen von dauerhaften Nachrichtenverbindungen unterscheiden, Antworten und Abbrüche richtig einordnen und eine Wiederverbindung planen, ohne alte Zustände oder Befehle blind zu wiederholen. C# bleibt der Anker; TypeScript zeigt den Browserzugriff. Der C++-Core bleibt hinter dem Backend.

Wir verbinden Asynchronität aus [Kapitel 10](10-asynchronitaet-und-nebenlaeufigkeit.md), Nachrichten aus [Kapitel 11](11-events-callbacks-und-nachrichten.md) und native Anbindung aus [Kapitel 16](16-native-anbindung-csharp-cpp.md).

**Aktueller Stand:** Das [Backend](../backend/README.md) bietet Infrastrukturstatus. Steuerung, Demo-Sitzungen und SignalR-Hubs sind noch nicht implementiert. Beispiele für spätere Echtzeitverbindungen in diesem Kapitel sind Entwürfe, keine neuen produktiven Endpunkte.

## A. Spickzettel

| Aufgabe | C# / ASP.NET Core | TypeScript / Browser | Rolle des C++-Core |
|---|---|---|---|
| Einzelne HTTP-Anfrage | `HttpClient` beziehungsweise API-Endpunkt | `fetch` | kein Netzwerkzugriff |
| JSON verarbeiten | `System.Text.Json` | parsen, als `unknown` prüfen | validiertes Ereignis entgegennehmen |
| Abbruch anfordern | `CancellationToken` | `AbortController` | beendet nicht automatisch einen nativen Aufruf |
| Laufende Nachrichten | z. B. SignalR-Hub / WebSocket-Endpunkt | SignalR-Client / `WebSocket` | Snapshots an Adapter liefern |
| Wiederverbinden | Anwendungs- und Bibliotheksvertrag | Backoff, Abbruch, Resynchronisierung | Instanz-/Sitzungslebensdauer separat regeln |
| Überlast begrenzen | begrenzte Queues und Limits | Versand begrenzen, Updates zusammenfassen | geordnete Aufrufe pro Instanz |

HTTP und WebSocket sind Transportmechanismen. Keiner davon definiert allein Sitzungen, Berechtigungen, Duplikatbehandlung oder fachliche Bestätigungen.

## B. HTTP: Anfrage und Antwort

Ein HTTP-Request benennt unter anderem Methode, Ziel und Header; eine Antwort enthält Statuscode, Header und gegebenenfalls einen Body. Einzelne Requests können vorhandene Transportverbindungen wiederverwenden. „Request/Response“ bedeutet nicht, dass für jede Anfrage eine neue TCP-Verbindung entstehen muss.

Für Pink Robin sind laut [API-Vertrag](../docs/api/README.md) vorhanden:

| Anfrage | Aktuelle Bedeutung |
|---|---|
| `GET /health/live` | API-Prozess lebt, Status 200 |
| `GET /health/ready` | derzeit absichtlich 503: Core nicht verbunden |
| `GET /api/v1/status` | Infrastrukturstatus, derzeit keine Steuerungs- oder Echtzeitfunktionen |

`200` beim Statusendpunkt bedeutet nicht, dass Core-Aufträge bereits möglich sind. Dafür die gemeldeten Fähigkeiten und Bereitschaft beachten. Ein Timeout bedeutet umgekehrt nicht, dass der Server einen bereits empfangenen Befehl nicht ausgeführt hat.

**Achtung, C#-Gehirn:** Netzwerkfehler, HTTP-Fehlerstatus, ungültiges JSON und fachliche Ablehnung sind unterschiedliche Fälle. Ein hilfreicher Adapter wirft sie nicht in einen einzigen „offline“-Zustand.

## C. C# als Anker: asynchronen Status laden

Das folgende .NET-Konsolenbeispiel lädt den bereits dokumentierten lokalen Statusendpunkt. Die Development-API muss dafür separat laufen:

```csharp
using System;
using System.Net.Http;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

public static class Program
{
    private static readonly HttpClient Client = new()
    {
        BaseAddress = new Uri("http://localhost:5080"),
        Timeout = TimeSpan.FromSeconds(10)
    };

    public static async Task Main()
    {
        using var cts = new CancellationTokenSource();
        try
        {
            using var response = await Client.GetAsync("/api/v1/status", cts.Token);
            response.EnsureSuccessStatusCode();
            using var stream = await response.Content.ReadAsStreamAsync(cts.Token);
            using var document = await JsonDocument.ParseAsync(
                stream, cancellationToken: cts.Token);

            Console.WriteLine(document.RootElement.GetRawText());
            // Vor fachlicher Verwendung den Statusvertrag prüfen.
        }
        catch (OperationCanceledException)
        {
            Console.WriteLine("Warten abgebrochen oder Zeitlimit erreicht");
        }
        catch (HttpRequestException error)
        {
            Console.WriteLine($"HTTP-/Verbindungsfehler: {error.Message}");
        }
        catch (JsonException)
        {
            Console.WriteLine("Antwort ist kein gültiges JSON");
        }
    }
}
```

Der Client wird wiederverwendet; Response, Stream und JSON-Dokument haben begrenzte Lebensdauer. In einem ASP.NET-Core-Dienst kann ein über `IHttpClientFactory` konfigurierter Client sinnvoll sein. Für langlebige Clients zusätzlich DNS-/Connection-Lebensdauer passend konfigurieren. [Microsoft: HttpClient-Richtlinien](https://learn.microsoft.com/en-us/dotnet/fundamentals/networking/httpclient-guidelines)

Die Demo gibt nur JSON aus und behauptet keine abgeschlossene DTO-Validierung. Auch Timeout und bewusster Nutzerabbruch können beide als `OperationCanceledException` sichtbar werden; für eine differenzierte UI den verwendeten Abbruch- und Zeitlimitvertrag auswerten.

## D. TypeScript: fetch und ein echtes Abbruchsignal

```typescript
async function loadStatus(signal: AbortSignal): Promise<unknown> {
    const response = await fetch("/api/v1/status", {
        signal,
        cache: "no-store"
    });
    if (!response.ok) {
        throw new Error(`Statusanfrage fehlgeschlagen: HTTP ${response.status}`);
    }
    const value: unknown = await response.json();
    return value;
}
```

Der relative Pfad erwartet dasselbe Origin oder einen passend eingerichteten Entwicklungsproxy. Alternativ muss ein ausdrücklich anderes Origin korrekt für den Browserzugriff konfiguriert sein. Das JSON bleibt bis zur Prüfung nach Kapitel 12 `unknown`.

`fetch` lehnt bei einem HTTP-Status wie 503 nicht automatisch ab. `response.ok` prüfen. Ein Abbruchsignal kann die Browseroperation abbrechen; es garantiert keinen Rollback auf dem Server. [MDN: Fetch verwenden](https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API/Using_Fetch)

### Nur die aktuelle Antwort übernehmen

Dieser Lernhelfer kombiniert Abbruch mit einer Generation. `accept` übernimmt nur das Ergebnis der neuesten noch aktiven Anfrage:

```typescript
function createLatestLoader(
    load: (signal: AbortSignal) => Promise<unknown>,
    accept: (value: unknown) => void
): { refresh: () => Promise<void>; stop: () => void } {
    let generation = 0;
    let current: AbortController | undefined;
    let stopped = false;

    return {
        async refresh(): Promise<void> {
            if (stopped) return;
            const mine = ++generation;
            current?.abort();
            const controller = new AbortController();
            current = controller;
            try {
                const value = await load(controller.signal);
                if (!stopped && mine === generation)
                    accept(value);
            } catch (error: unknown) {
                if (stopped || mine !== generation || controller.signal.aborted)
                    return;
                throw error; // aktueller Fehler bleibt beobachtbar
            }
        },
        stop(): void {
            stopped = true;
            ++generation;
            current?.abort();
        }
    };
}
```

Aufruferinnen müssen `refresh()` abwarten oder dessen Fehler behandeln; beim Abbau der Ansicht `stop()` aufrufen. Nach `stop()` ist dieser Helfer dauerhaft beendet. Für eine neue Ansicht einen neuen Helfer erzeugen. `accept` validiert vor der fachlichen Übernahme und sollte kurz und synchron sein.

Die Generation bleibt wichtig, wenn eine alte Operation trotz Abbruch noch ein Ergebnis liefert. Dieser Helfer koordiniert lokale Requests; er ist kein vollständiger Polling-, Sitzungs- oder Wiederverbindungsdienst.

## E. Wann eine dauerhafte Verbindung sinnvoll ist

HTTP-Polling lädt den aktuellen Zustand wiederholt. Das ist einfach, verursacht aber auch bei unverändertem Zustand Requests und macht die Aktualität vom Intervall abhängig. Beim Polling erst nach Abschluss eines Durchlaufs neu planen oder bewusst Überlappungen regeln; kein unbegrenztes `setInterval` mit sich stauenden Anfragen.

Eine WebSocket-Verbindung erlaubt nach dem Aufbau Nachrichten in beide Richtungen. Geeignet sind etwa laufende Snapshots oder Interaktionen mit kurzer Rückmeldezeit. Die Browser-API verwendet `open`, `message`, `error` und `close`; erst bei `OPEN` senden. `send()` bedeutet zunächst, dass Daten zur Übertragung angenommen beziehungsweise gepuffert wurden, nicht dass Robin den Auftrag ausgeführt hat.

WebSocket erhält die Nachrichtenreihenfolge innerhalb einer bestehenden Verbindung. Nach Abbruch und Neuverbindung entstehen daraus keine automatische Historie und keine garantierte „genau einmal“-Verarbeitung. Anwendungsbestätigungen, Nachrichtengrössen und Auftragskennungen bleiben eigene Verträge.

Die klassische Browser-WebSocket-API bietet keine vollständige automatische Backpressure-Steuerung. `bufferedAmount` hilft, ausstehenden Versand zu beobachten; auch empfangene Daten können die Anwendung überlasten. [MDN: WebSocket-API](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API)

## F. SignalR ist mehr als ein roher WebSocket

Für das Backend ist SignalR als späterer Schritt dokumentiert. SignalR bietet ein eigenes Protokoll und abstrahiert Transportdetails; je nach Konfiguration sind neben WebSockets auch andere Transporte möglich. Ein nativer `new WebSocket(...)`-Client spricht deshalb nicht automatisch korrekt mit einem SignalR-Hub.

Ein konzeptioneller Client mit eingerichtetem `@microsoft/signalr`-Paket könnte so aufgebaut werden:

```typescript
import * as signalR from "@microsoft/signalr";

// Lernentwurf: Diese Hub-Route und Nachricht sind noch nicht implementiert.
const connection = new signalR.HubConnectionBuilder()
    .withUrl("/hubs/robin")
    .withAutomaticReconnect()
    .build();

connection.on("Snapshot", (value: unknown) => {
    // Schema, Sitzung/Generation und Reihenfolge vor Übernahme prüfen.
    console.log(value);
});

async function start(): Promise<void> {
    await connection.start();
    // Danach Sitzungszuordnung und frischen Zustand nach Vertrag herstellen.
}

async function stop(): Promise<void> {
    await connection.stop();
}
```

Der Aufrufer behandelt Start- und Stopfehler. `withAutomaticReconnect()` behandelt nicht automatisch fehlgeschlagene erste `start()`-Aufrufe. Nach ausgeschöpfter Wiederverbindungsstrategie braucht die Anwendung ebenfalls eine ausdrückliche Entscheidung. Wiederverbindungsereignisse wie `onreconnecting`, `onreconnected` und `onclose` helfen beim UI-Zustand und der Resynchronisierung. [Microsoft: SignalR-JavaScript-Client](https://learn.microsoft.com/aspnet/core/signalr/javascript-client)

Der Code allein implementiert diese Abläufe noch nicht. Auch ein erneutes technisches Verbinden stellt nicht von selbst die fachliche Sitzung oder alle benötigten Abonnements wieder her. Vor erneutem Senden prüfen, welche Verbindung, Sitzung und Generation tatsächlich aktuell sind.

## G. Wiederverbindung ist ein fachlicher Ablauf

Ein geeigneter UI-Verbindungsautomat unterscheidet beispielsweise `disconnected`, `connecting`, `connected`, `reconnecting` und `stopped`. „Verbunden“ auf Transportebene ist noch nicht „Core bereit“.

Nach einem Abbruch:

1. Veraltete Anzeige als solche kennzeichnen und neue Aktionen nach Vertrag begrenzen.
2. Mit wachsender Wartezeit und Zufallsanteil erneut versuchen; Versuche und Wartezeiten begrenzen.
3. Bei bewusstem Stop sämtliche erneuten Startversuche verhindern.
4. Berechtigung und Sitzungszuordnung wiederherstellen beziehungsweise erneuern.
5. Einen aktuellen vollständigen Zustand beziehen und erst dann laufende Updates übernehmen.

Ein Snapshot kann den letzten sichtbaren Zustand ersetzen. Er ersetzt nicht zwangsläufig verlorene einmalige Ereignisse. Für solche Ereignisse braucht es gegebenenfalls eine Historie, Sequenznummern oder einen expliziten „nicht mehr verfügbar“-Vertrag.

Robins Core-Revision zählt fachliche Änderungen, nicht jede Übertragung und nicht jeden Tick. Sie ist nur im Kontext der passenden Instanz sinnvoll. Für Nachrichtenvollständigkeit, Neustarts oder neue Sitzungen kann ein eigener Envelope mit Instanz-/Generationskennung und Transportsequenz nötig sein. Eine kleinere Revision nach einem Serverneustart darf nicht für immer als veraltet verworfen werden.

**Achtung, C#-Gehirn:** Ein automatischer Retry eines schreibenden Requests kann dieselbe Aktion doppelt auslösen. Ein verlorener Response ist kein Nachweis einer verlorenen Aktion. Wiederholbare Befehle benötigen einen passenden Idempotenz- oder Duplikatvertrag auf dem Server.

## H. Pink Robin: Transport vor dem Core begrenzen

```text
Browser: HTTP / später SignalR
           ↓ Transport, Validierung, Berechtigung, Sitzungszuordnung
Backend: begrenzte Verarbeitung pro Sitzung
           ↓ geordneter nativer Aufruf mit vertrauenswürdiger Zeit
C++-Core: fachlicher Zustand
           ↓ Snapshot-Kopie
Backend: Ausgabe und Verteilung
           ↓
Browser: gültige aktuelle Daten darstellen
```

Ein langsamer Browser darf die Core-Verarbeitung nicht unbegrenzt blockieren. Für reine Zustandsanzeigen können ältere noch nicht gesendete Snapshots gegebenenfalls durch den neuesten ersetzt werden. Ein Auftrag, eine Fehlermeldung oder ein Kontaktende darf dagegen nicht pauschal wie ein ersetzbares Bild behandelt werden.

Queue-Kapazität, maximale Nachrichtengrösse und Verhalten bei Überlast gehören in den Vertrag. Disconnect beendet nicht automatisch die fachliche Sitzung. Für einen laufenden Kopfkontakt muss entschieden werden, ob Verbindungsverlust einen Abbruch erzeugt oder eine befristete Wiederaufnahme zulässt; sonst könnte ein verlorenes Kontaktende den Zustand dauerhaft festhalten. Das ist eine zukünftige Adapterentscheidung, keine neue Core-Regel dieses Kapitels.

Sichere Auslieferung verwendet HTTPS und bei WebSockets WSS. Der Reverse Proxy benötigt passende Routen und für WebSockets die entsprechende Unterstützung. CORS ist eine Browserzugriffsregel und keine Authentifizierung; für WebSocket-/Hub-Verbindungen Origin- und Berechtigungsprüfung nach dem verwendeten Stack konfigurieren. Niemals die Sitzungszuordnung allein aus einer beliebigen Browserbehauptung übernehmen.

Der aktuelle [Backend-Stand](../backend/README.md) trennt Liveness und Readiness. `/hubs` ist derzeit noch nicht implementiert. Das Frontend verhält sich weiterhin lokal; dieses Kapitel schaltet keine reale Steuerung oder Verbindung frei.

## I. Gezielt testen

Neben dem erfolgreichen Request sind besonders wertvoll:

- HTTP 503, ungültiges JSON und Transportfehler getrennt behandeln.
- Zwei Antworten absichtlich in umgekehrter Reihenfolge abschliessen; nur die aktuelle übernehmen.
- Ansicht während einer Anfrage schliessen; danach keine UI-Übernahme zulassen.
- Erstverbindung fehlschlagen lassen und Wiederverbindung unabhängig davon prüfen.
- Während Backoff stoppen; kein späterer Neustart darf erfolgen.
- Sitzung nach Neustart neu zuordnen und einen vollständigen Snapshot laden.
- Langsame Empfänger und volle Queues mit festgelegter Strategie testen.

Tests dürfen Bibliotheks-Reconnect nicht mit vollständig wiederhergestellter fachlicher Sitzung gleichsetzen. Der reale Reverse-Proxy-Pfad braucht zusätzlich einen Integrationstest.

## J. Miniübung

1. Zwei Statusanfragen A und B laufen. B endet zuerst, A später. Welche Antwort übernimmt `createLatestLoader`?
2. Warum reicht `current.abort()` allein nicht als allgemeiner Schutz vor alten Antworten?
3. Ein Herzauftrag ist beim Server angekommen, aber seine HTTP-Antwort geht verloren. Ist blindes erneutes Senden sicher?
4. SignalR meldet nach Abbruch wieder eine Verbindung. Welche fachlichen Schritte fehlen möglicherweise noch?
5. Warum kann ein 200-Status von `/api/v1/status` mit einem 503 von `/health/ready` vereinbar sein?

<details>
<summary>Lösung anzeigen</summary>

1. B, sofern der Helfer nicht gestoppt wurde. A besitzt danach eine alte Generation.
2. Alte Fortsetzungen oder nicht kooperierende Operationen können weiterhin Ergebnisse liefern. Vor Übernahme zusätzlich Aktualität prüfen.
3. Nein. Der Core könnte die Herzreaktion bereits begonnen haben. Erneutes Senden könnte deren Frist erneut starten.
4. Berechtigung, Sitzungszuordnung, gewünschte Abonnements und einen aktuellen konsistenten Zustand wiederherstellen.
5. Die Status-API funktioniert, während die benötigte Core-Anbindung noch nicht bereit ist. Die Endpunkte beantworten unterschiedliche Fragen.

</details>

## Merksätze – Achtung, C#-Gehirn!

- HTTP-Erfolg, Transportverbindung und fachliche Bereitschaft getrennt behandeln.
- Abbruch beendet keinen bereits ausgeführten Auftrag rückwirkend.
- Nur aktuelle, validierte Antworten der richtigen Sitzung übernehmen.
- WebSocket und SignalR sind nicht dasselbe Protokoll.
- Wiederverbindung braucht Resynchronisierung, nicht nur einen neuen Socket.
- Retries, Sequenzen und Überlastverhalten ausdrücklich entwerfen.
- Netzwerk bleibt im Adapter; der Core verarbeitet geordnete Ereignisse und Zeitwerte.

[Zurück zu Kapitel 16](16-native-anbindung-csharp-cpp.md) · [Inhaltsverzeichnis](README.md)
