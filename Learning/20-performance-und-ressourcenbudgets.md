# 20 – Performance und Ressourcenbudgets

## Lernziel

Du kannst ein messbares Performanceziel formulieren, den tatsächlichen Engpass finden und CPU, Speicher sowie Nachrichtenfluss begrenzen. Du unterscheidest fachliche Korrektheit von Geschwindigkeit und Einzelmessungen von belastbaren Vergleichen.

[Kapitel 19](19-logging-und-diagnose.md) hat Diagnoseinformationen eingeführt. Jetzt verwenden wir sie für gezielte Verbesserungen. **Die Zahlen und Budgets dieses Kapitels sind Rechen- und Lernbeispiele, keine gemessenen Leistungszusagen für Pink Robin. Es werden keine Produktivlimits oder Optimierungen eingebaut.**

## A. Spickzettel

| Frage | C# / Backend | TypeScript / Browser | C++17 / Core |
|---|---|---|---|
| Dauer messen | `Stopwatch` / Instrumentierung | `performance.now()` / Performance-Werkzeuge | `std::chrono::steady_clock` |
| CPU-Verbrauch untersuchen | Profiler / Trace | DevTools-Performanceprofil | nativer Profiler |
| Speicher untersuchen | Allokationen, GC, Heap | Heap-Snapshots, Listener und DOM | Allokationen, Ownership, native Werkzeuge |
| Arbeit begrenzen | begrenzte Queue und Admission-Limits | Requests und Versand begrenzen | begrenzte Eingaben und geordnete Aufrufe |
| Ausgabe entlasten | Snapshots nach Vertrag zusammenfassen | aktuelle Anzeige aktualisieren | keine zusätzliche Netzwerkverantwortung |
| Vergleich absichern | Release, gleiche Last, mehrere Läufe | gleiche Umgebung, warme / kalte Pfade trennen | gleicher Compiler, Flags und Hardware |

Ein schnelleres falsches Ergebnis ist keine Verbesserung. Nach einer Optimierung bleiben die Verhaltenstests aus Kapitel 13 und 15 verbindlich.

## B. Ein Ziel vor der Änderung formulieren

„Robin soll schnell reagieren“ ist noch kein prüfbares Ziel. Hilfreicher ist ein ausdrücklich vorläufiges Ziel wie:

> Unter einer festgelegten Anzahl aktiver Sitzungen sollen 95 Prozent der bestätigten Berührungen innerhalb eines vereinbarten Zeitbudgets sichtbar werden.

Dazu festlegen: Wo beginnt die Messung? Wo endet sie? Was zählt als Bestätigung? Welche Hardware, Verbindung und Last gelten? Auch Fehlerquote, maximaler Rückstau und Speicherbedarf gehören zur Beurteilung.

Ein beispielhaftes Gesamtbudget von 200 ms könnte so aufgeteilt werden:

| Abschnitt | Beispielbudget |
|---|---:|
| Browser bis Backend | 40 ms |
| Warten auf Verarbeitung | 20 ms |
| Core und Abbildung | 10 ms |
| Rückweg | 40 ms |
| Übernahme und Darstellung | 30 ms |
| Reserve | 60 ms |

Die Summe ist eine Planungsrechnung, kein Messergebnis. Einzelne p95-Werte lassen sich nicht ohne Weiteres zu einem Gesamt-p95 addieren. Für das Nutzerziel auch den gesamten Ablauf messen; bei verschiedenen Rechneruhren deren Grenzen berücksichtigen.

**Achtung, C#-Gehirn:** Ein niedriger Mittelwert kann lange Wartezeiten einzelner Nutzerinnen verdecken. Verteilungen, Ausreisser und Fehlerrate gemeinsam ansehen.

## C. Erst den Engpass bestimmen

Aus Kapitel 19 kennen wir die Abschnitte Request, Queue, Core und Ausgabe. Ist der Core in einem Beispiel 1 ms beschäftigt, der Request aber 800 ms unterwegs, löst eine halbierte Core-Dauer das Gesamtproblem kaum.

