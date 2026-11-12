# Änderungsprotokoll

## 0.1.8 – Zuverlässigere Wiederverbindung

### Verbessert

- Automatische Wiederverbindung bei kurzen Netzwerkunterbrechungen.
- Bestehende Session und gewählter Wetterort bleiben erhalten.
- Session-Token im sessionStorage ermöglicht die Wiederaufnahme
  nach einem Neuladen im selben Browser-Tab.
- Rückkehr zur Seite prüft die Verbindung unmittelbar.
- Befehlsabgleich verhindert doppelte Verarbeitung und das Nachspielen
  alter Berührungen.
- Wiederholungsversuche verwenden wachsende Warteabstände.

### Backend

- Inaktivitätsfrist konfigurierbar, standardmässig 15 Minuten.
- Feste maximale Session-Laufzeit von 30 Minuten entfernt.
- Abgelaufene Sessions werden weiterhin freigegeben.

### Tests und Dokumentation

- Tests für Wiederverbindung, Befehlsabgleich, Berührungsaufhebung,
  Warteabstände und Wiederaufnahme nach Neuladen ergänzt.
- Session-Lebensdauer mit deterministischer Testuhr geprüft.
- Hintergrundpause und Neuladen auf dem iPad erfolgreich getestet.
- Konfiguration und Grenzen der Wiederaufnahme dokumentiert.

### Versionen

- Web und Backend: 0.1.8
- Core: unverändert 0.1.6

## 0.1.7 – Portfolio zu Textgesprächen

### Aktualisiert

- Einstieg um Robins erste Texteingaben ergänzt.
- Architektur und Wetterbeispiel auf den Gesprächsablauf umgestellt.
- Zentrale Antworttexte aus dem Core beschrieben.
- Meilensteine für v0.1.5 und v0.1.6 ergänzt.
- Gesprächsdokumentation verlinkt und geplante Erweiterungen eingeordnet.

### Versionen

- Web: 0.1.7
- Core und Backend: unverändert 0.1.6

## 0.1.6 – Textgespräche mit Robin

### Neu

- Textfeld ersetzt die separate Wetterbedienung.
- Regelbasierte Erkennung von Wetterfragen, „Robin?“, Begrüssungen
  und Dank ohne LLM.
- Wetterdialog mit Rückfrage nach einer vierstelligen Schweizer PLZ.
- Kurzer Gesprächskontext erhält offene Ortsfragen bei Unterbrechungen.
- Gewählte PLZ bleibt über Gesprächs- und Prognoseablauf hinaus erhalten.
- Erneute Wetterfragen laden bei Bedarf neue Daten für den gewählten Ort.

### Schnittstellen

- Begrenzte UTF-8-Texteingaben und typisierte Gesprächsantworten ergänzt.
- Antworttexte werden zentral im Core erzeugt und über Backend
  und Angular ausgegeben.
- Wetterabrufe und Ergebnisse werden über Anfrage-IDs zugeordnet.
- Sequenzierte Textbefehle mit Schutz vor doppelter Verarbeitung.
- Standardwetter überschreibt den Gesprächsort nicht mehr.
- Ausgabeformat v2 um begrenzten Antworttext erweitert.

### Tests und Dokumentation

- Tests für Texterkennung, Gesprächsfrist, Ortswahl, Wetterergebnisse
  und Unterbrechungen ergänzt.
- Anzeige und Bestätigung gelieferter Core-Texte geprüft.
- Gesprächsablauf und Ausgabeschnittstellen dokumentiert.
- UTF-8 für Editor und MSVC-Build festgelegt.

### Versionen

- Web, Backend und Core: 0.1.6

## 0.1.5 – Technisches Portfolio

### Neu

- Eigene Projektseite unter `/projekt` mit Architekturübersicht,
  Wetterantwort-Beispiel, Meilensteinen und Dokumentationslinks.
- Navigation zwischen Startseite, Virtual Robin und Projektportfolio.

### Gestaltung

- Startseite auf kurzes Intro, Robin-Vorschau und zwei Einstiege reduziert.
- Technische Details auf die Projektseite verlagert.
- Ungenutzte Startseiten-Stile entfernt.
- Projekturheberschaft im Einstieg auf Sabrina Helfenberger korrigiert.

### Versionen

- Web: 0.1.5
- Core und Backend: unverändert 0.1.4

## 0.1.4 – Plattformfähigkeiten

### Neu

- Adapter melden beim Erstellen einer Core-Instanz ausdrücklich,
  ob Textausgabe und Outfit-Darstellung unterstützt werden.
