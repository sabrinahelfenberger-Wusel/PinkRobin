# Robins eigener Zustand

RobinState bündelt die aktuellen Zustandswerte einer Core-Instanz.

## Abgrenzung

| Bereich | Verantwortung |
| --- | --- |
| WorldModel | Umweltwissen, zunächst Wetter samt Herkunft und Gültigkeit |
| RobinState | Schlafmodus, Stimmung, Ausdruck, Kopfkontakt und aktive Verwandlung |
| WeatherBehavior | Vorschlag einer Wetterverwandlung |
| BehaviorController | Freigabe und Wetter-Cooldown |
| Core | Nachrichtenverarbeitung, Zustandsübergänge und Ausführung |

Das WorldModel beschreibt, was Robin über seine Umgebung weiss.
RobinState beschreibt, wie es Robin gerade geht und was er darstellt.

## Zustandswerte

- `mode`: schlafend, aufwachend oder wach
- `mood`: neugierig, glücklich oder anhänglich
- `expression`: aktueller Gesichtsausdruck
- `head_contact`: Kopfkontakt aktiv oder inaktiv
- `transformation`: aktuell aktive Verwandlung

Die Anfangswerte stehen direkt in RobinState.

## Zugriff und Änderungen

`Core::state()` liefert eine nur lesende Referenz.
Zustandsänderungen erfolgen durch die vorhandenen Core-Methoden.

Die Referenz gilt nur während der Lebensdauer der Core-Instanz.
Der Adapter muss Zugriffe weiterhin serialisieren.

Jede Core-Instanz besitzt ihren eigenen Zustand und ihr eigenes
WorldModel.

## Snapshot und native Schnittstelle

`Core::snapshot()` übernimmt die Zustandswerte aus RobinState und
ergänzt Core-Zeit und Revision.

Die Zahlenwerte der Enums und das Layout des nativen Snapshots
bleiben unverändert. C# und Angular erhalten weiterhin denselben
Zustandsvertrag.

## Ablaufregeln

Core-Zeit, Verwandlungsdauer und Revision bleiben im Core.
Der BehaviorController verwaltet die Verwandlungsfreigabe und
den Wiederholungsschutz für spontane Wetteraktionen.

Ein Wettervorschlag wird bei der Verarbeitung neu ermittelt.
Es gibt weder eine separate wartende Outfit-Absicht noch eine
feste Zehn-Sekunden-Wiederprüfungsfrist.

Die Aufteilung ist in [Verhalten.md](Verhalten.md) beschrieben.

RobinState besitzt keine eigene Uhr und führt keine Aktionen aus.
Die Zusammenführung verändert keine bestehenden Verhaltensregeln.

## Spätere Erweiterungen

Persönlichkeit, Gesprächszustand und eine allgemeine Koordination
mehrerer Aktionsarten werden in eigenen Schritten ergänzt.

## Prüfung

Die bestehenden Tests prüfen Begrüssung, Berührung, Neustart,
Zeitgrenzen, Isolation und Wetterverwandlungen.

Ein zusätzlicher Test prüft den direkten Zustandszugriff nach
einer Herzberührung und die Übereinstimmung mit dem Snapshot.