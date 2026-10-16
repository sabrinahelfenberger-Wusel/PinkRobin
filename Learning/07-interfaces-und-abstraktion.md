# 07 – Interfaces und Abstraktion

## Lernziel

Wir trennen den **Vertrag** einer Komponente von ihrer **Implementierung**. Für Pink Robin ist das wichtig, weil Ereignisse aus dem Angular-Simulator oder von echter Hardware kommen können.

## A. Spickzettel

| Konzept | C# | TypeScript | C++ |
|---|---|---|---|
| Vertrag | `interface` | `interface` | häufig abstrakte Basisklasse |
| Implementierung | `: ITouchSensor` | `implements TouchSensor` (optional für strukturelle Übereinstimmung) | `: public ITouchSensor` |
| Überschreiben | Implementierung der Interface-Methode | passende Methode | `override` |
| Abstrakte Methode | Interface-Methode | Interface-Methodensignatur | `virtual ... = 0` |
| Laufzeit-Typ | Interface ist Teil des .NET-Typsystems | Interface wird beim Kompilieren entfernt | Basisklasse existiert als C++-Typ |

## B. C# – unser Ausgangspunkt

```csharp
public interface ITouchSensor
{
    bool IsTouched();
}

public class SimulatedTouchSensor : ITouchSensor
{
    public bool IsTouched() => true;
}
```

Die Klasse erklärt ausdrücklich, dass sie den Vertrag erfüllt.

## C. TypeScript – strukturelles Typsystem

```typescript
interface TouchSensor {
    isTouched(): boolean;
}

class SimulatedTouchSensor implements TouchSensor {
    isTouched(): boolean {
        return true;
    }
}
```

Auch ein einfaches Objekt kann den Vertrag erfüllen:

```typescript
const sensor: TouchSensor = {
    isTouched: () => true
};
```

**Achtung, C#-Gehirn:** TypeScript prüft hier hauptsächlich die **Struktur**. Eine explizite `implements`-Deklaration ist nicht nötig, wenn die Form passt. Das Interface verschwindet bei der Übersetzung zu JavaScript und validiert keine eingehenden JSON-Daten zur Laufzeit.

## D. C++ – abstrakte Basisklasse

```cpp
class ITouchSensor
{
public:
    virtual ~ITouchSensor() = default;

    virtual bool isTouched() const = 0;
};

class SimulatedTouchSensor : public ITouchSensor
{
public:
    bool isTouched() const override
    {
        return true;
    }
};
```

- `virtual`: erlaubt dynamischen Polymorphismus.
- `= 0`: rein virtuelle Methode; die Basisklasse ist abstrakt.
- `override`: der Compiler prüft, dass tatsächlich überschrieben wird.
- `const` hinter `isTouched()`: die Methode darf den nicht-`mutable` Objektzustand nicht über `this` ändern.
- Virtueller Destruktor: wichtig, wenn abgeleitete Objekte über einen Basisklassenzeiger gelöscht werden.

Beispiel für Verwendung ohne Kopie:

```cpp
bool checkTouch(const ITouchSensor& sensor)
{
    return sensor.isTouched();
}
```

**Achtung, C#-Gehirn:** Nicht jedes Interface-Konzept muss in C++ durch Vererbung umgesetzt werden. Je nach Anforderung sind auch Templates, Concepts oder einfache Datenstrukturen sinnvoll.

## E. Pink Robin – Sensoren von Entscheidungen trennen

Für die geplante Architektur ist eine ereignisbasierte Grenze besonders nützlich:

```text
Angular-Simulator ──┐
                    ├── TouchEvent → ASP.NET Core → C++-Core
Echter Sensor ──────┘
```

Die Quelle erkennt eine Berührung. Der Core entscheidet, was Robin daraus macht.

Beispiel einer Nachricht:

```json
{
  "type": "touch",
  "target": "heart"
}
```

Eine mögliche Antwort des Core:

```json
{
  "type": "expression",
  "value": "happy"
}
```

Diese Nachrichten sind **Entwurfsbeispiele**, noch kein festgelegtes oder implementiertes Protokoll.

Das Interface beschreibt Fähigkeiten innerhalb einer Schicht; das **Event-Protokoll** beschreibt die Kommunikation zwischen den Schichten. Beides hat unterschiedliche Aufgaben.

## F. Miniübung

1. Definiere in TypeScript ein `interface RobinEventSource` mit `getNextEvent(): string | null`. Der Name vermeidet eine Verwechslung mit der eingebauten Browser-API `EventSource` für Server-Sent Events.
2. Implementiere eine Klasse `SimulatedEventSource`, die `"touch"` zurückgibt.
3. Übertrage den Vertrag nach C++ als abstrakte Klasse. Verwende dort `std::optional<std::string>` für den optionalen Rückgabewert.
4. Überlege: Warum ist es sinnvoll, dass der Core nur das Event kennt und nicht den konkreten Sensor?

## Merksätze

- **Abstraktion:** Wir beschreiben, was gebraucht wird, ohne jede Implementierung festzulegen.
- C# verwendet dafür häufig explizite Interfaces.
- TypeScript prüft Interfaces strukturell und entfernt sie zur Laufzeit.
- C++ verwendet für Laufzeit-Polymorphismus häufig abstrakte Basisklassen mit virtuellen Methoden.
- Pink Robins **Event-Grenze** kann Simulator und Hardware entkoppeln, ohne dass der Core jeden Sensortyp kennen muss.

[← 06 – Properties, Zugriff und Kapselung](06-properties-zugriff-und-kapselung.md) · [Inhaltsverzeichnis](README.md) · [08 – Vererbung und Polymorphismus →](08-vererbung-und-polymorphismus.md)
