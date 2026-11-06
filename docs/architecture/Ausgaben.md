# Core-Ausgaben

Der Core erzeugt typisierte Ausgaben. Sichtbare Antworten enthalten
zusätzlich den fertigen deutschen Text. Adapter übernehmen Transport,
Darstellung und Bestätigung.

Die Formulierungen liegen zentral in `core/src/output_text.cpp`.
Angular zeigt den gelieferten Text an.

## Ausgabearten

| Art | Inhalt | Verwendung |
| --- | --- | --- |
| `weatherAnswer` | Wettereinordnung und Text | Wetterantwort anzeigen |
| `conversationReply` | Antworttyp und Text | Gesprächsantwort anzeigen |
| `weatherFetchRequested` | Anfrage-ID und Schweizer PLZ | Wetterdaten durch einen Adapter laden |

Technische Wetterabrufe enthalten keinen Anzeigetext. Das Backend
übernimmt sie intern; sie werden nicht als sichtbare Ausgabe an
Angular weitergegeben.

Die Antworttypen bleiben als strukturierte Zusatzinformation erhalten.
Die Textanzeige benötigt keine lokale Zuordnung von Antworttypen zu
Formulierungen.

## Ablauf einer Texteingabe

1. Angular sendet `textReceived` mit Text und Befehlsnummer.
2. Das Backend übergibt die Eingabe an den Core.
3. Der Core verarbeitet die Eingabe und legt eine Ausgabe ab.
4. Angular liest die älteste sichtbare Ausgabe.
5. Angular zeigt deren Text acht Sekunden lang an.
6. Angular bestätigt die Ausgabesequenz.

Eine Wetterfrage kann zunächst eine Rückfrage nach der PLZ oder
einen technischen Wetterabruf auslösen. Den vollständigen Ablauf
beschreibt [Textgespräche](Textgespraeche.md).

## Begrenzter Text

Eine Ausgabe enthält höchstens 512 UTF-8-Bytes Anzeigetext.
Die gespeicherte Länge zählt Bytes, nicht Zeichen; ein abschliessendes
Nullzeichen ist nicht erforderlich.

Antworttyp und Text werden gemeinsam im Ausgabepuffer gespeichert.
Wiederholtes Lesen liefert denselben gespeicherten Text.

Ungültige Antworttypen oder zu lange Formulierungen werden abgewiesen.
Texte werden nicht still abgeschnitten.

C++-Quelldateien werden als UTF-8 gespeichert. Unter MSVC setzt CMake
für Core, Tests und Beispiele die Compileroption `/utf-8`.

## Wetterantworten

| Wert | Bedeutung |
| --- | --- |
| `unknown` | Keine aktuell gültigen Wetterdaten vorhanden |
| `dry` | Kein Niederschlag prognostiziert |
| `rain` | Niederschlag prognostiziert |
| `storm` | Mindestens 10 mm Niederschlag und Böen ab 60 km/h |

`storm` ist eine interne Einordnung des Prototyps und keine amtliche
Unwetterwarnung.

Die direkte Wetterantwort wertet vorhandenes Wetterwissen aus.
Der Gesprächsablauf kann bei fehlendem Wissen einen separaten Abruf
anfordern. Die Antwort selbst startet keine Verwandlung.

## Gesprächsantworten

| Wert | Bedeutung |
| --- | --- |
| `unknownInput` | Eingabe nicht verstanden |
| `askPostalCode` | Schweizer PLZ erfragen |
| `invalidPostalCode` | Eingabe ist keine erwartete PLZ |
| `weatherUnavailable` | Keine gültige Prognose geladen |
| `attention` | Auf „Robin?“ reagieren |
| `greeting` | Begrüssung beantworten |
| `gratitude` | Auf Dank reagieren |

## Ausgabepuffer

