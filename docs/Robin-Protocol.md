# Robin Protocol

Status: erster fachlicher Entwurf zur gemeinsamen Abstimmung. Entwurfsstand 0.1; dies ist noch keine implementierte Protokollversion.

## 1. Zweck und Geltungsbereich

Das Robin Protocol beschreibt die hardwareunabhängige Bedeutung der Kommunikation zwischen Robin, Smartphone, Modulen, Homestation und Simulatoren. Grundlagen sind das [Lastenheft](Lastenheft.md), die [Systemarchitektur](Systemarchitektur.md) und die [Robin Principles](Robin-Principles.md).

Dieser erste Entwurf behandelt die Verbindung eines bereits bewusst gekoppelten Smartphones mit Robin, den Austausch von Fähigkeiten und Zustand sowie einen begrenzten Auftrag mit Rückmeldung. Er beschreibt auch Abbruch, Ablehnung und Wiederverbindung.

Übertragungswege, konkrete Nachrichtenformate und Verfahren zum Identitätsnachweis werden später festgelegt. Die Erstkopplung wird in Abschnitt 3 fachlich beschrieben. Personenverwaltung, Updates, Livestreams und Synchronisation persönlicher Daten erhalten eigene Abläufe. Eine bestehende Verbindung erteilt für diese Funktionen keine pauschale Berechtigung.

## 2. Rollen und Grundregeln

Das Smartphone führt erweitertes Verhalten aus und sendet Vorschläge oder Aufträge. Der Robin-Kern im Kopf prüft und koordiniert deren lokale Ausführung. Module und Homestation führen die ihnen zugeordneten Aktionen aus und melden Ergebnisse zurück.

Robin bleibt die entscheidende Instanz für Sicherheit, Datenschutz, Benutzerregeln und tatsächlich verfügbare Fähigkeiten. Die Station verantwortet ihren Drehantrieb und Leuchtring; der Bauch verantwortet Energieversorgung und Ladeelektronik.

Virtual Robin bietet denselben fachlichen Vertrag an. Simulierte Fähigkeiten und Ergebnisse werden ausdrücklich als simuliert gekennzeichnet.

Nachrichten- und Feldnamen bilden den vorgeschlagenen fachlichen Wortschatz dieses Entwurfs. Ihre konkrete Codierung wird später festgelegt.

## 3. Verbindung und Sitzung

Eine Sitzung ist der zeitlich begrenzte, bestätigte Kommunikationszusammenhang zwischen zwei berechtigten Beteiligten. Jede neue Verbindung beziehungsweise Wiederverbindung erhält eine neue Sitzungskennung.

| Schritt | Austausch | Ergebnis |
| --- | --- | --- |
| 1 | Smartphone eröffnet Verbindung | Noch keine Steuerberechtigung |
| 2 | Beide Seiten weisen ihre Identität nach | Bekannte Kopplung und weiterhin gültige Rechte werden geprüft |
| 3 | Beide Seiten melden Rolle und unterstützte Protokollversionen | Eine gemeinsam unterstützte Version wird vereinbart |
| 4 | Robin bestätigt Sitzung und erlaubte Funktionen | Smartphone kennt die tatsächlich gewährten Rechte |
| 5 | Robin liefert Fähigkeiten und aktuellen Zustand | Smartphone kann passende Aufträge planen |
| 6 | Robin meldet Änderungen; Smartphone darf erlaubte Aufträge senden | Sitzung ist betriebsbereit |

Identitätsnachweis und Schutz gegen Manipulation müssen vor der Übermittlung geschützter Informationen und vor der Annahme von Steueraufträgen wirksam sein. Für die technische Umsetzung dieses Ablaufs wird später eine geeignete Sicherheitslösung ausgewählt.

Eine behauptete Rolle oder Gerätekennung allein ist kein Identitätsnachweis. Widerrufene Kopplungen werden abgewiesen. Ohne kompatible Version wird die Sitzung nicht für Steuerung freigegeben.

Bei noch nicht gekoppelten Geräten ist ausschliesslich ein separat definierter, bewusst ausgelöster Einrichtungsablauf möglich. Dieser Entwurf führt keine automatische Kopplung ein.

### 3.1 Pairing, Ownership und Sitzung

Pairing ist die dauerhafte, bewusst bestätigte Zuordnung eines Smartphones zu Robin mit festgelegten Rechten. Eine Sitzung ist dagegen eine einzelne Verbindung auf Grundlage dieser Zuordnung.

Die erste Besitzerzuordnung und das Hinzufügen weiterer Geräte sind unterschiedliche Vorgänge. Eine neue Kopplung darf weder eine vorhandene Ownership überschreiben noch automatisch Besitzerrechte erhalten.

Als Entwurfsentscheidung wird die erste Kopplung durch eine bewusste lokale Handlung an Robin geöffnet. Die genaue Bedienhandlung bleibt offen. Alleinige Nähe, Geräteerkennung, Berührung während normaler Interaktion oder ein RFID-Kontakt begründen keine Freigabe.

### 3.2 Ablauf der ersten Kopplung

| Schritt | Ablauf | Schutzwirkung |
| --- | --- | --- |
| 1 | Benutzer öffnet bewusst die Einrichtung an Robin | Zeitlich begrenztes Pairingfenster; sichtbar auf Robins Gesichtsanzeige |
| 2 | App erkennt Robin und startet eine Pairinganfrage | Noch keine Steuer- oder Datenrechte |
| 3 | Robin reserviert genau einen Kopplungsversuch | Andere Versuche erhalten keine parallele Freigabe |
| 4 | Beide Seiten bauen einen gegen Manipulation geschützten Austausch auf | Frische Nachweise werden an diesen Versuch gebunden |
| 5 | Beide zeigen einen übereinstimmenden Prüfnachweis | Benutzer kann feststellen, dass App und Robin am selben Versuch teilnehmen |
| 6 | Benutzer vergleicht den Nachweis und bestätigt bewusst an Robin und in der App | Kein automatisches Bestätigen aufgrund von Empfang oder Zeitablauf |
| 7 | Robin prüft seinen Einrichtungszustand und legt die erste Besitzerzuordnung mit Rechten an | Nur ein bislang unzugeordnetes Gerät kann so übernommen werden |
| 8 | Beide speichern die Zuordnung geschützt und bestätigen deren Abschluss | App zeigt Erfolg erst nach bestätigter Speicherung auf beiden Seiten |
| 9 | Pairingfenster wird geschlossen; bestätigte Sitzung wird aufgebaut | Steuerung erfolgt erst mit geprüften Rechten |

Der Prüfnachweis muss aus dem geschützten Austausch abgeleitet und an Identitäten, frische Versuchsdaten und vorgesehene Rechte gebunden sein. Das blosse Anzeigen desselben frei übertragenen Codes reicht nicht aus. Verfahren, Länge, Darstellung und Bestätigungselemente werden vor Implementierung festgelegt.

Eine Kopplung, deren Prüfung oder Abschluss unklar bleibt, erhält keine aktiven Rechte. Vorläufige Einträge dürfen nicht als abgeschlossene Kopplung verwendet werden. Nach abgebrochenem Abschluss müssen beide Seiten den bestätigten Stand ermitteln oder den Versuch verwerfen; das technische Abschlussverfahren wird separat spezifiziert.

### 3.3 Weiteres Gerät hinzufügen

Bei vorhandener Ownership muss ein berechtigter Besitzer das Hinzufügen eines Geräts freigeben. Der Versuch benötigt zusätzlich die bewusste lokale Bestätigung an Robin und den gegenseitigen Prüfnachweis.

Die zu vergebenden Rechte werden vor Abschluss angezeigt und bewusst gewählt. Standardmässig erhält das zusätzliche Gerät keine Besitzerrechte. Eine Erweiterung von Rechten ist ein eigener, berechtigt bestätigter Vorgang.

Eine Pairinganfrage eines unbekannten Smartphones ist keine Aufforderung, bestehende Zugänge zu entfernen oder Robin zurückzusetzen.

