# 01.2 – null, undefined und optionale Werte

Ein Wert ist nicht immer vorhanden. Pink Robin kennt zum Beispiel den Namen einer Person vielleicht noch nicht. C#, TypeScript und C++ modellieren diesen Zustand unterschiedlich.

## A. Spickzettel

| Aufgabe | C# | TypeScript | C++ |
|---|---|---|---|
| Optionaler Text | `string? name = null;` | `let name: string | null = null;` | `std::optional<std::string> name;` |
| Wert vorhanden? | `name is not null` | `name !== null` | `name.has_value()` |
| Wert verwenden | `name` nach Null-Prüfung | `name` nach Null-Prüfung | `name.value()` oder `*name` |
| Ersatzwert | `name ?? "Unbekannt"` | `name ?? "Unbekannt"` | `name.value_or("Unbekannt")` |

Für `std::optional` wird in C++ `#include <optional>` benötigt.

## B. C# als Ausgangspunkt

Mit aktivierten Nullable Reference Types zeigt das `?`, dass eine Referenz auch `null` sein darf:

```csharp
string? personName = null;

if (personName is not null)
{
    Console.WriteLine(personName);
}
```

Bei Werttypen gibt es ebenfalls Nullable Types:

```csharp
int? age = null;
```

Bei `int` ist `null` ohne `?` nicht zulässig. Bei Referenztypen wie `string` erzeugt eine mögliche Nullzuweisung mit aktivierten Nullable Reference Types in der Regel eine Warnung; die Annotation ist keine allgemeine Laufzeitsperre.

## C. TypeScript: null und undefined

TypeScript hat zwei wichtige leere Zustände.

### null

`null` kann ausdrücken, dass bewusst kein Wert vorhanden ist:

```typescript
let personName: string | null = null;

personName = "Sabrina";
```

Der Typ ist eine Union: `personName` darf entweder `string` oder `null` sein.

### undefined

`undefined` bedeutet häufig, dass kein Wert gesetzt wurde oder eine Eigenschaft nicht vorhanden ist:

```typescript
let personName: string | undefined;
```

Eine optionale Property wird mit `?` geschrieben:

```typescript
interface Person {
    name?: string;
}
```

Dann darf `name` vollständig fehlen:

```typescript
const person: Person = {};
```

> **Achtung, C#-Gehirn:** `name?: string` in TypeScript bedeutet nicht dasselbe wie `string? name` in C#. Das TypeScript-`?` an einer Property bedeutet, dass die Property optional ist.

Mit `strictNullChecks` akzeptiert ein normaler `string` nicht automatisch `null` oder `undefined`.

## D. C++: std::optional

C++ verwendet für einen optionalen Wert häufig `std::optional<T>`:

```cpp
#include <optional>
#include <string>

std::optional<std::string> personName;
```

Zunächst enthält `personName` keinen Wert.

Später kann einer gesetzt werden:

```cpp
personName = "Sabrina";
```

Prüfen und verwenden:

```cpp
if (personName.has_value())
{
    std::string name = personName.value();
}
```

Eine kompakte Alternative ist:

```cpp
if (personName)
{
    std::string name = *personName;
}
```

> **Achtung, C#-Gehirn:** Ein Pointer mit `nullptr` kann in C++ ebenfalls „nichts“ darstellen. Für einen Wert, der einfach optional sein soll, ist `std::optional<T>` aber meist die klarere Modellierung. Pointer bringen zusätzlich Referenz-, Lebensdauer- und möglicherweise Besitzsemantik ins Spiel.

## E. Ersatzwerte

Alle drei Sprachen können einen Standardwert liefern, wenn der eigentliche Wert fehlt.

### C#

```csharp
string displayName = personName ?? "Unbekannt";
```

### TypeScript

```typescript
const displayName = personName ?? "Unbekannt";
```

### C++

```cpp
std::string displayName = personName.value_or("Unbekannt");
```

## Pink-Robin-Beispiel

Robin erkennt eine Person, kennt ihren Namen aber zunächst noch nicht.

### C#

```csharp
string? personName = null;

string greeting = personName is not null
    ? $"Hallo {personName}!"
    : "Hallo!";
```

### TypeScript

```typescript
let personName: string | null = null;

const greeting = personName !== null
    ? `Hallo ${personName}!`
    : "Hallo!";
```

### C++

```cpp
std::optional<std::string> personName;

std::string greeting = personName
    ? "Hallo " + *personName + "!"
    : "Hallo!";
```

Das fachliche Konzept ist in allen drei Fällen gleich: Robin kann eine Information kennen oder noch nicht kennen. Die Sprachen modellieren diesen Zustand aber unterschiedlich.

## Übung

Robin hat drei Informationen über eine erkannte Person:

- `name`: möglicherweise noch unbekannt
- `attention`: immer vorhanden, Wert `0.8`
- `nickname`: in TypeScript darf diese Property vollständig fehlen

Schreibe eine kleine Darstellung zuerst in **TypeScript** und danach in **C++**.

Überlege dabei besonders:

1. Welcher Wert braucht in C++ `std::optional`?
2. Was ist in TypeScript der Unterschied zwischen `name: string | null` und `nickname?: string`?
3. Welcher Wert ist garantiert vorhanden?

## Merksatz

**Optional ist eine Information über den Zustand eines Wertes – nicht einfach nur eine andere Schreibweise für einen normalen Wert.**

## Als Nächstes

Aus einzelnen Variablen wird ein zusammengehöriger Robin-Zustand: Objekte, Interfaces, Klassen, Structs und Records.