- Jede Core-Instanz besitzt acht Ausgabeplätze.
- Sequenznummern beginnen bei 1 und steigen fortlaufend.
- Lesen entfernt keine Ausgabe.
- Bestätigt werden kann die älteste wartende Ausgabe.
- Bereits bestätigte Sequenznummern dürfen erneut bestätigt werden.
- Ein voller Puffer weist neue Ausgaben ab.
- Abgewiesene Ausgaben verbrauchen keine Sequenznummer.
- Bei erschöpftem Sequenzbereich werden neue Ausgaben abgewiesen.

Die Ausgabesequenz ist von der Befehlsnummer unabhängig.
Sequenznummern gelten nur innerhalb ihrer Core-Instanz beziehungsweise
Session.

## HTTP-Schnittstelle

Die Aufrufe verwenden den Header `X-Robin-Session`.

| Methode | Pfad | Zweck |
| --- | --- | --- |
| POST | `/api/v1/session/events` | Sequenzierte Eingabe senden |
| GET | `/api/v1/session/outputs/next` | Nächste sichtbare Ausgabe lesen; bei Bedarf internen Wetterabruf bearbeiten |
| POST | `/api/v1/session/outputs/ack` | Ausgabe bestätigen |

Das Lesen liefert HTTP 200 bei vorhandenem Inhalt, andernfalls HTTP 204.
Ein interner Wetterabruf kann die Antwort verzögern. Angular verarbeitet
die Ausgabeabfrage unabhängig von Zustandsabfragen und neuen Befehlen.

Beispiel einer Eingabe:

```json
{
  "sequence": 1,
  "event": "textReceived",
  "text": "Robin?"
}
```

Beispiel einer sichtbaren Antwort, ohne ungenutzte optionale Felder:

```json
{
  "sequence": "1",
  "kind": "conversationReply",
  "conversationReply": "attention",
  "text": "Ja? Ich bin hier! ♡"
}
```

Beispiel einer Bestätigung:

```json
{
  "sequence": "1"
}
```

Ausgabesequenzen werden als Dezimalstrings übertragen, damit JavaScript
auch grosse 64-Bit-Werte unverändert verarbeitet.

Eine Wiederholung des zuletzt angenommenen Befehls mit derselben
Befehlsnummer, demselben Ereignis und demselben Text erzeugt keine
erneute Eingabe. Abweichende Wiederholungen werden abgewiesen.

## Native Schnittstelle

`robin_core_get_next_output_v2` liefert Antwortmetadaten, Textlänge
und einen festen Textpuffer. Das während der Entwicklung von v0.1.6
erweiterte v2-Format benötigt 560 Bytes.

Das bisherige Ausgabeformat bleibt unverändert. Neue Adapter verwenden
v2 für Gesprächsantworten und technische Wetterabrufe.

## Darstellung und Bestätigung

Der Angular-Session-Service hält höchstens eine lokale Ausgabe bereit.
Solange sie wartet, wird sie nicht durch eine weitere ersetzt.

Robin zeigt den gelieferten Text über `displayedSpeech` an.
Die bestehenden Begrüssungs-, Berührungs- und Verwandlungstexte der
Darstellung bleiben separat erhalten.

Nach acht Sekunden versucht Angular die Ausgabe zu bestätigen.
Bei einem Fehler bleibt sie sichtbar; die Bestätigung wird nach
zwei Sekunden erneut versucht.

Der Antwort-Timer wird unabhängig von den Animationstimern verwaltet
und beim Wechsel der Ausgabe oder beim Zerstören der Komponente
bereinigt.

Technische Wetterabrufe bestätigt das Backend erst nach Übernahme
in seine eigene laufende Arbeit. Noch nicht übergebbare Ergebnisse
bleiben dort für einen erneuten Zustellversuch erhalten.

## Grenzen

Die Bestätigung belegt die Übernahme durch den Adapter. In Angular
erfolgt sie nach der Anzeigedauer. Sie ist kein allgemeines Signal
für abgeschlossene Aktionen.

Die Puffer liegen im Arbeitsspeicher. Session-Ende oder Backend-Neustart
können unbestätigte Ausgaben verwerfen. Dauerhafte oder exakt einmalige
Zustellung über Neustarts hinweg ist nicht zugesichert.