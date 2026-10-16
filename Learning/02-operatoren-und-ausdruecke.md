# 02 – Operatoren und Ausdrücke

## Lernziel

Du kannst Werte berechnen, vergleichen und logisch verknüpfen. Du erkennst Unterschiede bei Division, Gleichheit, Kurzschlussauswertung und Zahlenüberlauf in C#, TypeScript/JavaScript und C++17.

Ein **Ausdruck** berechnet einen Wert. Ein **Operator** verknüpft oder verändert Operanden, zum Beispiel `energy + 20`. Viele Zeichen sehen in allen drei Sprachen gleich aus; ihre Regeln sind nicht immer identisch.

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Rechnen | `+ - * / %` | `+ - * / %`, Potenz `**` | `+ - * / %` |
| Zuweisen | `=` | `=` | `=` |
| Wert verändern | `+=`, `-=`, `++`, `--` | ebenso | ebenso |
| Gleichheit | `==`, `!=`, abhängig vom Typ | meist `===`, `!==` | `==`, `!=`, abhängig vom Typ |
| Vergleichen | `< <= > >=` | ebenso | ebenso |
| Logik | `&&`, `||`, `!` | ebenso, aber Operandenwerte als Ergebnis möglich | `&&`, `||`, `!` |
| Auswahl als Ausdruck | `condition ? a : b` | ebenso | ebenso |
| Ersatz bei fehlendem Wert | `value ?? fallback` | `value ?? fallback` | z. B. `optional.value_or(fallback)` |

`^` ist in diesen Sprachen kein allgemeiner Potenzoperator, sondern bei geeigneten Typen bitweises XOR. C# verwendet für Potenzen beispielsweise `Math.Pow`, C++ `std::pow` aus `<cmath>`.

## B. C# als Anker: rechnen und zuweisen

Dieser Block lässt sich als eigenes C#-Top-Level-Programm ausführen:

```csharp
using System;

int energy = 50;
energy += 20;
Console.WriteLine(energy);       // 70
Console.WriteLine(5 / 2);        // 2: ganzzahlige Division
Console.WriteLine(5.0 / 2);      // 2.5, Ausgabeformat abhängig von Kultur
Console.WriteLine(-5 % 2);       // -1: Rest, kein stets positiver Modulo

bool canPlay = energy >= 20 && energy <= 100;
string action = canPlay ? "spielen" : "ruhen";
Console.WriteLine(action);       // spielen
```

Die Typen der Operanden bestimmen die Rechnung. Das Ergebnis von `5 / 2` nachträglich in `double` zu speichern macht die bereits abgeschnittene Nachkommastelle nicht wieder sichtbar. Mindestens einen Operanden vor der Division passend konvertieren.

`energy++` erhöht und liefert als Ausdruck den alten Wert; `++energy` liefert den erhöhten Wert. In einem einfachen eigenen Statement erhöhen beide um eins. Mehrere Veränderungen derselben Variablen nicht in schwer lesbare Ausdrücke zusammendrängen.

`=` weist zu, `==` vergleicht. Eine versehentliche Zuweisung in einer Bedingung kann bei booleschen Werten sogar gültig sein. Deutliche Namen, getrennte Zuweisungen und Compilerwarnungen helfen.

## C. TypeScript: number rechnet anders

```typescript
let energy = 50;
energy += 20;
console.log(energy);             // 70
console.log(5 / 2);              // 2.5
console.log(Math.trunc(5 / 2));   // 2
console.log(-5 % 2);             // -1

const canPlay = energy >= 20 && energy <= 100;
console.log(canPlay ? "spielen" : "ruhen");
```

Normales `number` verwendet Gleitkommazahlen. Es gibt keine automatische Integerdivision, nur weil beide Literale ohne Punkt geschrieben sind. `Math.trunc` schneidet Richtung null ab; `Math.floor` rundet Richtung minus unendlich. Bei negativen Zahlen sind diese Operationen verschieden.

