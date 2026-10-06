# Lastenheft: Pink Robin

## 1. Zweck und Projektphilosophie

Pink Robin ist eine modular aufgebaute Robotikplattform für einen sozialen Begleitroboter. Robin soll durch Mimik, Sprache und Bewegung natürlich mit Menschen interagieren, lokal schnell reagieren und komplexe Aufgaben bei Bedarf über ein externes KI-System lösen können.

Ziel ist der Aufbau einer langfristig wartbaren und erweiterbaren Plattform. Sie dient zugleich als Lernplattform für eingebettete Systeme, Softwareentwicklung, Elektronik, Mechanik und künstliche Intelligenz. Eine möglichst schnelle Fertigstellung hat keinen Vorrang vor Wartbarkeit, Reparierbarkeit und klaren Schnittstellen.

Hardware und Software müssen klar getrennt sein. Komponenten sollen austauschbar sein und definierte Schnittstellen besitzen. Standardkomponenten werden bevorzugt, um Wartbarkeit, Reparaturfähigkeit und Kosten zu optimieren.

Die [Robin Principles](Robin-Principles.md) sind die verbindliche Leitlinie für Architektur, Hardware, Software und Verhalten. Neue oder geänderte Anforderungen müssen gegen diese Principles geprüft werden. Konflikte müssen vor der Umsetzung bewusst geklärt werden.

## 2. Verbindlichkeit und Abgrenzung

Dieses Lastenheft beschreibt die benötigten Eigenschaften und Funktionen des Gesamtsystems. Konkrete Implementierungstechnologien, Datenformate, Betriebssysteme, Entwicklungsframeworks, Prozessoren und Sensorchips werden in nachgelagerten Architektur- und Umsetzungskonzepten festgelegt.

- **Muss** bezeichnet eine verbindliche Anforderung.
- **Soll** bezeichnet ein angestrebtes Ziel; Abweichungen müssen begründet und dokumentiert werden.
- **Kann** bezeichnet eine optionale Erweiterung.

Bereits vorhandene funktionale Anforderungen werden durch diese Überarbeitung erhalten und um systemweite Anforderungen ergänzt. Die bestehende Vorgabe zu elektrischen Kontakten der Beinmodule bleibt bestehen.

## 3. Projektziele und Nicht-Ziele

Robin soll:

- sozial und sympathisch wirken;
- modular aufgebaut, wartbar, reparierbar und erweiterbar sein;
- überwiegend aus Standardkomponenten bestehen;
- möglichst geringe Entwicklungskosten verursachen;
- langfristig weiterentwickelt werden können;
- schnell und verständlich auf Menschen und seine Umgebung reagieren;
- wesentliche Grundfunktionen ohne Internetverbindung bereitstellen.

Robin soll kein vollwertiger Haushaltsroboter sein, Menschen ersetzen oder industrielle Robotik nachbilden. Das Gesamtsystem darf nicht ausschliesslich cloudbasiert funktionieren.

## 4. Systemumfang

Das Gesamtsystem umfasst:

- Roboterkopf;
- Roboterbauch;
- austauschbare Beinmodule;
- Homestation mit Ladefunktion, 360°-Drehmöglichkeit und Leuchtring;
- Server und gegebenenfalls externe KI-Dienste;
- Webanwendung;
- mobile Companion-App;
- Virtual Robin und weitere Simulatoren als Entwicklungswerkzeuge.

Für jede Komponente müssen Verantwortlichkeiten, angebotene Fähigkeiten, benötigte Verbindungen und Verhalten bei Ausfall beschrieben werden. Die Homestation übernimmt hauptsächlich die Funktion der Ladestation.

## 5. Interaktion, Verhalten und Persönlichkeit

### 5.1 Soziale Interaktion

Robin muss Emotionen durch Mimik darstellen, Sprache aufnehmen und wiedergeben, Berührungen erkennen und Kopfbewegungen ausführen können. Robin muss lokal auf Sprache reagieren können. Das Grundverhalten muss auf dem Prozessor im Kopf ausgeführt werden; erweitertes Verhalten wird auf dem Smartphone ausgeführt. Ohne verbundenes Smartphone muss das Grundverhalten erhalten bleiben.

Robin soll Mimik, Sprache und Bewegung zu einem konsistenten, situationsgerechten Verhalten verbinden. Rückmeldungen sollen verständlich sein und auch Einschränkungen oder nicht verfügbare Funktionen vermitteln.

