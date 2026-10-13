# Automatische Web-Auslieferung auf Synology

## Ablauf

Push auf `dev` oder Pull Request: Angular-Tests, Build und NGINX-Containerprüfung.
Push auf `main`: dieselben Prüfungen, anschliessend Veröffentlichung des geprüften
Images in GitHub Container Registry (GHCR). Ein geplanter NAS-Auftrag prüft alle
fünf Minuten auf eine neue Version und startet nur bei geänderter Image-ID neu.
Lokal muss vorher kein Build ausgeführt werden.

Image: `ghcr.io/sabrinahelfenberger-wusel/pinkrobin-web:main`.
Zusätzlich erhält jeder Stand einen Tag `sha-<vollständige Git-Commit-ID>`.
Das Image enthält nur NGINX und fertige Frontend-Dateien, keinen Repository-Quellcode.
Das Update-Skript selbst wird nicht mit dem Web-Image aktualisiert.
Die Veröffentlichung macht das Paket nicht automatisch öffentlich.

## Einmal auf GitHub

1. Diese Änderungen zusammen mit dem Angular-Projekt nach `main` übernehmen.
2. Unter Actions den Workflow **Web CI and release** prüfen. Erst ein erfolgreicher
   main-Lauf stellt das Image bereit.
3. Das GHCR-Paket privat lassen. Für die NAS einen Classic-PAT mit nur
   `read:packages` und Zugriff auf das Paket erstellen. Token nicht ins Repository
   oder hier in den Chat kopieren. Falls die Organisation SSO nutzt, entsprechend
   autorisieren. Der CI-Push verwendet den automatisch bereitgestellten GITHUB_TOKEN.

## Einmal auf der Synology (DSM 7.1.1 / Docker)

Voraussetzungen: Docker, x86_64, NAS-Zugriff auf ghcr.io und NAS-Reverse-Proxy auf
`127.0.0.1:8080`. Der vorhandene HTTPS-Host `pinkrobin.wanderwusel.ch` bleibt bestehen.

1. `update-web.sh` in einen dauerhaften Verwaltungsordner kopieren, zum Beispiel
   `/volume1/docker/pinkrobin-deploy/update-web.sh`. Nur Administratoren dürfen die
   Datei verändern, weil sie später als root läuft.
2. Per SSH einmal **als root** `docker login ghcr.io -u DEIN_GITHUB_NAME` ausführen
   und den Read-Token als Passwort eingeben. Der geplante Auftrag muss denselben
   Benutzer und dessen Docker-Zugangskonfiguration verwenden. Das Login bleibt auf
   der NAS; GitHub braucht keinen SSH-Zugang zur NAS.
3. Bestehenden Container und seine Volume-/Port-Einstellungen dokumentieren.
   Vorhandene HTML-Dateien bleiben erhalten. Einen kurzen Wartungszeitraum wählen.
   Den bisherigen Container `pinkrobin-web` stoppen, seine automatische Neustartregel
   deaktivieren und ihn in `pinkrobin-web-static-backup` umbenennen. Nicht löschen.
   Er darf Port 8080 nicht mehr belegen.
4. Das Skript einmal ausführen:

   ```sh
   sh /volume1/docker/pinkrobin-deploy/update-web.sh
   ```

   Es erstellt den neuen Container `pinkrobin-web` mit Port `127.0.0.1:8080:80`.
   Kein bisheriges HTML-Volume in diesen Container einhängen: Die Dateien liegen
   bereits im Image. Danach die Domain, Startseite und Robin prüfen.
   `/version.txt` zeigt die ausgelieferte Commit-ID.
5. In DSM **Systemsteuerung → Aufgabenplaner → Erstellen → Geplante Aufgabe →
   Benutzerdefiniertes Skript** einen Auftrag als root erstellen, alle fünf Minuten:

   ```sh
   sh /volume1/docker/pinkrobin-deploy/update-web.sh
   ```

   Protokollausgabe speichern und DSM-Benachrichtigung bei Fehler aktivieren.
   Wenn die erste Migration scheitert, den neuen Container stoppen/entfernen und
   `pinkrobin-web-static-backup` zurückbenennen, mit der bisherigen Neustartregel
   starten. Die erste Migration ist noch nicht vom automatischen Rollback abgedeckt.

## Verhalten bei Updates und Rollback

