# 06 – Generics und Templates

## Lernziel

Wiederverwendbare, typsichere Funktionen und Datenstrukturen verstehen. C# dient als Ausgangspunkt; TypeScript und C++ werden daran verglichen.

## A. Spickzettel

| Konzept | C# | TypeScript | C++ |
|---|---|---|---|
| Generischer Typ | `List<T>` | `Array<T>` | `std::vector<T>` |
| Generische Funktion | `T Echo<T>(T value)` | `function echo<T>(value: T): T` | `template<typename T> T echo(T value)` |
| Einschränkung | `where T : ...` | `T extends ...` | `requires` / Concepts (C++20) |
| Laufzeit | Generics im .NET-Typsystem | Typen werden entfernt | Templates werden beim Kompilieren instanziiert |

## B. C# – unser Ausgangspunkt

```csharp
using System;

public static class Program
{
    public static T Echo<T>(T value)
    {
        return value;
    }

    public static void Main()
    {
        string name = Echo("Robin");
        int age = Echo(1);
        Console.WriteLine($"{name}: {age}");
    }
}
```

C#-Generics sind Teil des .NET-Typsystems; generische Typinformationen stehen auch zur Laufzeit zur Verfügung.

Ein weiteres Beispiel:

```csharp
public record RobinEvent<T>(string Type, T Payload);
```

## C. TypeScript – Generics

```typescript
function echo<T>(value: T): T {
    return value;
}

const robinName = echo("Robin");
const age = echo(1);
```

TypeScript leitet `T` aus dem Aufruf ab. Die Typinformationen werden bei der Übersetzung zu JavaScript entfernt.

Eine Funktion, die den letzten Eintrag einer Liste zurückgibt:

```typescript
function lastOrUndefined<T>(items: T[]): T | undefined {
    return items.length > 0
        ? items[items.length - 1]
        : undefined;
}
```

Das Ergebnis kann `undefined` sein, wenn das Array leer ist.

## D. C++ – Templates

```cpp
template<typename T>
T echo(T value)
{
    return value;
}
```

Verwendung:

```cpp
auto name = echo(std::string{"Robin"});
auto age = echo(1);
```

Für jede benötigte Kombination von Template-Argumenten kann der Compiler eine passende Instanziierung erzeugen.

Ein optionaler letzter Eintrag:

```cpp
#include <optional>
#include <vector>

template<typename T>
std::optional<T> lastOrEmpty(const std::vector<T>& items)
{
    if (items.empty())
        return std::nullopt;

    return items.back();
}
```

**Achtung, C#-Gehirn:** Ein C++-Template ist nicht nur ein anders geschriebenes Generic. Templates sind ein Mechanismus zur Compile-Time-Programmierung. Sie können auch Nicht-Typ-Parameter und Spezialisierungen besitzen.

## E. Einschränkungen

Generische Typen sollen manchmal bestimmte Fähigkeiten mitbringen.

### C#

Dieser Ausschnitt gehört in eine Klasse und setzt ein Interface `IAwakeable` mit der Methode `void Wake()` voraus:

```csharp
public static void Wake<T>(T robin) where T : IAwakeable
{
    robin.Wake();
}
```

### TypeScript

```typescript
interface Awakeable {
    wake(): void;
}

function wake<T extends Awakeable>(robin: T): void {
    robin.wake();
}
```

### C++20

```cpp
template<typename T>
concept Awakeable = requires(T robin) {
    robin.wake();
};

template<Awakeable T>
void wake(T& robin)
{
    robin.wake();
}
```

Hier beschreibt ein Concept Anforderungen an den Typ zur Kompilierzeit. Es ist nicht dasselbe wie eine abstrakte Basisklasse mit virtuellen Methoden.

## F. Pink Robin – generische Event-Daten

### TypeScript

```typescript
interface RobinEvent<T> {
    type: string;
    payload: T;
}

interface TouchData {
    target: string;
    durationMs: number;
}

const event: RobinEvent<TouchData> = {
    type: "touch",
    payload: {
        target: "heart",
        durationMs: 500
    }
};
```

### C#

```csharp
public record RobinEvent<T>(string Type, T Payload);
```

### C++

```cpp
#include <string>

template<typename T>
struct RobinEvent
{
    std::string type;
    T payload;
};
```

Die drei Modelle drücken eine ähnliche fachliche Idee aus. Für die Übertragung zwischen Angular, ASP.NET Core und C++ brauchen wir aber zusätzlich ein gemeinsames Nachrichtenformat und Laufzeitvalidierung.

**Achtung, C#-Gehirn:** TypeScript-`RobinEvent<TouchData>` prüft nicht automatisch, ob empfangenes JSON wirklich die richtige Struktur hat. Eine Typannotation ersetzt keine Validierung an der API-Grenze.

## G. Übung

Robin soll Ereignisse mit unterschiedlichen Nutzdaten verarbeiten:

- `TouchData`: `target` und `durationMs`
- `SoundData`: `volume` und `direction`

1. Erstelle in TypeScript zwei `RobinEvent<T>`-Werte.
2. Modelliere `TouchData` und `RobinEvent<T>` in C++.
3. Überlege, welche Informationen zur Laufzeit geprüft werden müssen, wenn die Daten über eine API eintreffen.

## Merksätze

- **Generics/Templates** erlauben wiederverwendbaren Code mit konkreten Typen.
- TypeScript-Generics dienen der statischen Prüfung und verschwinden zur Laufzeit.
- C#-Generics sind im .NET-Typsystem repräsentiert.
- C++-Templates werden beim Kompilieren instanziiert und unterstützen umfangreiche Compile-Time-Programmierung.
- Typisierte Event-Modelle sind noch kein vollständiges Kommunikationsprotokoll.

## Als Nächstes

Fehlerbehandlung: Exceptions, `try/catch`, Result-Typen und C++-Alternativen.
