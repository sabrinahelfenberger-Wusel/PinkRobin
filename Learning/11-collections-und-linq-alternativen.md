# 11 – Collections und LINQ-Alternativen

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

Hier taucht unser Muster aus Kapitel 5 wieder auf:

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

Bei einer C#-Liste führt ein ungültiger Index zu einer Exception. Ein JavaScript-Array liefert für eine nicht vorhandene gewöhnliche Indexeigenschaft normalerweise `undefined`; die TypeScript-Option `noUncheckedIndexedAccess` macht dieses Risiko auch bei vielen Indexzugriffen im Typ sichtbar.

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
using System.Collections.Generic;

var interests = new List<Interest>
{
    new("Musik", 0.9),
    new("Tanzen", 0.8)
};

public record Interest(string Name, double Score);
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

## H. Collections abfragen: LINQ und Alternativen

Robin soll Interessen mit einem Score von mindestens `0.8` auswählen, absteigend nach Score ordnen und nur die Namen ausgeben. Das ist dieselbe Aufgabe in allen drei Sprachen, aber nicht dieselbe Ausführungsstrategie.

| Aufgabe | C# LINQ to Objects | TypeScript-Array | C++17-Algorithmen |
|---|---|---|---|
| Filtern | `Where` | `filter` | `copy_if` oder Schleife |
| Abbilden | `Select` | `map` | `transform` oder Schleife |
| Ordnen | `OrderBy` / `OrderByDescending` | `sort` mit Comparator | `sort` / `stable_sort` |
| Mindestens ein Treffer | `Any` | `some` | `any_of` |
| Alle erfüllen Bedingung | `All` | `every` | `all_of` |
| Ersten Treffer suchen | `FirstOrDefault` | `find` | `find_if`, Ergebnis gegen `end()` prüfen |
| Summe / Reduktion | `Sum` / `Aggregate` | `reduce` | `accumulate` |
| Gruppieren | `GroupBy` | eigene Map-/Gruppierungslogik | eigene Map-/Gruppierungslogik |

Bei leerer Sequenz liefern die üblichen `All`-/`every`-/`all_of`-Prüfungen `true`. Ein erster Treffer kann fehlen; den jeweiligen Default-, Undefined- oder End-Iteratorvertrag beachten. `FirstOrDefault<int>` liefert bei einer leeren Folge standardmässig `0`, was nicht automatisch von einem tatsächlich gefundenen Nullwert unterscheidbar ist.

### C#: Abfrage formulieren, bewusst materialisieren

Das folgende Top-Level-Programm verwendet LINQ to Objects:

```csharp
using System;
using System.Linq;

var interests = new[]
{
    new Interest("Tanzen", 0.8),
    new Interest("Musik", 0.9),
    new Interest("Filme", 0.6)
};

var names = interests
    .Where(interest => interest.Score >= 0.8)
    .OrderByDescending(interest => interest.Score)
    .Select(interest => interest.Name)
    .ToArray();

Console.WriteLine(string.Join(", ", names)); // Musik, Tanzen

public sealed record Interest(string Name, double Score);
```

