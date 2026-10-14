# 04 – Collections: Listen, Arrays, Vektoren und Maps

Pink Robin muss oft mehrere Werte speichern: Interessen, bekannte Personen, Ereignisse oder verfügbare Aktionen. Dafür verwenden die drei Sprachen Collections.

## A. Spickzettel

| Aufgabe | C# | TypeScript | C++ |
|---|---|---|---|
| Dynamische Liste | `List<string>` | `string[]` / `Array<string>` | `std::vector<std::string>` |
| Element hinzufügen | `.Add(value)` | `.push(value)` | `.push_back(value)` |
| Anzahl | `.Count` | `.length` | `.size()` |
| Element per Index | `items[0]` | `items[0]` | `items[0]` |
| Schleife | `foreach` | `for...of` | range-based `for` |
| Key/Value | `Dictionary<K,V>` | `Map<K,V>` | `std::unordered_map<K,V>` / `std::map<K,V>` |

## B. C# als Ausgangspunkt

```csharp
var interests = new List<string>
{
    "Musik",
    "Tanzen",
    "Robotik"
};

interests.Add("Filme");

foreach (string interest in interests)
{
    Console.WriteLine(interest);
}
```

Die Liste wächst dynamisch.

## C. TypeScript Arrays

```typescript
const interests: string[] = [
    "Musik",
    "Tanzen",
    "Robotik"
];

interests.push("Filme");

for (const interest of interests) {
    console.log(interest);
}
```

Alternativ kann der Typ so geschrieben werden:

```typescript
const interests: Array<string> = [];
```

Beides beschreibt ein Array von Strings.

> **Achtung, C#-Gehirn:** TypeScript-`const` macht das Array nicht unveränderlich. Die Variable darf nicht auf ein anderes Array gesetzt werden, aber `push()` darf den Inhalt weiterhin verändern.

Soll der Compiler Änderungen am Array verhindern, kann ein readonly Array verwendet werden:

```typescript
const interests: readonly string[] = [
    "Musik",
    "Tanzen"
];
```

## D. C++ std::vector

Die typische dynamische Sequenz in modernem C++ ist `std::vector<T>`:

```cpp
#include <string>
#include <vector>

std::vector<std::string> interests{
    "Musik",
    "Tanzen",
    "Robotik"
};

interests.push_back("Filme");
```

Iteration:

```cpp
for (const std::string& interest : interests)
{
    // interest verwenden
}
```

Hier taucht unser Muster aus Kapitel 3 wieder auf:

```cpp
const std::string&
```

Das Element wird nicht unnötig kopiert und darf über diese Referenz nicht verändert werden.

Mit Typinferenz:

```cpp
for (const auto& interest : interests)
{
    // ...
}
```

## E. Zugriff per Index

In allen drei Sprachen sieht der normale Indexzugriff ähnlich aus:

```text
interests[0]
```

Aber die Sicherheitsmodelle unterscheiden sich.

In C++ prüft `vector[index]` den Bereich nicht zwingend. `vector.at(index)` führt eine Bereichsprüfung durch:

```cpp
std::string first = interests.at(0);
```

> **Achtung, C#-Gehirn:** Verlasse dich in C++ bei `operator[]` nicht auf eine automatische Exception bei ungültigem Index. Ein Zugriff außerhalb des gültigen Bereichs führt zu undefiniertem Verhalten.

## F. Key/Value-Daten

Robin könnte für Interessen nicht nur Namen, sondern auch Scores speichern.

### C#

```csharp
var interests = new Dictionary<string, double>
{
    ["Musik"] = 0.9,
    ["Tanzen"] = 0.8
};
```

### TypeScript

```typescript
const interests = new Map<string, number>([
    ["Musik", 0.9],
    ["Tanzen", 0.8]
]);
```

### C++

```cpp
#include <string>
#include <unordered_map>

std::unordered_map<std::string, double> interests{
    {"Musik", 0.9},
    {"Tanzen", 0.8}
};
```

`std::unordered_map` ist hash-basiert. `std::map` hält seine Schlüssel dagegen geordnet.

## G. Pink-Robin-Beispiel: Interessen

Ein erster einfacher Interessen-Zustand könnte so aussehen.

### TypeScript

```typescript
interface Interest {
    name: string;
    score: number;
}

const interests: Interest[] = [
    { name: "Musik", score: 0.9 },
    { name: "Tanzen", score: 0.8 }
];
```

### C#

```csharp
public record Interest(string Name, double Score);

var interests = new List<Interest>
{
    new("Musik", 0.9),
    new("Tanzen", 0.8)
};
```

### C++

```cpp
struct Interest
{
    std::string name;
    double score;
};

std::vector<Interest> interests{
    {"Musik", 0.9},
    {"Tanzen", 0.8}
};
```

Damit nähern wir uns bereits Robins echtem Interessenmodell.

## H. Welche Collection wann?

Als erste Faustregel:

- **Sequenz von Dingen:** C# `List<T>`, TypeScript Array, C++ `std::vector<T>`
- **Zuordnung Schlüssel → Wert:** C# `Dictionary<K,V>`, TypeScript `Map<K,V>`, C++ `std::unordered_map<K,V>`
- **Unveränderliche Sicht in TypeScript:** `readonly T[]`
- **Geordnete Schlüssel in C++:** `std::map<K,V>`

Die genaue Wahl hängt später von Zugriffsmustern, Lebensdauer und Performance-Anforderungen ab.

## I. Übung

Robin hat drei Interessen:

- Musik: 0.9
- Tanzen: 0.8
- Filme: 0.6

Modelliere sie zuerst als TypeScript-Array von `Interest`-Objekten und danach als C++-`std::vector<Interest>`.

Füge anschließend `Robotik` mit Score `0.7` hinzu.

Überlege außerdem: Warum ist eine Liste von `Interest`-Objekten für Robin möglicherweise nützlicher als nur eine Liste von Strings?

## Merksätze

- `List<T>`, TypeScript-Arrays und `std::vector<T>` erfüllen oft eine ähnliche fachliche Rolle.
- Ähnliche Aufgaben bedeuten nicht identische Laufzeitsemantik.
- TypeScript-`const` macht den Inhalt eines Arrays nicht automatisch unveränderlich.
- In C++ ist `const auto&` beim lesenden Durchlaufen von Objekten ein häufiges Muster.
- C++-`vector[index]` bietet nicht dieselbe Bereichssicherheit, die ein C#-Entwickler möglicherweise erwartet.
- Für Robins Interessen brauchen wir langfristig eher strukturierte Objekte als bloße Strings.

## Als Nächstes

Abstraktion und Verträge: Interfaces in C# und TypeScript sowie abstrakte Basisklassen und andere Möglichkeiten in C++.
