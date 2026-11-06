# Textgespräche

Robin verarbeitet für v0.1.6 einen kleinen, regelbasierten
Gesprächsumfang ohne LLM. Dieselbe Logik läuft im portablen Core;
Adapter übernehmen Eingabe, Wetterabruf und Darstellung.

## Texteingabe

Eine Eingabe enthält höchstens 160 UTF-8-Bytes. Der Core prüft
UTF-8, Länge und unzulässige Steuerzeichen. Leere Eingaben und
Eingaben, die ausschliesslich aus Leerraum bestehen, werden abgewiesen.

Angenommene Texte werden zusammen mit anderen geordneten Eingaben
gepuffert und durch `advance_to()` verarbeitet. Die Annahme allein
erzeugt noch keine Antwort.

Die Erkennung ignoriert ASCII-Grossschreibung, zusätzliche
ASCII-Leerzeichen und abschliessende Satzzeichen `?`, `!` und `.`.
Andere gültige Texte können als unbekannte Eingaben beantwortet werden.

## Erkannte Eingaben

| Eingabe | Reaktion |
| --- | --- |
| „Robin?“ | Aufmerksamkeit bestätigen |
| „Hallo“ oder „Hi“ | Begrüssung beantworten |
| „Danke“ | Dank beantworten |
| „Wie ist das Wetter?“ oder „Wie wird das Wetter?“ | Wetterdialog beginnen |
| Vierstellige Schweizer PLZ nach Ortsrückfrage | Wetter für diesen Ort anfordern |
| Andere Eingabe während einer PLZ-Rückfrage | Auf das erwartete PLZ-Format hinweisen |
| Andere Eingabe ohne offene Rückfrage | Begrenztes Verständnis freundlich erklären |

Die Erkennung ist keine allgemeine Sprachverarbeitung.
Freie Ortsnamen und beliebige Umformulierungen werden noch nicht
verstanden. Das vierstellige Format allein garantiert nicht, dass
für eine PLZ eine Prognose verfügbar ist.

## Gesprächskontext

Der Core merkt sich:

- das aktuelle Gesprächsthema,
- die erwartete Antwort,
- den Zeitpunkt der zuletzt erfolgreich verarbeiteten Texteingabe.

Nach 120 Sekunden ohne eine solche Eingabe läuft der Gesprächskontext
ab. Zustandsabfragen und Wetterergebnisse verlängern die Frist nicht.

Begrüssungen, Dank und Aufmerksamkeitsfragen erhalten eine offene
PLZ-Rückfrage. Dadurch kann Robin nach einer kurzen Unterbrechung
zum Wetterdialog zurückkehren.

Der Kontext gehört zur Core-Instanz beziehungsweise Session.
Unterschiedliche Personen werden nicht erkannt oder getrennt geführt.

## Gewählter Wetterort

Eine ausdrücklich eingegebene PLZ wird unabhängig vom kurzlebigen
Gesprächskontext gespeichert. Sie bleibt für die Lebensdauer der
Core-Instanz erhalten, auch wenn Gespräch oder Prognose ablaufen.

Die PLZ wird erst übernommen, wenn der zugehörige Wetterabruf
erfolgreich in den Ausgabepuffer aufgenommen wurde. Ein fehlgeschlagener
Abruf bedeutet nicht automatisch, dass die PLZ ungültig ist.

Der Ort wird nicht dauerhaft gespeichert. Eine neue Core-Instanz
beginnt ohne gewählte PLZ.

## Wetterdialog

Ohne gewählte PLZ fragt Robin nach einer vierstelligen Schweizer PLZ.
Eine Standardprognose ersetzt diese ausdrückliche Ortswahl nicht.

Nach Eingabe der PLZ:

1. Der Core erzeugt `WeatherFetchRequested` mit Anfrage-ID und PLZ.
2. Das Backend übernimmt die Arbeit und bestätigt die technische Ausgabe.
3. Der Wetterabruf läuft ausserhalb der Session-Sperre.
4. Das Backend übergibt Erfolg oder Fehlschlag mit derselben Anfrage-ID.
5. Der Core verarbeitet das passende Ergebnis und erzeugt eine Antwort.

Bei einer späteren Wetterfrage verwendet Robin gültiges Wetterwissen
für den gewählten Ort direkt. Fehlt eine passende gültige Prognose,
fordert er neue Daten für die gespeicherte PLZ an.

Das ist eine Aktualisierung auf Nachfrage. Eine regelmässige
Hintergrundaktualisierung für den Gesprächsort ist noch nicht umgesetzt.

## Fehler und verspätete Ergebnisse

Kann keine gültige Prognose übernommen werden, erzeugt Robin eine
freundliche Fehlermeldung und erwartet erneut eine PLZ.

Ergebnisse werden einer noch offenen Anfrage über ihre Anfrage-ID
zugeordnet. Nicht mehr passende oder nach Ablauf des Gesprächs
eintreffende Ergebnisse werden ignoriert.

Ein voller Ausgabepuffer verhindert das Entfernen einer noch nicht
verarbeitbaren Texteingabe. Das Backend behält Wetterergebnisse,
die wegen eines vollen Eingabepuffers noch nicht übergeben werden können,
für einen erneuten Zustellversuch.

## Standardwetter und Gesprächsort

Vor einer Ortswahl kann die Weboberfläche automatisch Wetter für
den Standardort an die Session liefern.

Sobald das Backend einen Wetterabruf aus dem Gespräch übernommen hat,
sperrt es diese Standardversorgung für die verbleibende Session.
Damit überschreibt eine spätere Standardprognose den Gesprächsort nicht.

Die Sperre bleibt auch nach einem fehlgeschlagenen Abruf bestehen.
Robin kann dann im Gespräch nach einer anderen PLZ fragen.

## Antworten und Darstellung

Sichtbare Antworten enthalten einen strukturierten Antworttyp und
den fertigen deutschen Text. Die Formulierungen liegen im Core.

Angular zeigt den gelieferten Text an und bestätigt die Ausgabe nach
der Anzeigedauer. Der separate Wetterbereich mit eigenem PLZ-Formular
wurde entfernt.

Details zu Puffer, Textgrenzen, Schnittstellen und Bestätigungen stehen
in [Core-Ausgaben](Ausgaben.md).

## Grenzen

- Kein LLM und kein allgemeines Weltwissen.
- Keine Sprachaufnahme oder Sprachausgabe.
- Keine Erkennung unterschiedlicher Sprecher.
- Keine dauerhafte Gesprächshistorie.
- Keine automatische Übernahme einer beliebigen vierstelligen Zahl
  als Ort ohne vorherige PLZ-Rückfrage.
- Keine persistente Speicherung des gewählten Orts.