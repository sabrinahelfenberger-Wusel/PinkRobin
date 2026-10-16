# 08 – Vererbung und Polymorphismus

## Lernziel

Du kannst eine Basisklasse und eine abgeleitete Klasse in C#, TypeScript und C++ lesen und schreiben. Du erkennst, wann ein Methodenaufruf zur Laufzeit die passende Implementierung auswählt, warum C++-Kopien dabei überraschen können und wann Komposition besser passt.

[Kapitel 7](07-interfaces-und-abstraktion.md) hat Verträge und abstrakte Basisklassen eingeführt. Werte, Referenzen und Ownership werden in [Kapitel 11](11-speicher-und-lebensdauer.md) vertieft. Die späteren Abschnitte hier greifen auf diese Themen vor; die Grundidee der virtuellen Methoden lässt sich zunächst unabhängig davon lesen. Die C++-Beispiele verwenden **C++17**.

## A. Spickzettel

| Konzept | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Klasse ableiten | `class Demo : Sensor` | `class Demo extends Sensor` | `class Demo : public Sensor` |
| Basiskonstruktor | `: base(name)` | `super(name)` | `: Sensor(name)` |
| Überschreibbare Methode | `virtual` oder `abstract` | normale Instanzmethode auf dem Prototyp | `virtual` oder rein virtuell |
| Überschreiben | `override` | `override` zur Compilerprüfung | `override` zur Compilerprüfung |
| Basisimplementierung aufrufen | `base.Read()` | `super.read()` | `Sensor::read()` |
| Abstrakte Klasse / Methode | `abstract` | `abstract` zur Typprüfung | mindestens eine rein virtuelle Methode: `virtual ... = 0` |
| Weitere Ableitung verhindern | `sealed` | kein direktes `sealed`-Schlüsselwort | `final` |
| Mehrere direkte Klassenbasen | nein | nein | ja |
| Mehrere Verträge erfüllen | mehrere Interfaces | mehrere Interfaces | z. B. mehrere abstrakte Basisklassen |

`override` hat nicht in allen Sprachen dieselbe Rolle. In TypeScript und C++ bestätigt es eine Überschreibung, die auch ohne dieses Wort möglich wäre. In C# gehört es zur Deklaration einer tatsächlichen Überschreibung.

## B. Was bedeutet „ist ein“?

Ein `SimulatedTouchSensor` **ist ein** `TouchSensor`: Code, der einen Touchsensor erwartet, soll auch den simulierten Sensor verwenden können. Das ist mehr als gemeinsame Methodenbezeichnungen. Die abgeleitete Klasse muss den zugesagten Vertrag einhalten: Wenn `IsTouched()` einen aktuellen Berührungszustand liefern soll, darf eine Unterklasse nicht plötzlich einen Neustart auslösen.

**Vererbung** stellt die Beziehung her und kann Implementierung wiederverwenden. **Polymorphismus** bedeutet hier: Derselbe Aufruf über einen Basistyp führt abhängig vom konkreten Objekt zu unterschiedlichem Verhalten. Interfaces können ebenfalls Polymorphismus ermöglichen; gemeinsame Implementierung ist dafür nicht erforderlich.

## C. C# als Anker: virtual, override und base

Dieses vollständige Programm demonstriert die Auswahl zur Laufzeit:

```csharp
using System;

public class TouchSensor
{
    public string Name { get; }

    public TouchSensor(string name)
    {
        Name = name;
    }

    public virtual bool IsTouched() => false;
    public virtual string Describe() => $"Sensor: {Name}";
}

public sealed class SimulatedTouchSensor : TouchSensor
{
    private readonly bool touched;

    public SimulatedTouchSensor(string name, bool touched) : base(name)
    {
        this.touched = touched;
    }

    public override bool IsTouched() => touched;
    public override string Describe() => $"{base.Describe()} (Simulation)";
}

public static class Program
{
    public static void Main()
    {
        TouchSensor sensor = new SimulatedTouchSensor("Herz", true);
        Console.WriteLine(sensor.IsTouched()); // True
        Console.WriteLine(sensor.Describe());  // Sensor: Herz (Simulation)
    }
}
```