### 5.2 Verhalten und Kontext

Die Verhaltenssteuerung muss Wahrnehmung, aktuellen Kontext, Benutzerentscheidungen, Systemzustand und Sicherheitsregeln berücksichtigen. Gleichzeitige oder widersprüchliche Handlungswünsche müssen nach nachvollziehbaren Prioritäten aufgelöst werden.

Robin soll Erinnerungen, Vorschläge und Unterstützung anbieten können. Er darf Menschen dabei nicht bewerten oder bevormunden. Unterstützungsfunktionen müssen jederzeit temporär übersteuert oder deaktiviert werden können.

Robin soll für Menschen verständlich erklären können, warum er gehandelt, einen Vorschlag gemacht oder Unterstützung angeboten hat. Technische Diagnoseinformationen müssen getrennt verfügbar bleiben.

### 5.3 Persönlichkeit und Beziehungen

Robin soll eine konsistente Persönlichkeit ausdrücken und Interessen, Präferenzen und Beziehungen zu bewusst registrierten Personen entwickeln können. Der Benutzer muss beeinflussen können, welche persönlichen Informationen dafür genutzt und gespeichert werden.

Persönlichkeit, Interessen und Beziehungen dürfen Sicherheitsregeln, Datenschutzvorgaben und bewusste Benutzerentscheidungen niemals überstimmen.

Technische Zustände sollen sozial verständlich ausgedrückt werden können, beispielsweise niedriger Akkustand als Müdigkeit. Die tatsächliche technische Ursache muss für Statusanzeige und Diagnose eindeutig verfügbar bleiben.

## 6. Wahrnehmung

Robin muss Personen und Berührungen erkennen können. Er soll relevante akustische und visuelle Ereignisse, Nähe und Hindernisse sowie seine Lage und Bewegung erfassen können, soweit dies für Interaktion und sicheren Betrieb erforderlich ist.

Das Wahrnehmungskonzept muss die benötigten Fähigkeiten beschreiben, ohne konkrete Sensorchips festzuschreiben. Sensordaten müssen über abstrahierte Schnittstellen bereitgestellt werden.

Wahrnehmung muss Unsicherheit, fehlende Daten und ausgefallene Sensoren berücksichtigen. Unsichere Erkennung darf nicht als sichere Identifikation behandelt werden. Sicherheitsrelevante Handlungen müssen bei unzureichender Wahrnehmung eingeschränkt oder unterbunden werden.

Unbekannte Personen dürfen für die aktuelle Interaktion wahrgenommen und unterschieden werden. Informationen über sie dürfen ohne bewusste Registrierung nicht dauerhaft gespeichert werden. Wiederholte Erkennung allein darf keine Registrierung auslösen.

### 6.1 Bilderkennung

Robin muss Kameraaufnahmen für klar definierte Wahrnehmungsaufgaben verarbeiten können. Bilderkennung soll Objekte, relevante Szenenmerkmale und bewusst registrierte Personen unterscheiden können. Ergebnisse müssen Unsicherheit berücksichtigen; Erkennen einer Person und Identifizieren einer registrierten Person sind getrennte Funktionen.

Erweiterte Bilderkennung soll über das gekoppelte Smartphone erfolgen. Welche grundlegenden Funktionen im Kopf laufen, wird anhand seiner Ressourcen festgelegt. Bildübertragung benötigt eine freigegebene Aufgabe, einen begrenzten Umfang und geschützten Zugriff. Sie darf nicht als verdeckter Livestream verwendet werden. Unbekannte Personen dürfen dadurch nicht dauerhaft registriert werden.

### 6.2 Audioerkennung und STT

Robin muss Audio über seine Mikrofone erfassen können. Er soll Sprachaktivität und relevante Geräusche erkennen sowie gesprochene Sprache mittels Speech-to-Text (STT) in Text umwandeln können. Einfache lokale Sprachreaktionen bleiben Teil des Grundverhaltens; umfangreicheres Sprachverständnis und erweiterte Verarbeitung erfolgen auf dem Smartphone.

Aufnahme, Übertragung und Verarbeitung müssen an einen definierten Zweck und eine Freigabe gebunden sein. Aufnahmezustände müssen für den Benutzer verständlich erkennbar sein. Rohaufnahmen und Transkripte dürfen nicht ohne festgelegten Zweck dauerhaft gespeichert werden.

