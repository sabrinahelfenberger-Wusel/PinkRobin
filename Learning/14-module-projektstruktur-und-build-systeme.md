# 14 – Module, Projektstruktur und Build-Systeme

## Lernziel

Du kannst nachvollziehen, wie mehrere Quelldateien zusammenarbeiten, wie Abhängigkeiten bekannt gemacht werden und wie daraus ein ausführbares Programm entsteht. C# ist der Anker; TypeScript-Module und C++17-Übersetzungseinheiten zeigen, warum ähnliche Schreibweisen unterschiedliche Aufgaben erfüllen.

Nach [Kapitel 13](13-testen-und-debugging.md) wollen wir Tests und Anwendung auch reproduzierbar bauen. **Die kleinen Projekte dieses Kapitels sind Lernbeispiele; sie ändern nicht die bestehende Projektstruktur von Pink Robin.**

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| Namen gruppieren | `namespace` | meist ES-Module | `namespace` |
| Namen verfügbar machen | `using` | `import` | z. B. qualifizierter Name oder `using` |
| Vertrag bekannt machen | Typ aus Quellcode / referenzierter Assembly | `export`, Typdeklaration, Import | Deklaration, oft in einem Header |
| Datei-/Projektabhängigkeit | `.csproj`, `ProjectReference` | Modulpfad und Build-/Laufzeitauflösung | `#include` und Build-Target-Abhängigkeiten |
| Fremdpaket | NuGet | Paketmanager und `package.json` | z. B. Paketverwaltung plus CMake-Targets |
| Bauen | .NET-SDK / MSBuild | Typprüfung, Übersetzung, ggf. Bundler | Compiler und Linker, hier über CMake |
| Typische Ausgabe | verwaltete Assembly, weitere Laufzeitdateien | JavaScript, ggf. gebündelte Assets | Objektdateien, Bibliotheken, Programm |

`using`, `import` und `#include` sind keine austauschbaren Übersetzungen derselben Operation.

## B. C#: Namespace, Projekt und Assembly

Ein Namespace ordnet Typnamen. Ein Projekt legt fest, welche Quellen und Abhängigkeiten gebaut werden. Eine Assembly ist eine resultierende .NET-Einheit mit Metadaten und Code. Ein Namespace kann sich über mehrere Dateien und Assemblies erstrecken; ein Ordner oder Namespace erzeugt nicht automatisch eine Projektgrenze.

Zwei Dateien im selben Konsolenprojekt, mit C# 10 oder neuer:

**`RobinState.cs`**

```csharp
namespace PinkRobin.Learning;

public sealed record RobinState(string Mood);
```

**`Program.cs`**

```csharp
using System;
using PinkRobin.Learning;

var state = new RobinState("curious");
Console.WriteLine(state.Mood);
```

