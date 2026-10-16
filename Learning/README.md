# Learning C#, TypeScript & C++ with Pink Robin

Dieses Verzeichnis ist ein wachsendes Nachschlagewerk und Lernbuch für Pink Robin.

## Lernprinzip

C# ist die Ausgangssprache. Jedes Thema besteht aus zwei Teilen:

1. **Spickzettel** – Wie schreibe ich das in C#, TypeScript und C++?
2. **Richtig verstehen** – Welche semantischen Unterschiede gibt es und warum?

Die Beispiele orientieren sich möglichst an Pink Robin, damit Lernen und Projektentwicklung zusammengehören.

## Lernpfad

24 Hauptkapitel plus die Vertiefung 01.2. Die Sprachgrundlagen stehen vor den Integrations- und Betriebsthemen. Die Nummerierung folgt dem Lernaufbau; weiterführende Querverweise sind als solche zu lesen.

### Grundlagen: Werte, Kontrollfluss und Objekte

- [01 – Variablen und Typen](01-variablen-und-typen.md)
- [01.2 – null, undefined und optionale Werte](01-2-null-und-optionale-werte.md)
- [02 – Operatoren und Ausdrücke](02-operatoren-und-ausdruecke.md)
- [03 – Bedingungen und Schleifen](03-bedingungen-und-schleifen.md)
- [04 – Funktionen, Methoden und Parameter](04-funktionen-methoden-parameter.md)
- [05 – Objekte, Interfaces, Klassen und Structs](05-objekte-interfaces-klassen-structs.md)
- [06 – Properties, Zugriff und Kapselung](06-properties-zugriff-und-kapselung.md)
- [07 – Interfaces und Abstraktion](07-interfaces-und-abstraktion.md)
- [08 – Vererbung und Polymorphismus](08-vererbung-und-polymorphismus.md)

### Typsystem, Daten und Laufzeit

- [09 – Generics und Templates](09-generics-und-templates.md)
- [10 – Collections und LINQ-Alternativen](10-collections-und-linq-alternativen.md)
- [11 – Speicher und Lebensdauer](11-speicher-und-lebensdauer.md)
- [12 – Fehlerbehandlung: Exceptions und Result-Typen](12-fehlerbehandlung.md)
- [13 – Module, Projektstruktur und Build-Systeme](13-module-projektstruktur-und-build-systeme.md)
- [14 – Events, Callbacks und Nachrichten](14-events-callbacks-und-nachrichten.md)
- [15 – Asynchronität und Nebenläufigkeit](15-asynchronitaet-und-nebenlaeufigkeit.md)
- [16 – Serialisierung, JSON und API-Grenzen](16-serialisierung-json-und-api-grenzen.md)

### Testen und Pink-Robin-Praxis

- [17 – Testen und Debugging](17-testen-und-debugging.md)
- [18 – Zustandsmaschinen und zeitgesteuertes Verhalten](18-zustandsmaschinen-und-zeitgesteuertes-verhalten.md)
- [19 – Native Anbindung: C# ↔ C++](19-native-anbindung-csharp-cpp.md)
- [20 – HTTP, WebSockets und Verbindungsmanagement](20-http-websockets-und-verbindungsmanagement.md)
- [21 – Architektur und Abhängigkeitsgrenzen](21-architektur-und-abhaengigkeitsgrenzen.md)
- [22 – Logging und Diagnose](22-logging-und-diagnose.md)
- [23 – Performance und Ressourcenbudgets](23-performance-und-ressourcenbudgets.md)
- [24 – Konfiguration und Auslieferung](24-konfiguration-und-auslieferung.md)

## Abdeckung der Grundlagen

| Thema | Kapitel |
|---|---|
| Variablen und Datentypen | 01 und 01.2 |
| Operatoren | 02 |
| Bedingungen und Schleifen | 03 |
| Funktionen und Parameter | 04 |
| Klassen und Objekte | 05 |
| Interfaces und Vererbung | 07 und 08 |
| Properties und Zugriff | 06 |
| Collections und LINQ-Alternativen | 10 |
| Generics und Templates | 09 |
| Speicherverwaltung | 11 |
| Exceptions und Fehler als Werte | 12 |
| Asynchrone Programmierung | 15 |
| Events und Callbacks | 14 |
| Module und Namespaces | 13 |
| JSON und Datenübertragung | 16, 19 und 20 |

## Beispiele ausführen

- Codeblöcke sind je nach Kontext vollständige Programme, zusammengehörige Dateiteile, einzelne Ausschnitte oder noch auszufüllende Übungen. Nicht sämtliche Blöcke eines Kapitels in dieselbe Datei kopieren. Angegebene Typen, Imports und Projektabhängigkeiten übernehmen.
- Für C# eignet sich ein aktuelles .NET-Lernprojekt; das vorhandene Backend verwendet .NET 10. Vollständige Programme getrennt ausführen. Bei Top-Level-Programmen stehen ausführbare Anweisungen vor Typdeklarationen; Methoden mit `public` gehören in einen Typ. Testframeworks und Logging benötigen die jeweils beschriebenen Projektverweise.
- TypeScript mit `strict` prüfen. Beispiele in getrennten Modulen halten, etwa durch `export {};`, damit Namen nicht versehentlich mit anderen Beispielen oder Browserglobals zusammenfallen. Die passende Laufzeit und Modulauflösung beachten; Browser-APIs sind nicht automatisch in Node verfügbar.
- C++ verwendet grundsätzlich C++17. Concepts in Kapitel 9 benötigen ausdrücklich C++20, `std::expected` in Kapitel 12 C++23 und passende Standardbibliotheksunterstützung. Header und zusammengehörige Quelldateien einbeziehen; das Projekt aus Kapitel 13 als Mehrdateienprojekt bauen.
- Netzwerk-, SignalR- und native C#-Beispiele benötigen die beschriebene Umgebung. Ein Lernbeispiel schafft noch keine neue Backendroute oder produktive Core-Verbindung.

Den ausgeführten Prüfumfang und verbleibende Grenzen dokumentiert der [Prüfbericht](PRUEFBERICHT.md).

Die [Zuordnung alter und neuer Dateinamen](NUMMERIERUNG.md) hilft beim Umstieg von bisherigen Links und Kapitelnummern.