Das Skript lädt zuerst das Image und prüft einen Kandidaten ohne veröffentlichten
Port. Dann wird der laufende Container gestoppt und als `pinkrobin-web-previous`
aufbewahrt. Erst danach startet die neue Version auf Port 8080. Scheitert ihr Start
oder Gesundheitstest, startet das Skript automatisch wieder den vorherigen Container.
Es gibt während des Wechsels eine kurze Unterbrechung. Der Test prüft NGINX und die
Einstiegsdatei; die gesamte HTTPS-/Browser-Kette wird bei der Einrichtung separat
geprüft. Ein unverändertes, aber ungesundes Image wird als Fehler gemeldet.

Für einen gezielten älteren Stand den geplanten Auftrag zunächst deaktivieren und
mit dessen Commit-ID ausführen:

```sh
PINKROBIN_IMAGE=ghcr.io/sabrinahelfenberger-wusel/pinkrobin-web:sha-COMMIT_ID \
  sh /volume1/docker/pinkrobin-deploy/update-web.sh
```

Erst nach einer korrigierten main-Version den Auftrag wieder aktivieren. Andernfalls
würde er wieder das aktuelle main-Image installieren. Kein automatisches Image-Pruning:
frühere Images bleiben für Rückkehr erhalten; Speicherverbrauch gelegentlich prüfen.

Konfiguration: `PINKROBIN_PORT` (Standard 8080), `PINKROBIN_BIND` (127.0.0.1) und
`PINKROBIN_IMAGE`. Änderungen müssen auch im geplanten Auftrag gesetzt werden.
Die Sperre `/tmp/pinkrobin-web-deploy.lock` verhindert parallele Durchläufe.
Eine nicht verfügbare Sperre liefert einen Fehlerstatus, damit DSM die konfigurierte
Fehlerbenachrichtigung versendet. Auch bei Rollback-Fehlern wird die eigene Sperre
freigegeben und die nötige manuelle Wiederherstellung gemeldet. Nach
Stromausfall kann eine verwaiste Sperre verbleiben: erst sicherstellen, dass kein
Deployment läuft, dann das leere Sperrverzeichnis entfernen.

## Nachweis und Grenzen

CI prüft npm ci, Timer-Tests, Angular-Build, Docker-Build, HTTP-Einstieg, Revision,
fehlende Assets und nicht vorhandene API-Pfade. Diese Prüfungen und die echte
NAS-Auslieferung wurden erfolgreich durchgeführt. Deploymenttests decken auch
Abbruch beim Stoppen/Umbenennen, blockierte Sperren und fehlgeschlagenen Rollback ab.
Ein Stromausfall oder SIGKILL kann nicht vom Skript abgefangen werden; dann Container
und Sperre vor der Wiederaufnahme manuell prüfen.

