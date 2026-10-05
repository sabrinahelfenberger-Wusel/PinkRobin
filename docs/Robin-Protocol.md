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

## 11. Erweiterung für die Homestation

Die Homestation wird als eigene Komponente mit Lade-, Dreh- und Lichtfähigkeiten beschrieben. Das Smartphone richtet seine Verhaltensaufträge an den Robin-Kern; dieser koordiniert Stationsaufträge.

Eine Stationsdrehung benötigt bestätigtes Andocken, betriebsbereiten Antrieb, zulässige Bewegungsgrenzen und die erforderlichen Ladebedingungen. Die Station setzt lokale Schutzreaktionen auch bei Kommunikationsverlust um.

Leuchtring-Aufträge können Zustandsanzeigen und Disco-Effekte betreffen. Sicherheitsrelevante Anzeigen haben Vorrang vor Disco-Effekten. Der erste Entwurf legt noch keine Winkelreferenz, Geschwindigkeit, Farbcodierung oder Choreografie fest.

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

Für die erste Aktion werden zusätzlich Nickbewegungsgrenzen, Gültigkeitsprüfung, Verhalten bei Verbindungsverlust und Kriterien für bestätigten Erfolg festgelegt. Die weiteren Funktionsbereiche erhalten eigene, auf diesem Grundablauf aufbauende Spezifikationen.
