# Pink Robin

🌐 **Projektwebsite:** [pinkrobin.wanderwusel.ch](https://pinkrobin.wanderwusel.ch)

![Pink Robin concept](assets/images/pink-robin-concept.png)

Begleitroboter mit Embedded-Software, gemeinsamem C++-Verhaltenskern und Angular-Weboberfläche.

**Stand:** Angular-Demo und ASP.NET-Core-Status-API laufen. Der erste portable C++-Core ist implementiert; seine Anbindung an Backend und Demo folgt.

## Projektstruktur

| Verzeichnis | Aufgabe / vorgesehene Technologien |
| --- | --- |
| [core/](core/) | Portabler C++-Verhaltenskern; native Tests und serverseitige Anbindung |
| [firmware/](firmware/) | Hardwareadapter und Roboter-Firmware; ESP32-S3, C/C++, ESP-IDF und FreeRTOS als vorgesehener Einstieg |
| [backend/](backend/) | ASP.NET Core / C# API, Demo-Sitzungen und nativer C++-Core; später EF Core / SQL |
| [web/](web/) | Angular, TypeScript und HTML/CSS; Websimulator und spätere Status-/Konfigurationsoberfläche |
| [mobile/](mobile/) | Companion-App mit .NET MAUI, C# und MVVM |
| [docs/](docs/) | Anforderungen, Systemarchitektur, Protokoll und Technologieentscheidungen |
| [tests/](tests/) | Komponentenübergreifende Integrations- und End-to-End-Szenarien |
| [tools/](tools/) | Entwicklungs-, Build- und Diagnosewerkzeuge |
| [assets/](assets/) | Bilder und weitere Projektressourcen |

## Ausdruck & Interaktion

Pink Robin soll Emotionen und Zustände nicht nur sprachlich, sondern auch visuell über Augen, Mimik und Bewegung vermitteln.

![Pink Robin – emotions](assets/images/pink-Robin_emotions.png)

## Architektur und Umsetzung

Der portable C++-Core läuft auf dem Roboter lokal und für Virtual Robin nativ auf dem Server hinter ASP.NET Core. Angular stellt den Simulator dar und kommuniziert über HTTPS sowie den vorgesehenen SignalR-Echtzeitkanal. Bibliothek oder eigener C++-Prozess bleibt als Backend-Anbindung offen. Der Websimulator benötigt eine Serververbindung; der physische Grundbetrieb bleibt offline verfügbar.

Die statische Startseite ist bereits unter https://pinkrobin.wanderwusel.ch über einen NGINX-Container und Synology-Reverse-Proxy mit HTTPS erreichbar. Angular, Backend, Core und Pipeline sind noch nicht implementiert.

Als nächste Architekturphase sind ein kleiner C++-Core, getrennte Backend-Sitzungen und deren Verbindung zum Angular-Gesicht mit Berührungsreaktion und Nickauftrag vorgesehen. Details: [Technologiekonzept](docs/Technologiekonzept.md).

## Repository und CI/CD

PinkRobin ist aktuell ein öffentliches Repository. Der erste C++-Core im Anfangsstadium ist ausdrücklich zur Veröffentlichung freigegeben (16. Oktober 2026). Die frühere Vorgabe eines privaten Cores ist für diesen frühen Stand aufgehoben; dies ist keine pauschale Freigabe späterer vertraulicher Bestandteile. Ein weiteres öffentliches Repository wird nicht automatisch aus main gespiegelt.

CI/CD ist im PinkRobin-Repository auf main geplant: prüfen, bauen und versionierte Frontend-/Backend-Releases bereitstellen. Die Synology soll freigegebene Releases mit begrenzten Leserechten abholen. Webseitenbereitstellung und öffentliche Quellcode-Freigabe sind getrennte Abläufe.

## Dokumentation

- [Lastenheft](docs/Lastenheft.md)
- [Systemarchitektur](docs/Systemarchitektur.md)
- [Technologiekonzept](docs/Technologiekonzept.md)
- [Robin Protocol](docs/Robin-Protocol.md)
- [Robin Principles – Privatsphäre & Ethik](docs/Robin-Principles.md)
- [Architekturübersicht der Verzeichnisse](docs/architecture/README.md)
- [API-Dokumentation](docs/api/README.md)
- [Entscheidungsprotokolle](docs/decisions/README.md)

## Backend-Vorbereitung

Ein ausführbares ASP.NET-Core-Grundgerüst mit Status-API, Gesundheitsprüfungen,
Core-Schnittstelle und eigenem CI-/Docker-Test ist unter [backend/](backend/)
vorbereitet. Die native Core-Anbindung und NAS-Auslieferung folgen separat.

## Erster nativer Core

Unter [core/](core/) ist der erste portable C++17-Verhaltenskern implementiert:
Aufwachen, Herzreaktion und Kopfstreicheln mit Herzaugen. CMake, Tests und eine
versionierte C-Schnittstelle bereiten die spätere C#-Anbindung vor. Das Backend
und Angular verwenden ihn noch nicht; die Live-Demo läuft bis zur Anbindung weiter
mit ihrer bisherigen Logik.