### 3.4 Rechteumfang

| Rechtebereich | Möglicher Umfang |
| --- | --- |
| Status lesen | Freigegebene Zustände und Fähigkeiten abfragen |
| Interaktion steuern | Freigegebene Mimik-, Audio-, Licht- und Bewegungsaufträge |
| Persönliche Daten verwalten | Bewusste Registrierung, Erinnerungen und Kontaktverknüpfungen |
| Einstellungen verwalten | Zulässige Konfiguration ändern |
| Wartung | Freigegebene Diagnose und Updates |
| Ownership verwalten | Geräte hinzufügen, Rechte ändern, Geräte widerrufen und Weitergabe veranlassen |
| Videoübertragung | Separater Zugriff mit zusätzlichen lokalen Bedingungen |

Diese Bereiche sind ein erster fachlicher Entwurf; konkrete Rollen und feine Rechte werden noch festgelegt. Ein Leserecht gewährt kein Steuerrecht. Interaktionssteuerung gewährt weder Personenverwaltung noch Videoübertragung.

Jeder Auftrag wird gegen die aktuell gültigen Rechte geprüft. Persönlichkeitspräferenzen, behauptete Rollen und weitergeleitete Nachrichten erweitern keine Berechtigungen. Auch ein Besitzer kann lokale Sicherheits- oder Datenschutzbedingungen nicht umgehen.

### 3.5 Wiederkehrender Verbindungsaufbau

Ein bereits gekoppeltes Smartphone verwendet keinen neuen Pairingversuch, sondern weist die bestehende Zuordnung nach. Beide Seiten prüfen frische, an den aktuellen Austausch gebundene Identitätsnachweise sowie den weiterhin gültigen Kopplungsstand.

Nach Prüfung der kompatiblen Version bestätigt Robin eine neue Sitzung mit ihren tatsächlich gewährten Rechten. Nachrichtenrahmen und Inhalt werden an diese Sitzung und die nachgewiesenen Beteiligten gebunden. Alte Sitzungsnachrichten dürfen nicht in einer neuen Sitzung wiederverwendet werden.

Nach Neustart kann eine gültig gespeicherte Kopplung erhalten bleiben; die vorherige Sitzung bleibt ungültig. Ein Sitzungsabbruch löscht eine Kopplung nicht automatisch.

### 3.6 Widerruf, Geräteverlust und Reset

Der berechtigte Besitzer kann eine Kopplung widerrufen. Robin beendet deren aktive Sitzungen und blockiert neue Verbindungen. Laufende Aufträge werden nach ihren festgelegten Regeln sicher beendet; Widerruf darf keine unbeaufsichtigte Fernsteuerung zurücklassen.

Ist Robin beim Widerruf nicht erreichbar, darf die App keinen wirksamen Widerruf am Roboter behaupten. Sie zeigt den Vorgang als ausstehend an. Serverunterstützung kann ergänzt werden, ersetzt aber keine noch nicht bei Robin wirksame Änderung.

Ein verlorenes Smartphone wird über ein anderes berechtigtes Besitzergerät widerrufen. Der Wiederherstellungsweg, wenn kein Besitzergerät mehr vorhanden ist, bleibt offen. Ein neu geöffnetes Pairingfenster allein darf diese Situation nicht zur unbefugten Übernahme nutzbar machen.

Ein vollständiger Reset entfernt persönliche Daten, Kopplungen und gespeicherte Zugangsinformationen und stellt den Einrichtungszustand her. Er erfordert einen separat definierten bewussten Ablauf. Neustart und Verbindungstrennung sind kein Reset.

### 3.7 Fehler, Fristen und Datenschutz

Pairingfenster und einzelne Versuche sind zeitlich begrenzt. Abbruch, abweichender Prüfnachweis, abgelaufene Frist, fehlende Bestätigung und ungültiger Einrichtungszustand verhindern den Abschluss.

Fehlversuche werden lokal begrenzt. Wartezeiten und Versuchslimits müssen Missbrauch reduzieren, ohne dauerhaft eine legitime Einrichtung zu blockieren. Konkrete Werte bleiben offen.

Ungekoppelte Geräte erhalten nur die zur bewussten Einrichtung notwendigen Informationen. Personeninformationen, Erinnerungen und laufende persönliche Aufgaben werden nicht offengelegt. Zugangsinformationen und private Sicherheitsnachweise dürfen nicht in Diagnoseprotokollen erscheinen.

### 3.8 Nachrichten des Verbindungsaufbaus

| Fachlicher Typ | Zweck |
| --- | --- |
| Verbindungsanfrage | Unterstützte Versionen und behauptete Rolle für einen neuen Austausch nennen |
| Identitätsprüfung | Frische gegenseitige Nachweise austauschen und prüfen |
| Pairinganfrage | Innerhalb des bewusst geöffneten Fensters eine neue Zuordnung anfragen |
| Pairingprüfung | Prüfnachweis und vorgesehenen Rechteumfang für den Benutzervergleich bereitstellen |
| Pairingbestätigung | Lokale und appseitige bewusste Bestätigung an denselben Versuch binden |
| Pairingabschluss | Geschützte Speicherung und endgültige Rechtefreigabe bestätigen |
| Sitzungsbestätigung | Neue Sitzung, vereinbarte Version und tatsächlich gewährte Rechte bestätigen |
| Ablehnung oder Abbruch | Austausch mit nachvollziehbarem Grund beenden |

Diese Bezeichnungen beschreiben Phasen, noch keine frei implementierbaren Sicherheitsnachrichten. Ein geeignetes etabliertes Sicherheitsverfahren muss den Austausch technisch absichern. Sicherheitsnachweise werden nicht durch selbst entworfene Verschlüsselungs- oder Identitätsverfahren ersetzt.

Vor Abschluss verwendete Nachrichten besitzen einen eindeutig abgegrenzten Versuchszusammenhang; sie dürfen nicht als Nachrichten einer bestätigten Sitzung behandelt werden. Ihre technischen Felder werden erst zusammen mit dem Sicherheitsverfahren festgelegt.

### 3.9 Prüfung

Virtual Robin prüft bewusste Erstkopplung, Hinzufügen eines Geräts mit begrenzten Rechten, verweigerte Übernahme eines bereits zugeordneten Robin, abweichenden Prüfnachweis, fehlende Bestätigung, Fristablauf und parallele Versuche.

Weitere Prüffälle sind abgebrochene Speicherung ohne Rechtefreigabe, erneuter Verbindungsaufbau, Neustart mit erhaltener Kopplung, ungültige alte Sitzungsnachrichten, Rechteänderung, wirksamer beziehungsweise ausstehender Widerruf und vollständiger Reset.

Die tatsächliche Sicherheit der Identitätsprüfung und geschützten Speicherung muss zusätzlich mit der ausgewählten technischen Umsetzung geprüft werden; eine fachliche Simulation weist diese Sicherheit nicht nach.

## 4. Fähigkeiten

Robin meldet, welche Funktionen grundsätzlich vorhanden und aktuell nutzbar sind. Eine Fähigkeit enthält:

- eindeutige fachliche Kennung und zuständige Komponente;
- unterstützte Aktionen und ihre Parameter mit Einheiten und Grenzen;
- Verfügbarkeit und gegebenenfalls Einschränkungsgrund;
- Voraussetzungen, benötigte Rechte und Abbruchverhalten;
- Verhalten bei Verbindungsverlust;
- Kennzeichnung realer oder simulierter Ausführung.

Beispiele sind Gesichtsausdruck, Nickbewegung, Audioausgabe, Radarwahrnehmung, Stationsdrehung und Leuchtring. Optionales RFID oder IR wird nur gemeldet, wenn es tatsächlich vorhanden und fachlich definiert ist.

Eine vorhandene Fähigkeit ist nicht automatisch ausführbar: Stationsdrehung erfordert beispielsweise eine verfügbare Station und bestätigtes Andocken. Endlose Rotation wird nur angeboten, wenn sie später ausdrücklich unterstützt wird.

