# 04 – Bedingungen und Schleifen

## Lernziel

Du kannst Entscheidungen und Wiederholungen ausdrücken, Schleifen kontrolliert beenden und typische Fehler bei Bedingungen, Indizes und asynchroner Wiederholung vermeiden. C# ist der Anker, TypeScript/JavaScript und C++17 werden daran verglichen.

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Bedingung | `if (...) { ... } else { ... }` | ebenso | ebenso |
| Fallunterscheidung | `switch`, auch Switch-Ausdruck | `switch` | `switch` |
| Gezählt wiederholen | `for` | `for` | `for` |
| Werte durchlaufen | `foreach` | `for...of` | range-based `for` |
| Solange Bedingung gilt | `while` | `while` | `while` |
| Mindestens einmal | `do ... while` | ebenso | ebenso |
| Schleife verlassen | `break` | `break` | `break` |
| Nächster Durchlauf | `continue` | `continue` | `continue` |

Die verwendeten kleinen Arrays sind zunächst nur Reihen von Beispielwerten. Collections vertieft Kapitel 11. `Main` beziehungsweise `main` dient in den vollständigen Programmen als Einstieg; Funktionen behandeln wir im nächsten Kapitel.

## B. C# als Anker: entscheiden und durchlaufen

```csharp
using System;

double[] strengths = { double.NaN, -1, 0, 4, 5, 9 };
int touches = 0;

foreach (double strength in strengths)
{
    if (!double.IsFinite(strength) || strength < 0)
        continue;

    if (strength >= 5)
        ++touches;
}

if (touches > 0)
    Console.WriteLine($"Berührungen: {touches}"); // 2
else
    Console.WriteLine("Keine Berührung");
```

`continue` überspringt den Rest dieses Durchlaufs. `break` würde die gesamte nächste umgebende Schleife verlassen. Ein `return` beendet dagegen die aktuelle Funktion beziehungsweise den Einstiegspunkt; die Begriffe nicht verwechseln.

C# erwartet eine boolesche Bedingung. `if (touches)` ist keine gültige Kurzform von `if (touches > 0)`. Geschweifte Klammern bei mehreren Anweisungen verwenden; auch bei einzelnen Anweisungen können sie spätere Änderungen sicherer machen.

## C. TypeScript: Wahrheit ist nicht dasselbe wie gültige Daten

```typescript
const strengths = [Number.NaN, -1, 0, 4, 5, 9];
let touches = 0;

for (const strength of strengths) {
    if (!Number.isFinite(strength) || strength < 0)
        continue;
    if (strength >= 5)
        ++touches;
}

console.log(touches > 0 ? `Berührungen: ${touches}` : "Keine Berührung");
```

