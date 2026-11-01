# Änderungsprotokoll

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