Ein Transkript ist eine unsichere Wahrnehmung und kein Identitäts- oder Berechtigungsnachweis. Sicherheitsrelevante Aktionen dürfen nicht allein aufgrund eines vermeintlich erkannten Sprachbefehls freigegeben werden.

### 6.3 TTS und Audioausgabe

Robin muss Text-to-Speech (TTS) für die Sprachausgabe unterstützen und Sprache über die Lautsprecher im Kopf wiedergeben können. Erweiterte Sprachsynthese soll auf dem Smartphone erfolgen; einfache lokale Sprachreaktionen müssen ohne Smartphone erhalten bleiben. Ob diese lokal synthetisiert oder als vorbereitete Ausgabe bereitgestellt werden, bleibt offen.

Sprachausgabe muss Lautstärke, begrenzte Dauer, Abbruch und Rückmeldungen unterstützen. Erzeugte Audiodaten und tatsächlich abgespielte Sprache müssen unterschieden werden. Prioritäten zwischen Sprache, Suchton und notwendigen Hinweisen müssen festgelegt werden.

## 7. Datenschutz und Sicherheit

### 7.1 Datenverarbeitung und Kontrolle

Kamera, Mikrofone und andere Wahrnehmungsfunktionen müssen Robins Interaktion und klar definierten Funktionen dienen. Robin darf nicht zur unnötigen Überwachung von Menschen eingesetzt werden.

Daten sollen lokal verarbeitet werden, soweit dies sinnvoll möglich ist. Server- oder Cloudverarbeitung muss einen klaren funktionalen Vorteil haben oder durch begrenzte lokale Ressourcen begründet sein.

Für personenbezogene Daten müssen Zweck, Speicherort, Aufbewahrung und Löschung nachvollziehbar beschrieben werden. Es dürfen nur notwendige Daten verarbeitet und gespeichert werden. Der Benutzer muss gespeicherte Personeninformationen und Erinnerungen einsehen, korrigieren und löschen können.

Personen müssen bewusst registriert werden. Verknüpfungen mit Kontakten oder anderen persönlichen Informationen müssen bewusst durch den Benutzer erfolgen und dürfen nicht automatisch aufgrund von Vermutungen hergestellt werden.

### 7.2 Zugriffsschutz

Zugriffe auf Steuerung, Konfiguration, personenbezogene Daten, Diagnose und Updates müssen durch angemessene Authentifizierung und Berechtigungen geschützt werden. Sicherheitsrelevante Kommunikation und gespeicherte sensible Daten müssen gegen unbefugten Zugriff und Manipulation geschützt sein.

Nicht autorisierte Steuerbefehle müssen abgewiesen werden. Ausfälle externer Dienste oder Verbindungen dürfen weder Zugriffsschutz noch Datenschutzvorgaben ausser Kraft setzen.

### 7.3 Livestream und Lost Mode

Der Server soll Livestreams bereitstellen und die Weboberfläche soll sie anzeigen können. Videoübertragung darf nur unter vorher definierten, nachvollziehbaren Bedingungen und mit geeignetem Zugriffsschutz aktiviert werden.

Eine aktive Videoübertragung muss für anwesende Personen unmittelbar an Robin erkennbar sein. Eine Anzeige ausschliesslich in einer entfernten App genügt nicht. Unbemerkte Aktivierung muss verhindert werden.

Robin muss einen Lost Mode unterstützen, der persönliche Daten schützt und Energie schont. Funktionen müssen dabei auf Wiederfinden, Sicherheit und notwendige Kommunikation reduziert werden. Aktivierung, Rückkehr in den Normalbetrieb und erlaubte Zugriffe müssen klar definiert und geschützt sein.

### 7.4 Robin finden

Robin muss eine eigene Funktion „Robin finden“ zum Wiederfinden in der Nähe über das eigene, gekoppelte Smartphone bereitstellen. Die App muss Robins energiesparendes Suchsignal erkennen und nach berechtigtem Verbindungsaufbau eine begrenzte Suchhilfe durch Ton und/oder Licht anfordern können.

Die Suche muss ohne Internet und ohne fremdes Suchnetzwerk funktionieren. Das Suchsignal darf unberechtigten Geräten keine persönlichen Daten oder dauerhaft öffentlich verfolgbare Besitzer- oder Roboterkennung offenlegen. Die technische Umsetzung bleibt offen.

