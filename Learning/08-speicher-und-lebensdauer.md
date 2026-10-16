# 08 – Speicher und Lebensdauer

## Lernziel

Du kannst erklären, ob eine Zuweisung einen Wert kopiert oder dasselbe Objekt zugänglich macht, wer eine Ressource besitzt und wie lange eine Referenz gültig bleibt. C# ist unser Anker; TypeScript läuft mit dem Speichermodell von JavaScript, und die C++-Beispiele verwenden **C++17**, passend zum Pink-Robin-Core.

Nach [Kapitel 7](07-fehlerbehandlung.md) interessiert uns besonders: Wer räumt auf, wenn eine Funktion endet oder eine Exception auftritt?

## A. Spickzettel

| Frage | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Speicher automatisch zurückgewinnen? | Garbage Collector (GC) | GC der JavaScript-Engine | Kein allgemeiner GC für normale C++-Objekte |
| Zuweisung eines Objekts | Bei `class`: Referenz kopieren; bei `struct`: Wert kopieren | Objektidentität bleibt bei Zuweisung geteilt | `T b = a`: kopieren, sofern der Typ kopierbar ist |
| Objekt kurz ausleihen | Referenztyp-Parameter; `ref` hat eine andere Bedeutung | Objektwert übergeben | `T&` / `const T&` |
| Alleiniger Besitzer | Konvention / API-Design | Konvention / API-Design | Wert oder `std::unique_ptr<T>` |
| Geteiltes Ownership | GC-Erreichbarkeit, kein Besitzerzähler | GC-Erreichbarkeit, kein Besitzerzähler | `std::shared_ptr<T>` |
| Zeitnah Ressourcen freigeben | `using` / `IDisposable` | Explizites Cleanup, oft `try/finally` | RAII / Destruktor |
| Ownership übertragen | API-Konvention | API-Konvention; spezielle Transfer-APIs separat | z. B. `std::move(uniqueOwner)` |

**Drei Fragen getrennt stellen:** Was wird kopiert? Wo liegt der Speicher? Wann endet die Lebensdauer? Keine dieser Fragen beantwortet automatisch die anderen.

## B. C# als Ausgangspunkt: Werte und Referenzen

```csharp
public sealed class RobinState
{
    public int Energy { get; set; }
}

public struct SensorValue
{
    public int Strength;
}

public static class Program
{
    public static void Main()
    {
        var first = new RobinState { Energy = 80 };
        var second = first;
        second.Energy = 20; // first.Energy ist ebenfalls 20
        var a = new SensorValue { Strength = 80 };
        var b = a;
        b.Strength = 20;    // a.Strength bleibt 80
    }
}
```

