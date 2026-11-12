# Begrenzte Session- und API-Prüfung

Stand: 12.11.2026  
Bezug: GitHub-Issue #48  
Arbeitsbranch: `security-session-api`

## Umfang

Geprüft wurden Sessionzugriffe, die Reihenfolge beim Wetterabruf,
HTTP-Anfragegrössen, ausgewählte ungültige Eingaben und die
Parallelisierung des Wettercaches.

Die Prüfung erfolgte lokal mit Test-Sessions und simulierten
Wetterantworten. Sie ist kein vollständiger Sicherheitsnachweis
für die öffentlich erreichbare Installation.

## Befunde und Korrekturen

### Sessionprüfung vor externem Wetterabruf

Der Endpunkt `POST /api/v1/session/weather` lud Wetterdaten vor
der Token- und Sessionprüfung. Dadurch konnten abzuweisende
Anfragen unnötige Wetterabrufe auslösen.

Jetzt wird zuerst das Tokenformat und anschliessend die Existenz
der Session geprüft. Nach dem Wetterabruf wird erneut geprüft,
ob die Session noch besteht.

Ein Interoptest mit zählender Wetterquelle bestätigt, dass unbekannte,
gelöschte und abgelaufene Sessions keinen Wetterabruf auslösen.
Eine gültige Session erreicht die Quelle weiterhin.

### HTTP-Anfragegrösse

Für den Backend-Server und den API-Pfad des Nginx-Webproxies wurde
eine Grenze von 8 KiB Anfrageinhalt gesetzt.

Das fachliche Textlimit von 160 UTF-8-Bytes bleibt zusätzlich bestehen.
Die HTTP-Grenze begrenzt auch Anfragen, bevor deren vollständiger
JSON-Inhalt fachlich verarbeitet wird.

Der lokale HTTP-Test bestätigt Status 413 für eine zu grosse Anfrage.
Die Proxykonfiguration muss zusätzlich durch den Container-/CI-Pfad
geprüft werden.

### Fehlerhaftes JSON

Die allgemeine Fehlerbehandlung wandelte eine BadHttpRequestException
für fehlerhaftes JSON in HTTP 500 um.

Der Exception-Handler erhält jetzt den vorgesehenen Status solcher
Anfragefehler. Unerwartete interne Fehler bleiben HTTP 500.

Der HTTP-Test bestätigt Status 400 für fehlerhaftes JSON.

## Geprüfte bestehende Schutzmechanismen

### Sessionzugriffe

HTTP-Tests prüfen Zustand, Befehle, Ausgabeabruf, Ausgabebestätigung
und Session-Wetteraktualisierung:

- Fehlende oder formal ungültige Tokens liefern 401.
- Unbekannte und gelöschte Session-Tokens liefern 404.
- Bestehende Tests prüfen getrennte Zustände zweier Sessions
  und den Schutz vor erneut ausgeführten Befehlen.

Der Session-Token ist ein Zugangsschlüssel. Wer einen gültigen Token
besitzt, kann auf dessen Session zugreifen. Es besteht keine zusätzliche
Benutzeranmeldung.

Diese Tests ersetzen keine vollständige Prüfung der Trennung sämtlicher
Daten und Ausgaben zwischen Sessions.

### Texteingaben

Leere Eingaben, reine Leerzeichen, zu lange Texte und ein Nullzeichen
werden mit HTTP 400 abgewiesen.

Das Limit wird anhand von UTF-8-Bytes geprüft. Die abgewiesenen
Testeingaben verbrauchen keine Befehlsnummer; danach bleibt eine gültige
Interaktion möglich.

### Wettercache

Der Wetterdienst verwendet einen gemeinsamen Cache für alle Orte.
Unterschiedliche oder unbekannte PLZ lösen bei gültigem Cache keine
erneuten Prognosedownloads aus.

Ein gezielt blockierter erster Abruf stellt sicher, dass der
Parallelitätstest tatsächlich überlappende Anfragen erzeugt.
Alle Aufrufer warten auf dieselbe Aktualisierung.

Bestehende Tests prüfen ausserdem Cache-Ablauf, Stundenwahl,
Datenzuordnung, fehlende Werte und die Fehlerpause nach einem
fehlgeschlagenen Abruf.

## Offene Aufgaben aus #48

- Rate-Limits und Ressourcenverbrauch aller öffentlichen Endpunkte
  prüfen, insbesondere Wetterzugriffe und wartende Anfragen.
- Tatsächliche Polling-Last und Hintergrundbetrieb messen.
- Tokenablage und Verhalten bei duplizierten Tabs untersuchen.
- CORS, Origin-Prüfung und Browser-Schutzmechanismen vollständig prüfen.
- Öffentliche Installation auf HTTPS, Proxyregeln, exponierte Ports
  und geeignete Sicherheitsheader prüfen.
- Logging, Datenschutz, Aufbewahrung und Zugriffsschutz prüfen.
- Abhängigkeiten, Container, Secrets und Betriebsrechte prüfen.
- Session-Isolation für sämtliche Daten- und Ausgabepfade weiter
  mit Negativtests absichern.

LLM, persistente Erinnerungen, Kamera, Mikrofon und Hardware benötigen
vor ihrer Einführung gesonderte Prüfungen.

## Reproduzierbare Prüfungen

Vom Repository-Hauptverzeichnis:

    dotnet run --project backend/tests/Interop -c Release
    dotnet run --project backend/tests/Weather -c Release

Bei laufendem Backend zusätzlich unter Windows:

    py -3 backend/tests/sessions.py

Der HTTP-Test verwendet standardmässig http://127.0.0.1:5080.
Er erzeugt eigene Test-Sessions und entfernt sie anschliessend.