# Backend-API, Vorbereitung v1

Implementiert im ASP.NET-Core-Projekt `backend/src/PinkRobin.Api`.
Die erste API liefert Infrastrukturstatus, keine simulierte Core-Steuerung.

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
  "sessionsAvailable": false,
  "commandsAvailable": false,
  "realtimeAvailable": false
}
```

Diese Antworten dürfen nicht gecacht werden (`Cache-Control: no-store`).
Die Status-API ist ohne Anmeldung lesbar und enthält keine personenbezogenen Daten.
Es gibt noch keine Auftrags-, Sitzungs- oder SignalR-Endpunkte. Die HTTP-API-Version
ist unabhängig vom fachlichen Entwurfsstand des [Robin Protocol](../Robin-Protocol.md).
Das Vorhandensein des Statusendpunkts behauptet keine Bereitschaft für Core-Aufträge.

Die spätere Demo braucht Sitzungszuordnung, Limits, validierte Ereignisse,
Duplikatbehandlung und Wiederverbindung. Zustände müssen pro Sitzung getrennt sein;
Browserrückmeldungen sind keine physischen Nachweise. Diese Verträge werden vor
Implementierung konkretisiert. Einrichtung: [Backend-Anleitung](../../backend/README.md).

Nach NAS-Netzwerkeinrichtung ist die Status-API über
`https://pinkrobin.wanderwusel.ch/api/v1/status` erreichbar. NGINX erhält den
API-Pfad unverändert; Infrastruktur-Health-Endpunkte bleiben nur intern erreichbar.
