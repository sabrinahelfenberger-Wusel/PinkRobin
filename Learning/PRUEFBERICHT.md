# Prüfbericht zum Lernbuch

**Historischer Prüfstand:** Die nachfolgenden Kapitelnummern beziehen sich auf die frühere Reihenfolge vor der Umstrukturierung. Neue Dateipfade stehen in der [Zuordnungstabelle](NUMMERIERUNG.md). Eine gesonderte Ergänzung zur neuen Reihenfolge folgt am Ende; frühere Ausführungsnachweise werden nicht stillschweigend auf neue Inhalte übertragen.

Prüfdatum: **16. Oktober 2026**. Umfang: Kapitel 1 bis 21 einschliesslich Kapitel 1.2, insgesamt 22 Kapiteldateien auf `dev`.

## Ergebnis

Der Gesamtdurchgang hat konkrete Fehler und missverständliche Ausführungskontexte in den frühen Kapiteln korrigiert. Die nachfolgend ausgewiesenen Prüfungen bestehen. Das ist keine pauschale Zusage, dass jeder Ausschnitt ohne seinen Kontext ein vollständiges Programm ist oder dass alle Schichten bereits produktiv verbunden sind.

## Korrekturen

| Kapitel | Änderung |
|---|---|
| 1 | C++-Stringheader ergänzt; TypeScript-Variablenname vermeidet das Browserglobal `name` |
| 1.2 | Compilerwarnungen bei nullable Referenzen von der Zulässigkeit von `null` bei Werttypen unterschieden |
| 2 | C++-Konstruktor initialisiert Member in Deklarationsreihenfolge; Stringheader im Klassenbeispiel ergänzt |
| 3 | `<algorithm>` für `std::min` ergänzt |
| 4 | C#-Top-Level-Beispiel mit Record in gültige Reihenfolge gebracht; Indexzugriff in JavaScript und C# ergänzt |
| 5 | Übungsinterface in `RobinEventSource` umbenannt, damit es nicht mit der Browser-API `EventSource` verschmilzt |
| 6 | Generisches C#-Beispiel in vollständigen Klassen-/Main-Kontext gesetzt; Voraussetzungen des Constraint-Ausschnitts erklärt; TypeScript-Namenskollision vermieden |
| 7 | Den hypothetischen State-Endpunkt ausdrücklich vom vorhandenen Statusendpunkt getrennt |
| Inhaltsverzeichnis | Gemeinsame Hinweise zu Ausschnitten, Sprachversionen, Modulen und benötigter Umgebung ergänzt |

## Tatsächlich ausgeführt

| Prüfung | Umfang / Ergebnis |
|---|---|
| TypeScript unter `strict` | 30 Blöcke aus den frühen und 15 aus den späteren Kapiteln sowie die zwei Module aus Kapitel 14, insgesamt 47 Blöcke: ohne Typfehler |
| TypeScript-Modulprojekt | `.mts`-Quellen aus Kapitel 14 mit NodeNext übersetzt und `.mjs`-Ausgabe ausgeführt; erwartete Ausgabe bestätigt |
| Eigenständige C++-Programme | 20 enthaltene Programme mit `main` kompiliert und ausgeführt; `-std=c++17 -Wall -Wextra -Werror -pedantic`, bei Bedarf mit aktuellem Core verbunden |
| CMake-Lernprojekt | Header und beide `.cpp`-Dateien aus Kapitel 14 gemeinsam gebaut und ausgeführt |
| C++-Konstruktorkorrektur | Klassenbeispiel aus Kapitel 2 in einem kleinen Aufrufer mit Warnungen als Fehler geprüft |
| C++20 / C++23 | Concept aus Kapitel 6 mit geeignetem Beispieltyp und `std::expected` aus Kapitel 7 mit kleinem Aufrufer kompiliert und ausgeführt |
| Vorhandener Core, Release | Aktuellen `core`-Stand mit CMake gebaut; CTest `behavior` und `c_abi` bestanden |
| Vorhandener Core, Sanitizer | Debug-Build mit `ROBIN_ENABLE_SANITIZERS=ON`; beide CTest-Prüfungen unter Address-/UndefinedBehavior-Sanitizern bestanden |
| JavaScript-Verhalten | Acht bereits vorbereitete Prüfskripte für Ablaufreihenfolge, Nachrichtenvalidierung, vier Node-Testfälle, aktuelle Antworten/Stop, Darstellung, Diagnoseprojektion, Frame-Bündelung und Konfiguration erneut ausgeführt |
| Dokumentstruktur | Kapitelverweise und elf unterschiedliche relative Repository-Ziele geprüft; keine fehlenden Ziele gefunden |
| JSON | Alle sechs als JSON markierten Dokumentblöcke erfolgreich geparst |

Bei TypeScript wurden getrennte Module verwendet. Wo ein Ausschnitt einen Typ aus dem vorherigen Block benötigt, wurde dieser ausgewiesene Kontext ergänzt. Übungslücken, das nicht definierte `connectToRobin`-Beispiel und der externe SignalR-Paketentwurf wurden nicht als eigenständige vollständige TypeScript-Programme gezählt. Die bereits vorbereiteten JavaScript-Prüfskripte prüfen Laufzeitverhalten; sie ersetzen keine Typprüfung. Diese erfolgte separat wie oben angegeben.

Das C++-Mehrdateienprojekt benötigt seinen eigenen Include-Pfad und die Implementierungsdatei. Eine isolierte Kompilierung nur seines `main.cpp` ist kein gültiger Build dieses Projekts. Absichtlich fehlerhafte Beispiele zu dangling references wurden nicht ausgeführt. Einzelne Fragmente ohne vollständigen Kontext sind nicht Bestandteil der Zahl von 20 vollständigen Programmen.