`bigint` hat eigene ganzzahlige Operatorregeln, lässt sich nicht beliebig mit `number` mischen und braucht einen eigenen JSON-Vertrag. Für dieses Kapitel bleiben wir bei `number`.

**Achtung, C#-Gehirn:** JavaScript-`+` kann auch Text verketten. `"5" + 2` ergibt `"52"`. TypeScript und Validierung helfen gegen unbeabsichtigte Mischungen; ein aus JSON gelesener Wert ist nicht automatisch eine Zahl.

## D. C++17: Operandentypen und gültiger Bereich

```cpp
#include <iostream>

int main()
{
    int energy = 50;
    energy += 20;
    std::cout << energy << '\n';             // 70
    std::cout << 5 / 2 << '\n';              // 2
    std::cout << 5.0 / 2 << '\n';            // 2.5
    std::cout << -5 % 2 << '\n';             // -1
    const bool canPlay = energy >= 20 && energy <= 100;
    std::cout << (canPlay ? "spielen" : "ruhen") << '\n';
    return energy == 70 && canPlay ? 0 : 1;
}
```

`%` ist in C++ für ganzzahlige Operanden definiert; für Gleitkommareste gibt es unter anderem `std::fmod`. Eine ganzzahlige Division durch null ist in C++ undefiniertes Verhalten. In C# führt sie zu einer Exception; normales JavaScript-`number` kann bei Division durch null `Infinity` oder `NaN` liefern. In keinem Fall ist das ein fachlich gültiger Sensorwert.

Zahlenbereiche ebenfalls beachten: Bei C# hängt Integerüberlauf vom `checked`-/`unchecked`-Kontext ab. Vorzeichenbehafteter C++-Integerüberlauf ist undefiniertes Verhalten; unsigned-Arithmetik läuft modular um. JavaScript-`number` verliert bei grossen Ganzzahlen Präzision. Nicht auf Überlauf als automatische Begrenzung auf 0 bis 100 vertrauen. [C#-Arithmetik](https://learn.microsoft.com/dotnet/csharp/language-reference/operators/arithmetic-operators)

## E. Gleichheit: Inhalt oder Identität?

| Fall | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Zahlen | `==`, unter Beachtung von Gleitkomma | `===` vermeidet Typumwandlung | `==`, unter Beachtung von Typ und Gleitkomma |
| Strings | `string ==` vergleicht Inhalt | `===` vergleicht Stringwert | `std::string ==` vergleicht Inhalt |
| Normale Objekte | Klassen oft Referenzgleichheit; Typ kann Operator überschreiben | `===` vergleicht Objektidentität | eigener Typ braucht geeigneten Operator; Structs haben in C++17 nicht automatisch `==` |
| Records | wertbasierte Gleichheit nach Record-Regeln | kein direkt gleichbedeutendes Record-Laufzeitmodell | eigenen Vergleich definieren |

In JavaScript erzeugt `{ energy: 80 } === { energy: 80 }` keine Inhaltsgleichheit: Es sind zwei verschiedene Objekte. Bei C++-Zeigern vergleicht `==` die Zeigerwerte, nicht automatisch die Inhalte. Insbesondere rohe C-String-Pointer nicht mit `std::string` verwechseln.

Gleitkommazahlen sind nicht beliebig exakt: `0.1 + 0.2` muss nicht exakt `0.3` ergeben. Eine Toleranz muss zur fachlichen Einheit und Genauigkeit passen; für exakte Integerzustände keine unnötige Toleranz einführen. `NaN` ist auch zu sich selbst nicht gleich; die jeweilige `IsNaN`-/`isNaN`- beziehungsweise Endlichkeitsprüfung verwenden.

## F. Kurzschluss und Ersatzwerte

Bei den eingebauten `&&` und `||` wird die rechte Seite nur ausgewertet, wenn sie für das Ergebnis nötig ist. So schützt beispielsweise `name !== null && name.length > 0` in TypeScript den Zugriff auf einen nullable String. Die entsprechenden Nullprüfungen funktionieren auch in C#.

