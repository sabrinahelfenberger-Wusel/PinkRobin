# 15 – Asynchronität und Nebenläufigkeit

## Lernziel

Du unterscheidest asynchrones Warten, Nebenläufigkeit und Parallelität. Du kannst in C# und TypeScript auf Ergebnisse warten, Abbruch anfordern und Fehler behandeln. In C++17 verstehst du Threads, Mutex und die Lebensdauer gemeinsam genutzter Daten.

[Kapitel 11](11-speicher-und-lebensdauer.md) hat Ownership und Lebensdauer erklärt. Jetzt kommt eine neue Frage hinzu: **Was passiert, wenn eine Operation erst später fertig wird oder mehrere Abläufe dieselben Daten verwenden?**

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Späteres Ergebnis | `Task<T>` | `Promise<T>` | z. B. `std::future<T>` |
| Asynchron warten | `await` | `await` | kein Standard-`co_await` in C++17 |
| Unabhängige Ergebnisse sammeln | `Task.WhenAll(...)` | `Promise.all(...)` | z. B. Futures einzeln mit `get()` abholen |
| Hintergrundberechnung | z. B. `Task.Run(...)` | Worker für eigene JS-Berechnungen | z. B. `std::thread`, `std::async` |
| Abbruch anfordern | `CancellationTokenSource.Cancel()` | `AbortController.abort()` bei unterstützten APIs | eigener kooperativer Vertrag |
| Gemeinsame Daten schützen | `lock`, bei asynchronem Zugriff z. B. `SemaphoreSlim` | Abläufe koordinieren; Worker-Kommunikation gesondert | `std::mutex` mit RAII-Lock |
| Arbeit vor Ende abschliessen | Tasks beobachten / abwarten | Promises behandeln / abwarten | Threads `join()`en; Ergebnisse und Fehler abholen |

Ein Task, Promise oder Future beschreibt ein Ergebnis oder einen Ablauf. Daraus folgt nicht automatisch, dass ein eigener Thread dafür existiert.

## B. Drei Begriffe sauber trennen

- **Asynchron:** Die Aufruferin kann weiterarbeiten, während eine Operation noch nicht fertig ist. Ein `await` kann die eigene Methode unterbrechen, ohne den wartenden Thread zu blockieren.
- **Nebenläufig:** Mehrere Abläufe sind während überlappender Zeiträume in Bearbeitung. Sie können sich auf einem Thread abwechseln.
- **Parallel:** Arbeit wird tatsächlich gleichzeitig ausgeführt, etwa auf mehreren CPU-Kernen.

Pink Robin kann auf eine HTTP-Antwort warten und währenddessen die Oberfläche weiter bedienen. Dafür muss nicht ein zweiter Thread deinen Anwendungscode ausführen. Zwei rechenintensive Bildanalysen gleichzeitig auszuführen ist dagegen ein möglicher Fall für Parallelität.

**Achtung, C#-Gehirn:** `async` bedeutet weder „neuer Thread“ noch „automatisch schneller“. Es erleichtert vor allem die Zusammenarbeit mit noch nicht abgeschlossenen Operationen.

## C. C#: Task, async und await

Diese eigenständige Konsolendemo benötigt C# mit `async Main` und eine aktuelle .NET-Laufzeit. Die Verzögerung simuliert eine asynchrone Operation; sie ist kein echter Core-Timer:

```csharp
using System;
using System.Threading;
using System.Threading.Tasks;

public static class Program
{
    private static async Task<string> LoadStateAsync(CancellationToken token)
    {
        await Task.Delay(50, token);
        return "curious";
    }

    public static async Task Main()
    {
        using var cts = new CancellationTokenSource();
        try
        {
            Task<string> pending = LoadStateAsync(cts.Token);
            Console.WriteLine("Oberfläche kann weiterarbeiten");
            string state = await pending;
            Console.WriteLine(state);
        }
        catch (OperationCanceledException) when (cts.IsCancellationRequested)
        {
            Console.WriteLine("Laden abgebrochen");
        }
    }
}
```

