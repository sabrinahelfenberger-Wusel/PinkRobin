# 01 – Variablen und Typen

## A. Spickzettel

| Aufgabe | C# | TypeScript | C++ |
|---|---|---|---|
| Ganzzahl | `int age = 1;` | `let age: number = 1;` | `int age = 1;` |
| Dezimalzahl | `double mood = 0.8;` | `let mood: number = 0.8;` | `double mood = 0.8;` |
| Boolean | `bool happy = true;` | `let happy: boolean = true;` | `bool happy = true;` |
| Text | `string name = "Robin";` | `let name: string = "Robin";` | `std::string name = "Robin";` |
| Typ ableiten | `var mood = 0.8;` | `let mood = 0.8;` | `auto mood = 0.8;` |
| Konstante | `const int max = 100;` | `const max = 100;` | `const int max = 100;` |

**Wichtig:** C# und C++ unterscheiden verschiedene numerische Typen wie `int` und `double`. TypeScript verwendet für normale JavaScript-Zahlen den Typ `number`.

## B. Richtig verstehen

### Typinferenz

Alle drei Sprachen können den Typ aus einem Initialwert ableiten.

#### C#

```csharp
var happiness = 0.8;
```

Der Compiler leitet `double` ab. Der statische Typ bleibt danach bestehen.

#### TypeScript

```typescript
let happiness = 0.8;
```

TypeScript leitet `number` ab. TypeScript-Typinformationen dienen hauptsächlich der statischen Prüfung und sind im erzeugten JavaScript größtenteils nicht mehr vorhanden.

#### C++

```cpp
auto happiness = 0.8;
```

Der Compiler leitet `double` ab. Auch hier bleibt der Typ fest.

> **Achtung, C#-Gehirn:** `var`, `let` und `auto` sehen ähnlich aus, haben aber nicht dieselbe Bedeutung.

## Pink-Robin-Beispiel

Wir wollen einen einfachen Zustand darstellen.

### C#

```csharp
string name = "Robin";
double happiness = 0.8;
bool awake = true;
```

### TypeScript

```typescript
let robinName: string = "Robin";
let happiness: number = 0.8;
let awake: boolean = true;
```

### C++

```cpp
#include <string>

std::string name = "Robin";
double happiness = 0.8;
bool awake = true;
```

Die Syntax ist ähnlich. Die zugrunde liegenden Typ- und Laufzeitmodelle unterscheiden sich jedoch teilweise deutlich. Diese Unterschiede werden in den folgenden Kapiteln schrittweise wichtig.

## Übung

Robin soll folgende Werte bekommen:

- Name: Robin
- Alter: 1
- Energie: 75.5
- Wach: true

Schreibe diese vier Variablen einmal in **TypeScript** und einmal in **C++**.

## Als Nächstes

`null`, `undefined`, Nullable Types und optionale Werte.