Für eine einfache Rechenhilfe: Benötigt ein Abschnitt 10 Prozent der Gesamtzeit, kann sein vollständiges hypothetisches Wegfallen höchstens diese 10 Prozent einsparen. Parallelität oder ein anderer Algorithmus muss an einem relevanten Abschnitt ansetzen.

Ein CPU-Profil zeigt, wo tatsächlich Rechenzeit verbraucht wird. Eine hohe Latenz bei niedriger CPU kann dagegen auf Warten, Locks, I/O oder Rückstau hinweisen. .NET bietet dafür unter anderem Zähler, Traces und Speicherdiagnose; Werkzeug und Plattform müssen zusammenpassen. [Microsoft: Diagnosewerkzeuge](https://learn.microsoft.com/dotnet/core/diagnostics/)

Vorgehen: reproduzierbare Last wählen, Ausgangswerte aufnehmen, eine konkrete Hypothese formulieren, eine Änderung durchführen und erneut unter denselben Bedingungen messen. Keine gleichzeitig vorgenommenen fünf Optimierungen, deren Wirkung sich danach nicht zuordnen lässt.

## D. C# als Anker: begrenzte Queue statt unbegrenzter Tasks

Eine Queue speichert Arbeit, erzeugt aber keine zusätzliche Verarbeitungskapazität. Treffen dauerhaft mehr Aufgaben ein als abgearbeitet werden können, wächst ein unbegrenzter Rückstau.

Das folgende vollständige .NET-Konsolenbeispiel zeigt einen begrenzten Channel mit kontrollierter Ablehnung. Es ist **keine bereits eingebaute Core-Sitzungsqueue**:

```csharp
using System;
using System.Threading.Channels;
using System.Threading.Tasks;

public static class Program
{
    public static async Task Main()
    {
        var queue = Channel.CreateBounded<int>(new BoundedChannelOptions(2)
        {
            SingleReader = true,
            SingleWriter = true,
            FullMode = BoundedChannelFullMode.Wait
        });

        bool first = queue.Writer.TryWrite(1);
        bool second = queue.Writer.TryWrite(2);
        bool third = queue.Writer.TryWrite(3);
        Console.WriteLine($"{first} {second} {third}"); // True True False

        queue.Writer.Complete();
        await foreach (int value in queue.Reader.ReadAllAsync())
            Console.WriteLine(value); // 1, danach 2
    }
}
```

Bei `FullMode.Wait` wartet `TryWrite` trotzdem nicht; es liefert bei vollem Channel `false`. `WriteAsync` kann dagegen auf Kapazität warten und sollte einen passenden Abbruchvertrag erhalten. Ein wartender Producer belegt weiterhin Kontext und gegebenenfalls Request-Ressourcen. Auch die Zahl wartender Producer muss begrenzt sein. [Microsoft: Channels](https://learn.microsoft.com/en-us/dotnet/core/extensions/channels)

Die `SingleReader`-/`SingleWriter`-Angaben müssen zur tatsächlichen Nutzung passen. Ein einziger Reader hilft bei geordneter Verarbeitung nur, wenn er die Arbeit auch wirklich nacheinander abschliesst. Startet er für jeden Eintrag ungeprüft einen neuen Task, geht diese Eigenschaft verloren.

Drop-Modi nicht pauschal für Berührungsereignisse einsetzen: Ein verlorenes Kontaktende kann eine Reaktion festhalten. Für ersetzbare Anzeigesnapshots ist eine andere Strategie möglich als für Befehle oder einmalige Fehler.

## E. Queue-Kapazität ist ein fachliches Budget

Eine Kapazität von 1000 Einträgen ist nicht automatisch besser als 20. Sie kann lediglich mehr veraltete Arbeit sammeln. Kapazität nach erlaubter Wartezeit, Eingabegrösse, Verarbeitungskapazität und fachlicher Bedeutung wählen.

Ein hypothetischer Verarbeiter mit 100 Aufgaben pro Sekunde braucht zum Leeren eines bereits vorhandenen Rückstaus von 200 Aufgaben ungefähr zwei Sekunden, **wenn keine neue Arbeit hinzukommt**. Das ist eine grobe Rechnung, keine allgemeine Latenzgarantie. Unterschiedliche Aufwände und Lastspitzen verändern sie.

Ein Limit pro Sitzung schützt ausserdem nicht vor beliebig vielen Sitzungen. Deshalb globale Kapazität, Sitzungszahl, maximale Nachrichtengrösse und Lebensdauer gemeinsam begrenzen. Bei Ablehnung eine klare Rückmeldung statt stillen Verlust vorsehen.

## F. TypeScript: nur den neuesten Anzeigestand pro Frame übernehmen

Für vollständige Snapshots kann eine Ansicht Zwischenstände nach Vertrag ersetzen. Der folgende Lernhelfer plant höchstens einen Renderaufruf pro Animationsframe und verwendet dann den zuletzt angebotenen Wert:

```typescript
function createFrameUpdater<T>(
    render: (value: T) => void
): { offer: (value: T) => void; stop: () => void } {
    let latest: T;
    let frame: number | undefined;
    let stopped = false;

    return {
        offer(value: T): void {
            if (stopped) return;
            latest = value;
            if (frame !== undefined) return;
            frame = requestAnimationFrame(() => {
                frame = undefined;
                if (!stopped) render(latest);
            });
        },
        stop(): void {
            stopped = true;
            if (frame !== undefined) cancelAnimationFrame(frame);
            frame = undefined;
        }
    };
}
```

Eingaben vor `offer` validieren und gegen falsche Sitzung beziehungsweise Generation absichern. Nach dem Stop wird dieser Helfer nicht neu gestartet. Er kopiert `T` nicht tief; angebotene Zustandsobjekte danach nicht unkontrolliert verändern. Fehler im Renderer benötigen einen eigenen UI-Fehlervertrag.

`requestAnimationFrame` ist eine Möglichkeit zur Darstellung, kein zuverlässiger fachlicher Timer. Hintergrundtabs können Aufrufe stark drosseln oder pausieren. Robins Reaktionsfristen bleiben im Core und werden nicht aus gezählten Frames berechnet.

Das Ersetzen von Zwischenständen eignet sich nicht für eine lückenlos benötigte Ereignisliste. Ein „Kopfkontakt beendet“-Befehl darf nicht verschwinden, nur weil inzwischen ein anderer Anzeigestand existiert.

Für lokale Dauervergleiche im Browser ist `performance.now()` geeigneter als die verstellbare Kalenderuhr. Die verfügbare Auflösung kann aus Datenschutzgründen reduziert sein; Zahlen mit vielen Nachkommastellen garantieren keine entsprechende Genauigkeit. [MDN: Zeitmessung](https://developer.mozilla.org/en-US/docs/Web/API/Performance_API/High_precision_timing)

## G. Speicher: Grösse, Lebensdauer und Anzahl

Ein grobes Speicherbudget lautet:

```text
Grundbedarf
+ aktive Sitzungen × Bedarf pro Sitzung
+ wartende Einträge × Bedarf pro Eintrag
+ Ausgabe-, Netzwerk- und Diagnosepuffer
+ Laufzeit- und Allokatorreserve
```

Der native ABI-Snapshot hat 40 Bytes. Das bedeutet nicht, dass 1000 gespeicherte C#- oder JavaScript-Nachrichten exakt 40 000 Bytes brauchen: Container, Referenzen, Objektheader, Strings, JSON und Puffer kommen hinzu. Auch ein `sizeof` eines C++-Containers enthält nicht automatisch dessen dynamischen Inhalt.

In C# und JavaScript nach unbeabsichtigt festgehaltenen Objekten, Listenern, Closures und laufenden Tasks suchen. GC kann nur freigeben, was nach den Laufzeitregeln nicht mehr benötigt beziehungsweise erreichbar ist. In C++ bleiben Ownership, Leaks und Fragmentierung wichtige Themen. Speicher, den ein Allokator oder GC reserviert hält, ist nicht ohne Weiteres derselbe Wert wie aktuell lebende Nutzdaten.

Pooling kann Allokationen reduzieren, verlängert aber Besitz und erhöht die Gefahr wiederverwendeter oder alter Inhalte. Nicht vor einer Messung komplizierte Pools einführen. Der [aktuelle Verhaltensteil des Core](../core/README.md) benötigt bereits keine dynamischen Allokationen; der C-Wrapper allokiert einen Handle pro Erzeugung.

## H. C++17: eine Messschleife richtig einordnen

Dieses vollständige Lernprogramm misst einen Batch gegen die echte Core-API. Es ist ein Messgerüst, **kein standardisierter Benchmark und kein Leistungsnachweis für Zielhardware**:

```cpp
#include "pinkrobin/core.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>

int main()
{
    constexpr std::uint64_t count = 100000;
    pinkrobin::Core core{0};
    std::uint64_t checksum = 0;

    const auto start = std::chrono::steady_clock::now();
    for (std::uint64_t i = 1; i <= count; ++i) {
        const auto result = core.dispatch(pinkrobin::Event::heart_touched, i);
        if (result != pinkrobin::Result::ok) return 1;
        checksum += core.snapshot().revision;
    }
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end - start).count();

    std::cout << "batch_ns=" << elapsed
              << " operations=" << count
              << " checksum=" << checksum << '\n';
    return 0;
}
```

Die ausgegebene Prüfsumme macht Ergebnisse beobachtbar; Compiler dürfen trotzdem weitreichend optimieren. Die gemessene Arbeit umfasst Dispatch, Snapshot, Statusprüfung und Schleifenaufwand. Eine Division durch die Anzahl liefert eine Batch-Durchschnittsgrösse, keine p95-Latenz einzelner Nutzerinteraktionen.

Für belastbare Vergleiche mehrere Läufe, Warm-up, Compileroptimierung, CPU-Frequenz, Hintergrundlast und reale Eingabeverteilung berücksichtigen. Ein Optimierer kann synthetische Arbeit stark vereinfachen; reinen Aufrufaufwand bei Bedarf mit geeigneten Benchmarkwerkzeugen untersuchen. Keine Konsolenausgabe innerhalb des gemessenen Hot Paths platzieren.

**Achtung, C#-Gehirn:** Ein Debug-Build, ein instrumentierter Sanitizer-Lauf und ein optimierter Release-Build beantworten unterschiedliche Fragen. Geringerer Messaufwand ist ausserdem nicht automatisch bessere fachliche Abdeckung.

## I. Häufige sinnvolle Verbesserungen

| Beobachtung | Mögliche gezielte Änderung | Danach prüfen |
|---|---|---|
| Viele gleiche Zustandsupdates | Ausgabe nach Revision beziehungsweise relevantem Zustand begrenzen | Vollständige Resynchronisierung bleibt möglich |
| UI zeichnet öfter als sichtbar nötig | Rendering pro Frame bündeln | keine benötigten Ereignisse verlieren |
| Viele kurzlebige Strings | Formatierung / Serialisierung anpassen | JSON-Vertrag bleibt gleich |
| Hohe Queue-Wartezeit | Einlass begrenzen, Erzeugungsrate senken, Verarbeitung untersuchen | Ablehnung ist sichtbar und fair |
| Eine langsame Sitzung bremst andere | Sitzungsisolation und Ausgabepuffer prüfen | globale Ressourcen bleiben begrenzt |
| Häufige P/Invoke-Aufrufe ohne Nutzen | fachlich passende Aufrufhäufigkeit prüfen | Zeitfortschritt und Reaktionsfristen bleiben korrekt |

Nicht einfach Revision als universellen Versandfilter verwenden: `time_ms` kann sich bei gleicher Revision ändern. Welche Felder die Anzeige benötigt und wann ein vollständiger Snapshot nötig ist, bestimmt der Ausgabe- und Wiederverbindungsvertrag.

Mehr Threads können unabhängige Arbeit verteilen, aber dieselbe Core-Instanz darf weiterhin nicht unkoordiniert parallel verändert werden. Ein langsamer Netzwerkempfänger gehört nicht unter die Core-Sperre. Caching braucht eine klare Ungültigkeitsregel; veralteter Zustand ist kein akzeptabler Preis für einen scheinbar schnellen Response.

## J. Lasttests und Abbruchgrenzen

Einen Lasttest mit festem Quellstand, Build, Hardware, Sitzungszahl, Ereignismix und Laufzeit beschreiben. Nicht nur erfolgreiche Requests zählen: Ablehnungen, Timeouts, abgebrochene Clients und verlorene Updates können sonst eine scheinbar bessere Latenz erzeugen.

Langsame Empfänger, Verbindungsabbrüche, kurze Lastspitzen und länger anhaltende Überlast gesondert prüfen. Auch beobachten, ob Speicher und Rückstau nach Lastende wieder auf ein plausibles Niveau zurückkehren. Ein lang laufender Test kann Lebensdauerfehler zeigen, die ein kurzer Durchsatztest nicht findet.

Normale Verhaltenstests nicht mit einer extrem engen, maschinenabhängigen Zeitgrenze instabil machen. Performancevergleiche in einer kontrollierten Umgebung und mit angemessenen Toleranzen durchführen. Ein einzelner CI-Ausreisser braucht Untersuchung statt automatisch eine Algorithmusänderung.

## K. Miniübung

1. Der Core beansprucht 2 von insgesamt 200 ms. Wie viel Gesamtzeit spart eine Halbierung dieses Abschnitts theoretisch?
2. Drei vollständige Snapshots kommen vor dem nächsten Frame an. Wie oft zeichnet `createFrameUpdater`, und welchen verwendet er?
3. Warum darf dieselbe Ersetzungsstrategie nicht pauschal für Eingabeereignisse gelten?
4. Ein Channel hat Kapazität 2 und `FullMode.Wait`. Was liefert der dritte `TryWrite`, wenn noch nichts gelesen wurde?
5. Warum ist die Batchdauer des C++-Programms keine Zusage für die Browserlatenz?

<details>
<summary>Lösung anzeigen</summary>

1. 1 ms, also 0,5 Prozent der ursprünglichen Gesamtzeit, sofern alle anderen Abschnitte unverändert bleiben.
2. Einmal mit dem zuletzt angebotenen Snapshot, solange nicht vorher gestoppt wurde.
3. Ereignisse können nicht ersetzbare fachliche Bedeutung haben, etwa das Ende eines Kontakts.
4. `false`. `TryWrite` wartet nicht; ein wartender Vollmodus ändert das nicht.
5. Queue, native Anbindung, Netzwerk, Ausgabe und Browserdarstellung sind dort nicht enthalten. Auch Last und Zielhardware unterscheiden sich.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Erst Ziel und Messgrenze festlegen, dann den tatsächlichen Engpass optimieren.
- Mittelwerte, Ausreisser, Fehlerquote und Ressourcenverbrauch gemeinsam betrachten.
- Eine Queue braucht Kapazität und Überlastvertrag; sie schafft keine zusätzliche Rechenleistung.
- Ersetzbare Snapshots von nicht ersetzbaren Ereignissen unterscheiden.
- Speicherbedarf hängt von Anzahl und Lebensdauer ab, nicht nur von Feldgrössen.
- Messgerüste, Profiler und Verhaltenstests haben unterschiedliche Aufgaben.
- Jede Optimierung erneut messen und gegen Robins fachliche Regeln testen.

[Zurück zu Kapitel 19](19-logging-und-diagnose.md) · [Inhaltsverzeichnis](README.md)
