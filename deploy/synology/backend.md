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
Status liefert `core.connected=false`. `/health/ready` bleibt absichtlich 503,
solange der C++-Core nicht verbunden ist. Die Docker-Gesundheitsprüfung verwendet
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

Erst nach erfolgreichem NAS-Test wird ein internes Docker-Netz zwischen Frontend
und Backend vorbereitet und das Frontend-NGINX für `/api/` weiterleiten. Die
bestehende Synology-HTTPS-Regel kann dann unverändert zum Frontend auf 8080 führen.
Aktuell liefert die öffentliche Website für `/api` weiterhin 404. Der Backend-Test
ist nur auf `127.0.0.1:5080` verfügbar; Angular verwendet weiterhin seine lokale
Demo-Logik. Sitzungen und SignalR werden separat implementiert.

## Updates

Der bestehende NAS-Auftrag `update-web.sh` aktualisiert ausschliesslich das Frontend.
Neue Backend-Images starten nicht automatisch auf der NAS. Eine eigene
Backend-Update-/Rollback-Konfiguration folgt nach dem bestandenen Runtime-Test.
So bleibt ein Runtime-Fehler vom Frontend-Deployment getrennt.

Version des laufenden Images prüfen:

```sh
docker inspect --format '{{index .Config.Labels "org.opencontainers.image.revision"}}' pinkrobin-backend
```
