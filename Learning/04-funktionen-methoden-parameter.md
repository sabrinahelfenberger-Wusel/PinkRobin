# 04 – Funktionen, Methoden und Parameter

Funktionen kapseln Verhalten. Für Pink Robin können sie zum Beispiel Energie verändern, eine Reaktion berechnen oder aus einem Ereignis eine Aktion erzeugen.

## A. Spickzettel

| Aufgabe | C# | TypeScript | C++ |
|---|---|---|---|
| Funktion mit Rückgabe | `double Clamp(double value)` | `function clamp(value: number): number` | `double clamp(double value)` |
| Keine Rückgabe | `void Blink()` | `function blink(): void` | `void blink()` |
| Parameter | `double energy` | `energy: number` | `double energy` |
| Optionaler Parameter | `string? name = null` | `name?: string` | z. B. `std::optional<std::string> name = std::nullopt` |
| Wert verändern | `ref double energy` | meist Objekt verändern oder Wert zurückgeben | `double& energy` |
| Nur lesen ohne Kopie | Referenztypen ohnehin per Referenzwert; `in` möglich | Objektreferenzwert | `const T& value` |

## B. Einfache Funktionen

### C#

```csharp
double IncreaseHappiness(double happiness)
{
    return happiness + 0.1;
}
```

### TypeScript

```typescript
function increaseHappiness(happiness: number): number {
    return happiness + 0.1;
}
```

### C++

```cpp
double increaseHappiness(double happiness)
{
    return happiness + 0.1;
}
```

In allen drei Beispielen bekommt die Funktion einen Zahlenwert und liefert einen neuen zurück.

## C. Methoden

Eine Methode gehört zu einem Objekt beziehungsweise Typ.

### C#

```csharp
public class Robin
{
    public double Happiness { get; private set; }

    public void Pet()
    {
        Happiness += 0.1;
    }
}
```

### TypeScript

```typescript
class Robin {
    happiness = 0.5;

    pet(): void {
        this.happiness += 0.1;
    }
}
```

### C++

```cpp
class Robin
{
public:
    double happiness = 0.5;

    void pet()
    {
        happiness += 0.1;
    }
};
```

## D. Übergabe von einfachen Werten

Bei einem einfachen Zahlenwert erzeugt eine normale Parameterübergabe konzeptionell einen eigenen Wert für den Parameter.

### C#

```csharp
void Recharge(double energy)
{
    energy = 100;
}

double energy = 50;
Recharge(energy);
```

Die Variable außerhalb der Funktion bleibt `50`.

### TypeScript

```typescript
function recharge(energy: number): void {
    energy = 100;
}

let energy = 50;
recharge(energy);
```

Auch hier bleibt die äußere Variable `50`.

### C++

```cpp
void recharge(double energy)
{
    energy = 100;
}

double energy = 50;
recharge(energy);
```

Auch in C++ bleibt die äußere Variable `50`.

## E. C# ref und C++ Referenzen

Wenn eine Funktion die ursprüngliche Variable verändern soll, unterscheiden sich C# und C++ syntaktisch deutlich.

### C#

```csharp
void Recharge(ref double energy)
{
    energy = 100;
}

double energy = 50;
Recharge(ref energy);
```

### C++

```cpp
void recharge(double& energy)
{
    energy = 100;
}

double energy = 50;
recharge(energy);
```

Das `&` im Parametertyp bedeutet hier Referenz.

> **Achtung, C#-Gehirn:** Beim C++-Aufruf sieht man nicht, dass `energy` per Referenz übergeben wird. `recharge(energy)` kann die Variable verändern, wenn die Funktionssignatur `double&` verwendet.

## F. C++ const-Referenzen

Große Objekte möchte man oft nicht unnötig kopieren. Soll die Funktion sie nur lesen, ist in C++ ein häufiges Muster:

```cpp
void printState(const RobinState& state)
{
    // state lesen, aber nicht verändern
}
```

`&` verhindert hier die Kopie und `const` verhindert Änderungen über diese Referenz.

Das Muster

```cpp
const T&
```

wird dir in C++ sehr häufig begegnen.

## G. TypeScript und Objekte

TypeScript beziehungsweise JavaScript übergibt auch Parameter als Werte. Bei Objekten ist dieser Wert jedoch eine Referenz auf das Objekt.

```typescript
interface RobinState {
    energy: number;
}

function recharge(state: RobinState): void {
    state.energy = 100;
}

const robin = { energy: 50 };
recharge(robin);
```

Danach ist `robin.energy` gleich `100`.

Die Funktion hat ihre lokale Kopie der Objektreferenz erhalten; beide Referenzen zeigen aber auf dasselbe Objekt.

> **Achtung, C#-Gehirn:** „Objekte werden per Referenz übergeben“ ist als Kurzform ungenau. In TypeScript/JavaScript wird der Referenzwert selbst als Wert übergeben.

## H. Pink-Robin-Beispiel: lieber Ergebnis zurückgeben

Für den Robin-Core können kleine, gut nachvollziehbare Funktionen oft angenehmer sein, wenn sie einen Wert berechnen und zurückgeben:

### TypeScript

```typescript
function pet(happiness: number): number {
    return Math.min(happiness + 0.1, 1);
}

let happiness = 0.6;
happiness = pet(happiness);
```

### C++

```cpp
#include <algorithm>

double pet(double happiness)
{
    return std::min(happiness + 0.1, 1.0);
}
```

Solche Funktionen sind leicht zu testen, weil Eingabe und Ausgabe klar sichtbar sind.

## I. Events und Actions als Vorschau

Später soll Angular nicht selbst entscheiden, dass Robin nach dem Streicheln glücklich aussieht.

Stattdessen:

```text
Angular
   │
   │ PetEvent
   ▼
ASP.NET Core
   │
   ▼
C++ Core
   │
   │ entscheidet
   ▼
ExpressionAction("happy")
```

Im Core könnte daraus später sinngemäß eine Funktion werden:

```cpp
Action handleEvent(const Event& event);
```

Damit kommen Funktionen direkt in unserer echten Pink-Robin-Architektur vor.

## Übung

Schreibe eine Funktion `feed`, die Robins Energie um `20` erhöht, aber niemals über `100`.

### TypeScript

Start:

```typescript
function feed(energy: number): number {
    // ...
}
```

### C++

Start:

```cpp
double feed(double energy)
{
    // ...
}
```

Teste gedanklich:

- `feed(50)` → `70`
- `feed(90)` → `100`
- `feed(100)` → `100`

## Merksätze

- Normale Parameterübergabe verändert eine übergebene Zahlenvariable nicht automatisch.
- C# kennzeichnet veränderbare Referenzparameter mit `ref`.
- C++ verwendet dafür Referenzen wie `T&`.
- `const T&` ist in C++ ein wichtiges Muster zum Lesen eines Objekts ohne unnötige Kopie.
- TypeScript-Objekte können innerhalb einer Funktion verändert werden, weil der kopierte Referenzwert weiterhin auf dasselbe Objekt zeigt.
- Funktionen mit klarer Eingabe und Rückgabe sind häufig besonders leicht zu testen.

[← 03 – Bedingungen und Schleifen](03-bedingungen-und-schleifen.md) · [Inhaltsverzeichnis](README.md) · [05 – Objekte, Interfaces, Klassen und Structs →](05-objekte-interfaces-klassen-structs.md)
