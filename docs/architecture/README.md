# Architektur

Die bestehenden Architekturunterlagen bleiben an ihren bisherigen Pfaden:

- [Systemarchitektur](../Systemarchitektur.md)
- [Technologiekonzept](../Technologiekonzept.md)
- [Robin Protocol](../Robin-Protocol.md)

Hier können künftig ergänzende Komponentendiagramme und Ablaufbeschreibungen abgelegt werden. Der aktuelle Verzeichnisaufbau trennt core, firmware, backend, web und mobile. PinkRobin ist die zentrale Entwicklungsbasis mit privater Zielsichtbarkeit. Ein öffentliches Repository erhält nur bewusst freigegebene Inhalte; der C++-Core bleibt privat. Virtual Robin verwendet Angular und ASP.NET Core mit nativem serverseitigem C++-Core. Firmware führt denselben portablen Core lokal aus.
