# 14 – Events, Callbacks und Nachrichten

## Lernziel

Du kannst eine Funktion als Callback weitergeben, Ereignisse abonnieren und wieder abmelden. Du unterscheidest lokale Benachrichtigungen von Nachrichten zwischen Browser, Backend und Core. Besonders wichtig: Wer hält wen am Leben, wann wird aufgerufen und wer behandelt Fehler?

Wir betrachten zunächst lokale Kommunikation. [Kapitel 15](15-asynchronitaet-und-nebenlaeufigkeit.md) vertieft anschliessend die nebenläufigen Abläufe. C# bleibt der Ausgangspunkt; die C++-Beispiele verwenden **C++17**.

## A. Spickzettel

| Konzept | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Funktion ohne Ergebnis | `Action<T>` | `(value: T) => void` | z. B. `std::function<void(T)>` |
| Funktion mit Ergebnis | `Func<T, TResult>` | `(value: T) => TResult` | z. B. `std::function<R(T)>` |
| Kurze Funktion | `value => ...` | `value => ...` | `[captures](T value) { ... }` |
| Benachrichtigung | `event`, z. B. `EventHandler<T>` | z. B. DOM-`EventTarget` | Bibliothek oder eigener Vertrag |
| Abonnieren | `+= handler` | `addEventListener(...)` | API-spezifisch |
| Abmelden | `-= handler` | `removeEventListener(...)` oder `AbortSignal` | API-spezifisch, oft RAII-Verbindung |
| Async-Callback | `Func<T, Task>` | `(value: T) => Promise<void>` | API-spezifisch; kein eingebautes `await` in C++17 |
| Netzwerk-Nachricht | serialisierte Daten | serialisierte Daten | definierte Daten-/ABI-Grenze |

Ein Callback ist eine Funktion, die anderer Code aufrufen darf. Ein Event ist ein Benachrichtigungsvertrag, häufig mit mehreren Empfängern. Eine Nachricht ist ein Datenpaket; sie ist keine über das Netzwerk übertragene Funktion.

## B. C#: Delegates, Action und Func

Ein Delegate ist ein typisierter Verweis auf aufrufbaren Code. Er kann auch mehrere Aufrufe in einer Aufrufliste enthalten. `Action<T>` steht für einen Aufruf ohne Rückgabewert, `Func<T, TResult>` für einen mit Ergebnis.

```csharp
using System;

public static class Program
{
    private static void ProcessTouch(int strength, Action<string> report)
    {
        if (strength >= 5)
            report("heart_touched");
    }

    public static void Main()
    {
        Func<int, bool> isStrongEnough = strength => strength >= 5;
        Action<string> log = message => Console.WriteLine(message);

        Console.WriteLine(isStrongEnough(8)); // True
        ProcessTouch(8, log);                // heart_touched
    }
}
```

`ProcessTouch` entscheidet hier, ob es den Callback aufruft. Der Aufruf erfolgt **synchron auf demselben Thread**. Weder die Lambda-Schreibweise noch der Delegate startet automatisch Hintergrundarbeit.

Die Funktion muss ihren Vertrag klar machen: Wird der Callback sofort oder später aufgerufen? Einmal oder mehrfach? Darf er fehlschlagen? Wird er gespeichert? Ohne diese Antworten lässt sich seine Lebensdauer nicht sicher beurteilen.

## C. C#-Events: veröffentlichen, abonnieren, abmelden

```csharp
#nullable enable
using System;

public sealed class TouchEventArgs : EventArgs
{
    public string Target { get; }
    public TouchEventArgs(string target) => Target = target;
}

public sealed class TouchSource
{
    public event EventHandler<TouchEventArgs>? Touched;

    public void SimulateTouch()
    {
        Touched?.Invoke(this, new TouchEventArgs("heart"));
    }
}

public static class Program
{
    public static void Main()
    {
        var source = new TouchSource();
        EventHandler<TouchEventArgs> handler =
            (_, e) => Console.WriteLine(e.Target);

        source.Touched += handler;
        try
        {
            source.SimulateTouch(); // heart
        }
        finally
        {
            source.Touched -= handler;
        }

        source.SimulateTouch(); // dieser Handler wird nicht mehr aufgerufen
    }
}
```