Fähigkeitslisten besitzen eine Revision. Änderungen werden mit neuer Revision gemeldet. Robin prüft bei jedem Auftrag die aktuelle Verfügbarkeit; eine zuvor empfangene Liste garantiert keine spätere Ausführbarkeit.

## 5. Zustand

Der Zustand beschreibt mindestens:

- Betriebszustand und laufende Einschränkungen;
- Energie- und Ladezustand, soweit bekannt;
- verfügbare Module und Stationsverbindung;
- Andockzustand;
- Verfügbarkeit des erweiterten Verhaltens;
- aktive Videoübertragung, falls vorhanden;
- für den jeweiligen Benutzer sichtbare laufende Aufträge und Fehler.

Unbekannte oder veraltete Werte werden ausdrücklich als solche gekennzeichnet. Zustandsmeldungen enthalten eine Revision sowie Angaben zu Aktualität und Quelle.

Nach Sitzungsaufbau wird zunächst ein vollständiger Zustand übermittelt. Danach können Änderungen übertragen werden. Vollzustand und Änderungsfolge müssen eindeutig zusammenpassen; fehlen Änderungen, fordert das Smartphone einen neuen Vollzustand an.

Der Zustand enthält standardmässig weder Bild- oder Audiorohdaten noch Personendaten oder persönliche Erinnerungen. Solche Daten benötigen später eigene berechtigte Abläufe.

## 6. Gemeinsame Nachrichteninformationen

Jede fachliche Nachricht muss ihrem Typ, Absender, Empfänger und ihrer Sitzung eindeutig zugeordnet werden können. Anfragen und Antworten werden durch eine Nachrichtenkennung verbunden.

Aufträge haben zusätzlich eine Auftragskennung, die für Wiederholungen und Statusabfragen erhalten bleibt. Antworten und Ereignisse zum Auftrag tragen dieselbe Auftragskennung.

Für Aufträge werden Gültigkeit, Ausführungsgrenzen und relevante Voraussetzungen beschrieben. Die spätere technische Spezifikation muss eine überprüfbare Frist trotz möglicher Uhrabweichungen festlegen. Eine einfache lokale Ankunftszeit genügt nicht, um bereits vor der Zustellung veraltete Aufträge zu erkennen.

### 6.1 Gemeinsamer Nachrichtenrahmen

Die folgenden Feldnamen sind der vorgeschlagene gemeinsame Wortschatz. Sie legen Bedeutung und logische Datentypen fest, aber noch keine Codierung, Feldlänge oder Transporttechnik.

| Feld | Pflicht | Bedeutung |
| --- | --- | --- |
| `protocol_version` | In bestätigter Sitzung | Vereinbarte Protokollversion |
| `message_type` | Immer | Art und fachlicher Zweck der Nachricht |
| `message_id` | Immer | Kennung dieser logischen Nachricht |
| `sender_id` | Immer | Absenderkennung, an die nachgewiesene Identität gebunden |
| `recipient_id` | Immer | Zuständiger Empfänger |
| `session_id` | In bestätigter Sitzung | Aktuelle Sitzung; keine eigenständige Zugangsberechtigung |
| `in_reply_to` | Bei direkter Antwort | Nachrichtenkennung der beantworteten Anfrage |
| `request_id` | Bei Auftrag, Statusabfrage, Abbruch und zugehöriger Meldung | Über Sitzungswechsel hinweg erhaltene Auftragskennung |
| `payload` | Immer | Zum Nachrichtentyp gehörende fachliche Inhalte |

Kennungen werden als undurchsichtige Werte behandelt. Sie enthalten keine Namen, Kontaktinformationen oder Geheimnisse. Eine Rolle oder Berechtigung wird nicht allein aus einem frei gelieferten Feld abgeleitet.

Vor Bestätigung einer Sitzung sind nur die separat definierten Nachrichten des Verbindungsaufbaus erlaubt. Sie verwenden noch keine bestätigte Sitzungskennung und nennen unterstützte Versionen in ihrem Inhalt. Ihr genauer Rahmen wird mit dem Identitätsnachweis festgelegt. Sie dürfen keine Steueraufträge oder geschützten Zustandsdaten transportieren.

### 6.2 Nachrichten- und Auftragskennung

Eine neue Anfrage oder Statusabfrage erhält eine neue `message_id`. Eine unveränderte erneute Zustellung derselben logischen Nachricht verwendet dieselbe Kennung. Nachrichtenkennungen müssen innerhalb der Sitzung je Absender eindeutig sein.

Die `request_id` bezeichnet dagegen den fachlichen Auftrag. Sie bleibt bei erneuter Anfrage, Statusabfrage, Abbruch und Wiederverbindung erhalten. Ihr Gültigkeitsbereich umfasst den nachgewiesenen Auftraggeber und den zuständigen Robin.

Eine Antwort hat eine eigene `message_id` und verweist über `in_reply_to` auf die Anfrage. Spätere Auftragsereignisse verwenden die `request_id`; sie benötigen keinen Bezug auf eine zwischenzeitlich verloren gegangene Anfrage.

Beispiel: Eine Nickanforderung trägt Nachrichtenkennung M1 und Auftragskennung A1. Nach verlorenem Ergebnis wird eine Statusabfrage M2 für A1 gesendet. Die Antwort M3 verweist auf M2 und A1. Sie führt keine neue Nickbewegung aus. Die Kennungen sind nur illustrative Platzhalter.

Bei Weiterleitung an die Station bleibt der ursprüngliche Auftraggeber nachvollziehbar. Der lokale Komponentenauftrag darf eine eigene Kennung besitzen, muss aber eindeutig dem übergeordneten Auftrag zugeordnet sein. Ein Smartphone darf Auftraggeber oder übergeordnete Kennung nicht nutzen, um Rechte eines anderen Geräts zu beanspruchen.

### 6.3 Nachrichtentypen des ersten Umfangs

| `message_type` | Fachlicher Zweck | Wesentliche Inhalte in `payload` |
| --- | --- | --- |
| `capabilities.get` | Fähigkeiten anfordern | Gewünschter zulässiger Umfang |
| `capabilities.snapshot` | Vollständige Fähigkeiten melden | Ursprung, Revision, Fähigkeiten |
| `state.get` | Aktuellen Zustand anfordern | Gewünschter zulässiger Umfang |
| `state.snapshot` | Vollständigen Zustand melden | Ursprung, Revision, Werte mit Aktualität und Quelle |
| `state.changed` | Zustandsänderung melden | Ursprung, Basisrevision, neue Revision, Änderungen |
| `action.request` | Neue Aktion anfordern oder bekannten Auftrag erneut anfragen | Ziel, Aktion, Parameter, Startgültigkeit, Fähigkeitsrevision und gegebenenfalls Ergebnisnachweis |
| `action.status.get` | Auftrag abfragen | Auftragskennung im Rahmen |
| `action.status` | Annahme, Ablehnung, Beginn, Fortschritt oder Endzustand melden | Auftragszustand, Statusrevision, tatsächliche Parameter, Ergebnis und gegebenenfalls Grund |
| `action.cancel` | Abbruch eines Auftrags anfordern | Auftragskennung im Rahmen; optionaler Grund |
| `motion.stop` | Berechtigten Bewegungsstopp anfordern | Eindeutiger Stoppumfang |
| `error` | Anfrage konnte nicht regulär verarbeitet werden | Stabiler Fehlergrund und verständliche Erklärung |

Eine direkte Antwort verwendet `in_reply_to`. Unaufgeforderte Zustands- oder Auftragsmeldungen besitzen keinen künstlichen Antwortbezug. Fehlende Empfangsbestätigungen ändern die Bedeutung von Annahme und Ergebnis nicht.

Ein bekannter Auftrag mit unzulässigen Aktionsparametern wird durch `action.status` als abgelehnt beantwortet. Ein nicht auswertbarer Nachrichtentyp oder beschädigter Rahmen kann eine `error`-Antwort erzeugen, sofern der Absender sicher zugeordnet werden kann. Eine unzuordenbare oder unberechtigte Nachricht erzeugt keinen gültigen Auftrag.