Die App muss aktuelle Erreichbarkeit von der zuletzt bestätigten Verbindung unterscheiden. Ausserhalb der Reichweite darf sie keinen aktuellen Standort behaupten. Eine genaue Entfernung oder Richtung gehört ohne geeignete technische Grundlage nicht zum festgelegten Umfang. Bei leerem Akku ist keine aktive Suche möglich.

Die Suchfunktion muss im Lost Mode verfügbar bleiben, soweit Energie und sichere Betriebsbedingungen dies erlauben. Sie kann auch unabhängig vom Lost Mode genutzt werden. Ein Suchauftrag darf weder den Lost Mode beenden noch Ownership oder Zugriffsrechte verändern.

### 7.5 Persönliche Daten und Offline-Bestand

Das Smartphone muss der führende Speicher für registrierte Personen, längerfristigen Beziehungskontext, Persönlichkeit und Erinnerungen sein. Der Kopf muss einen begrenzten freigegebenen Offline-Bestand halten, damit Grundpersönlichkeit, lokale Wiedererkennung und ausdrücklich übertragene Erinnerungen ohne Smartphone verfügbar bleiben.

Der Personenbestand im Kopf darf automatisch anhand bestätigter Begegnungshäufigkeit und Aktualität aktualisiert werden. Eine zusammenhängende Begegnung zählt einmal. Nur bewusst registrierte Personen dürfen aufgenommen werden; unsichere Identifikationen und unbekannte Personen dürfen keine dauerhaften Profile oder Begegnungshistorien erzeugen.

Bewusst angepinnte Personen müssen Vorrang haben. Das Ranking darf ausschliesslich die Datenhaltung steuern und weder Personen bewerten noch Berechtigungen oder Beziehungen festlegen. Die App muss Bestand und bewusste Übersteuerung ermöglichen.

Verdrängung aus dem Kopf darf das Smartphone-Profil nicht löschen. Eine bewusste Profil-Löschung muss dagegen auch lokale Kopien und zugehörige Begegnungsdaten entfernen. Bei nicht erreichbarem Kopf muss der noch ausstehende Abgleich erkennbar bleiben.

Abgleich muss doppelte Begegnungszählung und Wiederherstellung gelöschter Profile verhindern. Umfang, Gewichtung, Kapazität und Datenlebensdauer müssen festgelegt werden. Persönliche Daten werden standardmässig nicht auf dem Server gespeichert; Sicherung benötigt eine separate bewusste Freigabe.

### 7.6 Personenprofile

Der erste abgestimmte Profilumfang umfasst interne Personenkennung, Anzeigename, optionale Aussprache und Anrede, bewusst erfasste Wiedererkennungsmerkmale, optionale Referenzfotos, bewusst angelegte Kontaktverknüpfungen, gewünschte Sprache, ausdrücklich mitgeteilte Vorlieben und Grenzen sowie bewusst gespeicherte gemeinsame Erinnerungen.

Speicherverwaltung muss Begegnungszähler, letzte bestätigte Begegnung, Anheftung, Freigaben und Versionsstand getrennt von persönlichen Eigenschaften halten. Das Ranking darf nicht als Beziehungsbewertung verwendet werden.

Für persönliche Angaben müssen Herkunft, Bestätigungsstatus und erlaubte Nutzung nachvollziehbar sein. Vermutungen aus Gesprächen dürfen nicht automatisch als bestätigte Tatsachen gespeichert werden. Automatisch abgeleitete Personenbewertungen wie Zuverlässigkeit, Intelligenz oder psychischer Zustand gehören nicht zum Profilumfang.

Der Offline-Bestand im Kopf muss auf die für Wiedererkennung und freigegebene lokale Interaktion notwendigen Angaben beschränkt sein. Kontaktverknüpfungen verbleiben standardmässig auf dem Smartphone; gemeinsame Erinnerungen werden nur als freigegebene Auswahl übertragen.

Zugriffsregeln müssen unterscheiden, welche registrierte Person welche Informationen sehen oder ändern darf. Erkennen einer Person allein gewährt keine solchen Rechte. Das konkrete Verfahren zur sicheren Bestätigung berechtigter Personen wird separat festgelegt.

### 7.7 Themenliste pro Person und Speicherfilter

