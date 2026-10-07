# Robin-Backend

Vorgesehen: C# / ASP.NET Core Web API für den serverseitigen Virtual-Robin-Core. Das Backend verwaltet getrennte Demo-Sitzungen, validiert Aufträge und Rückmeldungen und bindet den nativen C++-Core an. Native Bibliothek mit C-ABI/P\u002fInvoke oder eigener Prozess bleibt offen.

Angular kommuniziert über HTTPS; SignalR ist für Zustände und Aktionen in Echtzeit vorgesehen. Bei Verbindungsverlust oder Sitzungsablauf werden alte Aufträge nicht automatisch wiederholt. Sitzung, Zeit, Speicher und Nachrichten werden begrenzt.

Spätere Dienste können Geräteverwaltung, Updates und EF-Core-/SQL-Persistenz ergänzen. Der erste Simulationsprototyp benötigt keine dauerhaften persönlichen Daten. API-Verträge liegen unter [docs/api/](../docs/api/).

Das Backend ist für den Websimulator erforderlich, nicht für das lokale Grundverhalten des physischen Roboters. Die Companion-App behält ihre eigenen Verantwortlichkeiten.

Stand: Verzeichnisgerüst; noch keine API, Core-Anbindung oder Datenbank implementiert. Siehe [Technologiekonzept](../docs/Technologiekonzept.md).
