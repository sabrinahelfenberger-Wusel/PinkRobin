# Gemeinsamer Robin-Kern

Portabler C++-Verhaltenskern für Zustände, Ereignisse, Auftragsprüfung und Aktionskoordination. Hardware, Netzwerk, Zeit und Oberfläche werden über Adapter angebunden.

Derselbe private Quellcode wird nativ für Firmware, Tests und den serverseitigen Websimulator gebaut. ASP.NET Core bindet ihn über eine C-Schnittstelle/P\u002fInvoke oder einen eigenen C++-Prozess an; diese Detailentscheidung bleibt offen. Jede Demo-Sitzung erhält getrennten Zustand.

Für die öffentliche Demo wird keine WebAssembly-Core-Datei ausgeliefert. Der physische Roboter führt seinen Core lokal und ohne Serverabhängigkeit aus.

Stand: Verzeichnisgerüst; noch keine Implementierung. Grundlage: [Technologiekonzept](../docs/Technologiekonzept.md).
