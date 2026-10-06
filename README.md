# Pink Robin

Begleitroboter mit Embedded-Software, gemeinsamem C++-Verhaltenskern und Angular-Weboberfläche. Das GitHub-Repository trägt derzeit den Namen **RedRobin**.

**Stand:** Konzept und Verzeichnisgerüst. Die unten genannten Anwendungen sind noch nicht implementiert.

## Projektstruktur

| Verzeichnis | Aufgabe / vorgesehene Technologien |
| --- | --- |
| [core/](core/) | Portabler C++-Verhaltenskern; native Tests und WebAssembly-Bindung |
| [firmware/](firmware/) | Hardwareadapter und Roboter-Firmware; ESP32-S3, C/C++, ESP-IDF und FreeRTOS als vorgesehener Einstieg |
| [backend/](backend/) | ASP.NET Core / C# REST API, Entity Framework Core und SQL |
| [web/](web/) | Angular, TypeScript und HTML/CSS; Websimulator und spätere Status-/Konfigurationsoberfläche |
| [mobile/](mobile/) | Companion-App mit .NET MAUI, C# und MVVM |
| [docs/](docs/) | Anforderungen, Systemarchitektur, Protokoll und Technologieentscheidungen |
| [tests/](tests/) | Komponentenübergreifende Integrations- und End-to-End-Szenarien |
| [tools/](tools/) | Entwicklungs-, Build- und Diagnosewerkzeuge |
| [assets/](assets/) | Bilder und weitere Projektressourcen |

## Architektur und Umsetzung

Der portable C++-Kern wird von Firmware und Websimulator gemeinsam verwendet. Im Browser wird er über WebAssembly angebunden. Das Backend ergänzt API und Persistenz; es ist keine Voraussetzung für den lokalen Websimulator. Die MAUI-App ist ein eigener Client.

Zuerst bleibt der im [Technologiekonzept](docs/Technologiekonzept.md) beschriebene Kern-/Simulator-Prototyp vorgesehen. Als erster späterer Full-Stack-Nachweis bietet sich an: Gerätestatus empfangen, speichern und in Angular anzeigen. Simulierte Daten werden als solche gekennzeichnet.

Die Ordner sind zunächst im bestehenden privaten Repository angelegt. Die geplante spätere Auslagerung von Kern und Simulator in eigene öffentliche Repositories bleibt eine separate Aufgabe.

## Dokumentation

- [Lastenheft](docs/Lastenheft.md)
- [Systemarchitektur](docs/Systemarchitektur.md)
- [Technologiekonzept](docs/Technologiekonzept.md)
- [Robin Protocol](docs/Robin-Protocol.md)
- [Robin Principles](docs/Robin-Principles.md)
- [Architekturübersicht der Verzeichnisse](docs/architecture/README.md)
- [API-Dokumentation](docs/api/README.md)
- [Entscheidungsprotokolle](docs/decisions/README.md)