Für bewusst registrierte Personen darf Robin eine begrenzte Stichwort- und Themenliste aus Gesprächen automatisch aktualisieren, wenn diese Speicherregel für die jeweilige Person freigegeben ist. Ein Eintrag enthält Thema, kurze Notiz, Herkunft und letzte Erwähnung. Gesprochenes Thema und ausdrücklich mitgeteilte persönliche Tatsache müssen unterschieden werden; Vermutungen dürfen keine bestätigten Angaben erzeugen.

Die vollständige Liste liegt auf dem Smartphone. Der Kopf erhält nur eine kleine freigegebene Auswahl. Wiederkehrende Themen können stärker gewichtet werden; ältere verlieren Gewicht. Die Liste muss einsehbar, korrigierbar und löschbar sein. Robin darf freigegebene Themen für eigene Gesprächseinstiege verwenden.

Gesundheits- und psychische Themen dürfen nicht dauerhaft in personenbezogenen Themenlisten, Erinnerungen, Profilnotizen oder daraus abgeleiteten Daten gespeichert werden. Der Ausschluss gilt auch bei ausdrücklicher Erwähnung oder Bestätigung und darf nicht durch andere Speicherwege umgangen werden.

Ein Filter muss vor jeder dauerhaften Übernahme greifen und Stichwörter, Zusammenfassungen, indirekte Formulierungen und abgeleitete Angaben berücksichtigen. Bei unklarer Einordnung darf nicht gespeichert werden. Gefilterte Inhalte dürfen keine dauerhaften Themengewichte, Rankingmerkmale oder späteren Gesprächseinstiege erzeugen.

Robin darf solche Inhalte im aktuellen Gespräch verarbeiten, soweit sie dafür erforderlich sind. Sie dürfen nicht in dauerhafte Gesprächsarchive, Diagnoseinhalte, Sicherungen oder den Offline-Bestand gelangen. Zweckgebundene temporäre Verarbeitung muss begrenzt sein und anschliessend verworfen werden.

## 8. Ownership, Pairing und Lebenszyklus

Robin muss eine eindeutige Zuordnung zu einem berechtigten Besitzer unterstützen. Ersteinrichtung und Pairing mit der Companion-App müssen bewusst erfolgen und unbefugte Übernahme verhindern.

Weitere berechtigte Geräte oder Benutzer sollen verwaltet und ihre Berechtigungen entzogen werden können. Das Entkoppeln eines Geräts muss dessen künftigen Zugriff wirksam beenden.

Neustart, Zurücksetzen von Einstellungen und vollständiges Zurücksetzen müssen klar unterschieden werden. Ein vollständiger Reset muss persönliche Daten, gespeicherte Zugangsinformationen und bestehende Kopplungen entfernen sowie einen definierten Einrichtungszustand herstellen.

Für Weitergabe oder Besitzerwechsel muss ein nachvollziehbarer Ablauf bestehen, der persönliche Daten und Zugriffsrechte des bisherigen Besitzers entfernt und eine bewusste Neueinrichtung ermöglicht.

## 9. Kommunikation und modulare Schnittstellen

### 9.1 Robin Protocol

Das Gesamtsystem muss ein dokumentiertes, hardwareunabhängiges Robin Protocol für die Kommunikation zwischen Robin, Companion-App, Simulatoren und gegebenenfalls Homestation und Server bereitstellen.

Das Protokoll muss Fähigkeiten, Befehle, Ereignisse, Statusinformationen, Konfiguration und Fehler nachvollziehbar beschreiben. Bedeutung und Verhalten der Nachrichten müssen unabhängig von konkreter Hardware und Übertragungsweg sein.

Protokollversionen und Fähigkeiten müssen erkennbar sein. Inkompatible Komponenten müssen erkannt und verständlich gemeldet werden. Erweiterungen sollen bestehende Funktionen möglichst kompatibel erhalten.

Verbindungsabbruch, Wiederverbindung, verzögerte oder mehrfach empfangene Nachrichten und ungültige Befehle müssen definiert behandelt werden. Veraltete Steuerbefehle dürfen nach einer Wiederverbindung keine unbeabsichtigten Handlungen auslösen.

### 9.2 Abstraktion und Austauschbarkeit

Wahrnehmung, Bewegung, Energieversorgung, Ausdruck, Verhaltenssteuerung, Speicherung und Kommunikation müssen über definierte, modulare Schnittstellen zusammenarbeiten.

Hardwareabhängige Funktionen müssen von der allgemeinen System- und Verhaltenslogik getrennt sein. Der Austausch eines Prozessors, Sensors, Aktors oder Moduls soll mit möglichst geringem Aufwand in der übrigen Software möglich sein.