JavaScript-`&&` und `||` liefern einen Operandenwert, nicht zwingend `boolean`. `0 || 80` ergibt `80`, obwohl null Energie ein gültiger Zustand sein könnte. `0 ?? 80` ergibt dagegen `0`; `??` ersetzt nur `null` und `undefined`. [TypeScript: Truthiness und Narrowing](https://www.typescriptlang.org/docs/handbook/2/narrowing)

In C# ersetzt `??` einen Nullwert. C++-`optional.value_or(fallback)` liefert einen Ersatz bei leerem Optional, ist aber kein eigener Kurzschlussoperator: Der Ausdruck für das Argument wird vor dem Funktionsaufruf ausgewertet. Für einen teuren nur bei Bedarf berechneten Ersatz eine ausdrückliche Bedingung verwenden.

Bitweise `&` und `|` nicht versehentlich statt `&&` und `||` einsetzen. Sie haben andere Aufgaben und bieten nicht dieselbe Kurzschlussauswertung. Überladene C++-Operatoren können zusätzliche Besonderheiten haben; die Erklärungen hier beziehen sich auf eingebaute Operatoren.

## G. Priorität und Lesbarkeit

Multiplikation bindet stärker als Addition: `2 + 3 * 4` ergibt `14`, `(2 + 3) * 4` ergibt `20`. Vergleichs-, Logik- und Zuweisungsoperatoren haben ebenfalls Prioritäten. Für gemischte fachliche Bedingungen sinnvolle Klammern setzen, statt die Leserin eine Tabelle nachschlagen zu lassen.

`0 <= energy <= 100` ist in keiner unserer Sprachen die richtige allgemeine Bereichsprüfung. Schreibe `energy >= 0 && energy <= 100`. In C++ wird sonst nach dem ersten Vergleich ein boolesches Ergebnis mit 100 verglichen; C# und TypeScript melden für die übliche numerische Form einen Typfehler.

Für Pink Robin muss eine Bereichsprüfung ausserdem vorher klären, ob der Wert endlich und im richtigen Datentyp angekommen ist. Eine Formel alleine ersetzt die Eingabegrenze nicht.

## H. Miniübung

1. Welche Ergebnisse liefern `5 / 2` in C#, TypeScript und C++ bei den gezeigten Literalen?
2. Warum ist `energy || 80` in TypeScript für eine gültige Energie von 0 falsch?
3. Was unterscheidet zwei gleich aussehende JavaScript-Objekte bei `===`?
4. Schreibe eine Bedingung für eine endliche Zahl zwischen 0 und 100 einschliesslich der Grenzen.

<details>
<summary>Lösung anzeigen</summary>

1. C# und C++: `2`; TypeScript/JavaScript-`number`: `2.5`.
2. `0` ist falsy und wird ersetzt. Bei ausschliesslich fehlenden Werten `??` verwenden und den restlichen Typvertrag prüfen.
3. Sie haben unterschiedliche Identität. Ein Inhaltsvergleich muss ausdrücklich definiert werden.
4. Beispielsweise TypeScript: `Number.isFinite(energy) && energy >= 0 && energy <= 100`; C#: `double.IsFinite(energy) && energy >= 0 && energy <= 100`; C++: `std::isfinite(energy) && energy >= 0 && energy <= 100` mit `<cmath>`.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Gleiche Zeichen bedeuten nicht in jeder Sprache dieselbe Rechnung.
- Zuweisung, Gleichheit, Objektidentität und Inhaltsgleichheit unterscheiden.
- Kurzschluss schützt Zugriffe; `||` ist in JavaScript kein gleichwertiger Ersatz für `??`.
- Zahlenbereiche, Gleitkomma und ungültige Eingaben ausdrücklich berücksichtigen.
- Fachliche Ausdrücke lesbar halten und Grenzen gezielt prüfen.

[← 01.2 – null, undefined und optionale Werte](01-2-null-und-optionale-werte.md) · [Inhaltsverzeichnis](README.md) · [03 – Bedingungen und Schleifen →](03-bedingungen-und-schleifen.md)
