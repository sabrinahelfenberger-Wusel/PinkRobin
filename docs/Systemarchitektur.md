# Systemarchitektur: Pink Robin

Status: erster Architekturentwurf zur gemeinsamen Abstimmung.

## 1. Zweck und Grundlagen

Dieses Dokument beschreibt die fachlichen Komponenten, ihre Zuständigkeiten, Datenhaltung und Kommunikation. Es bildet die Grundlage für die anschliessende Definition des Robin Protocol.

Massgeblich sind das [Lastenheft](Lastenheft.md) und die [Robin Principles](Robin-Principles.md). Die Architektur bevorzugt lokale Verarbeitung, erhält wesentliche Offline-Funktionen und ordnet Persönlichkeit den Sicherheitsregeln, Datenschutzvorgaben und bewussten Benutzerentscheidungen unter.

Die hier beschriebenen Komponenten sind logische Verantwortlichkeiten. Das Grundverhalten läuft auf dem Prozessor im Kopf; erweitertes Verhalten läuft auf dem Smartphone. Die weitere Aufteilung der Softwarekomponenten und ihrer Daten wird nachfolgend konkretisiert. Konkrete Prozessoren, Betriebssysteme, Frameworks, Sensorchips und Nachrichtenformate bleiben offen.

## 2. Ausgangspunkt des Entwurfs

Robin besitzt einen lokal betriebenen Kern auf dem Prozessor im Kopf. Dieser führt das Grundverhalten aus und koordiniert den lokalen Betrieb. Das Smartphone führt erweitertes Verhalten aus und arbeitet dabei mit dem lokalen Kern zusammen. Grundlegende Interaktion und sicherer Betrieb hängen weder von einem Server noch von einem verbundenen Smartphone ab.

Companion-App und Weboberfläche ermöglichen Bedienung und Verwaltung. Die Homestation ist hauptsächlich Robins Ladestation. Sie dreht Robin mit einem eigenen Drehantrieb um 360° im angedockten Zustand und besitzt einen Leuchtring für Disco-Modus und Zustandsanzeigen. Der Server stellt ergänzende Dienste bereit. Externe KI liefert Ergebnisse oder Handlungsvorschläge; ausführbare Aktionen werden weiterhin lokal auf Berechtigung, Sicherheit und Benutzerentscheidungen geprüft.

Virtual Robin ersetzt die physische Hardware durch simulierte Fähigkeiten und verwendet möglichst denselben Robin-Kern.

Diese Aufteilung ist der Arbeitsstand dieses Entwurfs. Die offenen Entscheidungen in Abschnitt 12 müssen vor einer verbindlichen Umsetzung geklärt werden.

## 3. Komponentenübersicht

| Komponente | Hauptverantwortung | Abhängigkeit im Grundbetrieb |
| --- | --- | --- |
| Robin-Kern im Kopf | Grundverhalten, lokaler Kontext, lokale Daten und Koordination | Lokal verfügbar |
| Sicherheits- und Zugriffsprüfung | Aktionen zulassen, begrenzen oder stoppen | Lokal verfügbar; Sicherheitsfunktionen zusätzlich an Modulen |
| Kopf | Prozessor und Speicher, Gesichtsanzeige, Audio, Wahrnehmung, Nickbewegung und Funkanbindung | Beherbergt den lokalen Robin-Kern |
| Bauch | Akku, Ladeelektronik, Radar, Status-LED und gegebenenfalls IR | Für physischen Betrieb erforderlich |
| Beinmodule | Ausführung freigegebener Bewegung und lokale Schutzreaktionen | Nur für entsprechende Bewegungsfähigkeiten erforderlich |
| Homestation (Ladestation) | Sicherer Ladebetrieb, eigener Drehantrieb für Robin (360°) und Leuchtring | Für Laden und Stationsfunktionen erforderlich; mobiler Grundbetrieb bleibt unabhängig |
| Smartphone / Companion-App | Erweitertes Verhalten, Einrichtung, Verwaltung und Benutzerinteraktion | Für erweitertes Verhalten erforderlich; Grundverhalten bleibt unabhängig |
| Weboberfläche | Berechtigte Fernbedienung und Verwaltung | Abhängig vom angebotenen lokalen oder externen Zugang |
| Server | Geräteverwaltung, Updates und optionale Fernfunktionen | Ergänzend |
| Externe KI-Dienste | Komplexe Verarbeitung und Vorschläge | Ergänzend |
| Virtual Robin / Smartphone-Simulator | Simulation von Fähigkeiten und Testszenarien | Entwicklungsumgebung |

