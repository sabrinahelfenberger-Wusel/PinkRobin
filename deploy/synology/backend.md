# Backend auf Synology einrichten

Das Backend wird unabhängig vom Frontend gestartet. Zunächst auf der vorhandenen
NAS die .NET-Runtime testen. Das Frontend und sein Update-Auftrag bleiben erhalten.
GitHub Actions veröffentlicht erst nach erfolgreichen API- und Containerprüfungen
auf main das Image `ghcr.io/sabrinahelfenberger-wusel/pinkrobin-backend:main`.
Das Paket ist zunächst privat; derselbe NAS-Registry-Zugang benötigt Leserechte auf
auch dieses Paket. Bei fehlendem Zugriff GHCR-Paketberechtigungen prüfen.

## 1. Image veröffentlichen

Änderungen auf dev prüfen und wie gewohnt nach main übernehmen. Unter Actions
müssen beim Workflow **Backend CI** sowohl verify als auch publish erfolgreich sein.
Vorher ist der neue Image-Tag noch nicht abrufbar. Frontend- und Backend-Releases
haben eigene Images; die Einrichtung ändert den laufenden Frontend-Container nicht.

## 2. Runtime auf der NAS prüfen

Per SSH als Sabrina anmelden, dann `sudo -i`. Als root:

```sh
docker pull ghcr.io/sabrinahelfenberger-wusel/pinkrobin-backend:main
docker ps -a --format 'table {{.Names}}\t{{.Status}}\t{{.Ports}}'
```

Wenn noch kein Container `pinkrobin-backend` existiert und Port 5080 frei ist:

```sh
docker run -d --name pinkrobin-backend --restart=no \
  --log-driver json-file --log-opt max-size=10m --log-opt max-file=3 \
  -p 127.0.0.1:5080:8080 \
  ghcr.io/sabrinahelfenberger-wusel/pinkrobin-backend:main
```

Der Backend-Port ist nur auf der NAS selbst erreichbar. Keine Routerfreigabe für
5080 einrichten. Der Container braucht weder Docker-Socket noch Host-Verzeichnisse.
Er läuft als unprivilegierter Benutzer und speichert noch keine Sitzungen/Daten.

Nach etwa 20 Sekunden:

```sh
docker ps -a --filter name=pinkrobin-backend
docker logs --tail 80 pinkrobin-backend
curl --fail --show-error http://127.0.0.1:5080/health/live
curl --fail --show-error http://127.0.0.1:5080/api/v1/status
```

Erwartet: Container Up (healthy), `/health/live` liefert `{"status":"alive"}` und
Status liefert `core.connected=true` und `implementation="native-cabi-v1"`.
`curl --fail http://127.0.0.1:5080/health/ready` muss 200 mit `{"status":"ready"}` liefern.
Bei 503 die Backend-Logs auf Lade-, Architektur- oder ABI-Fehler prüfen.
Der C++-Core wird im Backend-Image mitgeliefert; kein eigener Container ist nötig. Die Docker-Gesundheitsprüfung verwendet
Liveness, nicht diesen Readiness-Endpunkt.

## 3. Neustartregel erst nach erfolgreichem Test

```sh
docker update --restart=unless-stopped pinkrobin-backend
```

Wenn die Runtime abstürzt, Logs sichern und den Testcontainer stoppen:

```sh
docker logs --tail 100 pinkrobin-backend
docker stop pinkrobin-backend
```

Nicht blind neu starten oder Frontend-Container ändern. Die NAS hat Linux 3.10.108;
CI läuft auf neuerem Ubuntu. Laufzeit-/Kernelkompatibilität muss hier praktisch
bestätigt werden. Ein Container liefert keinen eigenen Kernel mit.

## 4. Verbindung mit der Website

Der erste Runtime-Test auf der Synology ist erfolgreich. Das Frontend-NGINX leitet
`/api/` an `pinkrobin-backend:8080` weiter, ohne den Pfad zu kürzen. Die vorhandene
HTTPS-Regel der Synology bleibt auf Frontend-Port 8080 gerichtet.

Einmalige Aktivierung:

1. NAS-Auftrag kurz deaktivieren und laufenden Durchlauf abwarten.
2. Neue `deploy/synology/update-web.sh` auf der NAS ersetzen.
3. Die Änderungen von dev nach main übernehmen und erfolgreiche Builds abwarten.
4. Als root `sh /volume1/docker/pinkrobin-deploy/update-web.sh` ausführen.
5. API prüfen, dann geplanten Auftrag wieder aktivieren.

Das Skript erstellt bei Bedarf das benutzerdefinierte Bridge-Netz `pinkrobin-internal`
und verbindet das vorhandene Backend sowie das Frontend damit. Der Name des Backend-
Containers muss `pinkrobin-backend` bleiben. Das Frontend behält zusätzlich die
Standard-Bridge für den bisherigen NAS-Proxy und die konfigurierte Besucher-IP.
Die Netzwerkzuordnung bleibt bei Neustarts erhalten; neue Frontend-Container werden
automatisch wieder verbunden. Bei späterem Ersatz des Backends ebenfalls dieses
Netz anschliessen. Kein pauschaler Vertrauensbereich für Besucher-IP-Header nötig.

Prüfung auf der NAS und im Browser:

```sh
docker network inspect pinkrobin-internal
curl --fail --show-error http://127.0.0.1:8080/api/v1/status
curl --fail --show-error https://pinkrobin.wanderwusel.ch/api/v1/status
```

Die Status-API ist bewusst öffentlich und nur lesend. Sie liefert dieselbe Antwort
wie der lokale Backend-Test. Es gibt weiterhin keine Steuerbefehle oder persönlichen
Daten. `/api` ohne folgenden Pfad bleibt 404; `/api/` ohne bekannten Endpunkt liefert
Backend-404. `/hubs` bleibt ebenfalls 404, bis ein geprüfter SignalR-Vertrag vorliegt.
Nicht erreichbares Backend liefert 502; die statische Angular-Seite bleibt nutzbar.
NGINX löst den Backendnamen über Docker-DNS neu auf, sodass ein Backendwechsel mit
neuer Container-IP nach kurzer DNS-Cachezeit erkannt wird.

Port 5080 bleibt ausschliesslich auf `127.0.0.1` für lokale Diagnose erreichbar.
Die öffentliche Kommunikation läuft ausschliesslich über HTTPS-Port 443. Angular
benutzt weiterhin seine lokale Demo-Logik; das Routing allein verbindet noch keine
Robin-Steuerung mit dem Backend. Die spätere Angular-API verwendet relative
`/api/`-URLs unter derselben Domain und benötigt deshalb kein zusätzliches CORS.

## Updates

Der bestehende NAS-Auftrag `update-web.sh` aktualisiert ausschliesslich das Frontend.
Neue Backend-Images starten nicht automatisch auf der NAS. Eine eigene
Backend-Update-/Rollback-Konfiguration folgt nach dem bestandenen Runtime-Test.
So bleibt ein Runtime-Fehler vom Frontend-Deployment getrennt.

Version des laufenden Images prüfen:

```sh
docker inspect --format '{{index .Config.Labels "org.opencontainers.image.revision"}}' pinkrobin-backend
```