### 6.4 Aktionsinhalt und Gültigkeit

Der Inhalt von `action.request` verwendet folgende gemeinsame Felder:

| Feld | Bedeutung |
| --- | --- |
| `target` | Fachliche Zielkomponente und Fähigkeit |
| `action` | Unterstützte Aktion |
| `parameters` | Zur Aktion gehörende Werte mit definierten Einheiten |
| `start_validity` | Überprüfbare begrenzte Startgültigkeit |
| `capability_revision` | Erwarteter Stand der Fähigkeit |
| `required_evidence` | Benötigter Ergebnisnachweis, falls für die Aktion definiert |

`start_validity` bezeichnet einen gemeinsamen fachlichen Vertrag, noch kein festgelegtes Zeitformat. Die Umsetzung muss verzögerte Zustellung, Uhrabweichungen und erneute Sitzungen berücksichtigen. Fehlt eine erforderliche Gültigkeitsangabe oder kann Robin sie nicht verlässlich prüfen, wird die Aktion abgelehnt.

`motion.stop` ist ein eigener Kontrollauftrag. Er wird nicht hinter normalen Aktionsaufträgen eingereiht und hängt nicht von deren Fähigkeitsrevision ab. Sitzung, Rechte und eindeutiger Stoppumfang müssen trotzdem geprüft werden. Seine Bestätigung unterscheidet Empfang, angenommene Stoppanforderung und tatsächlich bestätigten Bewegungsstillstand. Ein Auftragsabbruch bestätigt ebenfalls erst nach tatsächlichem Beenden den Endzustand.

### 6.5 Revisionen und Reihenfolge

Zustand und Fähigkeiten erhalten jeweils einen `origin_id`, der ihre aktuelle Ausführungsinstanz kennzeichnet. Nach Neustart beziehungsweise Verlust des Revisionsstands wird ein neuer Ursprung verwendet. Er ist keine Identität und keine Berechtigung.

Ein `state.snapshot` liefert den vollständigen freigegebenen Umfang für Ursprung und Revision. Eine Änderung enthält `base_revision` und `revision`. Das Smartphone wendet sie nur an, wenn Ursprung und Basisrevision zu seinem Stand passen. Ältere Meldungen werden nicht als neuer Zustand übernommen; Lücken oder ein neuer Ursprung erfordern einen Vollabgleich.

Für den ersten Umfang werden geänderte Fähigkeiten als vollständiges `capabilities.snapshot` übertragen. Ein Änderungsformat für Fähigkeiten wird noch nicht eingeführt.

Auftragsmeldungen tragen eine je Auftrag steigende `status_revision`. Ein alter Status darf einen neueren Status oder einen bestätigten Endzustand nicht überschreiben. Kann Robin einen Auftragsstatus nach Neustart nicht mehr nachweisen, wird das Ergebnis als unbekannt gekennzeichnet. Unbekannt ist kein zusätzlicher Ausführungs-Endzustand, sondern eine Aussage über fehlenden Nachweis.

### 6.6 Fehler, Grenzen und Erweiterungen

Ein Fehler enthält `reason_code` als stabilen fachlichen Grund und `explanation` als verständliche Erklärung. Eine wiederholbare Zustellung ist von einem neuen Ausführungsversuch zu unterscheiden. Fehlerantworten dürfen keine pauschale automatische Wiederholung einer Bewegung auslösen.

Vor Implementierung werden die erlaubten Größen und Verschachtelungen, Wertebereiche, Nachrichtenraten und Aufbewahrungsgrenzen festgelegt. Ungültige Nachrichten werden ohne Teil-Ausführung abgewiesen.

Pflichtfelder und unbekannte sicherheitsrelevante Inhalte dürfen nicht stillschweigend ignoriert werden. Erweiterungen müssen zwischen optionalen Zusatzinformationen und zwingend unterstützten Funktionen unterscheiden. Unbekannte Aktionen werden abgelehnt. Ein kompatibles Nachrichtenformat hebt keine fehlende Fähigkeit oder Berechtigung auf.

Der Nachrichtenrahmen und seine Inhalte müssen gegen Manipulation und Wiederverwendung ausserhalb der gültigen Sitzung geschützt werden. Das konkrete Verfahren wird mit der technischen Sicherheitsspezifikation festgelegt.

## 7. Auftrag und Rückmeldung

Ein Auftrag benennt die gewünschte Aktion, Zielkomponente, Parameter, Gültigkeit und Voraussetzungen. Er verlangt keine unmittelbare Hardwareansteuerung durch das Smartphone.

Robin prüft:

1. gültige Sitzung, Identität und Berechtigung;
2. bekannte Aktion und gültige Parameter;
3. Aktualität, Auftragskennung und Wiederholungsstatus;
4. verfügbare Fähigkeit und erfüllte Voraussetzungen;
5. Sicherheit, Datenschutz, Benutzerentscheidungen und Konflikte mit laufenden Aktionen.

### 7.1 Auftragszustände

| Zustand | Bedeutung |
| --- | --- |
| Angenommen | Robin hat den Auftrag zugelassen; noch keine Aussage über Beginn oder Erfolg |
| Abgelehnt | Auftrag wird nicht ausgeführt; mit nachvollziehbarem Grund |
| In Ausführung | Aktion hat begonnen |
| Erfolgreich beendet | Definiertes Ziel wurde bestätigt erreicht |
| Fehlgeschlagen | Angenommener Auftrag konnte nicht erfolgreich beendet werden |
| Abgebrochen | Ausführung wurde beendet, beispielsweise durch Benutzer oder Sicherheitsregel |

Eine Empfangsbestätigung bedeutet lediglich, dass die Nachricht angekommen ist. Sie ersetzt weder Annahme noch Erfolg.

Abgelehnt, erfolgreich beendet, fehlgeschlagen und abgebrochen sind Endzustände. Ein Auftrag besitzt genau einen Endzustand. Fortschrittsmeldungen sind optional; Beginn und Endzustand müssen nachvollziehbar sein.

Eine Fristüberschreitung im Smartphone ist kein Beweis für einen Fehler oder Abbruch bei Robin. Das Smartphone zeigt den Ausgang als unbekannt an und fragt den Status ab, statt die Aktion blind erneut anzufordern.

### 7.2 Ablehnungs- und Fehlergründe

Gründe umfassen fehlende Berechtigung, inkompatible Version, unbekannte Aktion, ungültige Parameter, abgelaufene Gültigkeit, fehlendes Modul, nicht erfüllte Voraussetzung, Sicherheitsregel, unzulässige Datenverwendung, belegte Ressource und technische Störung.

Die Antwort enthält einen stabilen fachlichen Grund und eine verständliche Erklärung. Technische Details sind nur für berechtigte Diagnose zugänglich. Ein fehlendes Recht darf nicht durch eine vereinfachte oder alternative Nachricht umgangen werden.

### 7.3 Doppelte Aufträge

Dieselbe Auftragskennung mit identischem Inhalt erzeugt keine zweite Ausführung. Robin liefert den bekannten Status oder das gespeicherte Ergebnis. Dieselbe Kennung mit verändertem Inhalt wird als Konflikt abgewiesen.

Die Wiederholungserkennung ist an den berechtigten Auftraggeber gebunden und muss auch nach einer Wiederverbindung für eine definierte Aufbewahrungsdauer funktionieren. Aufbewahrungsdauer und Speichergrenzen werden vor Implementierung festgelegt.

Nach Neustart oder Ablauf dieser Aufbewahrungsdauer kann der Ausgang unbekannt sein. Robin meldet dies ausdrücklich und führt einen fraglichen Auftrag nicht allein aufgrund einer Statusabfrage erneut aus. Das Smartphone muss vor einem neuen Auftrag den tatsächlichen Zustand abgleichen.

## 8. Abbruch und Bewegungsstopp