## 4. Robin-Kern

### 4.1 Wahrnehmung und Fähigkeiten

Hardwareadapter stellen Sensordaten und Ereignisse über fachliche Schnittstellen bereit. Eine Wahrnehmungskomponente leitet daraus Beobachtungen ab und kennzeichnet Unsicherheit, Aktualität und Quelle.

Die Fähigkeitenverwaltung kennt verfügbare Module, unterstützte Funktionen und Einschränkungen. Verhalten muss sich daran anpassen: Ein Robin ohne Beinmodul kann beispielsweise weiter sprechen und Mimik zeigen, aber keine Laufbewegung ausführen.

Unbekannte Personen werden nur für die aktuelle Interaktion unterschieden. Ihre Beobachtungen dürfen nicht automatisch zu dauerhaften Personeneinträgen werden.

### 4.2 Kontext und Verhalten

Die Kontextverwaltung führt relevante Beobachtungen, Benutzerwünsche, aktive Aufgaben und Systemzustände zusammen. Kurzfristiger Kontext und dauerhaft gespeicherte Informationen werden getrennt behandelt.

Die lokale Verhaltenssteuerung im Kopf entscheidet über grundlegende Reaktionen und führt das Grundverhalten aus. Die Verhaltenssteuerung auf dem Smartphone ergänzt dieses um erweitertes Verhalten. Eine einfache Persönlichkeit bleibt im lokalen Grundverhalten spürbar. Das Smartphone übernimmt die weitergehende Entwicklung von Persönlichkeit, Interessen und Beziehungen. Die genaue Datenhaltung wird separat festgelegt.

Die Aktionskoordination löst Konflikte zwischen gleichzeitig angeforderten Handlungen und steuert deren Ablauf. Jede ausführbare Aktion durchläuft die notwendigen Sicherheits-, Datenschutz- und Berechtigungsprüfungen.

Priorität haben physische Sicherheit und Datenschutz. Bewusste Benutzerentscheidungen und das Stoppen laufender Bewegungen haben Vorrang vor autonomem Verhalten und Persönlichkeitspräferenzen. Ein Benutzerwunsch darf eine Sicherheitsregel nicht ausser Kraft setzen.

### 4.2.1 Vorläufig abgestimmte Funktionszuordnung

Die folgende Aufteilung ist der gemeinsam abgestimmte Arbeitsstand. Sie wird bei der Erprobung überprüft; insbesondere der Umfang des lokalen Sprachverständnisses hängt von der verfügbaren Rechenleistung im Kopf ab.

| Grundverhalten im Kopf | Erweitertes Verhalten auf dem Smartphone |
| --- | --- |
| Mimik, Blinzeln und einfache Animationen | Kontextabhängige, komplexere Reaktionen |
| Berührung erkennen und unmittelbar reagieren | Persönlichkeit, Interessen und Beziehungen weiterentwickeln |
| Einfache lokale Sprachreaktionen | Gespräche und komplexeres Sprachverständnis |
| Nickbewegungen und grundlegende Bewegungsabläufe koordinieren | Übergreifende Abläufe und Disco-Choreografie planen |
| Auf Lage, Nähe und Hindernisse reagieren | Wahrnehmungen über längere Zeit zusammenführen |
| Akku, Laden und technische Zustände behandeln | Erinnerungen und Unterstützung planen |
| Sicherheitsregeln prüfen und Bewegungen stoppen | Externe KI-Dienste bei Bedarf einbinden |

