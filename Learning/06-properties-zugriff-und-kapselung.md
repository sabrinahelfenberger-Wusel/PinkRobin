# 06 – Properties, Zugriff und Kapselung

## Lernziel

Du kannst den Zugriff auf Objektzustand begrenzen und erklären, wie C#-Properties, JavaScript-Accessors und C++-Methoden vergleichbare Aufgaben erfüllen. Du unterscheidest Sichtbarkeit, Unveränderlichkeit und eine fachliche Zustandsregel.

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Öffentliches Feld | `public int Energy;` | `energy: number` | `public: int energy;` |
| Nur über API lesen | `public int Energy { get; }` oder eingeschränkter Setter | `get energy()` ohne Setter | `int energy() const` |
| Intern verändern | `private set`, Methoden, privates Feld | `private` / `#field`, Methoden | privates Feld, Methoden |
| Initialisieren | Konstruktor, ggf. `init` | Konstruktor / Feldinitialisierung | Konstruktor / Memberinitialisierung |
| Abgeleitete Klassen | `protected` | `protected` zur Typprüfung | `protected` |
| Assemblygrenze | `internal` | kein direkt gleiches Schlüsselwort | kein direkt gleiches Schlüsselwort |

Ein Getter ist ein lesender Zugriff, ein Setter ein schreibender. Kapselung schützt eine sinnvolle Regel, nicht nur die Schreibweise eines Feldes.

## B. C# als Anker: private Setter und init

Dieses vollständige Beispiel benötigt C# 9 oder neuer:

```csharp
using System;

public sealed class RobinStatus
{
    public string Name { get; init; } = "Robin";
    public int Energy { get; private set; } = 80;

    public bool TrySetEnergy(int value)
    {
        if (value < 0 || value > 100)
            return false;
        Energy = value;
        return true;
    }
}

public static class Program
{
    public static void Main()
    {
        var robin = new RobinStatus { Name = "Pink Robin" };
        Console.WriteLine(robin.Energy);             // 80
        Console.WriteLine(robin.TrySetEnergy(25));   // True
        Console.WriteLine(robin.TrySetEnergy(120));  // False
        Console.WriteLine(robin.Energy);             // weiterhin 25
        // robin.Energy = 120; // ausserhalb der Klasse nicht erlaubt
        // robin.Name = "Anders"; // nach Initialisierung nicht erlaubt
    }
}
```

`Energy` ist eine automatisch implementierte Property. Der Compiler stellt dafür Speicher bereit; die Property ist trotzdem nicht dasselbe Sprachkonstrukt wie ein öffentliches Feld. `private set` erlaubt nur intern das Setzen. Die Methode sichert zusätzlich den Bereich; der private Setter selbst prüft ihn nicht.

`init` erlaubt Zuweisungen während der vorgesehenen Initialisierungsphase, etwa im Objektinitialisierer. Es validiert einen Namen nicht automatisch und friert keine referenzierten Unterobjekte tief ein. `get` ohne öffentliches `set` ist ebenfalls keine pauschale Garantie für ein vollständig unveränderliches Objekt.

Wenn ein Getter berechnet statt speichert, kann dieselbe Propertysyntax trotzdem verwendet werden. Teure I/O, überraschende Seiteneffekte oder lange Operationen hinter einem gewöhnlichen Eigenschaftszugriff machen die API aber schwer verständlich; dafür explizite Methoden bevorzugen.

## C. TypeScript: Compilerzugriff und echte private Felder

```typescript
class RobinStatus {
    readonly name: string;
    #energy = 80;

    constructor(name: string) {
        this.name = name;
    }

    get energy(): number {
        return this.#energy;
    }

    trySetEnergy(value: number): boolean {
        if (!Number.isFinite(value) || value < 0 || value > 100)
            return false;
        this.#energy = value;
        return true;
    }
}

const robin = new RobinStatus("Pink Robin");
console.log(robin.energy);             // 80
console.log(robin.trySetEnergy(25));   // true
console.log(robin.trySetEnergy(120));  // false
console.log(robin.energy);             // 25
```

Das Beispiel verwendet JavaScript-`#`-Privatfelder und lässt sich mit einem passenden Ziel wie ES2022 übersetzen. `private energy` in TypeScript wäre dagegen primär eine Compiler-Zugriffsregel. `#energy` besitzt zusätzlich die private Laufzeitsemantik von JavaScript. Beides ersetzt keine Serverberechtigung.

`readonly name` verhindert eine normale Zuweisung durch typgeprüften TypeScript-Code nach der Initialisierung. Es erzeugt keine allgemeine JavaScript-Laufzeitsperre. Für einen Accessor ohne Setter gelten wiederum eigene JavaScript-Regeln. `readonly`, ein Getter ohne Setter und `Object.freeze` sind deshalb keine austauschbaren Begriffe.

Ein `set energy(value: number)` wäre ebenfalls möglich. Das Try-Muster zeigt hier bewusst, wie ein ungültiger Wert zurückgewiesen wird, ohne den bisherigen Zustand zu verändern. Anders als C#-`int` lässt TypeScript-`number` auch Bruchteile zu; falls ganze Energiepunkte verlangt sind, zusätzlich `Number.isInteger(value)` prüfen. Die fachliche Genauigkeit gehört in den Vertrag.

## D. C++17: gewöhnliche Methoden statt Standard-Properties

