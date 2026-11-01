# Aktionsvorschläge und Ressourcen

Ein ActionProposal beschreibt eine vorgeschlagene Aktion.
Es führt diese noch nicht aus.

## Typisierter Inhalt

ActionPayload enthält entweder:

- TransformationAction: gewünschte Verwandlung
- WeatherAnswerAction: Kennzeichnung einer geplanten Wetterantwort

Aktionsart und Ressourcen sind feste Eigenschaften des jeweiligen
Inhaltstyps. ActionProposal leitet sie über std::visit daraus ab.

## Ressourcen

| Ressource | Bedeutung |
| --- | --- |
| none | Keine Ausgaberessource |
| outfit | Verwandlung verändern |
| text_output | Antwort als Text ausgeben |

Die Werte sind Bitmasken. resources_conflict() prüft, ob benötigte
und belegte Ressourcen mindestens ein gemeinsames Bit besitzen.

Die Prüfung belegt oder reserviert selbst keine Ressource.

## Freigabe

BehaviorController::can_start_action() prüft Ressourcenkonflikte
und die Zustandsregeln der Aktionsart.

Ein Outfitwechsel wartet, wenn Robin beschäftigt ist.
Eine Wetterantwort darf während des Streichelns freigegeben werden,
sofern die Textausgabe frei ist.

Unbekannte Verwandlungswerte und Transformation::none werden
als ausführbare Transformationsvorschläge abgelehnt.

## Wetterablauf

Der Core erstellt aus dem Vorschlag von WeatherBehavior einen
ActionProposal. Die belegte Outfit-Ressource wird aus Robins
aktiver Verwandlung abgeleitet.

can_start_weather_reaction() verbindet die allgemeine
Aktionsfreigabe mit dem Wetter-Cooldown.

Bei Freigabe startet der Core die Verwandlung und merkt den
tatsächlichen Start beim Controller.

## Bisherige Grenzen

Die Wetterantwort ist als Aktionsart und Freigaberegel vorbereitet.
Anfrage, Antwortinhalt und tatsächliche Ausgabe fehlen noch.

Die Textressource wird bisher nur als übergebener Belegungswert
geprüft. Eine laufende Textausgabe wird noch nicht verwaltet.

Es gibt noch keine allgemeine Aktionswarteschlange, Priorisierung,
Unterbrechungslogik oder automatische Ressourcenreservierung.

## Tests

Der CTest-Bereich action prüft Aktionsart, Ressourcenzuordnung,
Konflikte und parallele Freigabe einer Antwort beim Streicheln.

Bestehende Wettertests prüfen weiterhin den ausgeführten
Verwandlungsablauf und den Wiederholungsschutz.