Die Tabellenzeilen beschreiben zwei Aufgabenbereiche; sie stellen keine zwingenden paarweisen Abhängigkeiten dar. Bewegungen und Lichteffekte werden von den jeweils zuständigen Modulen ausgeführt. Das Smartphone kann beispielsweise einen Disco-Ablauf planen; der Kopf prüft und koordiniert die Aufträge, während die Homestation Drehung und Leuchtring ausführt.

Eine einfache Persönlichkeit muss auch ohne Smartphone wahrnehmbar bleiben. Erweiterte Planung und Gespräche sind bei fehlender Smartphone-Verbindung eingeschränkt; unmittelbare Reaktionen, Sicherheit und Energiemanagement bleiben lokal verfügbar.

### 4.3 Ausdruck und Aktionen

Ausdrucksfunktionen koordinieren Mimik, Sprache und Bewegung. Ein technischer Zustand kann als Teil der Persönlichkeit dargestellt werden, bleibt aber im technischen Status eindeutig erkennbar.

Module erhalten begrenzte, freigegebene Aufträge. Sie melden Annahme, Fortschritt, Abschluss oder Fehler. Sicherheitskritische Module müssen auch bei ausbleibender Kernkommunikation in einen definierten sicheren Zustand wechseln können.

### 4.4 Zustände und Energie

Eine lokale Zustandsverwaltung koordiniert Einrichtung, Normalbetrieb, Ruhe, Laden, eingeschränkten Betrieb, Fehler, Update, Lost Mode und Reset. Online-Verfügbarkeit und aktive Videoübertragung werden als zusätzliche Zustandsinformationen behandelt.

Das Energiemanagement bewertet Akku, Ladeverbindung und relevante Temperaturinformationen. Es reduziert Funktionen kontrolliert und fordert rechtzeitig Laden oder einen sicheren Ruhezustand an.

## 5. Physische Module und Stationen

Kopf, Bauch und Beinmodule bieten Fähigkeiten über definierte Schnittstellen an. Ihre konkrete Elektronik darf die fachliche Verhaltenslogik nicht bestimmen.

### 5.1 Kopf

Der Kopf enthält:

- Prozessor für das Grundverhalten und lokale Koordination;
- Speicher;
- Anzeige für das Gesicht;
- Mikrofone und Lautsprecher;
- Nickmechanik mit Motor;
- Lagesensor;
- Kamera;
- Touchsensor;
- Antennen;
- gegebenenfalls einen RFID-Leser.

Der Kopf beherbergt den lokalen Robin-Kern und verantwortet die Anbindung seiner Wahrnehmungs- und Ausdrucksfunktionen. Der RFID-Leser ist eine optionale Ausstattung; sein Einsatzzweck ist noch offen.

### 5.2 Bauch

Der Bauch enthält:

- Akku;
- Ladeelektronik;
- Radar;
- Status-LED;
- gegebenenfalls IR.

Der Bauch verantwortet Energieversorgung und Laden und stellt Radarereignisse sowie seinen Status über definierte Schnittstellen bereit. Die konkrete Funktion der optionalen IR-Ausstattung bleibt offen. Die bisher geforderte Kommunikation des Bauchs mit Kopf und Beinmodulen bleibt erhalten; ihre technische Ausführung wird später festgelegt.

### 5.3 Beine und Homestation

Beinmodule verantworten ihre Bewegungssteuerung und lokalen Schutzreaktionen. Der Drehantrieb, der den angedockten Robin um 360° dreht, gehört zur Homestation.

Die Ladestation stellt die Ladeverbindung bereit. Die Zuständigkeit für Ladefreigabe, Akkuüberwachung und Abbruch bei unsicheren Bedingungen muss zwischen Bauch und Station eindeutig festgelegt werden.

Die Homestation übernimmt die Rolle der Ladestation und stellt Robins definierten Aufenthalts- und Ruheort bereit. Die Station besitzt einen eigenen Drehantrieb und dreht den angedockten Robin um 360°. Die Station verantwortet die Ausführung und lokale Absicherung dieser Drehbewegung; der Robin-Kern koordiniert freigegebene Drehaufträge. Die Ladeverbindung muss während der Drehung erhalten bleiben. Ob beliebig viele volle Umdrehungen möglich sind, bleibt offen.

