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
Die Sperre `/tmp/pinkrobin-web-deploy.lock` verhindert parallele Durchläufe. Nach
Stromausfall kann eine verwaiste Sperre verbleiben: erst sicherstellen, dass kein
Deployment läuft, dann das leere Sperrverzeichnis entfernen.

## Nachweis und Grenzen

CI prüft npm ci, Timer-Tests, Angular-Build, Docker-Build, HTTP-Einstieg, Revision,
fehlende Assets und nicht vorhandene API-Pfade. Die lokale Arbeitsumgebung hat kein
Docker; der Container- und NAS-Test wird erstmals durch GitHub bzw. auf der NAS
bestätigt. Einrichtung, GHCR-Zugang und Aufgabenplaner sind einmalige NAS-Schritte.

Quellen: [GitHub: Docker-Images veröffentlichen](https://docs.github.com/en/actions/tutorials/publish-packages/publish-docker-images),
[GHCR-Zugang](https://docs.github.com/en/packages/working-with-a-github-packages-registry/working-with-the-container-registry).