Das Smartphone kann den Abbruch eines berechtigten Auftrags anfordern. Die Bestätigung des Abbruchwunsches bedeutet noch nicht, dass die Bewegung beendet ist. Robin meldet den tatsächlichen Endzustand nach dem sicheren Beenden.

Ist der Auftrag bereits beendet, wird sein bestehender Endzustand zurückgegeben. Lokale Sicherheitsreaktionen können einen Auftrag ohne Smartphone-Anforderung abbrechen.

Ein berechtigter Bewegungsstopp hat Vorrang vor normalen Verhaltensaufträgen. Er benötigt keine aktuell übertragene Fähigkeitsliste. Er darf jedoch den Identitäts- und Zugriffsschutz nicht umgehen. Die lokal verfügbare Stoppaktion funktioniert unabhängig von einer Smartphone-Verbindung.

## 9. Verbindungsverlust und Wiederverbindung

Das Ende beziehungsweise der Verlust einer Sitzung wird nach einer vereinbarten Zeitgrenze erkannt. Kommunikationsausfall allein ist kein Anlass, Robins Grundverhalten zu beenden.

Für jede Aktion muss festgelegt sein, ob sie bei Verbindungsverlust sicher beendet wird oder als begrenzte lokale Aktion weiterlaufen darf. Dauerhaft ferngeführte Bewegungen werden sicher beendet. Fortsetzung darf nicht unbegrenzt durch einen alten Auftrag legitimiert werden.

Nach Wiederverbindung werden Identität, Rechte und Version erneut geprüft, eine neue Sitzung aufgebaut und Fähigkeiten sowie Zustand neu abgeglichen. Laufende oder kürzlich beendete Aufträge werden über ihre Auftragskennung abgefragt. Alte Nachrichten dürfen in der neuen Sitzung keine neue Ausführung auslösen.

## 10. Erster Beispielablauf: einmal nicken

Voraussetzung: bewusst gekoppeltes Smartphone, betriebsbereite Nickmechanik und gewährtes Recht für diese Aktion.

1. Smartphone und Robin bauen eine bestätigte Sitzung auf.
2. Robin meldet Nickbewegung als verfügbare Fähigkeit samt Grenzen.
3. Robin meldet den aktuellen Zustand.
4. Smartphone fordert mit einer neuen Auftragskennung einmaliges Nicken innerhalb der gemeldeten Grenzen an.
5. Robin prüft den Auftrag und meldet Annahme oder Ablehnung.
6. Bei Annahme führt der Kopf die begrenzte Bewegung aus und meldet Beginn und Endzustand.
7. Smartphone zeigt das bestätigte Ergebnis an.

Bei blockierter Nickmechanik oder unsicherer Lage wird der Auftrag abgelehnt oder eine bereits begonnene Bewegung sicher beendet. Wird die Ergebnisnachricht verloren, fragt das Smartphone denselben Auftrag ab; es löst nicht nochmals Nicken aus.


### 10.1 Fachlicher Vertrag

Die Aktion „einmal nicken“ bewegt den Kopf von einer bestätigten Ausgangsposition einmal nach unten und anschliessend zurück zu dieser Ausgangsposition. Sie enthält genau einen Bewegungszyklus. Mehrere Nickbewegungen und direkte dauerhafte Positionierung sind spätere, eigenständige Aktionen.

Die Winkel beziehen sich auf die Nickachse der Kopfmechanik, nicht auf die Lage des gesamten Roboters im Raum. Die Auslenkung ist eine positive Winkelgrösse in Grad; die Richtung „nach unten“ wird vom Hardwareadapter eindeutig umgesetzt. Der Lagesensor unterstützt die Beurteilung der Roboterlage, ersetzt aber nicht automatisch eine Rückmeldung der Nickposition.

### 10.2 Gemeldete Fähigkeit

Robin meldet für diese Aktion:

| Eigenschaft | Bedeutung |
| --- | --- |
| Verfügbarkeit | Aktuell ausführbar oder eingeschränkt, mit Grund |
| Auslenkungsbereich | Kleinste und grösste unterstützte Auslenkung in Grad |
| Geschwindigkeitsbereich | Unterstützte Bewegungsgeschwindigkeit in Grad pro Sekunde |
| Standardprofil | Lokal festgelegte Auslenkung und Geschwindigkeit |
| Positionsgrenzen | Zulässiger Bereich der Nickachse |
| Referenzzustand | Ob Ausgangsposition und Bewegungsreferenz bekannt sind |
| Ergebnisnachweis | Physisch bestätigt oder nur Ausführung des Bewegungsablaufs bestätigt |
| Laufzeitgrenze | Maximale Ausführungszeit in Millisekunden |
| Revision | Stand der gemeldeten Grenzen und Voraussetzungen |

Konkrete Zahlen werden anhand der Mechanik festgelegt. Die tatsächlich zulässige Auslenkung hängt zusätzlich von der aktuellen Ausgangsposition ab: Sowohl Zielposition als auch vollständiger Bewegungsweg müssen innerhalb der sicheren Grenzen liegen.

### 10.3 Auftragsparameter

| Parameter | Festlegung |
| --- | --- |
| Auftragskennung | Eindeutig beim berechtigten Auftraggeber; für Wiederholung und Statusabfrage beibehalten |
| Sitzung | Aktuelle bestätigte Sitzung |
| Ziel | Nickfunktion des Kopfes |
| Aktion | Einmal nicken |
| Auslenkung | Optional; Grad innerhalb des gemeldeten Bereichs |
| Geschwindigkeit | Optional; Grad pro Sekunde innerhalb des gemeldeten Bereichs |
| Startgültigkeit | Begrenzte Frist, innerhalb der die Bewegung beginnen darf |
| Erwartete Fähigkeitsrevision | Revision, auf deren Grundlage der Auftrag geplant wurde |

Fehlende optionale Bewegungsparameter werden aus dem gemeldeten lokalen Standardprofil übernommen. Robin bestätigt die tatsächlich verwendeten Werte bei Annahme. Ungültige Werte werden abgelehnt und nicht stillschweigend auf andere Werte begrenzt.

Die konkrete Darstellung der Frist wird mit der technischen Gültigkeitsprüfung festgelegt. Sie muss verzögerte Zustellung und Uhrabweichungen berücksichtigen. Zusätzlich begrenzt Robin die Ausführungsdauer lokal. Eine noch gültige Startfrist erlaubt keine unbegrenzt lange Bewegung.

### 10.4 Prüfung und Ausführung

Vor der Annahme prüft Robin die allgemeinen Auftragsregeln sowie Referenzzustand, sicheren Bewegungsweg, Lage, Energie, Mechanikstatus und freie Nickfunktion. Bei nicht mehr aktueller Fähigkeitsrevision fordert Robin einen neuen Abgleich an und lehnt den Auftrag ab.

Für den ersten Umfang wird keine Warteschlange für Nickaufträge vorgesehen. Ist die Nickfunktion belegt, wird ein neuer Auftrag abgelehnt. Wiederholungen desselben Auftrags werden weiterhin nach den Regeln zur Wiederholungserkennung behandelt.

Nach Annahme wird die Nickfunktion für diesen Auftrag reserviert. Unmittelbar vor Bewegungsbeginn werden die Voraussetzungen und die Startgültigkeit erneut geprüft. Die Ausgangsposition wird dabei erfasst und mit dem Auftrag verbunden.

Die Ausführung umfasst Abwärtsbewegung und Rückkehr. Währenddessen überwacht Robin die verfügbaren sicherheitsrelevanten Rückmeldungen. Eine Änderung von Zustand oder Grenzen kann die Bewegung beenden, auch wenn der Auftrag zuvor angenommen wurde.

### 10.5 Abschluss und Nachweis

Ein Ergebnis enthält den Endzustand, die verwendeten Parameter, den Ergebnisnachweis sowie die bekannte Endposition beziehungsweise eine ausdrückliche Kennzeichnung unbekannter Position.