Verwendete Werkzeuge: TypeScript **5.9.3**, Node.js **24.19.0**, Apple Clang **21.0.0**, CMake **4.4.2** auf macOS. Ergebnisse gelten für diese Umgebung; Zielplattformtests bleiben gesondert nötig.

Die nativen Repository-Tests lassen sich mit den Befehlen im [Core-README](../core/README.md) wiederholen. Die temporären Lernbeispiel-Prüfprogramme wurden nicht als neue Produktivimplementierung oder neues Testframework in das Repository aufgenommen.

## Noch nicht durch diese Prüfung nachgewiesen

- **C#-Build und -Ausführung:** In der Prüfumgebung steht kein .NET-SDK beziehungsweise keine .NET-Laufzeit zur Verfügung. C# wurde fachlich und auf sichtbare Strukturfehler geprüft, aber nicht kompiliert. Das betrifft auch NUnit-, Logging-, Options- und P/Invoke-Beispiele mit ihren jeweiligen Projektabhängigkeiten.
- **SignalR und Browserintegration:** Es wurde kein Hub eingerichtet und kein realer Browser-Backend-Core-Ablauf getestet. SignalR-Paketauflösung, Verbindungswechsel, Proxy und Browser-DOM benötigen ihre Zielumgebung.
- **Reale C#-Native-Anbindung:** Native Core- und C-ABI-Tests bestehen; sie ersetzen nicht den echten P/Invoke-Lauf mit SafeHandle, Dispose, Layout und passendem Publish-Artefakt.
- **Zielhardware und Auslieferung:** Keine NAS-Aktualisierung, kein Containerrelease, kein ESP32-Test und keine Leistungszusage. musl-/glibc-, Architektur- und Hostkernel-Kompatibilität sind am jeweiligen Zielartefakt zu prüfen.

## Nächster konkreter Prüfschritt

In einer Umgebung mit dem .NET-10-SDK die vollständigen C#-Programme jeweils getrennt bauen und ausführen, NUnit- und Logging-Beispiele in passenden Projekten prüfen und danach den Lernadapter aus Kapitel 16 gegen die echte native Bibliothek testen. Erst anschliessend kann die durchgängige Backendintegration als nachgewiesen gelten.

[Zum Inhaltsverzeichnis](README.md)

## Historischer Zwischenstand: Lernreihenfolge mit 01.2 und geschlossene Grundlagenlücken

Die neue Ausgabe umfasst **24 Hauptkapitel plus 01.2**, insgesamt 25 Kapiteldateien. Neu sind Operatoren (02), Bedingungen und Schleifen (03) sowie Properties und Kapselung (06). Collections (10) enthält nun einen vollständigen Vergleich von LINQ to Objects, JavaScript-Arraymethoden und C++17-Algorithmen, einschliesslich Ausführungszeitpunkt, Mutation und Materialisierung.

Der historische Bericht oben behält seine ursprünglichen Nummern und Prüfzahlen. Für die Umstrukturierung zusätzlich ausgeführt:

- Vier vollständige C++17-Programme aus den neuen Kapiteln und dem Abfragevergleich mit `-Wall -Wextra -Werror -pedantic` kompiliert und ausgeführt.
- Zehn TypeScript-Blöcke aus den drei neuen Kapiteln und dem erweiterten Collections-Kapitel mit TypeScript 5.9.3 unter `strict` übersetzt und als JavaScript ausgeführt. Zusätzliche Prüfungen decken Operatorenergebnisse, die Zahl gültiger Berührungen, abgelehnte Energiewerte sowie Abfragereihenfolge ohne Mutation der Quelle ab.
- Überschriften, Dateinummern, Inhaltsverzeichnis, neue Vor-/Zurück-Navigation und relative Links in der geplanten Dateistruktur geprüft.
- Bei den bisherigen Kapiteln die Codeblöcke auf Erhalt kontrolliert; geändert wurden dort Einordnung, Navigation und Kapitelverweise. Der Collections-Vergleich ist eine ausdrückliche inhaltliche Ergänzung.
- Alle Markdown-Dateien ausserhalb von `Learning` im zugrunde gelegten Repositorystand auf umzuschreibende Learning-Verweise geprüft; keine solchen Verweise gefunden.

Die C#-Beispiele bleiben mangels .NET-Laufzeit hier nicht ausgeführt. Auch diese neue Prüfung richtet keine Produktivintegration ein. Neue Nummern und Dateipfade stehen im [Inhaltsverzeichnis](README.md) und in der [Zuordnungstabelle](NUMMERIERUNG.md).

## Fortlaufende Nummerierung 01 bis 25

Der aktuelle Lernpfad ist vollständig von **01 bis 25** nummeriert. Das bisherige Kapitel 01.2 ist jetzt Kapitel 02; alle danach folgenden Kapitel sind um eins verschoben. Die oben dokumentierten Prüfstände bleiben historische Angaben mit ihren damaligen Nummern. Inhalte und Codebeispiele wurden bei dieser reinen Nummerierungsänderung nicht verändert.

Dateinamen, Kapitelüberschriften, Inhaltsverzeichnis, Abdeckungstabelle, Navigation und aktuelle Querverweise wurden auf Konsistenz geprüft. Die [Zuordnungstabelle](NUMMERIERUNG.md) zeigt sowohl die unmittelbar vorherige als auch die ursprüngliche Nummerierung. Die offenen C#- und Integrationsprüfungen bleiben unverändert offen.