```cpp
#include <iostream>

class RobinStatus {
public:
    int energy() const noexcept { return energy_; }

    bool trySetEnergy(int value) noexcept
    {
        if (value < 0 || value > 100)
            return false;
        energy_ = value;
        return true;
    }

private:
    int energy_ = 80;
};

int main()
{
    RobinStatus robin;
    if (!robin.trySetEnergy(25)) return 1;
    if (robin.trySetEnergy(120)) return 1;
    std::cout << robin.energy() << '\n'; // 25
    return robin.energy() == 25 ? 0 : 1;
}
```

Standard-C++17 hat keine direkt entsprechende C#-Propertysyntax. Der Getter ist eine normale Methode. Das hintere `const` verhindert Zustandsänderungen über diesen nicht-`mutable` Zugriff auf das Objekt. `noexcept` verspricht, dass die Funktion keine Exception nach aussen entweichen lässt; es ist keine Bereichsprüfung.

Der Getter liefert hier einen kleinen Wert zurück. Eine Rückgabe als veränderbare Referenz könnte den geschützten Zustand wieder direkt schreibbar machen. Eine const-Referenz vermeidet zwar bestimmte Änderungen über diesen Zugriff, braucht aber weiterhin eine gültige Lebensdauer. Speicher und Ownership vertieft Kapitel 11.

Nicht jedes reine Datenobjekt benötigt triviale Getter und Setter. Ein öffentliches C++-Struct kann für einen bewusst einfachen Snapshot genau richtig sein. Entscheidend ist, ob eine Invariante geschützt werden muss oder ob die Struktur ausdrücklich nur Daten transportiert. [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-struct)

## E. Sichtbarkeit ist ein Entwurfswerkzeug

`public` erlaubt Zugriff über die API. `private` beschränkt direkten Zugriff. `protected` öffnet ihn für Ableitungen; damit werden diese enger an die internen Regeln gebunden. Vererbung behandeln wir im folgenden Themenblock.

C#-`internal` beschreibt die Assemblygrenze, nicht einfach einen Ordner oder Namespace. Ein TypeScript-`export` bestimmt, welcher Modulname zugänglich ist, und ist keine direkte Entsprechung zu `public` oder C#-`internal`. C++-`class` ist standardmässig privat, `struct` öffentlich.

**Achtung, C#-Gehirn:** Private Felder sind kein Schutz vor einem nicht berechtigten Netzwerkaufruf. Zugriffskontrolle im Quellcode und Autorisierung im Backend lösen unterschiedliche Probleme.

## F. Unveränderlich: welche Ebene ist gemeint?

Ein nicht neu zuweisbares Feld kann weiterhin auf ein veränderliches Objekt zeigen. Das gilt beispielsweise für C#-`readonly`-Referenzfelder, TypeScript-`readonly`-Properties und viele weitere konstante Zugriffspfade.

Fragen deshalb getrennt stellen: Darf die Variable neu gebunden werden? Darf das referenzierte Objekt geändert werden? Werden verschachtelte Daten geteilt? Liefert die API einen unabhängigen Wert oder eine geliehene Sicht?

Für Pink Robin soll der Core seinen laufenden Zustand über kontrollierte Ereignisse ändern. Eine externe Methode `SetMood("happy")` würde diese Übergangsregeln umgehen. Ein Snapshot ist dagegen ausdrücklich eine Ausgabe; seine Darstellung darf intern anders organisiert sein als der laufende Zustand.

## G. Miniübung

1. Warum schützt `private set` allein noch nicht den Bereich 0 bis 100 gegen fehlerhaften internen Code?
2. Was unterscheidet TypeScript-`private` und JavaScript-`#field`?
3. Warum liefert das C++-Beispiel die Energie als `int` statt als `int&`?
4. Ergänze das TypeScript-Beispiel um die Regel, dass nur ganze Energiepunkte erlaubt sind.
5. Sollte ein Browser direkt Robins private Stimmung setzen dürfen?

<details>
<summary>Lösung anzeigen</summary>

1. Die Klasse kann den Setter weiterhin mit jedem passenden Integer aufrufen. Alle Schreibpfade müssen die Invariante einhalten.
2. TypeScript-`private` prüft primär statischen Zugriff; `#field` besitzt private JavaScript-Laufzeitsemantik.
3. Eine veränderbare Referenz würde Schreibzugriff auf das private Feld erlauben und die Bereichsprüfung umgehen.
4. Im Ablehnungszweig zusätzlich `!Number.isInteger(value)` prüfen.
5. Nein. Der Browser sendet eine erlaubte Eingabe; der Core entscheidet nach seinen fachlichen Übergangsregeln.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Eine Property ist mehr als eine andere Feldschreibweise; sie kann Verhalten besitzen.
- Sichtbarkeit, Unveränderlichkeit und fachliche Gültigkeit getrennt betrachten.
- C#-`init`, TypeScript-`readonly` und C++-`const` sind nicht gleichbedeutend.
- Nicht jede Datenstruktur braucht Accessors, aber geschützte Invarianten brauchen kontrollierte Schreibpfade.
- API-Zugriff im Code ersetzt keine Berechtigungsprüfung an der Netzwerkgrenze.

[← 05 – Objekte, Interfaces, Klassen und Structs](05-objekte-interfaces-klassen-structs.md) · [Inhaltsverzeichnis](README.md) · [07 – Interfaces und Abstraktion →](07-interfaces-und-abstraktion.md)