`using PinkRobin.Learning` erlaubt den kurzen Typnamen. Es lädt kein NuGet-Paket und fügt keine fehlende Projektreferenz hinzu. Alternativ wäre `new PinkRobin.Learning.RobinState(...)` möglich. Das file-scoped `namespace ...;` gilt für die Datei, nicht automatisch für Nachbardateien. [Microsoft: Namespaces](https://learn.microsoft.com/en-us/dotnet/csharp/fundamentals/program-structure/namespaces)

Bei einem üblichen SDK-Projekt werden C#-Quelldateien im Projektverzeichnis nach dessen Regeln automatisch einbezogen. Sonderfälle und explizite Ausschlüsse bleiben möglich. Eine Bibliothek in einem anderen Projekt braucht dagegen eine Referenz. Ein Ausschnitt in der konsumierenden `.csproj` könnte lauten:

```xml
<ItemGroup>
  <ProjectReference Include="../PinkRobin.Contracts/PinkRobin.Contracts.csproj" />
</ItemGroup>
```

Der Pfad ist ein Beispiel. Ein `PackageReference` verweist stattdessen auf ein Paket. Eine Solution organisiert Projekte, ersetzt aber deren Abhängigkeitsbeziehungen nicht. Vom passenden Projektverzeichnis dienen `dotnet build` zum Bauen und `dotnet test` zum Ausführen eingerichteter Tests.

**Achtung, C#-Gehirn:** Ein erfolgreiches `using` beweist nicht, dass eine native C++-Bibliothek installiert oder ladbar ist. Eine native `.dll` ist nicht automatisch eine verwaltete .NET-Assembly.

## C. TypeScript: Module exportieren und importieren

Ein Modul veröffentlicht gezielt Namen. Andere Module importieren diese Namen über einen Modulpfad. Laufzeitwerte wie Funktionen und Klassen unterscheiden sich von reinen TypeScript-Typen, die bei der Übersetzung verschwinden.

Für dieses kleine **Node-ESM-Lernprojekt** verwenden wir `.mts`-Quelldateien. Bei normaler TypeScript-Ausgabe entstehen daraus `.mjs`-Dateien:

**`mood.mts`**

```typescript
export type Mood = "curious" | "happy";

export function describeMood(mood: Mood): string {
    return mood === "happy" ? "Robin freut sich" : "Robin ist neugierig";
}
```

**`main.mts`**

```typescript
import { describeMood } from "./mood.mjs";
import type { Mood } from "./mood.mjs";

const mood: Mood = "happy";
console.log(describeMood(mood));
```

Der `.mjs`-Pfad benennt bewusst die spätere Laufzeitdatei; TypeScript kann ihn unter der passenden Node-Modulauflösung der `.mts`-Quelle zuordnen. In einem Lernverzeichnis mit eingerichtetem TypeScript-Compiler:

```sh
tsc --module nodenext --target ES2022 --strict --outDir dist mood.mts main.mts
node dist/main.mjs
```

Das Ergebnis lautet `Robin freut sich`. `import type` verschwindet aus der Laufzeitausgabe. Die Funktion muss dagegen im ausgeführten Modul verfügbar sein. `.mts`/`.mjs` umgehen hier die zusätzliche Frage, wie ein gewöhnliches `.ts`-/`.js`-Projekt über `package.json` zwischen ESM und CommonJS unterscheidet.

In einem Browser-/Angular-Projekt übernimmt ein eingerichteter Build andere Teile der Auflösung und Ausgabe. Deshalb diese Node-Konfiguration nicht blind in die Webanwendung kopieren. `module` und `moduleResolution` müssen zur tatsächlichen Laufzeit beziehungsweise zum Bundler passen. Ein TypeScript-Pfadalias allein schreibt einen Laufzeitimport nicht automatisch um. [TypeScript: Module und Auflösung](https://www.typescriptlang.org/docs/handbook/modules/reference)

**Achtung, C#-Gehirn:** Ein `import` kann Laufzeitcode ausführen, wenn das importierte Modul ausgewertet wird. Es ist mehr als ein kurzer Typname wie bei einem C#-`using`. TypeScript-Typprüfung ersetzt ausserdem keine Laufzeitvalidierung eingehender Daten.

## D. C++17: Deklaration, Definition und Übersetzungseinheit

Eine **Deklaration** macht einen Namen und dessen Typ bekannt. Eine **Definition** stellt die Funktion oder das Objekt bereit. Header werden durch `#include` in die jeweilige Übersetzungseinheit eingebracht. Der Compiler verarbeitet eine `.cpp`-Datei nach Vorverarbeitung zusammen mit ihren eingebundenen Inhalten.

Ein vollständiges kleines Projekt:

```text
robin-learning/
  CMakeLists.txt
  include/robin/mood.hpp
  src/mood.cpp
  src/main.cpp
```

**`include/robin/mood.hpp`**

```cpp
#ifndef ROBIN_LEARNING_MOOD_HPP
#define ROBIN_LEARNING_MOOD_HPP

namespace robin {
enum class Mood { curious, happy };
const char* describeMood(Mood mood) noexcept;
}

#endif
```

**`src/mood.cpp`**

```cpp
#include "robin/mood.hpp"

namespace robin {
const char* describeMood(Mood mood) noexcept
{
    return mood == Mood::happy ? "Robin freut sich" : "Robin ist neugierig";
}
}
```

**`src/main.cpp`**

```cpp
#include "robin/mood.hpp"
#include <iostream>

int main()
{
    std::cout << robin::describeMood(robin::Mood::happy) << '\n';
}
```

Der Header nennt den Vertrag; `mood.cpp` liefert die Implementierung. Jede `.cpp` wird grundsätzlich separat übersetzt. Danach verbindet der **Linker** die benötigten Definitionen. Den Header zu finden reicht deshalb nicht: Wenn `mood.cpp` oder seine Bibliothek im Build fehlt, kann der Linker `describeMood` nicht auflösen.

Der Include-Guard verhindert mehrfaches Einbringen dieses Headers innerhalb einer Übersetzungseinheit. Er verhindert keine mehrfachen Definitionen über verschiedene Übersetzungseinheiten hinweg. Eine gewöhnliche nicht-`inline`-Funktionsdefinition im Header könnte deshalb beim Linken Konflikte auslösen. Klassen- und Enum-Definitionen gehören dagegen üblicherweise in Header; Templates benötigen häufig ebenfalls dort sichtbare Definitionen. Für unterschiedliche Entitäten gelten unterschiedliche Regeln der **One Definition Rule**.

**Achtung, C#-Gehirn:** Keine `.cpp`-Datei als Ersatz für eine Build-Abhängigkeit inkludieren. Die Quellen werden als eigene Übersetzungseinheiten gebaut. `namespace robin` schafft Namen, lädt aber keine Bibliothek. C++20-Module sind ein anderes Sprachfeature und kein Bestandteil unseres C++17-Lernprojekts.

## E. CMake beschreibt Targets und Anforderungen

Die passende **`CMakeLists.txt`** für das Lernprojekt:

```cmake
cmake_minimum_required(VERSION 3.20)
project(RobinLearning LANGUAGES CXX)

add_library(robin_mood STATIC src/mood.cpp)
target_include_directories(robin_mood PUBLIC include)
target_compile_features(robin_mood PUBLIC cxx_std_17)
set_target_properties(robin_mood PROPERTIES CXX_EXTENSIONS OFF)

add_executable(robin_demo src/main.cpp)
target_link_libraries(robin_demo PRIVATE robin_mood)
```

`robin_mood` ist ein Bibliotheks-Target, `robin_demo` ein ausführbares Target. Durch die Verbindung mit `robin_mood` erhält die Demo auch dessen öffentliche Include-Anforderung und C++17-Anforderung. Der Pfad `include` muss deshalb nicht noch einmal global konfiguriert werden.

Bei `target_include_directories` und vergleichbaren Nutzungsanforderungen bedeutet:

- **PRIVATE:** Das Target selbst benötigt die Angabe.
- **INTERFACE:** Konsumenten benötigen sie.
- **PUBLIC:** Beide benötigen sie.

Bei `target_link_libraries` beeinflusst dies entsprechend die Nutzung und Weitergabe von Abhängigkeiten. Das ist keine C++-Zugriffssteuerung; `PRIVATE` versteckt keine Klassenmember. Bei statischen Bibliotheken können private Implementierungsabhängigkeiten für den endgültigen Linkschritt weiterhin relevant sein. [CMake: Buildsystem und Nutzungsanforderungen](https://cmake.org/cmake/help/latest/manual/cmake-buildsystem.7.html)

Vom Lernprojektverzeichnis:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Der erste Schritt konfiguriert und erzeugt das native Buildsystem. Der zweite führt den eigentlichen Build über den gewählten Generator aus. Bei einem Single-Config-Generator liegt das Programm typischerweise unter `build/robin_demo`, bei einem Multi-Config-Generator unter einem Konfigurationsunterordner wie `build/Release`. Unter Windows kommt normalerweise `.exe` hinzu. Buildausgaben von Quellen getrennt halten.

## F. Fehler der richtigen Phase zuordnen

| Phase | Typische Meldung / Symptom | Zuerst prüfen |
|---|---|---|
| Paketwiederherstellung / Konfiguration | Paket oder SDK fehlt | Version, Quelle, Toolchain und Projektdatei |
| Typprüfung / Kompilierung | unbekannter Typ, Header nicht gefunden | Imports, Referenzen, Include-Pfade und Signaturen |
| Linken | undefined reference / unresolved external | Definition vorhanden und richtige Bibliothek verbunden? |
| Linken | multiple definition | Definition unzulässig mehrfach erzeugt? |
| Laufzeit-Modulauflösung | Modul nicht gefunden | tatsächliche Ausgabe, Dateiendung, Auflösung der Laufzeit |
| Native Bibliothek laden | Datei oder Symbol nicht ladbar | Suchpfad, Architektur, Exporte und abhängige Bibliotheken |

Ein erfolgreicher Compilerlauf ist nicht dasselbe wie ein erfolgreicher Link- oder Programmstart. Und ein grüner Build prüft noch nicht die fachliche Reaktion; dafür bleiben die Tests aus Kapitel 13 nötig.

## G. Pink Robin: mehrere Builds, klare Grenzen

Die vorhandene [`core/CMakeLists.txt`](../core/CMakeLists.txt) definiert bereits:

- `pinkrobin_behavior`: statische C++17-Verhaltensbibliothek aus `src/core.cpp`.
- `pinkrobin_core`: native Shared Library mit C-Wrapper aus `src/c_api.cpp`, verbunden mit der Verhaltensbibliothek.
- `core_tests` und `c_api_tests`: native Tests, sofern `BUILD_TESTING` aktiviert ist.
- `robin_demo`: Konsolenbeispiel, sofern `ROBIN_BUILD_EXAMPLES` aktiviert ist.

Die [öffentliche C++-Deklaration](../core/include/pinkrobin/core.hpp) und die [C-ABI](../core/include/pinkrobin/c_api.h) sind unterschiedliche Schnittstellen. Der C-Wrapper macht aus der Verhaltensbibliothek keine .NET-Assembly. Eine spätere C#-Anbindung muss die passende native Datei ausliefern und korrekt laden. Das TypeScript-Frontend importiert diese serverseitige Bibliothek nicht als Browsermodul.

Der vorhandene Core-Build vom Repository-Stamm:

```sh
cmake -S core -B core/build -DCMAKE_BUILD_TYPE=Release
cmake --build core/build --config Release
ctest --test-dir core/build -C Release --output-on-failure
```

Browser, Backend und nativer Core können getrennte Buildketten besitzen und trotzdem über die vereinbarten Nachrichten und die ABI zusammenarbeiten. Nicht versuchen, einen C#-`using`, einen TypeScript-`import` oder einen C++-Header über diese Prozess- und Laufzeitgrenzen hinweg als Verbindung zu verwenden.

## H. Abhängigkeiten reproduzierbar halten

Projektdateien beschreiben direkte Abhängigkeiten; transitive Abhängigkeiten kommen über andere Pakete oder Targets hinzu. Ein **Lockfile** kann die tatsächlich aufgelösten Paketversionen festhalten. Seine genaue Rolle hängt vom verwendeten Paketmanager ab. Es ersetzt weder die SDK-Version noch native Compiler und Plattformvoraussetzungen.

Für einen nachvollziehbaren Build gehören Quellstand, Projektkonfiguration, relevante Paketauflösung und Toolchain zusammen. Keine Compiler-, Framework- oder Paketversion nur deshalb erhöhen, weil ein Beispiel eine neuere Syntax zeigt. Binärdateien und Build-Caches nicht mit dem fachlichen Quellcode verwechseln; ein Clean Build kann veraltete Artefakte als Fehlerquelle ausschliessen.

Abhängigkeitszyklen vermeiden: Ein Transportadapter darf den Core verwenden, der Core sollte dafür nicht das HTTP-Backend benötigen. Gemeinsame Verträge bewusst klein halten. Ein Ordner namens `shared` ist noch keine tragfähige Architektur.

## I. Miniübung

1. Im C++-Lernprojekt fehlt `src/mood.cpp` im Bibliotheks-Target. Warum genügt der Header nicht?
2. Die Funktion wird nicht-`inline` im Header definiert, den zwei `.cpp`-Dateien inkludieren. Warum hilft der Include-Guard nicht gegen den möglichen Linkerfehler?
3. Eine C#-Datei enthält den richtigen `using`, aber das andere Projekt ist nicht referenziert. Was fehlt?
4. Warum muss TypeScript den Typ `Mood` nicht als Laufzeitexport bereitstellen, die Funktion `describeMood` aber schon?
5. Darf die CMake-Einstellung `PRIVATE` eine öffentlich deklarierte C++-Methode vor Aufrufern verstecken?

<details>
<summary>Lösung anzeigen</summary>

1. Der Header liefert die Deklaration. Die Definition muss kompiliert und beim Linken verfügbar sein.
2. Der Guard wirkt innerhalb jeder einzelnen Übersetzungseinheit. Jede kann trotzdem ihre eigene unzulässige externe Definition erzeugen.
3. Eine passende Projekt- oder Paket-/Assembly-Referenz, nicht noch ein weiterer Namespace-Import.
4. TypeScript-Typen verschwinden bei der Übersetzung. Die tatsächlich aufgerufene Funktion muss im ausgeführten JavaScript existieren.
5. Nein. CMake-Nutzungsanforderungen steuern den Build; C++-Memberzugriff regelt die Sprache.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Namespace, Quelldatei, Projekt, Paket und Laufzeiteinheit sind verschiedene Begriffe.
- C#-`using` verkürzt Namen; TypeScript-`import` verbindet Module; C++-`#include` bringt Deklarationen und weitere Inhalte in eine Übersetzungseinheit.
- Eine bekannte Deklaration ist noch keine verfügbare Definition.
- Compiler, Linker, Bundler und Laufzeitauflösung können an unterschiedlichen Stellen scheitern.
- CMake-Targets beschreiben Quellen und weiterzugebende Anforderungen ausdrücklich.
- Pink Robins Schichten arbeiten über Verträge zusammen, auch wenn sie getrennt gebaut werden.

[Zurück zu Kapitel 13](13-testen-und-debugging.md) · [Inhaltsverzeichnis](README.md)
