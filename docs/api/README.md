# Backend-API, Vorbereitung v1

Implementiert im ASP.NET-Core-Projekt `backend/src/PinkRobin.Api`.
Die API liefert Infrastrukturstatus und getrennte, vom nativen Core gesteuerte Demo-Sitzungen.

- `GET /health/live`: 200, `{ "status": "alive" }`.
- `GET /health/ready`: 200 mit `{ "status": "ready" }`, wenn die native Core-Bibliothek
  ABI 1 unterstützt und eine Instanz erfolgreich angelegt/gelesen wurde. Bei
  Ladefehlern 503 Problem Details mit Titel `Core not connected`.
- `GET /api/v1/status`: 200 mit folgendem Vertrag:

```json
{
  "service": "PinkRobin.Api",
  "apiVersion": "1.0",
  "stage": "preparation",
  "core": { "connected": true, "implementation": "native-cabi-v1" },
  "sessionsAvailable": true,
  "commandsAvailable": true,
  "realtimeAvailable": false
}
```

Diese Antworten dürfen nicht gecacht werden (`Cache-Control: no-store`).
Die Status-API ist ohne Anmeldung lesbar und enthält keine personenbezogenen Daten.
Sitzungsendpunkte für die Demo sind vorhanden; SignalR ist noch nicht implementiert. Die HTTP-API-Version
ist unabhängig vom fachlichen Entwurfsstand des [Robin Protocol](../Robin-Protocol.md).
Das Vorhandensein des Statusendpunkts behauptet keine Bereitschaft für Core-Aufträge.

Die Demo trennt Zustände pro Sitzung. Browserrückmeldungen sind keine physischen
Nachweise. Einrichtung: [Backend-Anleitung](../../backend/README.md).

Nach NAS-Netzwerkeinrichtung ist die Status-API über
`https://pinkrobin.wanderwusel.ch/api/v1/status` erreichbar. NGINX erhält den
API-Pfad unverändert; Infrastruktur-Health-Endpunkte bleiben nur intern erreichbar.

## Demo-Sitzungen (v1)

`POST /api/v1/sessions` mit JSON `{ "reducedMotion": false }` erzeugt eine eigene
native Core-Instanz und antwortet mit HTTP 201: `{ "token": "…", "state": { … } }`.
Das zufällige Token ist eine Zugangsberechtigung und bleibt im Browser-Arbeitsspeicher;
es gehört weder in URLs noch in Logs. Folgende Anfragen benötigen den Header
`X-Robin-Session: <token>`:

| Anfrage | Wirkung |
| --- | --- |
| `GET /api/v1/session` | Aktuellen Core-Zustand lesen; Angular fragt alle 250 ms ab |
| `POST /api/v1/session/events` | JSON `{ "sequence": 1, "event": "heartTouched" }` senden |
| `DELETE /api/v1/session` | Instanz freigeben, HTTP 204 |

Ereignisse: `restart`, `heartTouched`, `headStarted`, `headEnded`,
`interactionCancelled`. Jede Sitzung beginnt mit Sequenz 0; neue Befehle müssen
fortlaufend nummeriert sein. Eine Wiederholung des letzten Befehls mit gleicher
Sequenz und gleichem Ereignis wird nicht erneut ausgeführt. Andere alte bzw.
übersprungene Sequenzen liefern 409, unbekannte Ereignisse 400. Zeitwerte nimmt
der Server ausschliesslich aus seiner monotonen Uhr.

Der Zustand enthält `mode` (`sleeping`, `waking`, `awake`), `mood` (`curious`,
`happy`, `affectionate`), `expression` (`sleeping`, `curious`, `happy`, `heartEyes`),
`headContact`, `timeMs`, `revision` und die bestätigte `sequence`.

Grenzen: 128 gleichzeitige Instanzen; Erstellung maximal 30-mal pro Minute je
vom Backend erkannter Quell-IP (hinter dem Proxy teilen Besucher diese Grenze);
maximal 30 neue Befehle pro Sekunde je Sitzung. Sitzungen enden nach zwei Minuten
Inaktivität oder spätestens nach 30 Minuten. Fehlende Tokens liefern 401,
unbekannte/abgelaufene Tokens 404, Überlast 429 bzw. 503. Eine Kontaktpause von
mehr als zwei Sekunden beendet Streicheln serverseitig. Ein Hintergrunddienst
prüft jede Sekunde und räumt auch ohne weitere Browseranfrage auf.

Bei reduzierter Bewegung überspringt der Server die Begrüssungsphase beim Anlegen
und Wiederholen. Die Berührungszeiten bleiben im Core. Bei Verbindungsfehlern
beendet Angular die Sitzung, kennzeichnet die lokale Demo und bietet einen
bewussten Neuverbindungsversuch an. Es spielt keine ungewissen Befehle in eine neue
Sitzung ein. Beim Verlassen der Seite stoppt die Abfrage; DELETE mit `keepalive`
ist best effort, die Ablaufzeit bleibt die Absicherung.