Die Homestation besitzt einen Leuchtring. Er stellt System- und Ladezustände dar und ermöglicht Lichteffekte im Disco-Modus. Der Robin-Kern koordiniert gewünschte Anzeigen und Effekte; die Station verantwortet deren lokale Ausgabe und meldet ihren tatsächlichen Zustand. Sicherheitsrelevante Zustandsanzeigen haben Vorrang vor Disco-Effekten.

Die Station bietet Lade-, Licht- und Drehfähigkeiten über definierte Schnittstellen an. Bei Kommunikationsausfall bleibt der Ladebetrieb lokal abgesichert; zustandsabhängige Anzeigen dürfen keinen nicht bestätigten Normalzustand vortäuschen. Zusätzliche Rechen- oder Sicherungsdienste gehören nicht zum derzeit festgelegten Umfang.

## 6. Bedienoberflächen und externe Dienste

### 6.1 Smartphone und Companion-App

Die App führt durch Ersteinrichtung, bewusste Kopplung und Besitzerzuordnung. Sie verwaltet berechtigte Zugänge, Einstellungen, Personenregistrierung, Kontaktverknüpfungen, Erinnerungen und Datenschutzentscheidungen.

Sie zeigt Zustand, Einschränkungen und Diagnose an und unterstützt Updates, Lost Mode, Reset und Weitergabe. Sie übermittelt Benutzerentscheidungen an die jeweils zuständige Systemkomponente und zeigt erst nach Bestätigung den tatsächlichen Systemstand.

Zusätzlich führt das Smartphone das erweiterte Verhalten aus. Dazu verarbeitet es die für die jeweilige Funktion notwendigen und freigegebenen Informationen und übermittelt daraus abgeleitete Vorschläge oder Aufträge an den Robin-Kern. Der lokale Kern prüft ausführbare Aktionen weiterhin auf Sicherheit, Datenschutz, Berechtigungen, Aktualität und verfügbare Fähigkeiten.

Ohne Smartphone oder bei unterbrochener Verbindung bleibt das Grundverhalten im Kopf verfügbar. Erweitertes Verhalten wird als nicht verfügbar oder eingeschränkt angezeigt. Die vorläufig abgestimmte Funktionszuordnung steht in Abschnitt 4.2.1. Erweitertes Verhalten auf dem Smartphone bedeutet nicht automatisch eine Abhängigkeit von Internet oder Cloud.

### 6.2 Weboberfläche

Die Weboberfläche ermöglicht die im Lastenheft vorgesehenen Status-, Einstellungs-, Firmware- und Livestream-Funktionen. Sie verwendet dieselben fachlichen Berechtigungen und Regeln wie die App.

Ob sie lokal auf Robin oder über den Server bereitgestellt wird, bleibt offen. Ein Fernzugang darf nicht allein deshalb Steuerrechte erhalten, weil eine Verbindung besteht.

### 6.3 Server und externe KI

Der Server stellt Geräteverwaltung, Updatebereitstellung, Konfigurationsdienste, eine Programmierschnittstelle und geschützte Fernfunktionen bereit.

Externe KI wird für klar abgegrenzte Aufgaben aufgerufen. Nur die dafür notwendigen und freigegebenen Daten werden übertragen. Ergebnisse besitzen keine unmittelbare Befugnis, Bewegungen, Registrierung, Livestreams oder dauerhafte Datenspeicherung auszulösen.

Für externe Aufgaben werden Zeitgrenzen, Abbruch und ein verständlicher Ersatz bei Nichtverfügbarkeit vorgesehen. Antworten auf bereits abgebrochene oder überholte Aufgaben werden nicht nachträglich als aktuelle Handlungsaufträge ausgeführt.

## 7. Datenhaltung und Zuständigkeit

