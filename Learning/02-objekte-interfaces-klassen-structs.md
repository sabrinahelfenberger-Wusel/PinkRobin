# 02 – Objekte, Interfaces, Klassen und Structs

Bisher haben wir einzelne Werte betrachtet. In Pink Robin gehören viele Werte aber logisch zusammen. Ein Zustand kann zum Beispiel Name, Energie und Wachzustand enthalten.

## A. Spickzettel

| Aufgabe | C# | TypeScript | C++ |
|---|---|---|---|
| Datenstruktur | `class` / `record` | `interface` / `type` | `struct` / `class` |
| Objekt erzeugen | `new RobinState(...)` | `const robin = { ... }` | `RobinState robin{...};` |
| Eigenschaft lesen | `robin.Energy` | `robin.energy` | `robin.energy` |
| Verhalten | Methoden | Funktionen/Methoden | Methoden |
| Schnittstelle | `interface` | `interface` | meist abstrakte Klasse / Concepts je nach Zweck |

> **Achtung, C#-Gehirn:** Ein TypeScript-`interface` ist keine Laufzeitstruktur. Nach der Übersetzung zu JavaScript existiert es nicht mehr.

## B. C# als Ausgangspunkt

Ein einfacher Zustand lässt sich zum Beispiel als Record modellieren:

```csharp
public record RobinState(
    string Name,
    double Energy,
    bool Awake
);
```

Verwendung:

```csharp
var robin = new RobinState("Robin", 75.5, true);

Console.WriteLine(robin.Energy);
```

Ein Record eignet sich besonders gut für Daten, die als Zustand oder Nachricht betrachtet werden.

Eine Klasse kann zusätzlich Verhalten und veränderlichen Zustand kapseln:

```csharp
public class Robin
{
    public string Name { get; }
    public double Energy { get; private set; }

    public Robin(string name, double energy)
    {
        Name = name;
        Energy = energy;
    }

    public void Rest()
    {
        Energy = 100;
    }
}
```

## C. TypeScript: Interface beschreibt die Form

```typescript
interface RobinState {
    name: string;
    energy: number;
    awake: boolean;
}
```

Ein passendes Objekt:

```typescript
const robin: RobinState = {
    name: "Robin",
    energy: 75.5,
    awake: true
};
```

Das Interface sagt dem TypeScript-Compiler, welche Form das Objekt haben soll.

Zur Laufzeit bleibt normales JavaScript übrig.

> **Achtung, C#-Gehirn:** Ein TypeScript-Interface erzeugt kein Objekt und kann eingehendes JSON nicht automatisch validieren.

TypeScript besitzt auch Klassen:

```typescript
class Robin {
    constructor(
        public name: string,
        public energy: number
    ) {}

    rest(): void {
        this.energy = 100;
    }
}
```

Für reine API-Daten oder Zustände sind Interfaces beziehungsweise Types häufig ausreichend.

## D. C++: struct und class

Ein einfacher Zustand kann in C++ sehr direkt als `struct` dargestellt werden:

```cpp
#include <string>

struct RobinState
{
    std::string name;
    double energy;
    bool awake;
};
```

Erzeugen:

```cpp
RobinState robin{"Robin", 75.5, true};
```

Lesen:

```cpp
double currentEnergy = robin.energy;
```

Ein wesentlicher Unterschied zu C# ist, dass ein solches Objekt in C++ sehr natürlich **als Wert** existiert.

C++ kennt ebenfalls Klassen:

```cpp
#include <string>

class Robin
{
private:
    double energy;

public:
    std::string name;

    Robin(std::string name, double energy)
        : energy(energy), name(name)
    {
    }

    void rest()
    {
        energy = 100;
    }
};
```

Bei `struct` sind Member standardmäßig `public`, bei `class` standardmäßig `private`. Beide können Daten und Methoden enthalten.

> **Achtung, C#-Gehirn:** C++-`struct` bedeutet nicht einfach „kleiner C#-Struct“. C++-`struct` und `class` sind sehr ähnlich; der wichtigste sprachliche Standardunterschied ist die voreingestellte Sichtbarkeit.

## E. Pink-Robin-Architektur

Für unsere Architektur können Zustände zwischen den Schichten ähnlich aussehen:

```text
Angular / TypeScript
        ↓
ASP.NET Core / C#
        ↓
Robin Core / C++
```

Beispiel eines fachlichen Zustands:

```json
{
  "name": "Robin",
  "energy": 75.5,
  "awake": true
}
```

Jede Schicht kann dafür ihr eigenes Sprachmodell besitzen.

### TypeScript

```typescript
interface RobinState {
    name: string;
    energy: number;
    awake: boolean;
}
```

### C#

```csharp
public record RobinState(
    string Name,
    double Energy,
    bool Awake
);
```

### C++

```cpp
struct RobinState
{
    std::string name;
    double energy;
    bool awake;
};
```

Die Syntax ist verschieden, aber alle drei Modelle beschreiben dieselbe fachliche Information.

## F. Wichtige Designidee

Nicht jede Schicht muss dieselben Klassen teilen.

Das gemeinsame Konzept ist wichtiger als eine technisch identische Implementierung:

```text
RobinState
 ├─ name
 ├─ energy
 └─ awake
```

Später definieren wir für Pink Robin ein klares Protokoll für Events und Actions. Dadurch kann der C++-Core unabhängig davon entscheiden, ob sein Gegenüber Angular, ASP.NET Core, ein Simulator oder echte Hardware ist.

## Übung

Erweitere den Zustand um:

- `happiness`: Zahl zwischen 0 und 1
- `personName`: kann unbekannt sein
- `awake`: immer vorhanden

Modelliere diesen Zustand einmal als TypeScript-`interface` und einmal als C++-`struct`.

## Merksätze

- Ein **Objekt** enthält zusammengehörige Daten und gegebenenfalls Verhalten.
- Ein TypeScript-`interface` beschreibt vor allem die **Form** von Daten für die statische Typprüfung.
- Ein C++-`struct` ist ein vollwertiger benutzerdefinierter Typ und kann auch Methoden besitzen.
- Gleiche fachliche Daten müssen nicht in jeder Programmiersprache technisch gleich implementiert sein.

## Als Nächstes

Funktionen und Methoden: Parameter, Rückgabewerte, optionale Parameter und der Unterschied zwischen Wert- und Referenzübergabe.
