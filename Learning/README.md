# Learning C#, TypeScript & C++ with Pink Robin

Dieses Verzeichnis ist ein wachsendes Nachschlagewerk und Lernbuch für Pink Robin.

## Lernprinzip

C# ist die Ausgangssprache. Jedes Thema besteht aus zwei Teilen:

1. **Spickzettel** – Wie schreibe ich das in C#, TypeScript und C++?
2. **Richtig verstehen** – Welche semantischen Unterschiede gibt es und warum?

Die Beispiele orientieren sich möglichst an Pink Robin, damit Lernen und Projektentwicklung zusammengehören.

## Lernpfad

25 fortlaufend nummerierte Kapitel. Die Sprachgrundlagen stehen vor den Integrations- und Betriebsthemen. Die Nummerierung folgt dem Lernaufbau; weiterführende Querverweise sind als solche zu lesen.

### Grundlagen: Werte, Kontrollfluss und Objekte

- [01 – Variablen und Typen](01-variablen-und-typen.md)
- [02 – null, undefined und optionale Werte](02-null-und-optionale-werte.md)
- [03 – Operatoren und Ausdrücke](03-operatoren-und-ausdruecke.md)
- [04 – Bedingungen und Schleifen](04-bedingungen-und-schleifen.md)
- [05 – Funktionen, Methoden und Parameter](05-funktionen-methoden-parameter.md)
- [06 – Objekte, Interfaces, Klassen und Structs](06-objekte-interfaces-klassen-structs.md)
- [07 – Properties, Zugriff und Kapselung](07-properties-zugriff-und-kapselung.md)
- [08 – Interfaces und Abstraktion](08-interfaces-und-abstraktion.md)
- [09 – Vererbung und Polymorphismus](09-vererbung-und-polymorphismus.md)

### Typsystem, Daten und Laufzeit

- [10 – Generics und Templates](10-generics-und-templates.md)
- [11 – Collections und LINQ-Alternativen](11-collections-und-linq-alternativen.md)
- [12 – Speicher und Lebensdauer](12-speicher-und-lebensdauer.md)
- [13 – Fehlerbehandlung: Exceptions und Result-Typen](13-fehlerbehandlung.md)
- [14 – Module, Projektstruktur und Build-Systeme](14-module-projektstruktur-und-build-systeme.md)
- [15 – Events, Callbacks und Nachrichten](15-events-callbacks-und-nachrichten.md)
- [16 – Asynchronität und Nebenläufigkeit](16-asynchronitaet-und-nebenlaeufigkeit.md)
- [17 – Serialisierung, JSON und API-Grenzen](17-serialisierung-json-und-api-grenzen.md)

### Testen und Pink-Robin-Praxis

- [18 – Testen und Debugging](18-testen-und-debugging.md)
- [19 – Zustandsmaschinen und zeitgesteuertes Verhalten](19-zustandsmaschinen-und-zeitgesteuertes-verhalten.md)
- [20 – Native Anbindung: C# ↔ C++](20-native-anbindung-csharp-cpp.md)
- [21 – HTTP, WebSockets und Verbindungsmanagement](21-http-websockets-und-verbindungsmanagement.md)
- [22 – Architektur und Abhängigkeitsgrenzen](22-architektur-und-abhaengigkeitsgrenzen.md)
- [23 – Logging und Diagnose](23-logging-und-diagnose.md)
- [24 – Performance und Ressourcenbudgets](24-performance-und-ressourcenbudgets.md)
- [25 – Konfiguration und Auslieferung](25-konfiguration-und-auslieferung.md)

## Abdeckung der Grundlagen

| Thema | Kapitel |
|---|---|
| Variablen und Datentypen | 01 und 02 |
| Operatoren | 03 |
| Bedingungen und Schleifen | 04 |
| Funktionen und Parameter | 05 |
| Klassen und Objekte | 06 |
| Interfaces und Vererbung | 08 und 09 |
| Properties und Zugriff | 07 |
| Collections und LINQ-Alternativen | 11 |
| Generics und Templates | 10 |
| Speicherverwaltung | 12 |
| Exceptions und Fehler als Werte | 13 |
| Asynchrone Programmierung | 16 |
| Events und Callbacks | 15 |
| Module und Namespaces | 14 |
| JSON und Datenübertragung | 17, 20 und 21 |

## Beispiele ausführen

- Codeblöcke sind je nach Kontext vollständige Programme, zusammengehörige Dateiteile, einzelne Ausschnitte oder noch auszufüllende Übungen. Nicht sämtliche Blöcke eines Kapitels in dieselbe Datei kopieren. Angegebene Typen, Imports und Projektabhängigkeiten übernehmen.
- Für C# eignet sich ein aktuelles .NET-Lernprojekt; das vorhandene Backend verwendet .NET 10. Vollständige Programme getrennt ausführen. Bei Top-Level-Programmen stehen ausführbare Anweisungen vor Typdeklarationen; Methoden mit `public` gehören in einen Typ. Testframeworks und Logging benötigen die jeweils beschriebenen Projektverweise.
- TypeScript mit `strict` prüfen. Beispiele in getrennten Modulen halten, etwa durch `export {};`, damit Namen nicht versehentlich mit anderen Beispielen oder Browserglobals zusammenfallen. Die passende Laufzeit und Modulauflösung beachten; Browser-APIs sind nicht automatisch in Node verfügbar.
- C++ verwendet grundsätzlich C++17. Concepts in Kapitel 10 benötigen ausdrücklich C++20, `std::expected` in Kapitel 13 C++23 und passende Standardbibliotheksunterstützung. Header und zusammengehörige Quelldateien einbeziehen; das Projekt aus Kapitel 14 als Mehrdateienprojekt bauen.
- Netzwerk-, SignalR- und native C#-Beispiele benötigen die beschriebene Umgebung. Ein Lernbeispiel schafft noch keine neue Backendroute oder produktive Core-Verbindung.

Den ausgeführten Prüfumfang und verbleibende Grenzen dokumentiert der [Prüfbericht](PRUEFBERICHT.md).

Die [Zuordnung alter und neuer Dateinamen](NUMMERIERUNG.md) hilft beim Umstieg von bisherigen Links und Kapitelnummern.