„Erfolgreich beendet“ erfordert einen abgeschlossenen Zyklus und eine Rückkehr zur Ausgangsposition innerhalb einer zuvor festgelegten Toleranz. Die Fähigkeit muss offenlegen, wie dieser Nachweis erbracht wird.

Kann die Hardware lediglich bestätigen, dass der geplante Bewegungsablauf ausgegeben wurde, darf das Ergebnis keine physisch bestätigte Rückkehr behaupten. Es wird entsprechend als Ablaufbestätigung gekennzeichnet. Benötigt ein Auftrag eine physische Bestätigung, die nicht verfügbar ist, muss er abgelehnt werden. Die Parametrisierung dieses Nachweisbedarfs wird vor Implementierung ergänzt.

Ein angenommener Auftrag, der wegen abgelaufener Startfrist nicht beginnt, endet fehlgeschlagen mit dem Grund „Startfrist abgelaufen“. Störungen und überschrittene Ausführungszeit führen ebenfalls zu einem erklärten Fehler nach sicherem Beenden. Ein Benutzerabbruch oder sicherheitsbedingter Abbruch wird als abgebrochen gemeldet.

### 10.6 Abbruch und Verbindungsverlust

Bei Abbruch oder Bewegungsstopp wird die Bewegung auf die für die Mechanik sichere Weise beendet. Eine automatische Rückfahrt zur Ausgangsposition ist dabei nicht vorgeschrieben: Sie könnte dem Stoppwunsch widersprechen oder bei einer Störung unsicher sein.

Für diesen ersten Nickauftrag wird als Entwurfsentscheidung festgelegt: Ein erkannter Verlust der Smartphone-Sitzung bricht eine noch laufende Bewegung sicher ab. Ein bereits abgeschlossener Auftrag behält seinen Endzustand. Die Zeit bis zur Erkennung des Verbindungsverlusts sowie das sichere Brems- beziehungsweise Halteverhalten müssen vor Hardwarebetrieb festgelegt werden.

Nach Wiederverbindung kann das Smartphone den Status abfragen. Eine Rückkehr zur Ausgangsposition erfolgt nicht durch Wiederholung oder Statusabfrage, sondern nur durch eine separat geprüfte neue Aktion.

### 10.7 Fachliche Prüffälle für den Nickauftrag

Virtual Robin prüft die Verwendung des Standardprofils, gültige und ungültige Parameter, veränderte Fähigkeitsrevision, unbekannte Referenz, belegte Nickfunktion, abgelaufene Startfrist, vollständigen Zyklus, fehlende physische Ergebnisbestätigung, Laufzeitüberschreitung und Abbruch in beiden Bewegungsphasen.

Zusätzlich werden Verbindungsverlust sowie doppelte Aufträge vor, während und nach Ausführung geprüft. Mechanische Grenzen, Positionsnachweis und sicheres Stoppen werden ergänzend an realer Hardware erprobt.

## 11. Erweiterung für die Homestation

Die Homestation wird als eigene Komponente mit Lade-, Dreh- und Lichtfähigkeiten beschrieben. Das Smartphone richtet seine Verhaltensaufträge an den Robin-Kern; dieser koordiniert Stationsaufträge.

Eine Stationsdrehung benötigt bestätigtes Andocken, betriebsbereiten Antrieb, zulässige Bewegungsgrenzen und die erforderlichen Ladebedingungen. Die Station setzt lokale Schutzreaktionen auch bei Kommunikationsverlust um.

Leuchtring-Aufträge können Zustandsanzeigen und Disco-Effekte betreffen. Sicherheitsrelevante Anzeigen haben Vorrang vor Disco-Effekten. Die folgenden Abschnitte konkretisieren den Leuchtring. Die Stationsdrehung wird anschliessend als relativer Auftrag beschrieben. Konkrete Grenzwerte, Farbcodierung und Choreografie bleiben offen.

### 11.1 Fachlicher Vertrag für den Leuchtring

Die Homestation stellt zwei getrennte Lichtfunktionen bereit: automatische Zustandsanzeigen und begrenzte dekorative Effekte. Das Smartphone kann einen dekorativen Effekt anfordern. Fachliche Zustände werden von den zuständigen lokalen Komponenten gemeldet; ein Smartphone darf durch einen frei gewählten Effekt keinen Lade- oder Sicherheitszustand vortäuschen.

Der Robin-Kern prüft Lichtaufträge und leitet sie an die Station weiter. Die Station setzt die Ausgabe um und sichert ihre lokalen Lade- und Fehleranzeigen auch ohne Smartphone ab.

### 11.2 Gemeldete Lichtfähigkeit

| Eigenschaft | Bedeutung |
| --- | --- |
| Verfügbarkeit | Station verbunden, Leuchtring betriebsbereit oder Einschränkungsgrund |
| Unterstützte Effekte | Verfügbare Effektkennungen, beispielsweise ruhiges Licht, sanftes Pulsieren oder Disco |
| Parameter je Effekt | Unterstützte Parameter, Einheiten, Grenzen und Standardwerte |
| Helligkeit | Unterstützter Bereich von 0 bis 100 Prozent der freigegebenen maximalen Helligkeit |
| Laufzeit | Zulässige Dauer und lokaler Standardwert in Millisekunden |
| Farbunterstützung | Unterstützte benannte Farben oder Paletten, soweit vorhanden |
| Ausgabeumfang | Ganzer Ring; segmentweise Ausgabe nur bei ausdrücklich gemeldeter Fähigkeit |
| Rückmeldung | Bestätigter Ausgabestatus und verfügbare Fehlererkennung |
| Revision | Stand der Fähigkeit und ihrer Grenzen |

Farben, Paletten und Effekte werden als unterstützte Auswahl angeboten. Dieser Entwurf setzt keine konkrete Farbcodierung voraus. Auch Disco bezeichnet hier einen benannten Effekt, keine bereits definierte Musiksteuerung oder Choreografie.

Der erste Umfang enthält keine freie Programmierung von Blinkfolgen oder einzelnen Lichtpunkten. Grenzwerte und Effektgestaltung werden bei der Erprobung festgelegt. Ein Lichtauftrag setzt keine Drehbewegung in Gang; koordinierte Licht- und Bewegungsabläufe werden später separat beschrieben.

### 11.3 Auftrag für einen dekorativen Effekt

| Parameter | Festlegung |
| --- | --- |
| Auftragskennung und Sitzung | Entsprechend den gemeinsamen Auftragsregeln |
| Ziel | Leuchtring der Homestation |
| Aktion | Dekorativen Effekt ausgeben |
| Effekt | Eine aktuell gemeldete Effektkennung |
| Helligkeit | Optional; Prozent innerhalb des gemeldeten Bereichs |
| Farbe oder Palette | Optional, soweit für den Effekt unterstützt |
| Dauer | Optional; begrenzt durch die gemeldete Laufzeit |
| Startgültigkeit | Begrenzte Frist für den Beginn |
| Erwartete Fähigkeitsrevision | Stand, auf dem der Auftrag basiert |

Fehlende optionale Werte werden aus dem gemeldeten Standardprofil übernommen. Die Annahme enthält die tatsächlich verwendeten Werte. Ungültige oder nicht unterstützte Parameter werden abgelehnt und nicht stillschweigend angepasst.

Für den ersten Umfang ist jeweils ein dekorativer Lichtauftrag aktiv. Ein weiterer Auftrag wird bei belegter Lichtfunktion abgelehnt. Ein Wechsel erfolgt nach bestätigtem Abbruch beziehungsweise Abschluss des bisherigen Auftrags. Wiederholungen derselben Auftragskennung starten den Effekt nicht neu und verlängern seine Laufzeit nicht.

### 11.4 Zustandsanzeigen und Priorität

Die Station unterscheidet lokal bekannte Zustände, beispielsweise Ladebereitschaft, Laden, Ladeabschluss und Ladefehler. Von Robin gemeldete Zustände müssen Quelle und Aktualität enthalten. Unbekannte oder veraltete Zustände dürfen nicht als bestätigter Normalzustand angezeigt werden.

