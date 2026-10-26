# 23 – Logging und Diagnose

## Lernziel

Du kannst einen Fehler über mehrere Schichten eingrenzen, strukturierte Logs schreiben und Logs, Metriken sowie Traces unterscheiden. Du erkennst, welche Kennungen zusammengehören und welche Informationen für eine Diagnose fehlen.

Nach [Kapitel 22](22-architektur-und-abhaengigkeitsgrenzen.md) kennen wir die Verantwortlichkeiten. Jetzt machen wir ihre Zusammenarbeit beobachtbar. **Alle Instrumentierungsbeispiele sind Lernentwürfe; dieses Kapitel richtet keinen Telemetriedienst ein und ändert keinen Produktivcode.**

## A. Spickzettel

| Werkzeug | Beantwortet vor allem | Pink-Robin-Beispiel |
|---|---|---|
| Log | Was geschah bei einem bestimmten Vorgang? | Core hat Ereignis wegen Rückwärtszeit abgelehnt |
| Metrik | Wie oft, wie viel, wie lange? | Anzahl Ablehnungen, Queue-Tiefe, Verarbeitungsdauer |
| Trace | Welche Schritte gehörten zu einem Ablauf und wie lange dauerten sie? | Request → Queue → Core → Antwort |
| Debugger | Wie sieht der laufende Zustand an dieser Stelle aus? | Kontaktflag und Zeitmarke vor dem Übergang |
| Profiler | Wo werden CPU, Speicher oder andere Ressourcen verbraucht? | hohe Allokationsrate im Ausgabeadapter |
| Health-Endpunkt | Lebt beziehungsweise funktioniert eine definierte Abhängigkeit? | API lebt, Core aber noch nicht bereit |