JavaScript erlaubt Truthiness: Unter anderem `0`, `""`, `null`, `undefined` und `NaN` sind falsy, leere Arrays und leere Objekte dagegen truthy. Eine Prüfung `if (strength)` würde den gültigen Wert 0 aussortieren und eine negative Zahl nicht automatisch ablehnen. Fachliche Bedingungen ausdrücklich schreiben. [TypeScript: Kontrollfluss und Narrowing](https://www.typescriptlang.org/docs/handbook/2/narrowing)

**Achtung, C#-Gehirn:** `for...of` liefert Arraywerte. `for...in` liefert aufzählbare Eigenschaftsschlüssel und ist nicht der normale Ersatz für `foreach` über Arraywerte. Für Index und Wert kann später etwa `entries()` hilfreich sein.

## D. C++17: klarer Ablauf ohne implizite Abkürzungen

```cpp
#include <array>
#include <cmath>
#include <iostream>
#include <limits>

int main()
{
    const std::array<double, 6> strengths{
        std::numeric_limits<double>::quiet_NaN(), -1, 0, 4, 5, 9
    };
    int touches = 0;
    for (const double strength : strengths) {
        if (!std::isfinite(strength) || strength < 0)
            continue;
        if (strength >= 5)
            ++touches;
    }
    std::cout << "Beruehrungen: " << touches << '\n';
    return touches == 2 ? 0 : 1;
}
```

C++ kann Zahlen und Pointer in Bedingungen verwenden; null beziehungsweise der Nullpointer wird als falsch behandelt. Das ist kein Ersatz für eine ausdrücklich geprüfte Grenze. Auch die Zahl `-1` ist als boolesche Bedingung wahr.

Bei indexbasierten Schleifen auf den richtigen Bereich achten: gültige Indizes liegen für eine Sequenz der Länge `n` üblicherweise bei `0` bis `n - 1`. `i <= size()` läuft einen Schritt zu weit. Für reines Durchlaufen ist eine wertbasierte Schleife oft einfacher und vermeidet unnötige signed-/unsigned-Vergleiche.

## E. for, while und do/while gezielt einsetzen

Eine gezählte Schleife hat Initialisierung, Bedingung und Fortschritt:

```typescript
for (let attempt = 0; attempt < 3; ++attempt) {
    console.log(`Versuch ${attempt + 1}`);
}
```

Dieser Ausschnitt startet keine Netzwerkoperation und ist keine fertige Retry-Strategie. Die Syntax lässt sich mit passenden Typdeklarationen nach C# und C++ übertragen.

`while` prüft vor jedem Durchlauf. `do/while` prüft danach und führt seinen Rumpf daher mindestens einmal aus. Jede Schleife braucht einen verständlichen Fortschritts- oder Abbruchvertrag. Ein `continue` vor einer manuell am Ende stehenden Zählererhöhung kann bei `while` versehentlich eine Endlosschleife erzeugen.

Eine Schleife wie `while (!finished) { }` blockiert beziehungsweise beschäftigt CPU. Im Browser kann sie die Oberfläche einfrieren; in C++-Nebenläufigkeit ist eine ungeschützt geteilte Variable zusätzlich problematisch. Wartemechanismen, Abbruch und asynchrone Abläufe behandeln wir später in Kapitel 16.

## F. switch: bekannte Varianten vollständig behandeln

Mit C# lässt sich aus einem bekannten Wert ein Text berechnen:

```csharp
using System;

string expression = "happy";
string eyes = expression switch
{
    "sleeping" => "geschlossen",
    "curious" => "rund",
    "happy" => "lachend",
    "heart_eyes" => "Herzen",
    _ => "unbekannter Ausdruck"
};
Console.WriteLine(eyes);
```

Das ist eine lokale Fallunterscheidung, keine vollständige Netzwerkvalidierung. Bei externen unbekannten Werten muss der Anwendungskontext festlegen, ob abgelehnt, ein Fehler angezeigt oder eine dokumentierte Ersatzdarstellung verwendet wird.

Ein klassisches `switch` besitzt `case`-Abschnitte. In JavaScript und C++ kann ohne `break` oder anderen Kontrollfluss der nächste Abschnitt mitlaufen (**Fallthrough**). C# lässt normales unbeabsichtigtes Weiterlaufen eines nicht leeren Abschnitts nicht auf dieselbe Weise zu. Mehrere leere Fallmarken können aber bewusst denselben Rumpf auswählen.

C++17-`switch` verarbeitet nicht direkt `std::string`-Fälle wie das C#-Beispiel. Für Strings passende `if/else`-Prüfungen verwenden oder zunächst ein bekanntes Enum abbilden. TypeScript-String-Unions helfen später bei vollständigen Fallunterscheidungen.

## G. Ereignisse nicht mit Polling verwechseln

Das Lernbeispiel zählt alle starken Messungen. Ein echter Sensor, der während derselben Berührung hundert starke Werte liefert, soll aber nicht zwingend hundert neue Herzereignisse erzeugen.

Der Adapter muss seinen Vertrag kennen: etwa einen Übergang von „nicht berührt“ zu „berührt“ erkennen und gegebenenfalls entprellen. Kopfkontakt besitzt zusätzlich ein Ende. Das Durchlaufen von Sensorwerten alleine entscheidet diesen Ereignisvertrag nicht.

Zeitabläufe ebenfalls nicht durch Schleifenzählen modellieren. „1000 Durchläufe“ sind keine verlässliche Sekunde. Der Core erhält explizite Zeitwerte; die Zustandsmaschinen im Praxisteil vertiefen das.

## H. Miniübung

1. Wie viele Berührungen zählt die Reihe `NaN, -1, 0, 4, 5, 9` nach unserer Regel?
2. Was ändert sich, wenn beim ersten ungültigen Wert `break` statt `continue` verwendet wird?
3. Warum ist `if ([])` in JavaScript kein Test auf ein nicht leeres Array?
4. Welche Schleifenform führt ihren Rumpf auch bei anfangs falscher Bedingung einmal aus?
5. Warum ist `i <= length` bei Indizes meist falsch?

<details>
<summary>Lösung anzeigen</summary>

1. Zwei: 5 und 9. Ungültige und negative Werte werden übersprungen.
2. Die Schleife endet sofort beim ersten `NaN`, ohne die späteren Werte zu verarbeiten.
3. Arrays sind Objekte und auch leer truthy. Ihre Länge ausdrücklich prüfen.
4. `do/while`.
5. Der Index `length` liegt bei einer gewöhnlichen Sequenz bereits hinter dem letzten Element. Für vollständiges reines Durchlaufen eine Werte-Schleife erwägen.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Bedingungen müssen den fachlichen Vertrag ausdrücken, nicht nur Truthiness ausnutzen.
- `continue`, `break` und `return` beenden unterschiedliche Teile des Ablaufs.
- Bei jeder Wiederholung Fortschritt, Grenzen und Abbruch prüfen.
- `for...of` liefert in JavaScript Arraywerte; `for...in` ist etwas anderes.
- Ein Messwertdurchlauf ist weder automatisch ein Ereignis noch eine Zeitbasis.

[← 03 – Operatoren und Ausdrücke](03-operatoren-und-ausdruecke.md) · [Inhaltsverzeichnis](README.md) · [05 – Funktionen, Methoden und Parameter →](05-funktionen-methoden-parameter.md)