Komponenten müssen ihre verfügbaren Fähigkeiten und Einschränkungen melden können. Simulatoren und reale Komponenten sollen dieselben fachlichen Schnittstellen verwenden.

## 10. Anforderungen an die Komponenten

### 10.1 Kopf

Der Kopf muss:

- Emotionen darstellen können;
- Sprache wiedergeben und aufnehmen können;
- Personen und Berührungen erkennen können;
- Kopfbewegungen ausführen können;
- lokal auf Sprache reagieren können;
- seine Fähigkeiten und Zustände über definierte Schnittstellen bereitstellen.

Der Kopf muss Prozessor, Speicher, Gesichtsanzeige, Mikrofone, Lautsprecher, eine Nickmechanik mit Motor, Lagesensor, Kamera, Touchsensor und Antennen enthalten. Ein RFID-Leser kann optional ergänzt werden; sein Einsatzzweck ist noch festzulegen.

### 10.2 Bauch

Der Bauch muss:

- die Energieversorgung übernehmen und den Akku laden;
- Drehbewegungen ermöglichen;
- Sensoren aufnehmen;
- mit den Beinen und dem Kopf kommunizieren;
- Energie- und Betriebszustände für das Gesamtsystem bereitstellen.

Der Bauch muss Akku, Ladeelektronik, Radar und eine Status-LED enthalten. Eine IR-Ausstattung kann optional ergänzt werden; ihre konkrete Funktion ist noch festzulegen. Die Drehung des angedockten Robin um 360° wird durch die Homestation ausgeführt.

### 10.3 Beine

Die Beine sollen modular austauschbar sein, selbstständig angesteuert werden und über elektrische Kontakte (Pogo-Pins) verbunden werden.

Modulwechsel und nicht verfügbare Beinmodule müssen erkannt werden. Bewegungsfunktionen müssen an die tatsächlich verfügbaren und betriebsbereiten Module angepasst werden.

### 10.4 Lade- und Homestation

Die Ladestation muss sicheres Laden ermöglichen und Ladebereitschaft, Ladevorgang, Ladeabschluss und Ladefehler erkennbar machen. Fehlende oder unterbrochene Ladeverbindungen müssen erkannt werden. Unsichere Ladebedingungen müssen zu einer Unterbrechung führen.

Robin soll einen niedrigen Akkustand erkennen und verständlich den Bedarf zum Laden anzeigen. Soweit seine Bewegungsfähigkeiten dies erlauben, soll Robin die Ladestation aufsuchen und andocken können. Erfolgloses Andocken muss erkannt und gemeldet werden.

Die Homestation muss hauptsächlich als Ladestation dienen und den angedockten Robin mit einem eigenen Drehantrieb um 360° drehen können. Sie muss Drehaufträge über eine definierte Schnittstelle entgegennehmen und die Drehbewegung sicher ausführen können. Die Ladeverbindung muss während der Drehung erhalten bleiben. Die konkrete mechanische Umsetzung und die Möglichkeit endloser Rotation werden später festgelegt.

Die Homestation muss einen Leuchtring besitzen, der Zustände darstellen und Lichteffekte für einen Disco-Modus ausgeben kann. Sicherheitsrelevante Zustandsanzeigen müssen Vorrang vor Disco-Effekten haben. Die Lichtfunktionen müssen über definierte Schnittstellen steuerbar sein.

Die Homestation soll einen definierten Aufenthalts- und Ruheort bereitstellen. Ihr Ausfall darf wesentliche Grundfunktionen von Robin ausserhalb der Station nicht verhindern.

### 10.5 Server

Der Server soll:

- OTA-Updates bereitstellen;
- Roboter verwalten;
- Konfiguration speichern;
- geschützte Livestreams bereitstellen;
- eine dokumentierte Programmierschnittstelle bereitstellen.

Serverfunktionen müssen Berechtigungen und Datenschutzvorgaben berücksichtigen. Konflikte zwischen lokaler und serverseitiger Konfiguration sowie das Verhalten bei fehlender Verbindung müssen definiert sein.

### 10.6 Mobile Companion-App

Das Smartphone muss das erweiterte Verhalten ausführen und mit dem lokalen Grundverhalten im Kopf zusammenarbeiten. Daraus abgeleitete Aktionen müssen lokal auf Sicherheit, Datenschutz, Berechtigungen und verfügbare Fähigkeiten geprüft werden. Die genaue Zuordnung der Verhaltensfunktionen muss dokumentiert werden.