Diese Signale ergänzen sich. Ein einzelner Text „Fehler“ erklärt keine Ursache; eine reine Fehlerrate erklärt keinen konkreten Ablauf. [OpenTelemetry: Signale](https://opentelemetry.io/docs/concepts/signals/)

## B. C# als Anker: strukturierte Logs

Mit `ILogger` bleiben eine feste Nachrichtenvorlage und ihre Werte getrennt. Ein Logprovider kann Felder dadurch durchsuchbar speichern.

Der folgende Ausschnitt gehört in ein Projekt mit `Microsoft.Extensions.Logging`-Referenz, etwa ein entsprechend eingerichtetes ASP.NET-Core-Projekt. Er zeigt eine vollständige Klasse, aber keinen eigenen Host:

```csharp
using Microsoft.Extensions.Logging;

public sealed class CoreDiagnostics
{
    private readonly ILogger<CoreDiagnostics> logger;

    public CoreDiagnostics(ILogger<CoreDiagnostics> logger)
    {
        this.logger = logger;
    }

    public void Report(string operationId, uint status,
        ulong timeMs, ulong revision)
    {
        if (status == 0)
        {
            logger.LogDebug(
                "Core-Aufruf {OperationId} erfolgreich bei {CoreTimeMs}, Revision {Revision}",
                operationId, timeMs, revision);
        }
        else
        {
            logger.LogWarning(
                "Core-Aufruf {OperationId} abgelehnt: Status {CoreStatus}, Zeit {CoreTimeMs}",
                operationId, status, timeMs);
        }
    }
}
```

`{CoreStatus}` ist ein Feldname in einer konstanten Vorlage. Eine interpolierte Zeichenfolge wie `$"Status {status}"` erzeugt stattdessen schon vor dem Logging den fertigen Text und verliert diese explizite Struktur. Platzhalterwerte werden bei `ILogger` positionsbezogen zugeordnet; sorgfältig auf Anzahl und Reihenfolge achten.

Ein Logscope kann gemeinsame Werte über mehrere Meldungen führen. Ob Scopes und Felder in der Ausgabe erscheinen, hängt vom Provider und seiner Konfiguration ab. Ein JSON-Console-Formatter ist eine mögliche Ausgabeform, aber nicht jeder Logaufruf ergibt automatisch ein zentral gespeichertes JSON-Dokument. [Microsoft: Logging](https://learn.microsoft.com/en-us/dotnet/core/extensions/logging)

Teure Snapshot-Formatierung nicht ungeprüft als Argument eines deaktivierten Debug-Logs ausführen. Bei häufigen Aufrufen helfen eine Levelprüfung oder quellgeneriertes Logging. Ein Log sollte die Reaktionszeit nicht durch aufwendige Arbeit unter einer Core-Sperre bestimmen.

## C. Welche Schwere passt?

| Level | Typische Verwendung |
|---|---|
| Trace / Debug | Details eines Ablaufs, gezielt für Diagnose aktiviert |
| Information | relevante normale Lebenszyklusereignisse und Betriebsänderungen |
| Warning | auffälliger, behandelter Zustand; Arbeit kann kontrolliert weitergehen |
| Error | eine beabsichtigte Operation ist fehlgeschlagen |
| Critical | schwerwiegender Ausfall, der den Dienst grundsätzlich beeinträchtigt |

Nicht jeder ungültige Browserrequest ist ein Systemfehler. Und „Core absichtlich noch nicht angebunden“ muss nicht bei jeder Gesundheitsprüfung einen neuen Error erzeugen. Häufige Ablehnungen können als Metrik und begrenzte Diagnoselogs sinnvoller sein.

Für Exceptions die Überladung mit Exceptionobjekt verwenden, beispielsweise `logger.LogError(error, "Ausgabe für {OperationId} fehlgeschlagen", operationId)`. So kann der Provider Stack und Typ erhalten. Nicht dieselbe Exception in jeder Schicht erneut als unabhängigen Error protokollieren. Eine zuständige Grenze meldet den Fehlschlag mit dem nötigen Kontext; andere Schichten reichen ihn weiter oder übersetzen ihn.

## D. Kennungen mit unterschiedlichen Aufgaben

Eine Kennung sollte eine klare Bedeutung haben:

- **Request-ID:** ein einzelner HTTP-Aufruf.
- **Operation-ID:** ein fachlicher Versuch oder Auftrag nach festgelegtem Vertrag.
- **Trace-ID / Span-ID:** Diagnosekontext eines instrumentierten Ablaufs beziehungsweise Schritts.
- **Sitzungs-/Instanzkennung:** welcher Robin-Zustand betroffen ist.
- **Generation:** welche Lebensphase nach Neustart oder Neuverbindung gemeint ist.
- **Revision:** fachlicher Änderungsstand des Core, keine universelle Transportsequenz.

Nicht jede Meldung benötigt alle Felder. Für einen Core-Aufruf sind häufig Operation, Instanz, Ereigniskennung, übergebene Zeit, Status und eine verfügbare Revision hilfreich. Einen Snapshot nur nach erfolgreichem Lesen als gültig protokollieren; nicht nach einem Fehler einen Standardwert als echten Zustand ausgeben.

Eine vom Browser gelieferte Kennung ist ungeprüfte Eingabe. Länge und Format begrenzen, sie nicht als Berechtigung behandeln und nicht beliebige Texte unkontrolliert in Logs übernehmen. Eine Diagnose-ID ist kein Geheimnis, mit dem sich ein Auftrag autorisieren lässt.

**Achtung, C#-Gehirn:** Die Core-Revision ersetzt weder Request-ID noch vollständige Ereignishistorie. Ein einziger Aufruf kann mehrere fällige Übergänge enthalten; ein Tick ohne fachliche Änderung erhöht die Revision nicht zwingend.

## E. TypeScript: gezielte Diagnose statt rohe Nachrichten

Ein kleiner Lernhelfer erstellt ein bewusst eingeschränktes Diagnoseobjekt. Die Revision bleibt als Dezimalstring erhalten, passend zu Kapitel 17:

```typescript
type CoreOutcome = {
    operationId: string;
    status: "ok" | "rejected";
    revision: string;
};

function toDiagnostic(value: CoreOutcome): {
    event: string;
    operationId: string;
    outcome: string;
    revision: string;
} {
    return {
        event: "core_outcome_received",
        operationId: value.operationId,
        outcome: value.status,
        revision: value.revision
    };
}

console.debug(toDiagnostic({
    operationId: "op-demo-1",
    status: "ok",
    revision: "9007199254740993"
}));
```

Das Modell muss vor diesem Schritt bereits geprüft sein. `console.debug` zeigt lokal ein Objekt; es stellt keine zentrale Sammlung oder garantierte Speicherung her. Für einen Diagnosesnapshot ist eine bewusst erzeugte Kopie hilfreicher als eine riesige veränderliche Objektstruktur, deren spätere DevTools-Anzeige verwirrend sein kann.

Browserlogs sind ausserdem kein Beweis für die tatsächliche Core-Verarbeitung. „Nachricht gesendet“ und „Core hat Ereignis ausgeführt“ sind unterschiedliche Beobachtungen. Beim Melden unterscheiden, ob der Client sendet, der Server annimmt oder das fachliche Ergebnis bestätigt wurde.

## F. Tracing: einen Ablauf in Schritte zerlegen

Ein Trace verbindet zeitlich zusammengehörige Schritte, sogenannte Spans. In .NET ist `Activity` der etablierte Begriff für einen solchen Schritt. `ActivitySource` kann eigene Schritte erzeugen; ohne interessierten Listener kann `StartActivity` `null` liefern. Deshalb nullable Rückgaben berücksichtigen. [Microsoft: Distributed Tracing](https://learn.microsoft.com/en-us/dotnet/core/diagnostics/distributed-tracing-concepts)

```csharp
using System.Diagnostics;

public static class RobinTracing
{
    public static readonly ActivitySource Source = new("PinkRobin.Learning");

    public static void RunStep(System.Action action)
    {
        using var activity = Source.StartActivity("core.dispatch");
        try
        {
            action();
            activity?.SetTag("core.outcome", "ok");
        }
        catch
        {
            activity?.SetStatus(ActivityStatusCode.Error);
            throw;
        }
    }
}
```

Das ist Instrumentierung, keine vollständige OpenTelemetry-Konfiguration. Erfassung, Sampling und Export werden separat eingerichtet. Der Helfer ist synchron und passt nicht unverändert für einen Callback, der nur einen noch laufenden Task startet.

Ein P/Invoke-Aufruf überträgt keinen Tracekontext automatisch in beliebige C++-Instrumentierung. Im Backend kann ein Span den nativen Aufruf umschliessen. Da der Core kurz und synchron arbeitet, muss er dafür nicht selbst von einem Telemetrieframework abhängig werden.

Bei HTTP unterstützen viele Frameworks standardisierte Kontextweitergabe. Bei eigenen Queues oder Nachrichten den Kontext gezielt weiterführen oder verknüpfen; nicht einfach erwarten, dass jeder neue Hintergrundablauf ihn korrekt erbt. Diagnosesampling bedeutet zudem, dass nicht jede Operation als vollständiger Trace gespeichert wird.

## G. Metriken: wenige sinnvolle Dimensionen

Für Pink Robin wären beispielsweise hilfreich:

| Metrik | Zweck | Sinnvolle begrenzte Dimension |
|---|---|---|
| Anzahl Core-Aufrufe | Verkehr und Ablehnungen erkennen | Ereignistyp, Ergebniscode |
| Queue-Tiefe | Rückstau erkennen | Queue-Kategorie |
| Queue-Wartezeit | Verzögerung vor Verarbeitung | Verarbeitungskategorie |
| Core-Verarbeitungsdauer | Aufwand des synchronen Schritts | Operationstyp |
| Aktive Sitzungen | Kapazität beobachten | Dienstinstanz nach Telemetrievertrag |
| Wiederverbindungsversuche | instabile Verbindung erkennen | Ausgang / Fehlerkategorie |

Session-IDs oder Operation-IDs nicht als beliebig wachsende Metriklabels verwenden. Jede neue Kombination von Attributwerten kann eine weitere Zeitreihe erzeugen. Diese **Kardinalität** kostet Speicher und Verarbeitung. Einzelvorgänge gehören eher in gezielte Logs und Traces. [OpenTelemetry: Metriken](https://opentelemetry.io/docs/concepts/signals/metrics/)

Ein Mittelwert allein kann lange Ausreisser verstecken. Verteilungen beziehungsweise Histogramme helfen bei Latenzen; Einheit und Messgrenze müssen feststehen. „Core dauert 2 ms“ ist ohne Angabe, ob Queue-Warten und Versand enthalten sind, kaum vergleichbar.

## H. Zeitstempel und Dauer nicht verwechseln

Ein UTC-Zeitstempel hilft, Meldungen verschiedener Prozesse grob zusammenzuführen. Eine monotone Dauer misst, wie lange ein Schritt im selben Zeitkontext braucht. Core-Millisekunden seit Instanzbeginn sind keine UTC-Zeit und nicht mit der Zeitbasis einer anderen Instanz direkt vergleichbar.

Uhren verschiedener Rechner können abweichen. Deshalb nicht allein nach sortierten UTC-Zeitstempeln auf eine strenge Ereignisreihenfolge schliessen. Kennungen, lokale Sequenzen und der explizite Verarbeitungsvertrag ergänzen die Zeitinformation.

Für einen langsamen Vorgang mindestens diese Grenzen unterscheiden:

```text
Request empfangen
    ↓ Queue-Wartezeit
Core-Schritt beginnt
    ↓ Core-Verarbeitungsdauer
Snapshot kopiert
    ↓ Aufbereitung und Versand
Client übernimmt aktuellen Zustand
```

Ein schneller Core kann mit einer überlasteten Queue oder einem langsamen Empfänger zusammen trotzdem eine langsame Anzeige ergeben. Messen an den Verantwortungsgrenzen hilft, die richtige Ursache zu bearbeiten.

## I. C++17-Praxis: Diagnose im Adapter

Das folgende vollständige Lernprogramm verwendet die echte [Core-API](../core/include/pinkrobin/core.hpp). Es protokolliert nur kontrollierte Werte und hält das Logging ausserhalb einer Core-Sperre; im Beispiel gibt es keine konkurrierenden Zugriffe:

```cpp
#include "pinkrobin/core.hpp"
#include <chrono>
#include <iostream>

int main()
{
    pinkrobin::Core core{0};
    const auto start = std::chrono::steady_clock::now();
    const auto result = core.dispatch(pinkrobin::Event::heart_touched, 100);
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - start).count();

    std::clog << "operation=heart_touch status="
              << static_cast<unsigned>(result)
              << " duration_ns=" << elapsed;
    if (result == pinkrobin::Result::ok) {
        const auto snapshot = core.snapshot();
        std::clog << " core_time_ms=" << snapshot.time_ms
                  << " revision=" << snapshot.revision;
    }
    std::clog << '\n';
    return result == pinkrobin::Result::ok ? 0 : 1;
}
```

`duration_ns` ist eine Darstellung in Nanosekunden, keine Zusage nanosekundengenauer Messauflösung. Ein einzelner solcher Lauf ist kein belastbarer Performancebenchmark. Logging, Scheduler und Messaufwand beeinflussen das Ergebnis.

`std::clog` und dieser kontrollierte Text sind noch kein zentraler strukturierter Telemetrieexport. Für variable externe Texte wären zusätzlich korrekte Kodierung und Escaping nötig. Der Core selbst bekommt durch das Beispiel keine Datei-, Konsolen- oder Netzwerkabhängigkeit; der aufrufende Adapter verantwortet die Ausgabe.

## J. Diagnose an einem konkreten Fehler

Angenommen, der Browser zeigt nach einer Herzberührung weiter neugierige Augen. Dann prüfen wir schrittweise:

1. Hat die UI die Eingabe erkannt und welchen Modus verwendet sie – lokale Demo oder angebundener Core?
2. Hat das Backend den Request angenommen, validiert und der richtigen Sitzung zugeordnet?
3. Wurde der Core-Aufruf ausgeführt oder blieb er in der Queue / wurde abgelehnt?
4. Welchen Status, Ausdruck und welche Revision hatte der erfolgreiche Snapshot?
5. Wurde genau diese Antwort versandt und im Client noch als aktuell akzeptiert?
6. Hat die Darstellung den gültigen Ausdruck richtig abgebildet?

Ein Core-Status `time_reversed` lenkt die Suche etwa zur Zeitbasis oder Ereignisreihenfolge. Ein erfolgreicher Snapshot mit `happy`, der im Browser nicht erscheint, lenkt sie zur Ausgabe- und Clientseite. Nicht vorschnell die fachliche Reaktionsregel ändern, wenn der Fehler an einer anderen Grenze liegt.

Der [aktuelle Backend-Status](../backend/README.md) meldet die Core-Anbindung noch als nicht hergestellt. Eine funktionierende lokale Animation ist deshalb kein Beleg für einen durchgängigen Backend-Core-Ablauf. Liveness und Readiness beantworten weiterhin unterschiedliche Fragen.

## K. Datenmenge und Vertraulichkeit begrenzen

Keine Tokens, Cookies, Passwörter, vollständigen personenbezogenen Nachrichten oder unnötigen Sensordaten protokollieren. Diagnosekennungen können ebenfalls zuordenbar sein; Zugriff, Aufbewahrung und Weitergabe bewusst regeln. Ein Hash macht eine Kennung nicht automatisch anonym.

Für häufige normale Vorgänge Metriken oder Sampling verwenden, statt jede Sekunde vollständige Zustände zu schreiben. Fehlerfluten begrenzen und verworfene Diagnoseeinträge sichtbar zählen. Eine Logqueue darf nicht unbeschränkt wachsen, wenn der Empfänger ausfällt.

Fachliche Verarbeitung darf bei einer üblichen Diagnoseausgabe-Störung nicht unbemerkt ihr Verhalten ändern. Andererseits sind verbindliche Audit-Anforderungen ein eigener Vertrag; ein best-effort Debug-Log erfüllt sie nicht automatisch. Für Pink Robin keine zusätzliche Auditpflicht aus dem Wort Logging ableiten.

## L. Miniübung

1. Ein Request dauert 800 ms, der Core-Schritt 1 ms. Welche zwei weiteren Abschnitte würdest du zuerst messen?
2. Warum ist die Session-ID als Metriklabel bei beliebig vielen Sitzungen problematisch?
3. Die UI meldet „gesendet“, das Backend meldet Core-Status `2`. War der Auftrag fachlich erfolgreich?
4. Warum beweist eine fehlende Logzeile nicht, dass der Code nicht ausgeführt wurde?
5. Welche Felder helfen, einen Snapshot mit einer alten Antwort einer vorherigen Instanz auseinanderzuhalten?

<details>
<summary>Lösung anzeigen</summary>

1. Queue-Wartezeit sowie Aufbereitung/Versand und Clientübernahme getrennt ansehen; der Core ist nicht automatisch der Engpass.
2. Die Anzahl der Zeitreihen kann unkontrolliert wachsen. Aggregierte Dimensionen verwenden und Einzelkennungen in passende Logs/Traces legen.
3. Nein. Status `2` bedeutet rückwärts laufende Zeit. „Gesendet“ beschreibt nur einen früheren Transportschritt.
4. Level-Filter, Sampling, fehlender Provider, Pufferung, Exportfehler oder Abbruch können die Ausgabe verhindern.
5. Instanz-/Sitzungskontext und Generation, dazu die Revision und passende Operation-/Requestkennung. Revision allein reicht nicht.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Logs beschreiben Vorgänge, Metriken aggregieren, Traces verbinden Schritte.
- Feste Vorlagen mit getrennten Werten erleichtern zuverlässige Auswertung.
- Request, Operation, Sitzung, Generation und Core-Revision haben unterschiedliche Bedeutungen.
- Diagnose an Verantwortungsgrenzen aufbauen und Dauer von Kalenderzeit trennen.
- Fehlende Telemetrie ist kein sicherer Nichtausführungsnachweis.
- Logging gehört nicht als blockierende Infrastruktur in den fachlichen Core.
- Datenmenge, Kardinalität und sensible Inhalte bewusst begrenzen.

[← 22 – Architektur und Abhängigkeitsgrenzen](22-architektur-und-abhaengigkeitsgrenzen.md) · [Inhaltsverzeichnis](README.md) · [24 – Performance und Ressourcenbudgets →](24-performance-und-ressourcenbudgets.md)