Die Entwurfspriorität lautet:

1. Sicherheitsrelevante Fehler- und Warnanzeigen.
2. Andere aktuell erforderliche Zustandsanzeigen.
3. Dekorative Effekte.

Die konkrete Zuordnung von Zuständen zu Anzeigeprofilen und Prioritäten wird separat festgelegt. Auch die Entscheidung, welche normalen Zustände dauerhaft sichtbar sein müssen, bleibt offen. Ein aktiver Ladevorgang blockiert deshalb nicht automatisch jeden Disco-Effekt.

Ist eine höherrangige Anzeige bereits aktiv und beansprucht den Ring, wird der dekorative Auftrag mit einem erklärten Prioritätskonflikt abgelehnt. Wird eine solche Anzeige während eines Effekts erforderlich, wird der Effekt abgebrochen und die Zustandsanzeige ausgegeben. Er startet anschliessend nicht automatisch erneut.

Ein dekorativer Ausschaltwunsch beziehungsweise Helligkeit null schaltet nur die dekorative Ausgabe aus. Er darf notwendige Zustandsanzeigen nicht unterdrücken.

### 11.5 Ausführung und Ergebnis

Robin meldet Annahme erst nach Prüfung von Rechten, Parametern, Revision, Aktualität, Stationsverfügbarkeit und Priorität. Die Meldung „In Ausführung“ folgt erst auf die Bestätigung der Station, dass der Effekt gestartet wurde.

Die Station begrenzt die Laufzeit lokal. Bei Ablauf beendet sie den dekorativen Effekt und kehrt zur aktuell erforderlichen Zustandsanzeige beziehungsweise zum festgelegten Ruhezustand des Rings zurück. Der Auftrag endet erfolgreich, wenn die definierte Laufzeit und das anschliessende Beenden bestätigt wurden.

Ein Abbruchwunsch beendet den dekorativen Auftrag und gibt den Ring für die aktuelle Zustandsanzeige frei. Eine höherrangige Anzeige führt ebenfalls zum Endzustand „Abgebrochen“, mit entsprechendem Grund.

Rückmeldungen enthalten Auftrag, Effekt, verwendete Parameter, Beginn, Endzustand und Einschränkungen. Die Station unterscheidet bestätigte Ausgabeansteuerung von tatsächlich überprüfter Lichtemission. Ohne geeignete Rückmeldung darf sie keine optische Prüfung behaupten.

### 11.6 Verbindungsverlust und Fehler

Als Entwurfsentscheidung wird festgelegt: Bei erkanntem Verlust der Smartphone-Sitzung bricht Robin deren aktiven dekorativen Auftrag ab. Bei Verlust der Verbindung zwischen Robin und Station beendet die Station den Effekt nach einer lokal festgelegten Ausfallfrist selbstständig. Die Auftragslaufzeit bildet eine zusätzliche Obergrenze.

Lokale Sicherheits- und Ladeanzeigen bleiben unabhängig davon wirksam. Nicht mehr aktuelle Robin-Zustände werden als unbekannt behandelt. Nach Wiederverbindung werden Station, Priorität und Ausgabe abgeglichen; ein alter Effekt wird nicht automatisch fortgesetzt.

Fehlen Rückmeldungen zum Beenden, zeigt das Smartphone den Auftrag als unbekannt an. Es behauptet weder, der Ring sei ausgeschaltet, noch, der Effekt laufe weiter. Die Station beendet den Effekt anhand ihrer lokalen Grenzen.

Ein erkannter Leuchtringfehler wird im technischen Status gemeldet. Der sichere Ladebetrieb muss bei Ausfall der dekorativen Lichtfunktion erhalten bleiben; erforderliche Sicherheitsreaktionen werden unabhängig von der Anzeige ausgeführt.

### 11.7 Beispiel und Prüfung

Beispiel: Das Smartphone fordert einen von der Station angebotenen Disco-Effekt mit Standardhelligkeit und begrenzter Dauer an. Robin prüft und bestätigt die verwendeten Werte, die Station startet den Effekt und bestätigt den Beginn. Nach Ablauf wird der Effekt beendet und die aktuelle Zustandsanzeige wieder dargestellt.

Virtual Robin prüft Standardwerte, ungültige Parameter, fehlende Station, belegten Ring, höhere Anzeigepriorität vor und während der Ausgabe, Laufzeitende, Benutzerabbruch, doppelte Zustellung ohne Laufzeitverlängerung und beide Arten von Verbindungsverlust.

Reale Hardwareprüfungen ergänzen Effektgestaltung, Helligkeit, Anzeigeerkennbarkeit, lokale Laufzeitbegrenzung und Verhalten bei Kommunikationsausfall.

### 11.8 Fachlicher Vertrag für Stationsdrehung

Die Homestation dreht den angedockten Robin mit ihrem eigenen Antrieb. Der erste Drehauftrag ist eine begrenzte relative Drehung aus der bei Bewegungsbeginn bestätigten Ausgangsposition. Er endet an der Zielposition und enthält keine automatische Rückfahrt.

Eine angeforderte volle Umdrehung darf nur angenommen werden, wenn die Station den vollständigen Bewegungsweg aus der aktuellen Position freigeben kann. Die Anforderung einer 360°-Drehmöglichkeit allein legt weder endlose Rotation noch beliebig wiederholbare Umdrehungen fest.

Absolute Ausrichtung, kontinuierliches Drehen und gekoppelte Disco-Choreografie sind spätere Erweiterungen.

### 11.9 Gemeldete Drehfähigkeit

| Eigenschaft | Bedeutung |
| --- | --- |
| Verfügbarkeit | Stationsantrieb betriebsbereit oder Einschränkungsgrund |
| Drehrichtungen | Unterstützte Richtungen, von oben auf die Station betrachtet |
| Winkelbereich | Unterstützte relative Drehwinkel in Grad |
| Geschwindigkeitsbereich | Unterstützte Geschwindigkeit in Grad pro Sekunde |
| Standardgeschwindigkeit | Lokal festgelegter Standardwert |
| Bewegungsreferenz | Bekanntheit der Position und gegebenenfalls mechanischer Grenzen |
| Aktuell zulässiger Weg | Noch freigegebener Drehweg je Richtung aus der aktuellen Position |
| Ladeverbindung | Bestätigter, unbekannter oder gestörter Zustand |
| Andockzustand | Bestätigt angedockt, nicht angedockt oder unbekannt |
| Ergebnisnachweis | Physisch bestätigte Drehung oder nur bestätigter Bewegungsablauf |
| Laufzeitgrenze | Maximale Ausführungsdauer in Millisekunden |
| Revision | Stand der Fähigkeit und ihrer Grenzen |

Positionswerte müssen zum Drehantrieb der Station gehören. Ein Lagesensor im Kopf ist allein kein bestätigter Nachweis des Stationswinkels. Bei mechanisch begrenzter Rotation muss die Station den bisher genutzten Drehweg berücksichtigen; mehrfach zugestellte oder neu angeforderte Umdrehungen dürfen die Grenze nicht umgehen.

Zahlenwerte für Winkel, Geschwindigkeit, Positionstoleranz und Laufzeit werden nach Erprobung der Mechanik festgelegt.

### 11.10 Auftragsparameter

| Parameter | Festlegung |
| --- | --- |
| Auftragskennung und Sitzung | Entsprechend den gemeinsamen Auftragsregeln |
| Ziel | Drehantrieb der Homestation |
| Aktion | Begrenzte relative Drehung |
| Richtung | Im oder gegen den Uhrzeigersinn, von oben betrachtet |
| Winkel | Positive Winkelgrösse in Grad innerhalb des gemeldeten Bereichs |
| Geschwindigkeit | Optional; Grad pro Sekunde innerhalb des gemeldeten Bereichs |
| Startgültigkeit | Begrenzte Frist für den Bewegungsbeginn |
| Erwartete Fähigkeitsrevision | Stand, auf dem der Auftrag geplant wurde |
| Benötigter Ergebnisnachweis | Physisch bestätigte Drehung oder ausdrücklich akzeptierte Ablaufbestätigung |