Die Companion-App soll:

- verfügbare Roboter erkennen und eine Verbindung herstellen;
- Ersteinrichtung, Pairing und Ownership-Verwaltung unterstützen;
- Firmware aktualisieren;
- Einstellungen ändern und Status anzeigen;
- Kontext und Unterstützungsfunktionen einschliesslich Erinnerungen verwalten;
- bewusst registrierte Personen und Kontaktverknüpfungen verwalten;
- Datenschutz, gespeicherte Daten und Berechtigungen verwalten;
- lokale sowie servergestützte Funktionen und deren Verfügbarkeit anzeigen;
- Lost Mode, Reset und Weitergabe unterstützen;
- verständliche Diagnose- und Fehlermeldungen bereitstellen.

Sicherheitsrelevante Aktionen müssen gegen unbefugte Nutzung geschützt und ihre Auswirkungen für den Benutzer verständlich sein.

### 10.7 Weboberfläche

Die Weboberfläche soll Livestreams anzeigen, Roboterstatus darstellen, Einstellungen ermöglichen und Firmware verwalten.

Zugriffsschutz, Berechtigungen, Datenschutz und die Bedingungen für Videoübertragung müssen auch in der Weboberfläche eingehalten werden.

## 11. Offline-Fähigkeit und Systemzustände

### 11.1 Wesentliche Offline-Fähigkeit

Robin muss ohne Internetverbindung sicher betrieben werden können. Wesentliche Grundfunktionen müssen lokal erhalten bleiben: Sicherheitsreaktionen, grundlegende Wahrnehmung, Mimik, Bewegung im Rahmen verfügbarer Fähigkeiten, lokale Sprachreaktionen, Energiemanagement und die Anzeige seines Systemzustands.

Für jede Funktion muss dokumentiert sein, ob sie vollständig lokal, eingeschränkt offline oder nur mit externen Diensten verfügbar ist. Ausgefallene externe Funktionen müssen verständlich angezeigt werden. Wiederhergestellte Verbindungen dürfen keine unkontrollierten nachträglichen Handlungen auslösen.

### 11.2 Zustände und Übergänge

Das System muss mindestens Ersteinrichtung, Normalbetrieb, Ruhe beziehungsweise Energiesparen, Laden, eingeschränkten Betrieb, Fehlerzustand, Update, Lost Mode und Reset nachvollziehbar unterscheiden.

Online- und Offline-Verfügbarkeit sowie aktive Videoübertragung müssen zusätzlich eindeutig erkennbar sein; sie können mehrere Betriebszustände betreffen.

Für Zustände und Übergänge müssen erlaubte Funktionen, Auslöser, Prioritäten und Rückmeldungen dokumentiert werden. Sicherheits- und Datenschutzregeln müssen in jedem Zustand gelten. Nach Start oder Neustart muss Robin einen definierten, sicheren Zustand herstellen.

## 12. Fehlertoleranz, sichere Bewegung und Energiemanagement

Robin muss Ausfälle von Sensoren, Aktoren, Modulen, Kommunikation und externen Diensten erkennen, soweit sie seinen Betrieb beeinflussen. Nicht betroffene Funktionen sollen weiter verfügbar bleiben. Einschränkungen müssen verständlich angezeigt werden.

Bei unsicherem Betriebszustand müssen Bewegungen begrenzt oder gestoppt werden. Eine verfügbare Benutzeraktion zum Stoppen laufender Bewegungen muss Vorrang vor autonomem Verhalten haben. Wiederanlauf und Fehlerbehebung dürfen keine unbeabsichtigten Bewegungen auslösen.

Akkustand, Ladezustand und relevante Temperaturzustände müssen überwacht werden. Bei niedrigem Akkustand muss der Funktionsumfang kontrolliert reduziert werden; vor kritischer Entladung muss ein sicherer Zustand hergestellt werden.

Energiesparen muss verfügbare Ressourcen, Ruhephasen, Lost Mode und deaktivierte Funktionen berücksichtigen. Sicherheitsfunktionen dürfen durch Energiesparen nicht unzulässig eingeschränkt werden.

## 13. Sichere und robuste Updates

Firmware und andere aktualisierbare Systembestandteile müssen über einen geschützten und nachvollziehbaren Updateablauf aktualisiert werden können.

