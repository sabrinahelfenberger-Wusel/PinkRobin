# Robin-Weboberfläche

Vorgesehen: Angular, TypeScript und HTML/CSS für Gesicht, Bedienung, simulierte Sensoren und Diagnose. Ein typisierter Angular-Service verbindet die Oberfläche über HTTPS und den vorgesehenen SignalR-Echtzeitkanal mit dem [ASP.NET-Core-Backend](../backend/).

Der C++-Core läuft nativ auf dem Server; Angular enthält weder Core-Quellcode noch dessen WebAssembly-Ausgabe. TypeScript-Adapter zeigen freigegebene simulierte Aktionen und melden zugeordnete Ergebnisse zurück.

Der Simulator benötigt eine Serververbindung. Verbindungsverlust, Wiederverbindung und Sitzungsablauf werden sichtbar behandelt; vor neuen Aufträgen erfolgt ein Zustandsabgleich. Die erste Demo verwendet synthetische Daten ohne Kamera-/Mikrofonzugriff.

Stand: Verzeichnisgerüst; noch kein Angular-Projekt initialisiert. Die statische Startseite ist bereits auf der Synology erreichbar. Docker und CI/CD: [Technologiekonzept](../docs/Technologiekonzept.md).