- Spontane Wetterverwandlungen starten nur mit Outfit-Fähigkeit.
- Direkte Wetterantworten und Verwandlungen melden ausdrücklich,
  wenn die benötigte Fähigkeit fehlt.
- Wetterwissen bleibt auch ohne Outfit-Darstellung verfügbar.

### Schnittstellen

- Fähigkeitsübergabe über C-ABI, C#-Adapter und Core-Gateway ergänzt.
- Web-Sessions melden Textausgabe und Outfit-Darstellung.
- Ohne ausdrückliche Angaben sind beide Fähigkeiten deaktiviert.
  Bestehende Aufrufer des alten Erstellungsaufrufs müssen für
  Wetterantworten und Verwandlungen umgestellt werden.
- Fähigkeiten bleiben für die Lebensdauer einer Instanz unverändert.

### Tests und Dokumentation

- Tests für fehlende und teilweise unterstützte Fähigkeiten ergänzt.
- Ungültige Angaben an der C-Schnittstelle werden geprüft.
- Fähigkeitsvertrag und Adapterverantwortung dokumentiert.

## 0.1.3 – Robin im Portfolio

### Neu

- Projektseite auf den Funktionsstand von Robin 0.1.2 aktualisiert.
- Statische Robin-Vorschau mit Sprechblase und direktem Demo-Link.
- Ablaufgrafik für das Zusammenspiel von Angular, Backend und Core.
- Ausblick auf geplante Texteingabe, 3D-Darstellung und Roboterhardware.
- Navigation zu Technik und Ausblick ergänzt.

### Gestaltung

- Rosa Lichtschein und dezente Hover-Bewegung für die Robin-Vorschau.
- Darstellung für schmale Bildschirme angepasst.
- Reduzierte Bewegung berücksichtigt und alte Projektkarten-Stile entfernt.

### Versionen

- Web: 0.1.3
- Core und Backend: unverändert 0.1.2

## 0.1.2 – Wetterantworten und Core-Struktur

### Neu

- Robin beantwortet über einen Button die Frage „Wie wird das Wetter?“
  anhand seiner gespeicherten Prognose.
- Ohne gültige Wetterdaten gibt Robin eine ausdrückliche Rückmeldung.
- Antworten erscheinen acht Sekunden lang und werden danach bestätigt.
- Die Versionsanzeige unter Robin übernimmt die Web-Paketversion.

### Core und Backend

- Gemeinsamer Nachrichtenvertrag mit Herkunft und Empfangszeit.
- Begrenzter Interaktionspuffer und ersetzbare wartende Wetterprognose.
- WorldModel für Wetterwissen und RobinState für Robins eigenen Zustand.
- Wetterverhalten und Aktionskoordination in eigene Module ausgelagert.
- Typisierte Ausgaben mit Sequenznummern, begrenztem Puffer und
  wiederholbarer Bestätigung über C-ABI, Backend und Angular.
- Direkte Wetterantworten warten nicht auf Outfit-Freigabe oder
  den Cooldown spontaner Wetteraktionen.
- Spontane Wetteraktionen können beim nächsten freien
  Verarbeitungszeitpunkt starten; die feste Wiederprüfungsfrist entfällt.

### Tests und Dokumentation

- Tests für Nachrichtenreihenfolge, Puffergrenzen, Wettergültigkeit,
  Session-Trennung, Ausgabebestätigung und Anzeigedauer ergänzt.
- Core- und Backend-Tests nach Themen aufgeteilt.
- Architektur und Ausgabeweg mit Ablaufgrafik dokumentiert.

### Bekannte Grenzen

- Ausgabe- und Session-Daten liegen im Arbeitsspeicher und überstehen
  einen Backend-Neustart nicht.
- Die Wetterfrage wird über einen Button ausgelöst.
  Freie Texteingabe, Spracherkennung und Sprachausgabe folgen später.

## 0.1.1 – Wetterkommentare

### Neu

- Robin kommentiert seinen Regenmantel:
  „Gut, dass ich meinen Regenmantel dabei habe!“
- Robin kommentiert seine Octopus-Verwandlung:
  „Bei dem Wetter brauche ich wohl acht Arme!“

### Verhalten

- Wetterkommentare erscheinen während der aktiven Verwandlung,
  wenn Robin wach, neugierig und nicht beim Streicheln ist.
- Berührungsreaktionen haben Vorrang vor Wetterkommentaren.
- Nach Ende der Verwandlung erscheint wieder der normale Text.

### Tests

- Automatisierte Tests für beide Kommentare, den Vorrang der
  Berührungsantwort und die Rückkehr zum normalen Text ergänzt.