Viele LINQ-Operatoren auf `IEnumerable<T>` führen die eigentliche Abfrage erst beim Durchlaufen aus (**deferred execution**). `ToArray` materialisiert das Ergebnis jetzt. Eine erneute Enumeration einer nicht materialisierten Abfrage kann die Quelle erneut lesen und erneut rechnen. `OrderBy` benötigt für seine Sortierung die ganze Eingabe; deferred bedeutet nicht automatisch streaming oder ohne Zwischenspeicher. [Microsoft: LINQ-Abfragen](https://learn.microsoft.com/dotnet/csharp/linq/get-started/introduction-to-linq-queries)

Materialisierung kopiert nicht automatisch einen gesamten verschachtelten Objektgraphen. LINQ über `IQueryable<T>` kann ausserdem zu einem externen Provider übersetzt werden; dessen erlaubte Operationen und Ausführung sind nicht pauschal dieselben wie LINQ to Objects.

### TypeScript: Arrayoperationen sind hier unmittelbar

```typescript
type Interest = { name: string; score: number };

const interests: Interest[] = [
    { name: "Tanzen", score: 0.8 },
    { name: "Musik", score: 0.9 },
    { name: "Filme", score: 0.6 }
];

const names = interests
    .filter(interest => interest.score >= 0.8)
    .sort((a, b) => b.score - a.score)
    .map(interest => interest.name);

console.log(names.join(", ")); // Musik, Tanzen
```

Die gezeigten Array-`filter`-/`map`-Aufrufe erzeugen unmittelbar neue Arrays. `sort` verändert das Array, auf dem es aufgerufen wird. Hier ist das die neue gefilterte Folge; `interests.sort(...)` würde dagegen die ursprüngliche Reihenfolge verändern. Die enthaltenen Objekte bleiben bei `filter` zunächst geteilt, nicht tief kopiert.

Ohne Comparator sortiert JavaScript nicht allgemein numerisch nach Score. Einen fachlichen Comparator angeben und bei externen Daten vorher ungültige Zahlen ausschliessen. Diese konkrete Pipeline ist keine verzögerte LINQ-Abfrage.

### C++17: Bereiche, Ausgabeziele und Speicher sichtbar machen

```cpp
#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

struct Interest {
    std::string name;
    double score;
};

int main()
{
    const std::vector<Interest> interests{
        {"Tanzen", 0.8}, {"Musik", 0.9}, {"Filme", 0.6}
    };
    std::vector<Interest> selected;
    std::copy_if(interests.begin(), interests.end(), std::back_inserter(selected),
        [](const Interest& value) { return value.score >= 0.8; });
    std::stable_sort(selected.begin(), selected.end(),
        [](const Interest& a, const Interest& b) { return a.score > b.score; });

    std::vector<std::string> names;
    std::transform(selected.begin(), selected.end(), std::back_inserter(names),
        [](const Interest& value) { return value.name; });

    for (const auto& name : names)
        std::cout << name << '\n';
    return names == std::vector<std::string>{"Musik", "Tanzen"} ? 0 : 1;
}
```

Algorithmen brauchen gültige Iteratorbereiche und ein geeignetes Ausgabeziel. `back_inserter` fügt Elemente hinzu; ein Iterator auf ein leeres Ziel allein schafft keinen Speicher für Ausgaben. `stable_sort` erhält hier die bisherige Reihenfolge gleich bewerteter Einträge. Ein Comparator mit `>=` statt `>` wäre keine gültige strikte Ordnung.

Das Beispiel kopiert bewusst Daten und zeigt alle Zwischenergebnisse. Eine einzige Schleife kann für kleine feste Aufgaben einfacher oder sparsamer sein. C++20-Ranges und Views bieten weitere Möglichkeiten, gehören aber nicht zur benötigten C++17-Grundlage.

**Achtung, C#-Gehirn:** LINQ, JavaScript-Arraymethoden und C++-Algorithmen können dieselbe Frage beantworten, unterscheiden sich aber bei Ausführungszeitpunkt, Mutation, Allokationen und Lebensdauer. Wähle zuerst eine korrekte verständliche Lösung; optimiere nach Messung.

### Miniübung zu Abfragen

Erweitere die Eingabe um `Robotik` mit Score `0.85`. Erwartete Namensfolge: `Musik, Robotik, Tanzen`. Prüfe zusätzlich, ob irgendein Score mindestens `0.95` erreicht, und ob alle Scores zwischen `0` und `1` liegen. Für eine leere Eingabe lautet die erste Antwort `false`, die zweite bei den genannten All-Operatoren `true`.


## I. Welche Collection wann?

Als erste Faustregel:

- **Sequenz von Dingen:** C# `List<T>`, TypeScript Array, C++ `std::vector<T>`
- **Zuordnung Schlüssel → Wert:** C# `Dictionary<K,V>`, TypeScript `Map<K,V>`, C++ `std::unordered_map<K,V>`
- **Unveränderliche Sicht in TypeScript:** `readonly T[]`
- **Geordnete Schlüssel in C++:** `std::map<K,V>`

Die genaue Wahl hängt später von Zugriffsmustern, Lebensdauer und Performance-Anforderungen ab.

## J. Übung

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

[← 10 – Generics und Templates](10-generics-und-templates.md) · [Inhaltsverzeichnis](README.md) · [12 – Speicher und Lebensdauer →](12-speicher-und-lebensdauer.md)