| Datenart | Führende Instanz im Entwurf | Umgang |
| --- | --- | --- |
| Aktueller Kontext und unbekannte Personen | Robin-Kern | Kurzfristig; begrenzte Lebensdauer; keine automatische Registrierung |
| Registrierte Personen und Beziehungen | Lokale Datenverwaltung des Robin-Kerns | Bewusste Registrierung; einsehbar, korrigierbar und löschbar |
| Persönlichkeit und Präferenzen | Einfache Ausprägung im Robin-Kern; Weiterentwicklung auf dem Smartphone | Datenhaltung und Abgleich noch offen; Grundpersönlichkeit bleibt ohne Smartphone verfügbar |
| Erinnerungen und persönliche Einstellungen | Robin-Kern | Lokal verfügbar; App dient der Verwaltung |
| Ownership und Geräteberechtigungen | Lokale Zugriffsverwaltung | Geschützt; Kopplungen bewusst erteilen und widerrufen |
| Technischer Zustand | Zuständiges Modul, zusammengeführt im Kern | Aktuell; mit Quelle und Verfügbarkeit |
| Diagnoseereignisse | Erzeugende Komponente | Datenarm, zugriffsgeschützt und zeitlich begrenzt |
| Verfügbare Updatepakete | Updatebereitstellung des Servers | Herkunft, Unversehrtheit und Kompatibilität lokal prüfen |
| Gewünschte serverseitige Konfiguration | Server | Änderungsvorschlag; lokale Annahme und Bestätigung erforderlich |

Für persönliche Roboterinformationen ist der lokale Stand führend. Eine optionale Synchronisation oder Sicherung auf einem Server muss bewusst eingerichtet werden. Sie benötigt Regeln für Konflikte, Löschung, Wiederherstellung und Zugriff; sie ist in diesem Entwurf noch nicht festgelegt.

Kontaktverknüpfungen werden bewusst in der App vorgenommen. Daraus folgt keine pauschale Übertragung des gesamten Adressbuchs an Robin oder einen Server.

Ein vollständiger Reset entfernt persönliche Daten, Zugangsinformationen und Kopplungen. Falls später externe Kopien eingeführt werden, muss deren Löschung oder Trennung im Ablauf ebenfalls definiert werden.

## 8. Kommunikationswege und Robin Protocol

| Verbindung | Ausgetauschte Informationen |
| --- | --- |
| Kopf / Bauch / Beine ↔ Robin-Kern | Fähigkeiten, Beobachtungen, Status, freigegebene Aktionen und Fehler |
| Smartphone / Companion-App ↔ Robin | Einrichtung, Benutzerentscheidungen, Konfiguration, Personenverwaltung, freigegebener Kontext für erweitertes Verhalten, Vorschläge, Aufträge und Status |
| Robin ↔ Homestation | Lade- und Andockstatus, Leuchtring-Anzeigen, Disco-Effekte und Drehaufträge samt Rückmeldungen |
| Robin ↔ Server | Berechtigte Geräteverwaltung, Updates und freigegebene externe Aufgaben |
| Weboberfläche ↔ zuständiger lokaler oder externer Dienst | Berechtigte Bedien- und Statusfunktionen |
| Simulator ↔ Robin-Kern | Dieselben fachlichen Fähigkeiten, Ereignisse und Aktionen wie reale Adapter |

Diese Verbindungen beschreiben fachliche Beziehungen. Sie legen keine konkrete Netzwerktopologie fest. Eine Weiterleitung über Bauch, Station oder Server muss die ursprüngliche Berechtigung und den Auftrag erhalten.

Das Robin Protocol definiert gemeinsame Bedeutungen für Fähigkeiten, Befehle, Ereignisse, Zustände, Konfiguration und Fehler. Für interne Modulverbindungen dürfen angepasste Transportwege verwendet werden, solange die fachliche Bedeutung erhalten bleibt.

Für die anschliessende Protokolldefinition werden mindestens benötigt:

- Identität, Rolle, Protokollversion und Fähigkeiten der Beteiligten;
- Zuordnung von Auftrag, Bestätigung, Ergebnis und Fehler;
- Aktualität, Gültigkeitsdauer und Abbruch von Aufträgen;
- Verhalten bei Duplikaten, Verbindungsabbruch und Wiederverbindung;
- Berechtigungen und Ablehnungsgründe;
- Meldung von Zustands- und Fähigkeitsänderungen.