Bei der Klasse kopiert `second = first` die Referenz. Beide Variablen erreichen dasselbe Objekt. Beim Struct wird der Wert kopiert. Enthält ein Struct selbst ein Referenzfeld, wird dabei dessen Referenz kopiert, kein vollständiger Objektbaum. Das ist eine **flache Kopie**. Auch `record` ist ohne `struct` ein Referenztyp; wertbasierte Gleichheit allein bedeutet keine Werttyp-Semantik. [C#-Werttypen](https://learn.microsoft.com/en-us/dotnet/csharp/language-reference/builtin-types/value-types)

Normale Parameter werden in C# als Wert übergeben: Bei einem Klassenobjekt ist dieser Wert eine Referenz. Ein Methodenparameter kann das Objekt verändern; seine eigene Neuzuweisung ersetzt aber nicht die Variable der Aufruferin. Dafür wäre beispielsweise `ref` nötig.

Der GC gewinnt Speicher von nicht mehr erreichbaren verwalteten Objekten zurück. Das geschieht nicht garantiert beim Verlassen eines Blocks. Ein noch registrierter Eventhandler kann ein Objekt weiter erreichbar halten; GC verhindert deshalb nicht jede Form unerwünschter Speicherbindung. [GC-Grundlagen](https://learn.microsoft.com/en-us/dotnet/standard/garbage-collection/fundamentals)

Dateien oder native Handles brauchen einen eigenen Ressourcenvertrag:

```csharp
using System.IO;

public static class RobinLog
{
    public static void Write()
    {
        using var writer = new StreamWriter("robin.log");
        writer.WriteLine("Robin ist wach");
    } // Dispose wird auch beim normalen Exception-Unwinding aufgerufen
}
```

`Dispose` gibt Ressourcen frei; das verwaltete Objekt wird dadurch nicht zwangsläufig sofort aus dem Speicher entfernt. Eine C#-Finalizer-Syntax wie `~RobinState()` ähnelt optisch einem C++-Destruktor, hat aber keine deterministische Scope-Lebensdauer. Für native Handles ist `SafeHandle` das passende .NET-Werkzeug.

## C. TypeScript: JavaScript verwaltet den Speicher

```typescript
const first = { energy: 80, sensor: { strength: 5 } };
const second = first;
second.energy = 20;
console.log(first.energy); // 20

const copy = { ...first };
copy.energy = 50;
copy.sensor.strength = 9;
console.log(first.energy);          // 20: eigenes äusseres Objekt
console.log(first.sensor.strength); // 9: verschachteltes Objekt geteilt
```

Zuweisung und Parameterübergabe kopieren Werte. Bei Objekten führt dieser Wert zum selben Objekt; bei primitiven Werten wie `number` gibt es keine veränderbare gemeinsame Objektidentität. Spread erzeugt eine flache Kopie. `const` verhindert eine Neuzuweisung der Variablen, nicht das Verändern des Objekts; TypeScript-`readonly` ist eine Typprüfung, kein allgemeines Einfrieren zur Laufzeit.

TypeScript fügt keinen eigenen GC hinzu. Der GC der Engine kümmert sich um unerreichbare Objekte. Ein aktiver Timer kann über seine Callback-Closure weiterhin Daten festhalten. Timer, Listener und Verbindungen brauchen explizites Cleanup. Auf `FinalizationRegistry` lässt sich kein zeitkritisches Aufräumen stützen. [JavaScript-Speicherverwaltung](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Memory_management)

```typescript
function startRobinPolling(poll: () => void): () => void {
    const timer = setInterval(poll, 1000);
    return () => clearInterval(timer);
}

const stop = startRobinPolling(() => console.log("Zustand laden"));
try {
    // Während dieser Lebensphase wird gepollt.
} finally {
    stop();
}
```

In einer Oberfläche gehört der Aufruf von `stop()` an deren tatsächliches Lebensende, etwa in den Cleanup-Hook der Komponente. Das Beispiel beendet den Timer unmittelbar und dient nur der Cleanup-Struktur.

## D. Stack und Heap: hilfreiche Begriffe, keine Typregel

**Stack** bezeichnet typischerweise Speicher für Funktionsaufrufe und lokale Daten. **Heap** bezeichnet typischerweise dynamisch verwalteten Speicher. Compiler und Laufzeiten dürfen die konkrete Ablage optimieren; Sprachsemantik ist keine Landkarte physischer Adressen.

| Beispiel | Aussage, auf die du dich im Code stützen kannst |
|---|---|
| C#-Struct als Feld eines Klassenobjekts | Sein Wert ist Bestandteil des enthaltenden Objekts; ein Struct liegt nicht immer auf dem Stack |
| C#-Struct wird geboxt | Es entsteht ein verwaltetes Objekt mit einer Kopie des Werts |
| JavaScript-Objekt | Seine Identität und Erreichbarkeit zählen; TypeScript legt keinen Speicherort fest |
| C++ `State state;` in einem Block | Automatische Speicherdauer; Destruktion beim Verlassen des Blocks |
| C++ `std::vector<State> states;` lokal | Der Vector selbst hat automatische Speicherdauer; sein Elementpuffer wird dynamisch verwaltet |
| C++ `static State state;` | Statische Speicherdauer statt normaler Block-Lebensdauer |

C++ unterscheidet automatische, statische, threadlokale und dynamische **Speicherdauer**. **Lebensdauer** beschreibt, wann ein Objekt in diesem Speicher tatsächlich existiert. Ein Speicherbereich kann schon reserviert sein, bevor darin ein Objekt konstruiert wird. Für unseren Alltagscode übernehmen Container und RAII diese Details.

**Achtung, C#-Gehirn:** Weder „Werttyp = Stack“ noch „Klasse = Heap“ ist eine verlässliche sprachübergreifende Regel. In C++ unterscheiden sich `class` und `struct` hauptsächlich durch voreingestellte Zugriffsrechte und Vererbungszugriffe, nicht durch Wert- oder Referenzsemantik.

## E. C++: Wert, Referenz und Pointer

Das folgende Beispiel ist ein vollständiges C++17-Programm:

```cpp
#include <cassert>

struct State {
    int energy = 80;
};

void recharge(State& state) { state.energy = 100; }
int readEnergy(const State& state) { return state.energy; }

int main()
{
    State first;
    State second = first; // eigener Wert
    second.energy = 20;
    assert(first.energy == 80);

    State& alias = first; // weiterer Name für first, kein neuer State
    recharge(alias);
    assert(readEnergy(first) == 100);

    State* observer = &first; // nicht besitzender Pointer
    observer->energy = 60;
    assert(first.energy == 60);
}
```

Eine Referenz muss bei ihrer Initialisierung gebunden werden und kann danach nicht auf ein anderes Objekt umgebunden werden. `alias = second` würde `second` in das referenzierte Objekt schreiben. Eine gültige Referenz bezeichnet ein Objekt; es gibt keinen normalen Nullzustand wie bei einem Pointer. Trotzdem kann sie durch eine zu kurze Objektlebensdauer ungültig werden.

`const State&` verhindert Änderungen durch diesen Zugriff, macht das Objekt aber weder unabhängig noch global unveränderlich. Referenzen und rohe Pointer verlängern die Lebensdauer ihres Ziels grundsätzlich nicht. Sonderregeln für die Bindung temporärer Objekte sind kein allgemeines Ownership-Modell.

## F. RAII und Destruktoren: Aufräumen gehört zum Objekt

**RAII** steht für *Resource Acquisition Is Initialization*. Ein Objekt übernimmt eine Ressource, und sein Destruktor gibt sie beim Lebensende frei. Das gilt für Speicher, Dateien und Locks. Dieses Muster ist eine zentrale Empfehlung der [C++ Core Guidelines, R.1](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii).

```cpp
#include <fstream>
#include <stdexcept>

void writeRobinLog()
{
    std::ofstream log{"robin.log"};
    if (!log)
        throw std::runtime_error{"Log konnte nicht geöffnet werden"};
    log << "Robin ist wach\n";
} // ~basic_ofstream schliesst die Datei
```

Automatische lokale Objekte werden in umgekehrter Konstruktionsreihenfolge zerstört. Bei normalem Exception-Unwinding werden bereits vollständig konstruierte lokale Objekte ebenfalls aufgeräumt. Harte Prozessabbrüche bieten diese Garantie nicht. Fehler beim Schreiben oder Schliessen müssen bei wichtigen Daten zusätzlich geprüft werden; Destruktoren sollen keine Exceptions nach aussen werfen.

Ein Destruktor heisst beispielsweise `~Connection()`. Meist schreiben wir ihn nicht selbst: `std::string`, `std::vector`, Streams und Smart Pointer erledigen das bereits. **Rule of Zero:** Eigene Klassen aus solchen Mitgliedern zusammensetzen, statt manuell Speicherverwaltung zu programmieren. Ein eigener Destruktor kann ausserdem die implizite Erzeugung von Move-Operationen verhindern.

## G. unique_ptr, shared_ptr und weak_ptr

Werte sind der Ausgangspunkt. Wenn dynamische Lebensdauer nötig ist, macht `std::unique_ptr` einen einzelnen Besitzer sichtbar. Gemeinsames Ownership begründet man bewusst. [Core Guidelines, R.20 und R.21](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-owner)

```cpp
#include <cassert>
#include <memory>
#include <utility>

struct State { int energy = 80; };

int main()
{
    auto owner = std::make_unique<State>();
    State& borrowed = *owner;

    // auto duplicate = owner; // Compilerfehler: nicht kopierbar
    auto nextOwner = std::move(owner);
    assert(!owner);
    borrowed.energy = 90; // gültig: das State-Objekt lebt weiterhin
    assert(nextOwner->energy == 90);

    nextOwner.reset(); // State wird hier zerstört
    // borrowed.energy = 10; // FEHLER: dangling reference, nicht ausführen

    auto first = std::make_shared<State>();
    auto second = first; // weiterer Besitzer desselben State
    std::weak_ptr<State> observer = first; // besitzt nicht mit
    first.reset();
    assert(!observer.expired()); // second hält State am Leben
    second.reset(); // letzter starker Besitzer weg: State zerstört
    assert(observer.expired());
    if (auto temporaryOwner = observer.lock()) {
        temporaryOwner->energy = 50; // nur bei erfolgreichem lock()
    }
}
```

`shared_ptr` zählt starke Besitzer und ist kein tracing GC. Ein Zyklus aus gegenseitigen `shared_ptr` kann Objekte dauerhaft halten; eine passende Rückbeziehung wird dann beispielsweise `weak_ptr`. Der Kontrollblock kann noch existieren, nachdem das verwaltete Objekt zerstört wurde. Referenzzählung kostet Aufwand und schützt den Inhalt nicht automatisch vor gleichzeitigen Zugriffen.

**Achtung, C#-Gehirn:** Eine C++-Referenz ist keine GC-Wurzel. Auch eine Referenz auf den Inhalt eines Smart Pointers bleibt nur gültig, solange das Ziel lebt. Einen geliehenen Pointer darfst du nicht nochmals unabhängig in einen besitzenden Smart Pointer einpacken: zwei Besitzerverwaltungen können denselben Speicher doppelt freigeben.

## H. Kopieren und Move-Semantik

```cpp
#include <cassert>
#include <string>
#include <utility>
#include <vector>

struct EventBatch {
    std::vector<std::string> events;
};

int main()
{
    EventBatch original{{"heart_touched", "head_started"}};
    EventBatch copy = original;
    copy.events[0] = "restart";
    assert(original.events[0] == "heart_touched");

    EventBatch moved = std::move(original);
    assert(moved.events[0] == "heart_touched");
    original.events.clear(); // gültiger Aufruf nach dem Move
}
```

Hier kopiert der Vector seine Strings; `copy` ist unabhängig. Bei Mitgliedern wie `shared_ptr` würde eine Kopie hingegen Ownership teilen. Was eine Kopie bedeutet, bestimmt der Typ.

`std::move` ist eine Typumwandlung, die Move-Operationen ermöglicht; sie verschiebt selbst nichts. Wenn passend, kann der empfangende Konstruktor Ressourcen übernehmen. Der Quellgegenstand existiert weiter. Für verschobene Standardbibliothekstypen gilt normalerweise „gültig, aber nicht näher bestimmter Zustand“; benutze nur Operationen, deren Voraussetzungen erfüllt sind. Bei `unique_ptr` ist die verschobene Quelle garantiert leer. Eigene Typen brauchen einen klaren Vertrag.

Ein Move kann fehlen oder bei `const` nicht passen, sodass kopiert wird. Und eine Rückgabe als Wert kann durch Copy Elision ganz ohne Kopie oder Move entstehen: `return result;` ist meist die richtige Schreibweise, ohne vorschnelles `std::move(result)`.

## I. Dangling references erkennen

**Fehlerbeispiel – nicht ausführen:**

```cpp
struct State { int energy = 80; };

const State& broken()
{
    State local;
    return local; // local endet beim Verlassen der Funktion
}
```

Ein späterer Zugriff über diese Referenz hat **undefiniertes Verhalten**. Das bedeutet weder einen sicheren Absturz noch ein verlässliches Ergebnis. Die einfache Lösung gibt einen Wert zurück:

```cpp
struct State { int energy = 80; };

State safe()
{
    return State{};
}
```

Weitere Fallen: Eine Vector-Reallokation kann Referenzen, Pointer und Iteratoren auf seine Elemente ungültig machen. Auch `std::string_view` besitzt seinen Text nicht. Für später ausgeführte Callbacks ist `[&local]` gefährlich, wenn `local` vorher endet. Für wartende Ereignisse lieber benötigte Werte kopieren oder Ownership ausdrücklich übertragen. Sanitizer helfen beim Finden solcher Fehler, ersetzen aber keinen Lebensdauervertrag.

## J. Pink-Robin-Core-Praxis

Der [aktuelle Core](../core/README.md) besitzt pro Roboter oder Demo-Sitzung eine Instanz. Sein Verhaltensteil braucht keine dynamischen Allokationen. `snapshot()` liefert bereits einen Wert; die spätere Veränderung des Core verändert diesen Snapshot nicht. Das folgende Beispiel verwendet die echte [C++-API](../core/include/pinkrobin/core.hpp):

```cpp
#include "pinkrobin/core.hpp"
#include <cassert>

int main()
{
    pinkrobin::Core robin{0};
    const auto before = robin.snapshot();
    const auto result = robin.dispatch(pinkrobin::Event::heart_touched, 100);
    assert(result == pinkrobin::Result::ok);
    const auto after = robin.snapshot();
    assert(before.mood == pinkrobin::Mood::curious);
    assert(after.mood == pinkrobin::Mood::happy);
}
```

Die [C-Schnittstelle](../core/include/pinkrobin/c_api.h) erzeugt dagegen einen dynamischen Handle mit `robin_core_create`, den die Aufruferin mit `robin_core_destroy` freigibt. Hier ist ein **Lernbeispiel für einen C++-Adapter**, keine bereits eingebaute neue Core-Funktion:

```cpp
#include "pinkrobin/c_api.h"
#include <memory>
#include <stdexcept>

struct CoreDeleter {
    void operator()(robin_core_handle* handle) const noexcept
    {
        robin_core_destroy(handle);
    }
};
using CoreOwner = std::unique_ptr<robin_core_handle, CoreDeleter>;

void runSession()
{
    CoreOwner core{robin_core_create(0)};
    if (!core)
        throw std::runtime_error{"Core konnte nicht erstellt werden"};

    const auto result = robin_core_advance(core.get(), 4000);
    if (result != ROBIN_OK)
        throw std::runtime_error{"Zeitfortschritt fehlgeschlagen"};
} // auch beim Fehler: genau einmal robin_core_destroy
```

Der eigene Deleter ist entscheidend: `delete` wäre nicht der Vertrag dieser C-API. `.get()` leiht den Handle nur für den Aufruf aus. Der Adapter muss Zugriffe auf eine Instanz serialisieren und erst nach Abschluss aller Zugriffe zerstören. `shared_ptr` allein würde diese Synchronisierung nicht lösen.

Für die geplante C#-Anbindung übernimmt ein `SafeHandle` mit passendem `ReleaseHandle` denselben Ressourcenvertrag; `using` beendet die gewünschte Nutzung kontrolliert. Diese Anbindung ist laut Core-README noch ein nächster Schritt. Es werden keine C++-Objekte oder Smart Pointer direkt über die ABI gereicht. Snapshots werden in Speicher der Aufruferin kopiert; ein zerstörter Handle darf nicht mehr verwendet werden.

## K. Miniübung

Vorhersagen, dann als C++17-Programm ausführen:

```cpp
#include <iostream>
#include <memory>
#include <utility>

struct State { int energy = 80; };

int main()
{
    auto a = std::make_unique<State>();
    State snapshot = *a;
    State& live = *a;
    auto b = std::move(a);
    b->energy = 25;

    std::cout << snapshot.energy << ' ' << live.energy << '\n';
    b.reset();
    // std::cout << live.energy; // Was wäre hier falsch?
}
```

1. Welche zwei Zahlen werden ausgegeben?
2. Ist `a` nach dem Move leer? Lebt das State-Objekt weiter?
3. Warum wird `live` erst nach `b.reset()` ungültig?
4. Wie würdest du einen Zustand für einen späteren UI-Callback aufbewahren?

<details>
<summary>Lösung anzeigen</summary>

Die Ausgabe ist `80 25`. `snapshot` ist ein eigener Wert. `live` bezeichnet weiter das von `b` besessene Objekt. `a` ist nach dem Move leer; das Zielobjekt selbst wurde nicht verschoben. `b.reset()` zerstört es, dadurch wird `live` dangling. Für den Callback einen benötigten Snapshot als Wert erfassen, statt eine Referenz auf eine kurzlebige lokale Variable oder ein möglicherweise zerstörtes Objekt zu behalten.

</details>

## Merksätze – Achtung, C#-Gehirn!

- GC verwaltet erreichbaren Speicher; zeitnahes Ressourcen-Cleanup braucht einen eigenen Vertrag.
- Wert- und Referenzsemantik sagen nichts Verlässliches über Stack oder Heap aus.
- C++-Objekte sind standardmässig Werte; Kopierbarkeit und Kopierverhalten hängen vom Typ ab.
- RAII koppelt Freigabe an Lebensdauer. Standardtypen ersparen meist eigene Destruktoren.
- `unique_ptr` besitzt allein, `shared_ptr` teilt Ownership, Referenzen leihen aus.
- `std::move` ermöglicht eine Übergabe; eine geliehene Referenz hält kein Objekt am Leben.
- Pink Robin braucht klare Besitzer pro Sitzung und unabhängige Snapshots für spätere Verarbeitung.

[Zurück zu Kapitel 7](07-fehlerbehandlung.md) · [Inhaltsverzeichnis](README.md)