Richtung und Winkel sind erforderlich. Bei fehlender Geschwindigkeit wird der gemeldete Standardwert verwendet und bei Annahme bestätigt. Ein Winkel von null ist kein Bewegungsauftrag und wird abgelehnt. Ungültige Werte werden nicht stillschweigend begrenzt.

Ist kein Ergebnisnachweis angegeben, wird physische Bestätigung verlangt. Eine reine Ablaufbestätigung darf nur ausdrücklich angefordert und nur für entsprechend freigegebene Anwendungen akzeptiert werden. Sie ersetzt keinen für sicheren Betrieb erforderlichen Positionsnachweis.

### 11.11 Prüfung und Ausführung

Der Robin-Kern prüft Sitzung, Rechte, Parameter, Gültigkeit, Revision und übergreifende Sicherheitsregeln. Die Station prüft zusätzlich bestätigtes Andocken, Antrieb, vollständigen Bewegungsweg, Ladeverbindung und ihre lokalen Schutzbedingungen.

Unbekanntes Andocken oder unbekannte sicherheitsrelevante Grenzen führen zur Ablehnung. Die Station darf keine bekannte mechanische Grenze überschreiten, auch wenn der angeforderte Winkel allgemein unterstützt wird.

Für den ersten Umfang ist nur ein Drehauftrag aktiv; weitere Drehaufträge werden bei belegtem Antrieb abgelehnt. Derselbe Auftrag wird nach den Wiederholungsregeln behandelt und nicht erneut ausgeführt.

Die Annahme folgt erst nach Bestätigung und Reservierung durch die Station. Unmittelbar vor dem Beginn werden Voraussetzungen und Startgültigkeit erneut geprüft. Die Station erfasst die Ausgangsposition beziehungsweise Bewegungsreferenz, bestätigt die verwendeten Parameter und meldet den Beginn.

Die Ladeverbindung muss während der Drehung erhalten bleiben. Das bedeutet nicht, dass ständig Ladestrom fliessen muss: Ladeabschluss oder ein regulärer Ladezustandswechsel sind keine Unterbrechung der Verbindung.

Bei Verlust der Ladeverbindung, des sicheren Andockzustands oder anderer erforderlicher Schutzbedingungen wird die Drehung sicher beendet. Die konkreten Erkennungs- und Stoppverfahren werden vor Hardwarebetrieb festgelegt.

### 11.12 Abschluss, Fehler und Abbruch

Ein erfolgreicher physisch bestätigter Auftrag hat den angeforderten relativen Winkel innerhalb der festgelegten Toleranz erreicht und die Bewegung beendet. Eine zurückgelegte volle Umdrehung muss als Bewegungsweg bestätigt werden; dieselbe Endausrichtung wie am Anfang beweist für sich allein keine 360°-Drehung.

Das Ergebnis enthält verwendete Parameter, Nachweisart, bekannten zurückgelegten Winkel, Endposition beziehungsweise Bewegungsreferenz und Endzustand. Nicht verfügbare Werte werden als unbekannt gekennzeichnet. Eine Ablaufbestätigung behauptet keine gemessene Drehung.

Störung und überschrittene Laufzeit enden nach sicherem Beenden als fehlgeschlagen. Benutzerstopp oder Eingriff einer Sicherheitsregel enden als abgebrochen, mit nachvollziehbarem Grund. Erreicht die Station das Ziel bereits vor einem verspäteten Abbruchwunsch, bleibt der bestätigte Erfolgszustand erhalten.

Ein Stopp führt nicht automatisch zur Ausgangsposition zurück. Eine Rückfahrt benötigt einen neuen geprüften Auftrag. Bremsen, Halten oder Freigeben des Antriebs erfolgen nach dem für die Mechanik sicheren Verfahren; ein konkretes Verfahren wird hier nicht vorweggenommen.

### 11.13 Verbindungsverlust

Als Entwurfsentscheidung gilt: Bei erkanntem Verlust der Smartphone-Sitzung bricht der Robin-Kern deren laufenden Drehauftrag ab. Bei Verlust der Verbindung zwischen Kern und Station beendet die Station die Drehung nach ihrer lokalen Ausfallfrist selbstständig. Die lokale Laufzeitgrenze bleibt zusätzlich wirksam.

Nach Wiederverbindung werden Auftrag, Position, Andockzustand, Ladeverbindung und zulässiger Restweg abgeglichen. Der verbleibende Teil einer Drehung wird nicht automatisch fortgesetzt. Ist der Ausgang unbekannt, muss die Station vor weiteren Bewegungen eine sichere Referenz beziehungsweise sichere Grenzen herstellen.

### 11.14 Beispiel und Prüfung

Das Smartphone fordert eine von der Station angebotene relative Drehung im Uhrzeigersinn mit einem zulässigen Winkel an. Der Kern und die Station prüfen den Auftrag. Die Station dreht Robin, erhält dabei die Ladeverbindung und meldet nach Ende den tatsächlichen Ergebnisnachweis.

Virtual Robin prüft beide Richtungen, Standardgeschwindigkeit, unzulässige Parameter, nicht bestätigtes Andocken, unbekannte Referenz, überschrittenen Restweg, belegten Antrieb, volle Umdrehung, Abbruch und Laufzeitüberschreitung.

Zusätzlich werden Verlust der Ladeverbindung und beider Kommunikationsverbindungen, fehlender Positionsnachweis, verlorenes Ergebnis und doppelte Zustellung ohne zweite Drehung geprüft. Reale Hardwareprüfungen ergänzen mechanische Grenzen, Andocksicherheit, Positionsnachweis, Erhalt der Ladeverbindung und sicheren Stopp.

## 12. Prüfung des ersten Protokollumfangs

Virtual Robin soll mindestens folgende Szenarien reproduzierbar abbilden:

- bestätigte Verbindung mit kompatibler Version;
- abgewiesene Verbindung ohne gültige Rechte oder kompatible Version;
- Fähigkeits- und Zustandsabgleich;
- erfolgreicher, abgelehnter und während der Ausführung fehlgeschlagener Nickauftrag;
- doppelte Auftragszustellung ohne zweite Ausführung;
- verlorene Ergebnisnachricht mit anschliessender Statusabfrage;
- Abbruch und Bewegungsstopp;
- Verbindungsverlust und neue Sitzung ohne Wiederholung alter Aktionen;
- verlorene Zustandsänderung mit erneutem Vollabgleich.

Dies sind fachliche Prüfszenarien; eine konkrete Implementierung besteht mit diesem Dokument noch nicht.

## 13. Offene Festlegungen

Vor einer Implementierung werden Nachrichtencodierung, Übertragungswege, sichere Identitätsprüfung, Sitzungs- und Nachrichtenkennungen, Versionsregeln sowie konkrete Zeit- und Speichergrenzen festgelegt.

Für die erste Aktion werden die in Abschnitt 10 beschriebenen Regeln technisch konkretisiert: Zahlenwerte für Bewegungsgrenzen, Standardprofil und Positionstoleranz, tatsächlicher Positionsnachweis, Darstellung des benötigten Ergebnisnachweises, Gültigkeitsprüfung, Ausführungszeit und sicheres Stoppen. Für den Leuchtring bleiben konkrete Effekt- und Anzeigeprofile, Zustandsprioritäten, Laufzeit- und Ausfallfristen sowie Rückmeldemöglichkeiten vor Implementierung festzulegen. Für die Stationsdrehung bleiben insbesondere mechanische Grenzen, Möglichkeit wiederholter voller Umdrehungen, Referenz- und Positionsnachweis, Andock- und Ladeverbindungsüberwachung, Stoppverfahren sowie Zeitgrenzen festzulegen. Endlose Rotation ist weiterhin offen. Die weiteren Funktionsbereiche erhalten eigene, auf diesem Grundablauf aufbauende Spezifikationen.