Updates müssen auf vertrauenswürdige Herkunft, Unversehrtheit und Kompatibilität geprüft werden. Stromversorgung und Betriebszustand müssen für die Durchführung geeignet sein. Während eines Updates muss Robin einen definierten sicheren Zustand einnehmen.

Unterbrochene oder fehlgeschlagene Updates dürfen Robin nicht dauerhaft unbrauchbar machen. Ein bekannter funktionsfähiger Stand oder ein geeigneter Wiederherstellungsmodus muss verfügbar sein.

Updatefortschritt, Erfolg und Fehler müssen angezeigt werden. Persönliche Daten und Konfiguration sollen erhalten bleiben; notwendige Änderungen müssen nachvollziehbar behandelt werden.

## 14. Diagnose und Debugging

Das System muss nachvollziehbare Status-, Ereignis- und Fehlerinformationen für Entwicklung und Wartung bereitstellen. Dazu gehören verfügbare Module, Software- und Protokollversionen, Verbindungszustände, Energiezustand und technische Ursachen eingeschränkter Funktionen.

Verhaltensentscheidungen sollen für Entwicklung und Debugging mit ihrem relevanten Kontext nachvollzogen werden können. Die technische Diagnose muss von Robins sozialer Darstellung unterschieden werden.

Diagnosezugriffe müssen berechtigt sein. Protokollierung und Diagnoseexport müssen Daten minimieren und persönliche Informationen sowie Zugangsinformationen schützen. Erweiterte Diagnosefunktionen dürfen nicht unbemerkt dauerhafte Überwachung erzeugen.

## 15. Simulierbarkeit und Entwicklungsanforderungen

Die Plattform muss Entwicklung und Prüfung wesentlicher Funktionen ohne vollständige Roboterhardware ermöglichen.

Virtual Robin muss Interaktion, Verhalten, Systemzustände und Kommunikation über die fachlichen Schnittstellen abbilden können. Ein Smartphone-Simulator soll als weitere Entwicklungs- und Erprobungsumgebung unterstützt werden.

Wahrnehmungsereignisse, Modulzustände, Energiezustände und Fehler sollen gezielt simuliert werden können. Reproduzierbare Szenarien müssen insbesondere Offline-Betrieb, Verbindungsabbruch, Datenschutzregeln, Lost Mode, Modulwechsel und fehlgeschlagene Updates prüfbar machen.

Grenzen und Abweichungen der Simulation gegenüber realer Hardware müssen dokumentiert werden. Sicherheitsrelevantes physisches Verhalten muss zusätzlich an geeigneter realer Hardware geprüft werden.

## 16. Qualitätsanforderungen und Randbedingungen

Das Gesamtsystem soll modular, einfach wartbar und erweiterbar sein. Softwarearchitektur und fachliche Schnittstellen müssen hardwareunabhängig gestaltet werden. Wiederverwendung von Komponenten und Logik soll unterstützt werden.

Lokale Reaktionen sollen eine geringe Latenz aufweisen. Energieverbrauch und Entwicklungskosten sollen gering gehalten werden. Konkrete Zielwerte, Messbedingungen und Abnahmekriterien müssen vor der Umsetzung der jeweiligen Funktion festgelegt werden.

Standardkomponenten sollen bevorzugt verwendet werden. Sonderanfertigungen sollen sich auf Leiterplatten und Gehäuse beschränken. Prozessorwechsel sollen mit möglichst geringem Softwareaufwand möglich sein.

## 17. Nachweis und Abnahme

Für jede umgesetzte Anforderung muss ein geeigneter Nachweis durch Prüfung, Demonstration oder dokumentierte Bewertung vorgesehen werden. Die Zuordnung zwischen Anforderung und Nachweis muss nachvollziehbar sein.

Die Abnahme muss bestehende Modul- und Bedienfunktionen sowie die ergänzten systemweiten Anforderungen berücksichtigen. Besonders zu prüfen sind bewusste Personenregistrierung und Kontaktverknüpfung, sichtbare Videoübertragung, Zugriffsschutz, Offline-Grundfunktionen, Lost Mode, Reset und Weitergabe, sichere Zustandsübergänge sowie Wiederherstellung nach einem fehlgeschlagenen Update.

Neue Funktionen müssen zusätzlich gegen die Prüfliste der Robin Principles bewertet werden. Noch offene Zielwerte und technische Entscheidungen müssen ausdrücklich als offen dokumentiert werden.
