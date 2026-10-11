# Robin-Weboberfläche

Vorgesehen: Angular, TypeScript und HTML/CSS für Gesicht, Bedienung, simulierte Sensoren und Diagnose. Ein typisierter Angular-Service verbindet die Oberfläche über HTTPS und den vorgesehenen SignalR-Echtzeitkanal mit dem [ASP.NET-Core-Backend](../backend/).

Der C++-Core läuft nativ auf dem Server; Angular enthält weder Core-Quellcode noch dessen WebAssembly-Ausgabe. TypeScript-Adapter zeigen freigegebene simulierte Aktionen und melden zugeordnete Ergebnisse zurück.

Der Simulator benötigt eine Serververbindung. Verbindungsverlust, Wiederverbindung und Sitzungsablauf werden sichtbar behandelt; vor neuen Aufträgen erfolgt ein Zustandsabgleich. Die erste Demo verwendet synthetische Daten ohne Kamera-/Mikrofonzugriff.

Stand: Angular-Demo mit Virtual Robin vorhanden. Die statische Startseite ist bereits auf der Synology erreichbar. Docker und CI/CD: [Technologiekonzept](../docs/Technologiekonzept.md).

## Angular-Demo lokal starten

Voraussetzung: Node.js 22.12 oder neuer aus der 22er-LTS-Reihe.

```sh
git switch dev
git pull --ff-only origin dev
cd web
npm install
npm start
```

Danach http://localhost:4200 öffnen. `npm run build` erzeugt die Ausgabe unter
`dist/pinkrobin-web/browser/`. `npm test` prüft Begrüssung, Wiederholen,
Bewegungseinstellung und Timer-Aufräumen mit einer deterministischen Uhr.

Die Angular-Demo übernimmt SVG, dunkles Layout und Begrüssungsablauf aus
`virtual-robin/`. Die statischen Originaldateien bleiben als Referenz erhalten.
`AppComponent` enthält die Navigation zwischen Startseite (`/`) und Robin
(`/virtual-robin`). `HomeComponent` übernimmt die statische Projektstartseite;
`RobinPageComponent` enthält die Robin-Seite und den Wiederholen-Button; `RobinComponent`
verwaltet Figur, Sprechblase und Animationen mit Signalen für zoneless Angular.
Bei reduzierter Bewegung erscheint die Begrüssung sofort ohne Animationen.
Wiederholen, Änderungen der Bewegungseinstellung und Komponentenabbau räumen
laufende Timer auf; beim Abbau wird auch der Media-Query-Listener entfernt.

Die Navigation verwendet Hash-URLs (`/#/virtual-robin`), damit auch eine
statische Auslieferung ohne spezielle Serverregeln und direktes Neuladen
funktioniert. Die Originale `index.html` und `virtual-robin/` bleiben unverändert.

## CI/CD und Synology

GitHub Actions prüft `dev` und Pull Requests. Nach einem Push auf `main` wird das
geprüfte NGINX-Image auf GHCR veröffentlicht. Ein Synology-Auftrag holt es alle fünf
Minuten ab, prüft es und wechselt den Container mit Rückkehr zum bisherigen Stand
bei Fehlern. Einrichtung und Migration des bestehenden Containers:
[Synology-Deployment](../deploy/synology/README.md).

Robins Herz reagiert auf Klick, Touch und Tastatur (Enter/Leertaste): eine kurze
Nachricht, Blinzeln und sanftes Leuchten. Bei reduzierter Bewegung erscheint nur
die Nachricht. Berührungen unterbrechen bei Bedarf die Begrüssung; Wiederholen
und Seitenwechsel räumen auch die Reaktionstimer auf.