Durch `event` können externe Nutzer abonnieren und abmelden, aber den Delegate nicht einfach ersetzen oder das Event selbst auslösen. Der Herausgeber kontrolliert die Veröffentlichung. [Microsoft: Events](https://learn.microsoft.com/en-gb/dotnet/csharp/events-overview)

Bei diesem gewöhnlichen feldbasierten Event erfolgt `Invoke` synchron. Eine unbehandelte Exception eines Handlers unterbricht den Aufruf; nachfolgende Handler werden dann nicht mehr ausgeführt. Der Herausgeber braucht bei Bedarf einen ausdrücklich anderen Fehlervertrag. `?.Invoke` verhindert einen Aufruf ohne Abonnenten, macht aber nicht den gesamten Herausgeber threadsicher.

Abonnements können Empfänger über deren Instanzmethoden oder Closures erreichbar halten. Ein langlebiger Herausgeber kann dadurch eine längst geschlossene Ansicht festhalten. Darum am passenden Lebensende abmelden und bei Lambdas den verwendeten Delegate speichern. Zweimal `+=` kann denselben Handler zweimal registrieren; ein einzelnes `-=` entfernt dann nur eine passende Registrierung. [Microsoft: Abonnieren und Abmelden](https://learn.microsoft.com/en-us/dotnet/csharp/programming-guide/events/how-to-subscribe-to-and-unsubscribe-from-events)

**Achtung, C#-Gehirn:** Zwei separat geschriebene Lambdas sind kein verlässliches Paar zum An- und Abmelden. Bewahre den tatsächlich registrierten Handler auf. Ein GC ersetzt diesen Ressourcenvertrag nicht.

### Was ist mit async-Handlern?

Ein `async`-Handler für ein normales `EventHandler`-Event wird zu `async void`. Der Herausgeber kann seinen Abschluss nicht abwarten und Fehler nach einer asynchronen Unterbrechung nicht über den Eventaufruf beobachten. UI-Eventhandler können diese Form benötigen; dann müssen sie ihre Fehler selbst behandeln.

Für einen gezielt abwartbaren einzelnen Callback ist beispielsweise `Func<TouchEventArgs, Task>` geeigneter. Bei mehreren Empfängern muss die API ausdrücklich festlegen, ob nacheinander oder gemeinsam gewartet wird und was bei Fehlern passiert. Ein Multicast-Delegate mit `Task` als Rückgabe liefert beim normalen Aufruf nur den Task des letzten Handlers, nicht automatisch einen Gesamttask.

## D. TypeScript: Funktionen und DOM-Eventlistener

```typescript
type TouchNotice = { type: "touch"; target: "heart" | "head" };
type TouchHandler = (notice: TouchNotice) => void;

function simulateTouch(handler: TouchHandler): void {
    handler({ type: "touch", target: "heart" });
}

simulateTouch(notice => console.log(notice.target)); // heart
```

Auch das ist zunächst ein synchroner Funktionsaufruf. Für die Oberfläche verwenden wir beispielsweise einen echten Browserlistener:

```typescript
function connectHeartButton(
    button: HTMLButtonElement,
    onTouch: TouchHandler
): () => void {
    const handler = () => onTouch({ type: "touch", target: "heart" });
    button.addEventListener("click", handler);
    return () => button.removeEventListener("click", handler);
}

// Beim Aufbau der Ansicht: const disconnect = connectHeartButton(button, onTouch);
// Beim tatsächlichen Abbau der Ansicht: disconnect();
```

Die Closure bewahrt genau den registrierten Handler auf. Für die Entfernung müssen Eventtyp, Funktionsidentität und `capture`-Einstellung passen. Alternativ können zusammengehörige Listener mit einem `AbortSignal` registriert und durch `controller.abort()` gemeinsam entfernt werden. [MDN: removeEventListener](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/removeEventListener)

Anders als C#-Events registriert `addEventListener` denselben Callback für denselben Typ und dieselbe Capture-Einstellung nicht erneut als zweiten identischen Eintrag. Zwei frisch erzeugte Lambdas sind aber unterschiedliche Funktionen und können doppelte Reaktionen verursachen. Bei Objektmethoden ausserdem auf `this` achten: Eine Pfeilfunktion wie `() => view.onTouch()` erhält den gewünschten Objektbezug; ein neues `bind(...)` bei jedem Aufruf erzeugt wieder eine andere Funktion. [MDN: addEventListener](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener)

DOM-Ereignisse können durch die Baumstruktur laufen: Capture-Phase, Ziel und gegebenenfalls Bubbling. Das hat ein einfacher Funktionscallback nicht automatisch. Ein eigener `EventTarget.dispatchEvent(...)` ruft Listener synchron auf; Browserklicks werden durch den Browser eingeplant. DOM-Listener-Exceptions werden als unbehandelte Fehler gemeldet und verhalten sich nicht wie die oben gezeigte C#-Multicast-Aufrufliste. [MDN: dispatchEvent](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/dispatchEvent)

**Achtung, C#-Gehirn:** DOM-Eventlistener warten nicht auf ein zurückgegebenes Promise. Ein asynchroner Listener braucht daher eine beobachtete Fehlerbehandlung, etwa einen Wrapper, der `sendTouch().catch(reportError)` aufruft. TypeScripts `void`-Callbacktyp allein verbietet nicht jede versehentlich übergebene asynchrone Funktion.

## E. C++17: Lambdas und std::function

Eine Lambda erzeugt ein Objekt mit einem Aufrufoperator. Die Capture-Liste bestimmt, welche Daten darin gespeichert oder ausgeliehen werden:

```cpp
#include <cassert>
#include <functional>

int main()
{
    int energy = 80;
    auto snapshot = [energy] { return energy; }; // Kopie
    auto live = [&energy] { return energy; };    // geliehen

    energy = 25;
    assert(snapshot() == 80);
    assert(live() == 25);

    std::function<int()> stored = snapshot;
    assert(stored() == 80);
}
```

`std::function<int()>` kann unterschiedliche kopierbare aufrufbare Objekte hinter einer einheitlichen Signatur aufbewahren. Es besitzt sein gespeichertes Funktionsobjekt, aber nicht automatisch dessen referenzierte Ziele. Ein leerer `std::function` wirft beim Aufruf `std::bad_function_call`; optionalen Callback vor dem Aufruf prüfen. Speicherung kann dynamischen Speicher benötigen; für feste lokale Aufrufe kann `auto` oder ein Template genügen.

### Lebensdauer: eine Closure kann länger leben als ihr Ursprung

**Fehlerbeispiel – die zurückgegebene Funktion nicht aufrufen:**

```cpp
#include <functional>

std::function<int()> broken()
{
    int local = 80;
    return [&local] { return local; }; // local endet, Referenz bleibt hängen
}
```

Die sichere Wertvariante:

```cpp
#include <cassert>
#include <functional>

std::function<int()> safe()
{
    int local = 80;
    return [local] { return local; };
}

int main()
{
    auto callback = safe();
    assert(callback() == 80);
}
```

Eine Kopie eines Pointers ist weiterhin nur eine Adresse. Insbesondere `[this]` hält das Objekt nicht am Leben; auch `[=]` in einer Memberfunktion kann auf diesen Pointer hinauslaufen. Ein Snapshot als Wert ist oft die einfachere Lösung. Bei bewusst geteilter Lebensdauer kann ein `weak_ptr` im Callback mit `lock()` vorübergehend einen Besitzer anfordern. [Core Guidelines, F.53 und F.54](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-reference-capture)

Eine Lambda mit eingefangenem `unique_ptr` ist normalerweise nicht kopierbar und passt deshalb nicht in C++17-`std::function`. Nicht allein deshalb zu `shared_ptr` wechseln: Zuerst prüfen, ob ein Template oder ein anderer expliziter Ownership-Vertrag besser passt.

## F. Lokale Benachrichtigung oder Nachricht über eine Grenze?

| Lokal im Programm | Zwischen Browser, Backend und Core |
|---|---|
| Empfänger ist aufrufbarer Code | Empfänger verarbeitet definierte Daten |
| Signaturen prüft oft der Compiler | Eingehende Daten benötigen Laufzeitvalidierung |
| Fehler können direkt zum Aufrufer zurücklaufen | Transportfehler und fachliches Ergebnis getrennt behandeln |
| Reihenfolge hängt vom lokalen Aufrufvertrag ab | Verzögerung, Wiederholung und Verbindungsabbruch einplanen |
| Abmeldung entfernt eine Registrierung | Abmeldung macht bereits gesendete Nachrichten nicht rückgängig |

Ein **Befehl** fordert etwas an: „Neustart ausführen“. Ein **Ereignis** berichtet etwas Geschehenes: „Herz wurde berührt“. Ein **Snapshot** beschreibt einen Zustand zu einem bestimmten Zeitpunkt. Die Begriffe helfen beim Entwurf; sie legen noch kein Transportprotokoll fest.

Dieses JSON ist ein **Entwurfsbeispiel**, kein neu festgelegter Pink-Robin-Endpunkt:

```json
{
  "type": "touch",
  "target": "heart"
}
```

TypeScript-Typen oder C#-Klassen machen fremdes JSON nicht automatisch gültig. Das Backend prüft unter anderem erlaubte Werte und Sitzungszugehörigkeit. Zeitwerte für den Core stammen vom vertrauenswürdigen Adapter. Wo Wiederholungen möglich sind, braucht die Verarbeitung einen klaren Vertrag zu Duplikaten; „genau einmal“ entsteht nicht durch das Wort Event.

## G. Pink Robin: von der Berührung zum Snapshot

```text
Button / Sensor
      ↓ lokale Erkennung
Adapter: prüfen, zuordnen, geordnet einreihen
      ↓ Core-Ereignis
Core entscheidet über den Zustand
      ↓ Snapshot als Wert
Ausgabeadapter / Backend
      ↓ Nachricht oder lokale Darstellung
Anzeige
```

Der [aktuelle C++-Core](../core/README.md) hat weder Netzwerklistener noch eingebaute Callback-Registrierung. Seine [API](../core/include/pinkrobin/core.hpp) nimmt `Event` entgegen und liefert `Snapshot` als Wert. Ein Enum namens `Event` ist noch kein C#-ähnliches Abonnementsystem.

Das folgende **Lernbeispiel für einen Ausgabeadapter** verwendet die echte Core-API und einen einzelnen synchronen Callback. Es ergänzt keine neue Funktion im Core:

```cpp
#include "pinkrobin/core.hpp"
#include <cassert>
#include <functional>
#include <utility>

class RobinAdapter {
public:
    using OnSnapshot = std::function<void(pinkrobin::Snapshot)>;

    explicit RobinAdapter(OnSnapshot callback)
        : onSnapshot_(std::move(callback)) {}

    pinkrobin::Result touchHeart(std::uint64_t now)
    {
        const auto result = core_.dispatch(pinkrobin::Event::heart_touched, now);
        if (result != pinkrobin::Result::ok)
            return result;

        const auto snapshot = core_.snapshot();
        const auto callback = onSnapshot_;
        if (callback)
            callback(snapshot);
        return result;
    }

private:
    pinkrobin::Core core_{0};
    OnSnapshot onSnapshot_;
};

int main()
{
    int notifications = 0;
    RobinAdapter adapter{[&notifications](pinkrobin::Snapshot snapshot) {
        assert(snapshot.mood == pinkrobin::Mood::happy);
        ++notifications;
    }};

    const auto result = adapter.touchHeart(100);
    assert(result == pinkrobin::Result::ok);
    assert(notifications == 1);
}
```

Hier lebt der Zähler länger als der Adapter und sämtliche synchronen Aufrufe. Der Callback bekommt einen unabhängigen Snapshot. Sein Fehler würde nach bereits erfolgter Core-Änderung zur Aufruferin weiterlaufen; das Beispiel führt keinen Rollback durch. Wer den Handler ersetzt, speichert, asynchron ausführt oder mehrere Empfänger ergänzt, muss diesen Vertrag erweitern.

Für produktive Adapter aus Kapitel 15 gilt: Core-Zugriffe pro Sitzung serialisieren, den Snapshot kopieren und fremde Callbacks beziehungsweise Netzwerkversand **ausserhalb einer Core-Sperre** ausführen. Ein Callback kann langsam sein oder erneut Adaptercode aufrufen; das ist Reentrancy. Ein Mutex allein löst solche Rückrufe nicht. Eine geordnete Ausgabe-Queue kann die Verarbeitung entkoppeln.

Bei einem Abonnementsystem zusätzlich festlegen, was An- oder Abmelden während einer Benachrichtigung bedeutet. Das Entfernen eines Handlers kann einen bereits laufenden Callback nicht einfach zurückholen. Beim Schliessen der Sitzung deshalb neue Registrierungen und Arbeit stoppen, aktive Arbeit abschliessen und erst dann die Ressourcen freigeben.

## H. Miniübung

1. Registriere im C#-Eventbeispiel denselben Handler zweimal. Wie oft wird er beim ersten `SimulateTouch()` aufgerufen? Was bleibt nach einem einzelnen `-=` übrig?
2. Warum entfernt `removeEventListener("click", () => onTouch())` nicht den zuvor mit einer anderen Lambda registrierten Listener?
3. Gib aus einer C++-Funktion einen Callback zurück, der einen kopierten `int`-Snapshot liefert. Warum darf dieser nach Ende der Funktion noch aufgerufen werden?
4. Entscheide: Ist `heart_touched` ein lokaler Delegate, eine fachliche Ereigniskennung oder automatisch eine Netzwerk-Nachricht?
5. Ein Callback wirft nach erfolgreichem `dispatch`. Darf der Adapter das Herzereignis blind erneut senden?

<details>
<summary>Lösung anzeigen</summary>

1. Zuerst zweimal. Nach dem einmaligen Abmelden bleibt eine passende Registrierung übrig; auch der letzte Aufruf im Beispiel würde dann noch eine Ausgabe erzeugen.
2. Weil es ein anderes Funktionsobjekt ist. Den ursprünglichen Handler oder einen zugehörigen Cleanup-Vertrag aufbewahren.
3. Beispielsweise `return [value] { return value; };`. Die Closure besitzt ihre eigene Kopie statt einer Referenz auf eine zerstörte lokale Variable.
4. Im aktuellen Core ist es eine fachliche Ereigniskennung im Enum. Transport und lokale Callback-Mechanik sind getrennte Fragen.
5. Nein. Die fachliche Änderung ist bereits erfolgt; blindes Wiederholen kann eine zweite Reaktion auslösen. Fehler bei Verarbeitung und Ausgabe getrennt behandeln und Wiederholungen ausdrücklich entwerfen.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Ein Callback startet nicht automatisch einen Thread oder eine asynchrone Operation.
- C#-`event` kontrolliert den Zugriff auf eine Registrierung; ein öffentlicher Delegate allein bietet diesen Schutz nicht.
- An- und Abmeldung brauchen passende Handler und eine klare Lebensdauer.
- DOM-Listener, C#-Events und eigene C++-Callbacks haben unterschiedliche Fehler- und Registrierungsregeln.
- C++-Closures besitzen nicht automatisch die Objekte hinter Referenzen oder Pointern.
- Ein Core-Ereignis, ein lokaler Funktionsaufruf und eine Netzwerk-Nachricht sind verschiedene Ebenen.
- Für Pink Robin: Eingaben prüfen, Core geordnet aufrufen, Snapshots kopieren und Ausgabe separat behandeln.

[← 13 – Module, Projektstruktur und Build-Systeme](13-module-projektstruktur-und-build-systeme.md) · [Inhaltsverzeichnis](README.md) · [15 – Asynchronität und Nebenläufigkeit →](15-asynchronitaet-und-nebenlaeufigkeit.md)