Konkrete Nachrichtenformate werden erst im Protokolldokument festgelegt.

## 9. Typische Abläufe

### 9.1 Lokale Interaktion

Eine Wahrnehmung erzeugt eine Beobachtung. Der Kern aktualisiert den Kontext, wählt eine Reaktion und prüft diese gegen Regeln und Zustand. Die zuständigen Module führen freigegebene Aktionen aus und melden Ergebnisse zurück.

### 9.2 Komplexe externe Aufgabe

Der Kern prüft Verfügbarkeit und erlaubte Datenverwendung, beauftragt einen externen Dienst und wartet innerhalb einer definierten Zeitgrenze. Das Ergebnis wird als Information oder Vorschlag verarbeitet. Jede daraus abgeleitete Aktion wird erneut lokal geprüft.

### 9.3 Personenregistrierung

Der Benutzer startet die Registrierung bewusst über die App. Robin führt den vorgesehenen lokalen Erfassungsablauf durch und bestätigt den gespeicherten Eintrag. Eine Kontaktverknüpfung ist eine separate bewusste Entscheidung.

### 9.4 Livestream

Ein berechtigter Benutzer fordert Videoübertragung an. Robin prüft die definierten Bedingungen und aktiviert eine lokal sichtbare Anzeige, bevor Bilddaten übertragen werden. Kann die Anzeige nicht sichergestellt werden, wird die Übertragung abgelehnt oder beendet. Eine verlorene Bedienverbindung darf keinen unbegrenzt unbeaufsichtigten Stream hinterlassen; die Beendigungsbedingungen werden im Protokoll konkretisiert.

### 9.5 Update

Die lokale Updatekoordination prüft Paket, Kompatibilität, Energie und Betriebszustand. Robin nimmt einen sicheren Zustand ein, führt das Update aus und bestätigt anschliessend seine Betriebsfähigkeit. Bei Fehlern bleibt ein funktionsfähiger Stand oder Wiederherstellungsmodus verfügbar.

### 9.6 Robin finden

„Robin finden“ ist eine lokale Suchfunktion zwischen Robin und seinem gekoppelten Smartphone. Robin stellt ein energiesparendes, datenschutzgerechtes Suchsignal bereit. Die App erkennt das eigene Gerät und unterscheidet bestätigte Erreichbarkeit, letzte Verbindung und aktuell unbekannten Zustand.

Das Suchsignal allein erlaubt keine Steuerung. Nach geschütztem Verbindungsaufbau prüft der Robin-Kern einen Suchhilfeauftrag und koordiniert eine zeitlich begrenzte Ton- oder Lichtausgabe. Ohne aktive Smartphone-Verbindung muss Robin im Ruhe- und Lost Mode weiterhin auffindbar bleiben, soweit seine Energiereserven dies erlauben.

Robins Lautsprecher und Gesichtsanzeige sind mögliche Suchhilfen; weitere lokale Anzeigen werden nur bei gemeldeter Fähigkeit verwendet. Der Leuchtring der Homestation kann ergänzen, ersetzt aber keine Suchhilfe an einem Robin ausserhalb der Station.

Die Suche benötigt weder Internet noch fremde Geräte oder ein Suchnetzwerk. Genaue Entfernung, Richtung und Kartenortung sind nicht festgelegt. Die App kann lokal den Zeitpunkt der letzten bestätigten Verbindung halten; dies ist kein aktueller Standortnachweis.

Suchsignal, Energieprofil, erreichbare Reichweite und geschützte Geräteerkennung müssen vor Umsetzung konkretisiert werden. Der Suchauftrag hebt den Lost Mode nicht auf.

## 10. Betrieb bei Ausfällen