Der **statische Typ** der Variablen ist `TouchSensor`. Der **Laufzeittyp** des Objekts ist `SimulatedTouchSensor`. Der statische Typ bestimmt, welche Member zugänglich sind. Bei einer virtuellen Methode entscheidet der Laufzeittyp über die gewählte Überschreibung.

`base.Describe()` ruft gezielt die Implementierung der Basis auf. Ohne diesen gezielten Basisaufruf würde ein Aufruf von `Describe()` innerhalb derselben Überschreibung erneut diese Überschreibung erreichen und könnte endlos rekursiv werden.

### Überschreiben ist nicht Verdecken

Dieses zweite Beispiel ist absichtlich anders:

```csharp
using System;

public class Sensor
{
    public virtual string Read() => "Basis";
}

public class DemoSensor : Sensor
{
    public new string Read() => "Demo";
}

public static class Program
{
    public static void Main()
    {
        var demo = new DemoSensor();
        Sensor sensor = demo;
        Console.WriteLine(demo.Read());   // Demo
        Console.WriteLine(sensor.Read()); // Basis
    }
}
```

`new` verdeckt den geerbten Member; es ersetzt nicht dessen virtuellen Slot. Die Auswahl hängt hier vom statischen Zugriffstyp ab. Mit `override` statt `new` würden beide Aufrufe `Demo` liefern. Ohne `new` wäre das Verdecken grundsätzlich weiterhin möglich, aber mit einer Compilerwarnung. [Microsoft: override und new](https://learn.microsoft.com/en-us/dotnet/csharp/programming-guide/classes-and-structs/knowing-when-to-use-override-and-new-keywords)

Eine abstrakte C#-Klasse kann nicht direkt instanziiert werden. Eine abstrakte Methode besitzt keine Implementierung und muss von einer konkreten Unterklasse überschrieben werden. C#-Klassen haben eine direkte Klassenbasis; mehrere Interfaces sind möglich. `protected` macht einen Member in abgeleiteten Klassen zugänglich, `private` beschränkt den Zugriff auf den deklarierenden Typ. Den Zustand der Basis möglichst privat halten und gezielte Operationen anbieten. [Microsoft: Vererbung](https://learn.microsoft.com/en-us/dotnet/csharp/fundamentals/tutorials/inheritance)

## D. TypeScript: extends und super, aber kein virtual

```typescript
class TouchSensor {
    constructor(public readonly name: string) {}

    isTouched(): boolean { return false; }
    describe(): string { return `Sensor: ${this.name}`; }
}

class SimulatedTouchSensor extends TouchSensor {
    constructor(name: string, private readonly touched: boolean) {
        super(name);
    }

    override isTouched(): boolean { return this.touched; }
    override describe(): string {
        return `${super.describe()} (Simulation)`;
    }
}

const sensor: TouchSensor = new SimulatedTouchSensor("Herz", true);
console.log(sensor.isTouched()); // true
console.log(sensor.describe());  // Sensor: Herz (Simulation)
```

Normale Instanzmethoden werden in JavaScript dynamisch über das Objekt und seine Prototypkette gefunden; ein `virtual` ist dafür nicht nötig. `super(name)` initialisiert im abgeleiteten Konstruktor die Basis, bevor auf `this` zugegriffen werden darf. `super.describe()` erreicht die Basismethode.

`override` wird beim Übersetzen entfernt. Es prüft, dass ein entsprechender Member in der Basis existiert. Mit `noImplicitOverride` fordert TypeScript die Kennzeichnung von Überschreibungen geerbter konkreter Member. Auch `abstract` ist eine TypeScript-Prüfung, keine JavaScript-Laufzeitsperre. [TypeScript-Handbuch: Klassen](https://www.typescriptlang.org/docs/handbook/2/classes)

**Achtung, C#-Gehirn:** `implements` stellt einen Vertrag sicher, erbt aber keinen Code. `extends` baut die Vererbungsbeziehung auf. Interfaces verschwinden zur Laufzeit; eine echte JavaScript-Klasse und ihre Prototypkette bleiben erhalten. Methoden als Klassenfelder mit Pfeilfunktionen verhalten sich bei Initialisierung und `super` anders als die hier gezeigten Prototypmethoden.

## E. C++: öffentliche Vererbung und virtuelle Methoden

```cpp
#include <cassert>
#include <memory>
#include <string>
#include <utility>

class TouchSensor {
public:
    explicit TouchSensor(std::string name) : name_(std::move(name)) {}
    virtual ~TouchSensor() = default;

    virtual bool isTouched() const { return false; }
    virtual std::string describe() const { return "Sensor: " + name_; }

private:
    std::string name_;
};

class SimulatedTouchSensor final : public TouchSensor {
public:
    SimulatedTouchSensor(std::string name, bool touched)
        : TouchSensor(std::move(name)), touched_(touched) {}

    bool isTouched() const override { return touched_; }
    std::string describe() const override {
        return TouchSensor::describe() + " (Simulation)";
    }

private:
    bool touched_;
};

bool checkTouch(const TouchSensor& sensor)
{
    return sensor.isTouched();
}

int main()
{
    SimulatedTouchSensor local{"Herz", true};
    assert(checkTouch(local)); // geliehen, keine dynamische Allokation nötig

    std::unique_ptr<TouchSensor> owner =
        std::make_unique<SimulatedTouchSensor>("Kopf", true);
    assert(owner->isTouched());
    assert(owner->describe() == "Sensor: Kopf (Simulation)");
} // virtueller Destruktor: auch abgeleiteten Teil korrekt zerstören
```

`public` bei der Vererbung erlaubt die öffentliche Verwendung als Basistyp. Bei `class Demo : Base` wäre die Vererbung standardmässig **privat**. Das ist keine öffentliche „ist ein“-Schnittstelle.

`virtual` aktiviert die dynamische Auswahl. Eine nichtvirtuelle Methode wird durch eine gleichnamige Methode der Unterklasse nicht nachträglich virtuell. `override` entdeckt versehentliche Signaturabweichungen, beispielsweise ein vergessenes `const`. Ein expliziter Aufruf `TouchSensor::describe()` umgeht die virtuelle Auswahl für diesen Aufruf.

Der öffentliche virtuelle Destruktor ist hier nötig, weil `unique_ptr<TouchSensor>` mit seinem Standard-Deleter ein abgeleitetes Objekt über einen Basispointer löscht. Ohne virtuellen Basisdestruktor wäre das undefiniertes Verhalten. Ein Smart Pointer repariert diese Basisklasse nicht automatisch. [Core Guidelines, C.35](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-dtor-virtual)

Eine rein virtuelle Methode lautet beispielsweise `virtual bool isTouched() const = 0;`. Damit ist die Klasse abstrakt, wie der Vertrag in Kapitel 7. C++ erlaubt mehrere Klassenbasen; mehrdeutige Basen und virtuelle Vererbung brauchen aber zusätzliche Regeln. Für unsere Beispiele genügt eine flache Hierarchie.

## F. Konstruktoren und Destruktoren

In allen drei Sprachen muss die Basiskonstruktion abgeschlossen sein, bevor der Rumpf des abgeleiteten Konstruktors normal weiterarbeitet. Initialisierungsdetails von Feldern unterscheiden sich zwischen den Sprachen.

In C++ gilt konkret: virtuelle Basen zuerst, dann direkte Basen, danach Member in **Deklarationsreihenfolge**, zuletzt der Konstruktorrumpf. Die Reihenfolge der Initialisierungsliste ändert diese Ordnung nicht. Beim Zerstören läuft zuerst der abgeleitete Destruktorrumpf, danach werden Member und Basen in umgekehrter Reihenfolge zerstört.

**Keine überschreibbaren Methoden aus Konstruktoren als Erweiterungspunkt verwenden:**

- C# kann dabei bereits eine abgeleitete Überschreibung aufrufen, bevor deren Konstruktor fertig ist.
- JavaScript/TypeScript kann eine abgeleitete Methode erreichen, bevor die abgeleiteten Instanzfelder initialisiert sind.
- C++ ruft während der Basis-Konstruktion und -Destruktion keine Überschreibung eines weiter abgeleiteten Teils auf. Ein virtueller Aufruf einer rein virtuellen Funktion für das gerade konstruierte oder zerstörte Objekt ist undefiniertes Verhalten.

Darum benötigte Daten über Konstruktorparameter übergeben und Aktionen, die ein fertiges Objekt voraussetzen, erst nach vollständiger Konstruktion starten.

## G. Object-Slicing: die C++-Wertfalle

Dieses vollständige Beispiel zeigt einen definierten, aber oft unerwünschten Effekt:

```cpp
#include <cassert>
#include <string>
#include <vector>

struct Sensor {
    virtual ~Sensor() = default;
    virtual std::string read() const { return "Basis"; }
};

struct DemoSensor : Sensor {
    std::string read() const override { return "Demo"; }
};

std::string byValue(Sensor sensor) { return sensor.read(); }
std::string byReference(const Sensor& sensor) { return sensor.read(); }

int main()
{
    DemoSensor demo;
    Sensor copy = demo; // nur den Basisanteil kopieren
    assert(copy.read() == "Basis");
    assert(byValue(demo) == "Basis");
    assert(byReference(demo) == "Demo");

    std::vector<Sensor> values;
    values.push_back(demo); // ebenfalls Slicing
    assert(values.front().read() == "Basis");
}
```

`copy` ist ein neues vollständiges `Sensor`-Objekt. Der abgeleitete Anteil wird nicht mitkopiert. `virtual` kann den verlorenen Anteil nicht wiederherstellen. Bei einer C#-Klassenreferenz `Sensor sensor = demo` oder einer TypeScript-Zuweisung wird dagegen kein solcher Basiswert erzeugt.

Für kurzzeitige Nutzung: `const Sensor&`. Für eine besitzende Sammlung unterschiedlicher Unterklassen: beispielsweise `std::vector<std::unique_ptr<Sensor>>`. Die Referenz muss gültig bleiben; beim Smart Pointer braucht die Basis einen geeigneten Destruktor. Abstrakte Basen können gar nicht als vollständige Werte erzeugt werden. Für konkrete polymorphe Basen lässt sich öffentliches Kopieren bewusst unterbinden. [Core Guidelines: Slicing vermeiden](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-slice)

## H. Vererbung oder Komposition für Pink Robin?

**„Ist ein“:** Ein simulierter Touchsensor kann einen Touchsensor-Vertrag erfüllen. **„Hat ein“:** Ein Adapter hat einen Core und eine Ereignisquelle. Ein Adapter ist deshalb keine Unterklasse des Core.

Der aktuelle [`pinkrobin::Core`](../core/include/pinkrobin/core.hpp) ist bereits `final`. Seine Ereignisse und Snapshots sind Werte; er benötigt keine Sensor-Klassenhierarchie. Hardware und Simulator liefern Ereignisse über Adapter. Diese bestehende Grenze erhalten wir.

Das folgende C++17-Beispiel ist ein **Entwurf für die Zusammensetzung eines Adapters**, keine bereits vorhandene Repository-API:

```cpp
#include "pinkrobin/core.hpp"
#include <cassert>
#include <cstdint>

struct TouchInput {
    virtual ~TouchInput() = default;
    virtual bool isTouched() const = 0;
};

struct DemoInput final : TouchInput {
    bool isTouched() const override { return true; }
};

class RobinAdapter {
public:
    explicit RobinAdapter(const TouchInput& input) : input_(input), core_{0} {}

    pinkrobin::Result sample(std::uint64_t now)
    {
        const bool touched = input_.isTouched();
        const auto result = (touched && !wasTouched_)
            ? core_.dispatch(pinkrobin::Event::heart_touched, now)
            : core_.advance_to(now);
        if (result == pinkrobin::Result::ok)
            wasTouched_ = touched;
        return result;
    }

    pinkrobin::Snapshot snapshot() const { return core_.snapshot(); }

private:
    const TouchInput& input_; // geliehen: Quelle muss Adapter überleben
    pinkrobin::Core core_;   // eigener Wert, Komposition
    bool wasTouched_ = false;
};

int main()
{
    DemoInput input;
    RobinAdapter adapter{input};
    assert(adapter.sample(100) == pinkrobin::Result::ok);
    assert(adapter.snapshot().mood == pinkrobin::Mood::happy);
}
```

Die Quelle wird polymorph ausgetauscht, der Core wird als Mitglied besessen. Das Beispiel erzeugt ein Herzereignis beim Übergang zu „berührt“, statt bei jedem Poll erneut. Produktive Adapter brauchen darüber hinaus den passenden Vertrag für Entprellung, Zeit und serialisierte Zugriffe. Ein Interface lohnt sich, wenn wir tatsächlich mehrere austauschbare Implementierungen benötigen; für einen festen Fall kann eine einfache konkrete Komponente genügen.

Komposition erleichtert das Kombinieren unabhängiger Fähigkeiten. Vererbung ist sinnvoll, wenn Unterklassen langfristig denselben Vertrag erfüllen und die Basisklasse bewusst für Erweiterung entworfen ist. Gleiche Felder oder ein paar gemeinsame Codezeilen allein begründen noch keine gute Hierarchie.

## I. Miniübung

1. Ersetze im C#-Verdeckungsbeispiel `new` durch `override`. Sage beide Ausgaben voraus.
2. Ergänze in TypeScript einen `HardwareTouchSensor`, dessen Konstruktor eine Funktion `() => boolean` erhält. `isTouched()` soll deren Ergebnis zurückgeben. Verwende ihn über eine Variable vom Typ `TouchSensor`.
3. Erkläre im C++-Slicing-Beispiel, warum `byValue(demo)` und `byReference(demo)` unterschiedliche Ergebnisse haben.
4. Ergänze für den C++-Adapter eine zweite `TouchInput`-Implementierung, die `false` liefert. Ändere den Core nicht. Welche Stimmung erwartest du beim ersten Sample bei `100` ms?

<details>
<summary>Lösung und Hinweise anzeigen</summary>

1. Beide Ausgaben sind `Demo`: Die virtuelle Überschreibung gilt auch beim Zugriff über `Sensor`.
2. Die neue Klasse verwendet `extends TouchSensor`, ruft `super(name)` auf und implementiert `override isTouched(): boolean { return this.readTouch(); }`. Die übergebene Funktion wird beispielsweise als `private readonly readTouch: () => boolean` gespeichert.
3. Der Wertparameter enthält nur einen kopierten Basisanteil. Die Referenz bezeichnet weiterhin das vollständige `DemoSensor`-Objekt und erlaubt den virtuellen Aufruf seiner Überschreibung.
4. Ohne Berührung wird nur die Zeit fortgeschrieben; nach 100 ms bleibt die Stimmung `curious`. Der Core ist eine enthaltene Komponente und benötigt dafür keine neue Unterklasse.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Vererbung verspricht Austauschbarkeit, nicht nur Codewiederverwendung.
- C# und C++ brauchen virtuelle Basismethoden für diese dynamische Auswahl; normale JavaScript-Prototypmethoden werden bereits dynamisch gefunden.
- C#-`new` verdeckt; `override` überschreibt.
- C++-`override` hilft, Signaturfehler zu entdecken; ein virtueller Destruktor ermöglicht sicheres Löschen über die Basis.
- Eine C++-Kopie als Basiswert kann slicen. Referenzen und Pointer erhalten den Zugriff auf das vollständige Objekt, besitzen es aber nicht automatisch.
- Konstruktoren sind kein guter Ort für überschreibbare Erweiterungsaufrufe.
- Pink Robins Adapter **hat einen Core**. Dafür verwenden wir Komposition.

[← 07 – Interfaces und Abstraktion](07-interfaces-und-abstraktion.md) · [Inhaltsverzeichnis](README.md) · [09 – Generics und Templates →](09-generics-und-templates.md)