Quellen: [GitHub: Docker-Images veröffentlichen](https://docs.github.com/en/actions/tutorials/publish-packages/publish-docker-images),
[GHCR-Zugang](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry).

## Eine aktualisierte Skriptversion installieren

Änderungen am Skript kommen durch den Image-Pull nicht auf die NAS. Nach einem Pull
von dev die Datei `deploy/synology/update-web.sh` auf der NAS unter
`/volume1/docker/pinkrobin-deploy/update-web.sh` ersetzen. Währenddessen den geplanten
Auftrag deaktivieren und einen bereits laufenden Durchlauf erst beenden lassen.
Datei mit Unix-Zeilenenden (LF) übertragen. Container-Stop und Umbenennung aus der
Erstmigration sind dafür nicht nötig.

Per SSH als root:

```sh
chown root:root /volume1/docker/pinkrobin-deploy/update-web.sh
chmod 700 /volume1/docker/pinkrobin-deploy/update-web.sh
sh -n /volume1/docker/pinkrobin-deploy/update-web.sh
sh /volume1/docker/pinkrobin-deploy/update-web.sh
```

Auch vorhandene DSM-ACLs prüfen: nur Administratoren sollen Schreibrechte auf das
als root ausgeführte Skript besitzen. Anschliessend den geplanten Auftrag wieder
aktivieren. Zum Speichern der Ausgaben kann dessen Befehl lauten:

```sh
sh /volume1/docker/pinkrobin-deploy/update-web.sh >> /volume1/docker/pinkrobin-deploy/update.log 2>&1
```

Die Protokolldatei muss regelmässig archiviert oder begrenzt werden; automatische
Rotation ist aktuell nicht im Update-Skript implementiert. Fehlerbenachrichtigung
in DSM nur für unnormal beendete Durchläufe aktivieren und E-Mail-Versand testen.

## Besucher-IP im Zugriffsprotokoll

`web/nginx.conf` übernimmt `X-Real-IP` nur vom Synology-Reverse-Proxy unter
`172.17.0.1` (beobachtete Docker-Gateway-Adresse). DSM setzt diesen Header selbst
auf die Adresse der eingehenden Verbindung. Die NGINX-Zugriffsprotokolle enthalten
damit die Besucher-IP, Uhrzeit, Anfrage, Status, Referrer und User-Agent.
Ein weiterer vorgeschalteter Proxy wie Cloudflare würde eine erneute Prüfung der
Vertrauenskette erfordern. Bei Änderung des Docker-Netzes die Proxy-Adresse prüfen
und die Konfiguration anpassen; keinen pauschalen Vertrauensbereich eintragen.

Nach dem Deployment per SSH als root ansehen:

```sh
docker logs --since 10m pinkrobin-web
```

Zum Vergleich die Website über Mobilfunk öffnen. Lokale Gesundheitstests bleiben
als `127.0.0.1` sichtbar. IP-Adressen identifizieren Netzwerkanschlüsse, keine
Personen. Die Zugriffsprotokolle liegen zusätzlich dauerhaft in täglichen Dateien auf der
NAS. Sie bleiben auch nach Containerwechseln erhalten; Details folgen unten.

## Dauerhafte Zugriffsprotokolle (14 Tage)

Die neue Skriptversion bindet `/volume1/docker/pinkrobin-logs/` nach
`/var/log/pinkrobin` in den Container ein. Das Image schreibt dort
`access-JJJJ-MM-TT.log` nach UTC-Datum. Der Containerstart setzt die Verzeichnisrechte
für den NGINX-Benutzer (750); die Dateien sind per SSH als root lesbar. Im DSM
File Station kann ein Administrator je nach bestehenden Freigabe-/ACL-Rechten
zusätzliche Leserechte benötigen. Nur für dieses eigene Protokollverzeichnis verwenden.

Die Bereinigung läuft mit dem geplanten Update-Auftrag, auch wenn kein neues Image
vorliegt: Zugriffsdateien mit einem Änderungsalter ab 14 Tagen werden gelöscht.
Andere Dateien und Unterverzeichnisse bleiben erhalten. Fällt der Auftrag aus,
läuft auch die Bereinigung nicht; bei Ausfall der NAS entstehen keine neuen Logs.
Docker-Konsolenprotokolle sind auf drei Dateien zu je 10 MB pro Container begrenzt.
NGINX-Fehler bleiben in der Konsole; das tägliche Archiv enthält HTTP-Zugriffe.

Einmalige Übernahme:

1. Image-Änderungen von dev nach main übernehmen und den erfolgreichen Build abwarten.
2. Den geplanten NAS-Auftrag deaktivieren, laufenden Durchlauf abwarten und die neue
   `update-web.sh` auf die NAS kopieren (siehe Skriptaktualisierung oben).
3. Das Skript als root ausführen. Der Protokollordner wird automatisch erstellt.
   Auch bei gleicher Image-ID wird ein Container ohne passenden Mount einmal ersetzt.
4. Die Webseite öffnen und die heutige Datei prüfen. Auftrag wieder aktivieren.

```sh
ls -l /volume1/docker/pinkrobin-logs/
tail -n 30 /volume1/docker/pinkrobin-logs/access-$(date -u +%F).log
```

`PINKROBIN_LOG_DIR` erlaubt einen anderen absoluten Ordner. Dann dieselbe Einstellung
bei jedem manuellen und geplanten Aufruf verwenden. Die Rückkehr zu einer alten
Image-Version erhält den bisherigen Container samt dessen Mounts. Vor dieser
Einrichtung entstandene Docker-Logs werden nicht nachträglich ins Archiv kopiert.

Die 14-Tage-Regel begrenzt das Alter, nicht die maximale Tagesgrösse. Sehr viele
Zugriffe können eine Tagesdatei gross machen; NAS-Speicher überwachen. Zeitangaben
sind UTC (in der Schweiz je nach Jahreszeit eine oder zwei Stunden später).

Technische Grundlage: [NGINX-Zugriffsprotokolle](https://nginx.org/en/docs/http/ngx_http_log_module.html).

## Internes API-Netz

Das Update-Skript verbindet Frontend und ein vorhandenes `pinkrobin-backend`
mit `pinkrobin-internal`. Das Netz wird automatisch erstellt. Konfigurierbar über
`PINKROBIN_API_NETWORK`; Frontend und Backend müssen dasselbe Netz verwenden.
Das Standard-Bridge-Netz des Frontends bleibt bestehen, damit NAS-Portmapping und
Besucher-IP-Vertrauen weiter passen. Einrichtung und API-Prüfung:
[Backend auf Synology](backend.md).
