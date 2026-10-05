# Robin Protocol

Status: erster fachlicher Entwurf zur gemeinsamen Abstimmung. Entwurfsstand 0.1; dies ist noch keine implementierte Protokollversion.

## 1. Zweck und Geltungsbereich

Das Robin Protocol beschreibt die hardwareunabhängige Bedeutung der Kommunikation zwischen Robin, Smartphone, Modulen, Homestation und Simulatoren. Grundlagen sind das [Lastenheft](Lastenheft.md), die [Systemarchitektur](Systemarchitektur.md) und die [Robin Principles](Robin-Principles.md).

Dieser erste Entwurf behandelt die Verbindung eines bereits bewusst gekoppelten Smartphones mit Robin, den Austausch von Fähigkeiten und Zustand sowie einen begrenzten Auftrag mit Rückmeldung. Er beschreibt auch Abbruch, Ablehnung und Wiederverbindung.

Übertragungswege, konkrete Nachrichtenformate und Verfahren zum Identitätsnachweis werden später festgelegt. Die Erstkopplung, Personenverwaltung, Updates, Livestreams und Synchronisation persönlicher Daten erhalten eigene Abläufe. Eine bestehende Verbindung erteilt für diese Funktionen keine pauschale Berechtigung.

## 2. Rollen und Grundregeln

Das Smartphone führt erweitertes Verhalten aus und sendet Vorschläge oder Aufträge. Der Robin-Kern im Kopf prüft und koordiniert deren lokale Ausführung. Module und Homestation führen die ihnen zugeordneten Aktionen aus und melden Ergebnisse zurück.

Robin bleibt die entscheidende Instanz für Sicherheit, Datenschutz, Benutzerregeln und tatsächlich verfügbare Fähigkeiten. Die Station verantwortet ihren Drehantrieb und Leuchtring; der Bauch verantwortet Energieversorgung und Ladeelektronik.

Virtual Robin bietet denselben fachlichen Vertrag an. Simulierte Fähigkeiten und Ergebnisse werden ausdrücklich als simuliert gekennzeichnet.

Die folgenden Nachrichtennamen sind fachliche Bezeichnungen, keine Festlegung von Feldnamen oder Codierung.

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

Leuchtring-Aufträge können Zustandsanzeigen und Disco-Effekte betreffen. Sicherheitsrelevante Anzeigen haben Vorrang vor Disco-Effekten. Die folgenden Abschnitte konkretisieren den Leuchtring. Winkelreferenz und Geschwindigkeit der Stationsdrehung sowie konkrete Farbcodierung und Choreografie bleiben offen.

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

Für die erste Aktion werden die in Abschnitt 10 beschriebenen Regeln technisch konkretisiert: Zahlenwerte für Bewegungsgrenzen, Standardprofil und Positionstoleranz, tatsächlicher Positionsnachweis, Darstellung des benötigten Ergebnisnachweises, Gültigkeitsprüfung, Ausführungszeit und sicheres Stoppen. Für den Leuchtring bleiben konkrete Effekt- und Anzeigeprofile, Zustandsprioritäten, Laufzeit- und Ausfallfristen sowie Rückmeldemöglichkeiten vor Implementierung festzulegen. Die weiteren Funktionsbereiche erhalten eigene, auf diesem Grundablauf aufbauende Spezifikationen.