| Ausfall | Erwartetes Verhalten |
| --- | --- |
| Internet oder Server nicht verfügbar | Lokale Interaktion, Sicherheit und Energiemanagement bleiben verfügbar; externe Aufgaben melden Einschränkung |
| Smartphone nicht verbunden oder erweitertes Verhalten nicht verfügbar | Grundverhalten im Kopf bleibt erhalten; laufende Smartphone-Aufträge werden definiert beendet oder eingeschränkt; veraltete Aufträge werden nicht nachträglich ausgeführt |
| Homestation nicht verfügbar | Mobiler Grundbetrieb bleibt erhalten; Laden, Stationslicht und stationsgebundene Drehfähigkeit melden Nichtverfügbarkeit |
| Sensor oder Modul ausgefallen | Fähigkeit wird eingeschränkt; betroffene Aktionen werden begrenzt oder gestoppt |
| Robin-Kern antwortet nicht | Sicherheitskritische Module wechseln anhand lokaler Regeln in einen sicheren Zustand |
| Energie kritisch | Kontrollierte Funktionsreduktion und sicherer Zustand |
| Update fehlgeschlagen | Rückkehr zum funktionsfähigen Stand oder Wiederherstellung |
| Verbindung wiederhergestellt | Zustände und Berechtigungen abgleichen; veraltete Aufträge nicht automatisch nachholen |

## 11. Virtual Robin und Diagnose

Virtual Robin führt möglichst denselben Kern aus wie der physische Roboter. Simulierte Adapter ersetzen Wahrnehmung, Ausdruck, Bewegung und Energieversorgung. Testwerkzeuge können Ereignisse, Fehler und Verbindungszustände reproduzierbar erzeugen.

Ein Smartphone-Simulator kann seine vorhandenen Fähigkeiten anbieten und fehlende Robotermodule simulieren. Die genaue Nutzung von Kamera, Audio und Bewegungssensorik wird später festgelegt.

Diagnose verbindet Beobachtungen, Entscheidungen, freigegebene Aktionen und Ergebnisse nachvollziehbar. Persönliche Inhalte und Zugangsinformationen werden dabei minimiert und geschützt. Benutzerverständliche Erklärungen bleiben getrennt von technischen Diagnoseinformationen.

Simulation prüft fachliche Abläufe und Regeln. Lade-, Dreh- und Leuchtring-Funktionen der Homestation werden ebenfalls simuliert. Physische Sicherheit, Ladebetrieb und reale Bewegungen benötigen ergänzende Hardwareprüfungen.

## 12. Offene Entscheidungen und nächster Schritt

Vor der verbindlichen Umsetzung sind insbesondere zu klären:

1. Detaillierung der vorläufig abgestimmten Funktionszuordnung: Umfang lokaler Sprachreaktionen, Datenhaltung und Abgleich mit dem Smartphone sowie lokale Schutzverantwortung jedes Moduls.
2. Mechanische Umsetzung des stationsseitigen Drehantriebs, Erhalt der Ladeverbindung und mögliche Begrenzung auf eine volle Umdrehung beziehungsweise endlose Rotation.
3. Betrieb und Zugangsweg der Weboberfläche sowie Umfang der Fernsteuerung.
4. Umfang persönlicher Langzeitdaten, Aufbewahrungszeiten und optionale Synchronisation.
5. Rechte weiterer Benutzer sowie Wiederherstellung der Ownership bei Verlust eines gekoppelten Geräts.
6. Messbare Zielwerte für Reaktionszeiten, Energie und sichere Kommunikationsausfälle.
7. Kleinster erster Entwicklungsumfang für Virtual Robin.

Als erster Entwicklungsumfang wird vorgeschlagen: Fähigkeiten melden, Wahrnehmungsereignis verarbeiten, Kontext aktualisieren, eine begrenzte Reaktion wählen und ausführen, Zustand anzeigen sowie Verbindungs- und Energieausfälle simulieren.

Nach Abstimmung der Zuständigkeiten folgt das Robin Protocol. Dabei wird zunächst der gemeinsame fachliche Vertrag für diesen ersten Ablauf definiert und anschliessend um Personenverwaltung, externe Aufgaben, Updates und Fernfunktionen erweitert.