Eine `async`-Methode beginnt normalerweise synchron und läuft bis zu einem `await`, dessen Operation noch nicht fertig ist. Ein bereits abgeschlossener Task muss dort keine Unterbrechung auslösen. Wo die Fortsetzung läuft, hängt vom Kontext ab; ASP.NET Core besitzt nicht denselben UI-Kontext wie eine Desktop-Oberfläche. [Microsoft: Asynchrone Programmierung](https://learn.microsoft.com/en-us/dotnet/csharp/asynchronous-programming/)

Für echte I/O die vorhandene asynchrone API verwenden, etwa `HttpClient.GetAsync(..., token)`. Einen synchronen Netzwerkaufruf in `Task.Run` zu verpacken ist kein Ersatz für eine passende I/O-API. `Task.Run` kann für CPU-Arbeit sinnvoll sein; im Server muss zusätzliche Parallelität bewusst begrenzt werden.

### Gemeinsam warten oder nacheinander?

```csharp
// Ausschnitt innerhalb einer async-Methode; beide Funktionen liefern Task<string>.
Task<string> stateTask = LoadStateAsync(token);
Task<string> settingsTask = LoadSettingsAsync(token);
string[] values = await Task.WhenAll(stateTask, settingsTask);
```

Hier werden beide Operationen vor dem Warten auf das Gesamtergebnis aufgerufen. Zwei einzelne Aufrufe mit jeweils sofortigem `await` würden sie nacheinander abarbeiten. `WhenAll` erzeugt nicht selbst neue Threads und bricht Geschwisteroperationen bei einem Fehler nicht automatisch ab. Es wird erst abgeschlossen, wenn alle übergebenen Tasks abgeschlossen sind. [Microsoft: Task.WhenAll](https://learn.microsoft.com/en-us/dotnet/api/system.threading.tasks.task.whenall)

**Achtung, C#-Gehirn:** `.Result` und `.Wait()` blockieren. Das kann UI-Deadlocks begünstigen und im Server Threads unnötig binden. Asynchron möglichst bis zur aufrufenden Grenze weiterreichen. `async void` bleibt hauptsächlich passenden Eventhandlern vorbehalten; normale Methoden geben `Task` zurück, damit Abschluss und Fehler beobachtbar sind.

## D. TypeScript: Promise und Event Loop

```typescript
async function loadRobinState(signal: AbortSignal): Promise<unknown> {
    const response = await fetch("/api/robin/state", { signal });
    if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
    }
    const data: unknown = await response.json();
    return data; // Struktur vor fachlicher Verwendung validieren
}

async function showRobinState(): Promise<void> {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 5000);
    try {
        const state = await loadRobinState(controller.signal);
        console.log(state);
    } catch (error: unknown) {
        if (controller.signal.aborted) {
            console.log("Abbruch angefordert / Zeitlimit erreicht");
        } else {
            console.error("Laden fehlgeschlagen", error);
        }
    } finally {
        clearTimeout(timeout);
    }
}

// Beispielsweise aus einem UI-Eventhandler aufrufen:
// await showRobinState();
```

Dies ist ein Browserbeispiel mit einer angenommenen API-Route, kein Nachweis, dass dieser Endpunkt bereits existiert. `unknown` erinnert daran, dass ein erfolgreiches JSON-Parsing noch keine fachliche Validierung ist.

Innerhalb eines JavaScript-Agenten läuft ein Job bis zum Ende; anderer JavaScript-Code desselben Agenten wird nicht mitten in einem synchronen Ausdruck eingeschoben. `await` erlaubt jedoch eine spätere Fortsetzung, zwischen der andere Jobs oder Microtasks laufen können. Promise-Fortsetzungen verwenden die Microtask-Queue. Lange synchrone Schleifen blockieren weiterhin die Oberfläche. [MDN: JavaScript-Ausführungsmodell](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Execution_model)

TypeScript schafft keine zusätzlichen Threads. Browser können Worker und andere Abläufe parallel ausführen; bei gemeinsamem Speicher gelten zusätzliche Synchronisationsregeln. Für typische UI-Aufgaben arbeiten wir zunächst mit Promises und Nachrichten.

`Promise.all([first, second])` sammelt Ergebnisse in Eingabereihenfolge, lehnt aber bereits ab, sobald eine Eingabe ablehnt. Die anderen Operationen laufen ohne eigenen Abbruchvertrag weiter. Für alle einzelnen Erfolgs- und Fehlerergebnisse gibt es `Promise.allSettled`. Anders als `Task.WhenAll` wartet eine abgelehnte `Promise.all` also nicht auf den Abschluss sämtlicher Eingaben, bevor sie ablehnt. [MDN: Promise.all](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Promise/all)

### Logische Race Condition auch ohne mehrere Threads

Eine Nutzerin öffnet Robin A, kurz danach Robin B. Die Anfrage für B ist schneller; danach trifft die alte Antwort für A ein und überschreibt die Anzeige. Das ist ein Reihenfolgefehler, obwohl die UI nur einen JavaScript-Thread verwendet.

Mögliche Lösung: alte Anfragen abbrechen **und** vor dem Übernehmen prüfen, ob die Antwort noch zur aktuellen Anfrage beziehungsweise Sitzung gehört. Abbruch allein garantiert nicht, dass keine ältere Fortsetzung mehr ausgeführt wird.

## E. Abbruch ist eine Bitte, kein Rückgängigmachen

Ein `CancellationToken` oder `AbortSignal` muss bis zur unterstützenden Operation weitergereicht werden. CPU-Schleifen müssen den Abbruch selbst regelmässig prüfen. Ein Token beendet keinen beliebigen laufenden Code gewaltsam.

Auch ein Zeitlimit kann nur das Warten beenden, während die eigentliche Arbeit weiterläuft, wenn die verwendete API keinen Abbruch unterstützt. Netzwerkabbruch bedeutet zudem nicht, dass der Server eine bereits empfangene Änderung rückgängig macht. [Microsoft: Warten und Operation abbrechen unterscheiden](https://learn.microsoft.com/en-us/dotnet/standard/asynchronous-programming-patterns/cancel-non-cancelable-async-operations)

Für Pink Robin: Beim Schliessen einer Sitzung neue Arbeit stoppen, laufende Zugriffe abschliessen lassen und erst danach die Core-Ressource freigeben. Ein „Abbruch angefordert“ ist noch kein „alles beendet“.

## F. C++17: Threads, Mutex und RAII

Ein `std::thread` führt Arbeit nebenläufig aus. Das folgende vollständige Programm schützt einen gemeinsamen Zähler:

```cpp
#include <cassert>
#include <mutex>
#include <thread>

int main()
{
    int processed = 0;
    std::mutex mutex;

    auto work = [&] {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard<std::mutex> lock{mutex};
            ++processed;
        } // lock-Destruktor gibt Mutex frei
    };

    std::thread worker{work};
    work();
    worker.join();
    assert(processed == 2000);
}
```

Beide Abläufe verwenden denselben Mutex für denselben Zähler. `join()` wartet blockierend auf das Ende des Threads. Die per Referenz erfassten lokalen Variablen leben bis danach weiter. Ohne den Schutz könnten die konkurrierenden Zugriffe auf `processed` eine **Data Race und damit undefiniertes Verhalten** verursachen. Ein einzelnes `++` ist hier keine garantierte atomare Operation. [C++ Core Guidelines: Nebenläufigkeit](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-concurrency)

`std::lock_guard` bindet die Sperre per RAII an den Scope. Ein passendes atomares Integer könnte diesen einzelnen Zähler ebenfalls schützen; Atomics machen aber nicht automatisch ganze Abläufe über mehrere Felder atomar. Sperren kurz halten und keine Netzwerkoperationen oder unbekannten Callbacks unter der Sperre ausführen.

Ein noch `joinable()`-fähiger `std::thread` ruft beim Zerstören `std::terminate` auf, auch wenn seine Threadfunktion schon fertig ist. Deshalb ist in produktivem C++17-Code ein exception-sicherer Joining-Wrapper oder eine passende Aufgabenverwaltung nötig. Das einfache Beispiel führt zwischen Threadstart und `join()` nur die gezeigte Arbeit aus. `detach()` ist kein allgemeiner Ersatz: Dann muss die Lebensdauer aller verwendeten Daten unabhängig gesichert werden. C++20 bietet `std::jthread` mit automatischem Join und kooperativem Stop-Vertrag; das gehört nicht zum C++17-Core.

### Ergebnisse und Fehler mit std::async

```cpp
#include <cassert>
#include <future>
#include <stdexcept>

int main()
{
    auto pending = std::async(std::launch::async, [] {
        return 42;
    });
    const int result = pending.get();
    assert(result == 42);

    auto failing = std::async(std::launch::async, []() -> int {
        throw std::runtime_error{"Analyse fehlgeschlagen"};
    });
    bool caught = false;
    try {
        (void)failing.get();
    } catch (const std::runtime_error&) {
        caught = true;
    }
    assert(caught);
}
```

`future.get()` wartet bei Bedarf **blockierend** und holt das Ergebnis oder wirft die gespeicherte Exception. Bei einem normalen `std::future` lässt sich das Ergebnis nur einmal mit `get()` konsumieren. Ohne explizites `std::launch::async` darf `std::async` auch eine verzögerte Ausführung wählen. Bei Futures aus `std::async` kann sogar das Freigeben des letzten zugehörigen Handles auf den Abschluss warten. Das ist kein direktes Gegenstück zum nicht blockierenden `await`. [C++-Arbeitsentwurf: Futures](https://www.eel.is/c++draft/futures)

Eine Exception, die dagegen unbehandelt aus einer nackten `std::thread`-Funktion austritt, beendet das Programm über `std::terminate`. Fehler müssen dort selbst abgefangen und an die verantwortliche Stelle übermittelt werden. C++17 enthält weder Standard-Coroutinen noch einen allgemeinen Abbruchmechanismus für Futures.

## G. Deadlocks und Reihenfolge

Ein Deadlock entsteht etwa, wenn Ablauf A Sperre X hält und auf Y wartet, während Ablauf B Y hält und auf X wartet. Eine feste Sperrreihenfolge oder passende Mehrfach-Locks helfen; in C++17 kann `std::scoped_lock` mehrere Mutexes gemeinsam sperren. Eine bereits gehaltene nichtrekursive Mutex erneut auf demselben Thread zu sperren ist ebenfalls ein Fehler.

In C# darf innerhalb eines `lock`-Blocks kein `await` stehen. Für asynchrones exklusives Zutrittsrecht kann ein `SemaphoreSlim(1, 1)` mit `WaitAsync(token)` und `Release()` im `finally` dienen. Nur nach erfolgreichem Eintritt freigeben. Das Semaphore erst entsorgen, wenn keine Nutzer oder Wartenden mehr existieren.

Eine Sperre verhindert überlappende Zugriffe, garantiert aber nicht automatisch die fachlich richtige Ereignisreihenfolge. Zwei vor dem Sperren ermittelte Zeitstempel können in umgekehrter Reihenfolge am Core ankommen. Eine serialisierte Queue mit klarer Reihenfolge ist für solche Abläufe oft verständlicher als viele unabhängige Tasks mit Locks.

## H. Pink-Robin-Core: ein geordneter Besitzer pro Sitzung

Der [aktuelle Core](../core/README.md) erzeugt selbst keine Threads, liest keinen Systemtimer und greift nicht auf Netzwerk oder Hardware zu. Der Adapter liefert monotone Zeit und serialisiert sämtliche Zugriffe auf dieselbe Instanz. Das gilt auch für Snapshots. Ein `const`-Aufruf ist kein Freibrief zum gleichzeitigen Lesen während einer Änderung.

Empfohlener **Adapterentwurf**, keine neue bereits eingebaute API:

```text
Sensorereignisse ─┐
HTTP-Anfragen ────┼→ begrenzte Queue pro Sitzung → ein Verarbeiter → Core
Zeitfortschritt ──┘                                  ↓
                                             Snapshot als Wert
                                                    ↓
                                             Ausgabe / Netzwerk
```

Der Verarbeiter legt die Reihenfolge fest und übergibt Zeitwerte gemäss einem konsistenten monotonen Zeitvertrag. Bei Verarbeitung mit aktueller Zeit wird diese erst an der serialisierten Stelle ermittelt. Falls Ereigniszeiten fachlich erhalten bleiben müssen, benötigt der Adapter stattdessen eine ausdrückliche Ordnungs- und Verspätungsregel. Der Core lehnt rückwärts laufende Zeit ab.

Nach dem synchronen Core-Aufruf wird ein Snapshot kopiert. Langsames Senden erfolgt danach ausserhalb des Core-Zugriffs. Die Queue bekommt eine Kapazitätsgrenze und eine Strategie für Überlast; unbegrenztes Einreihen verlagert das Problem nur in den Speicher. Unabhängige Sitzungen können nebenläufig laufen, ohne denselben Robin-Zustand zu teilen.

Beim Beenden:

1. Neue Einträge für diese Sitzung verhindern und den Zeitgeber stoppen.
2. Wartende Arbeit gemäss Vertrag abarbeiten oder verwerfen; laufende Arbeit kontrolliert beenden.
3. Den Abschluss des Verarbeiters abwarten und seine Fehler beobachten.
4. Erst danach den nativen Handle beziehungsweise dessen `SafeHandle` freigeben.

`shared_ptr` schützt nicht automatisch den Inhalt, und ein GC schützt einen bereits freigegebenen nativen Handle nicht vor fachlich falscher Nutzung. Auch ein Browser-Request-Abbruch darf keine Core-Instanz zerstören, während ein anderer Zugriff sie verwendet.

## I. Miniübung

Sage die Ausgabe dieser TypeScript-Funktion voraus und führe sie danach aus:

```typescript
async function demo(): Promise<void> {
    console.log("A");
    const pending = Promise.resolve("C");
    console.log("B");
    console.log(await pending);
    console.log("D");
}

const finished = demo();
console.log("E");
void finished.catch(console.error);
```

1. Warum erscheint `E` vor `C`, obwohl das Promise bereits erfüllt ist?
2. Würde eine lange synchrone Schleife vor dem `await` die Oberfläche blockieren?
3. Entferne gedanklich die Sperre aus dem C++-Zählerbeispiel: Warum ist „manchmal weniger als 2000“ keine vollständige Beschreibung des Fehlers?
4. Eine alte Robin-Antwort überschreibt eine neuere Anzeige. Wie verhinderst du das zusätzlich zum Netzwerkabbruch?

<details>
<summary>Lösung anzeigen</summary>

Die Reihenfolge ist `A, B, E, C, D`. In JavaScript setzt `await` die Funktion auch bei diesem bereits erfüllten Promise später über eine Microtask fort. Der aufrufende synchrone Code kann zuerst `E` ausgeben. Eine lange synchrone Schleife blockiert weiterhin denselben Agenten und damit normalerweise die UI.

In C# kann `await` bei einem bereits abgeschlossenen Task synchron weiterlaufen; diese Detailregel ist also nicht identisch. Ohne Mutex hat das C++-Beispiel eine Data Race und somit undefiniertes Verhalten, keine begrenzte Auswahl bloss ungenauer Zählerwerte. Bei UI-Anfragen eine aktuelle Anfragekennung oder Sitzungsversion speichern und vor jeder Übernahme vergleichen.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Asynchrones Warten, Nebenläufigkeit und Parallelität sind unterschiedliche Eigenschaften.
- `await` ist keine Garantie für einen neuen Thread oder für eine Unterbrechung nach denselben Regeln in jeder Sprache.
- Tasks und Promises brauchen eine verantwortliche Stelle, die Abschluss und Fehler beobachtet.
- Abbruch ist kooperativ und macht bereits erfolgte Änderungen nicht rückgängig.
- Auch ein einzelner UI-Thread kann logische Reihenfolgefehler haben.
- In C++ brauchen gemeinsame veränderliche Daten Synchronisierung und Threads einen abgesicherten Lebenszyklus.
- Pro Pink-Robin-Sitzung Zugriffe ordnen, Snapshots als Werte weitergeben und Ressourcen erst nach Ende aller Zugriffe zerstören.

[← 14 – Events, Callbacks und Nachrichten](14-events-callbacks-und-nachrichten.md) · [Inhaltsverzeichnis](README.md) · [16 – Serialisierung, JSON und API-Grenzen →](16-serialisierung-json-und-api-grenzen.md)
