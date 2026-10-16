# 16 – Serialisierung, JSON und API-Grenzen

## Lernziel

Du kannst Daten in JSON umwandeln und wieder einlesen, ohne Deserialisierung mit fachlicher Validierung zu verwechseln. Du kennst die Unterschiede zwischen C#-DTOs, TypeScript-Typen und der C-Schnittstelle des C++-Core.

[Kapitel 14](14-events-callbacks-und-nachrichten.md) hat lokale Events von Nachrichten getrennt. Jetzt verfolgen wir eine Nachricht über eine Sprachgrenze. **Die Nachrichtenformate dieses Kapitels sind Lernentwürfe, keine neu festgelegten oder implementierten Pink-Robin-Endpunkte.**

## A. Spickzettel

| Aufgabe | C# | TypeScript / JavaScript | C++17 |
|---|---|---|---|
| JSON schreiben | `JsonSerializer.Serialize(dto)` | `JSON.stringify(value)` | Bibliothek im zuständigen Adapter |
| JSON lesen | `JsonSerializer.Deserialize<T>(json)` | `JSON.parse(text)` | Parserbibliothek im Adapter |
| Ungeprüfte Daten | nullable DTO oder `JsonDocument` | zunächst `unknown` | Parserergebnis, danach geprüftes Modell |
| Vertrag ausdrücken | DTO mit bewusst gewählten Feldern | `type` / `interface` plus Laufzeitprüfung | eigener Typ / Enum plus Prüfung |
| Fehlende oder falsche Werte | explizit validieren | explizit validieren | explizit validieren |
| Exakte grosse Ganzzahlen | z. B. `ulong`, bewusst auf JSON abbilden | `number` nur im sicheren Bereich | z. B. `std::uint64_t` |
| Native Schnittstelle | P/Invoke und passendes Struct-Layout | kein direkter Aufruf unserer Serverbibliothek | feste C-ABI statt C++-Objektlayout |

**Serialisierung** macht aus Daten eine übertragbare Darstellung. **Deserialisierung** liest diese Darstellung zurück. Beides führt weder Methoden noch die ursprüngliche Objektidentität oder eine vollständige Klassenhierarchie automatisch mit.

## B. Drei Prüfungen statt einer

Eine Berührungsnachricht könnte so aussehen:

```json
{
  "type": "touch",
  "target": "heart"
}
```

Wir prüfen nacheinander:

1. **Syntax:** Ist der Text gültiges JSON?
2. **Struktur:** Ist es ein Objekt mit den erwarteten Feldern und Datentypen?
3. **Fachliche Zulässigkeit:** Ist dieser Ereignistyp an dieser Stelle erlaubt, und darf diese Sitzung die Aktion auslösen?

`{"type":"touch","target":42}` ist gültiges JSON, erfüllt aber nicht unseren Vertrag. Auch `null`, `[]` oder `true` können gültige JSON-Dokumente sein, obwohl wir ein Objekt erwarten. JSON kennt keine Kommentare, kein `undefined` und keine Zahlenwerte `NaN` oder `Infinity`. [MDN: JSON-Grundlagen](https://developer.mozilla.org/en-US/docs/Learn_web_development/Core/Scripting/JSON)

Für das folgende kleine Lernprotokoll erlauben wir **nur** `type: "touch"` und `target: "heart"`. Kopfkontakt hat im echten Core getrennte Beginn-/Ende-Ereignisse; ein einzelnes generisches `head` würde diesen Lebenszyklus nicht ausreichend beschreiben.

## C. C# als Anker: DTO und System.Text.Json

Ein **DTO** (*Data Transfer Object*) beschreibt die Daten an der Übertragungsgrenze. Es muss nicht dieselbe Klasse sein wie der interne Fachzustand.

Dieses vollständige Konsolenbeispiel benötigt **.NET 8 oder neuer**, weil es unbekannte JSON-Felder mit `UnmappedMemberHandling` ablehnt:

```csharp
#nullable enable
using System;
using System.Text.Json;
using System.Text.Json.Serialization;

public sealed class TouchDto
{
    [JsonPropertyName("type")]
    public string? Type { get; init; }

    [JsonPropertyName("target")]
    public string? Target { get; init; }
}

public static class Program
{
    private static readonly JsonSerializerOptions Options = new()
    {
        PropertyNameCaseInsensitive = false,
        UnmappedMemberHandling = JsonUnmappedMemberHandling.Disallow
    };

    public static bool TryParseTouch(string json, out TouchDto? value)
    {
        value = null;
        try
        {
            var dto = JsonSerializer.Deserialize<TouchDto>(json, Options);
            if (dto is null || dto.Type != "touch" || dto.Target != "heart")
                return false;

            value = dto;
            return true;
        }
        catch (JsonException)
        {
            return false;
        }
    }

    public static void Main()
    {
        const string json = "{\"type\":\"touch\",\"target\":\"heart\"}";
        if (TryParseTouch(json, out var touch))
            Console.WriteLine(JsonSerializer.Serialize(touch, Options));

        Console.WriteLine(TryParseTouch("{\"target\":42}", out _)); // False
        Console.WriteLine(TryParseTouch("null", out _));            // False
    }
}
```

Die nullable Felder machen fehlende Angaben sichtbar. Die Prüfung erzwingt die erlaubten Werte. Eine leere Zeichenfolge oder `null` wird dadurch ebenfalls abgelehnt. `JsonPropertyName` legt die Namen unabhängig von C#-Schreibweise und globaler Naming-Policy fest.

Die Optionen gelten hier ausdrücklich für diese Serializer-Aufrufe. ASP.NET Core kann eigene JSON-Voreinstellungen verwenden; die Konfiguration eines lokalen `JsonSerializerOptions`-Objekts konfiguriert nicht automatisch alle API-Endpunkte. Standardmässig können unbekannte Eigenschaften beim Deserialisieren ignoriert werden. [Microsoft: Unbekannte Felder](https://learn.microsoft.com/en-us/dotnet/standard/serialization/system-text-json/missing-members)

**Achtung, C#-Gehirn:** `required`, `string` statt `string?` oder eine erfolgreiche Deserialisierung allein garantieren keinen sinnvollen fachlichen Wert. Anwesenheit, `null` und erlaubter Inhalt sind getrennte Fragen. Bei untrusted Eingaben zusätzlich Nachrichtengrösse und Verschachtelung begrenzen; der Transportadapter ist dafür mitverantwortlich.

### Enums nicht versehentlich zum Protokoll machen

`System.Text.Json` schreibt Enums standardmässig numerisch. Ein `JsonStringEnumConverter` kann Namen verwenden; numerische Eingaben lassen sich über `allowIntegerValues: false` verbieten. Für stabile externe Schreibweisen ist eine bewusste Abbildung oft verständlicher als das direkte Veröffentlichen interner Enum-Namen. [Microsoft: Namen und Enum-Werte](https://learn.microsoft.com/en-us/dotnet/standard/serialization/system-text-json/customize-properties)

Unbekannte Werte ausdrücklich behandeln. Ein Cast von einer Zahl zu einem Enum beweist nicht, dass dafür ein benannter oder erlaubter Wert existiert. Ein zukünftiger Enum-Wert darf nicht unbemerkt zu einer anderen Robin-Reaktion werden.

## D. TypeScript: unknown prüfen statt Typ behaupten

```typescript
type TouchMessage = {
    type: "touch";
    target: "heart";
};

function isTouchMessage(value: unknown): value is TouchMessage {
    if (typeof value !== "object" || value === null || Array.isArray(value)) {
        return false;
    }

    const object = value as Record<string, unknown>;
    const keys = Object.keys(object);
    return keys.length === 2
        && Object.prototype.hasOwnProperty.call(object, "type")
        && Object.prototype.hasOwnProperty.call(object, "target")
        && object.type === "touch"
        && object.target === "heart";
}

function parseTouch(text: string): TouchMessage | undefined {
    let value: unknown;
    try {
        value = JSON.parse(text);
    } catch {
        return undefined;
    }
    return isTouchMessage(value) ? value : undefined;
}

const touch = parseTouch('{"type":"touch","target":"heart"}');
console.log(touch?.target);                          // heart
console.log(parseTouch('{"type":"touch"}'));       // undefined
console.log(parseTouch('{"type":"touch","target":42}')); // undefined
```

Der Type Guard prüft die tatsächlichen Werte und weist zusätzliche Felder für diesen Entwurf zurück. Das interne `as Record<string, unknown>` dient nur dem Zugriff nach der Objektprüfung. Es ersetzt keine Prüfung der Inhalte.

**Achtung, C#-Gehirn:** `JSON.parse(text) as TouchMessage` konvertiert oder validiert nichts. Es sagt dem TypeScript-Compiler nur, dass er dir glauben soll. Auch ein generischer HTTP-Aufruf mit Rückgabetyp `TouchMessage` ist ohne passende Laufzeitprüfung kein Beleg für gültige Daten.

Für grössere Verträge kann eine Schema- oder Validierungsbibliothek die Prüfungen bündeln. Wichtig bleibt, dass alle beteiligten Schichten denselben Vertrag mit denselben Grenzfällen durchsetzen. Doppelte JSON-Schlüssel müssen bei einem strengen Protokoll gesondert geregelt werden; ein bereits erzeugtes Objekt zeigt nicht mehr, dass der Text denselben Schlüssel mehrfach enthielt.

## E. Fehlend, null und undefined

| Darstellung | Mögliche Bedeutung in einem ausdrücklich definierten Vertrag |
|---|---|
| Feld fehlt | keine Angabe oder bei einer Teiländerung „unverändert lassen“ |
| `"personName": null` | ausdrücklich kein Name oder „Zuordnung löschen“ |
| `"personName": ""` | leere Zeichenfolge; nur gültig, wenn der Vertrag das erlaubt |
| TypeScript `undefined` | kein JSON-Wert; vor der Übertragung bewusst behandeln |

Die Bedeutungen entstehen durch das Protokoll, nicht durch JSON selbst. Im Berührungsbeispiel sind fehlende und nullwertige Pflichtangaben beide ungültig. Für ein späteres Patch-Protokoll könnten wir sie dagegen unterscheiden müssen.

Bei `JSON.stringify({ name: undefined })` fehlt das Feld im Ergebnis. In einem Array wird ein entsprechender Eintrag zu `null`. Auch nicht endliche Zahlen werden beim üblichen Stringify zu `null`; `bigint` verursacht ohne eigene Umwandlung einen Fehler. JSON ist deshalb kein verlustfreier Klon beliebiger JavaScript-Objekte. [MDN: JSON.stringify](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON/stringify)

## F. Grosse Ganzzahlen und Zeit

JavaScript-`number` kann Ganzzahlen nur bis `9_007_199_254_740_991` durchgehend exakt darstellen (`Number.MAX_SAFE_INTEGER`). C#-`ulong` und C++-`std::uint64_t` reichen weiter. Ein als JSON-Zahl übertragenes 64-Bit-Feld kann daher bereits beim Einlesen in JavaScript Präzision verlieren. Ein nachträgliches `BigInt(number)` repariert die verlorenen Stellen nicht. [MDN: sichere Ganzzahlen](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/MAX_SAFE_INTEGER)

Für Felder, die den ganzen 64-Bit-Bereich benötigen, kann der externe Vertrag Dezimalstrings wählen, zum Beispiel `"revision": "9007199254740993"`. Alternativ begrenzt er den zulässigen Zahlenbereich ausdrücklich. Bei Strings Form und Bereich prüfen, nicht bloss auf „irgendwie als Zahl lesbar“ vertrauen.

Zeitwerte brauchen zusätzlich eine Einheit und einen Bezug: Millisekunden seit Sitzungsbeginn sind kein UTC-Datum. Der Core erhält monotone Millisekunden vom vertrauenswürdigen Adapter. Browserzeitstempel nicht ungeprüft als Core-Zeit übernehmen.

## G. C++17 und die native API: kein JSON im Core

Die C++17-Standardbibliothek enthält keinen JSON-Parser. Falls ein C++-Adapter JSON direkt annimmt, benötigt er eine bewusst gewählte Parserbibliothek. In unserer Pipeline kann bereits das C#-Backend JSON lesen und validieren; der Core bekommt nur das passende Ereignis.

Dieses vollständige **Lernbeispiel für den Mapping-Schritt** verarbeitet bereits extrahierte Felder. Es ist kein JSON-Parser. Es verwendet die aktuelle [Core-API](../core/include/pinkrobin/core.hpp):

```cpp
#include "pinkrobin/core.hpp"
#include <cassert>
#include <optional>
#include <string_view>

std::optional<pinkrobin::Event> mapTouch(
    std::string_view type, std::string_view target)
{
    if (type == "touch" && target == "heart")
        return pinkrobin::Event::heart_touched;
    return std::nullopt;
}

int main()
{
    pinkrobin::Core core{0};
    const auto event = mapTouch("touch", "heart");
    assert(event.has_value());
    const auto result = core.dispatch(*event, 100);
    assert(result == pinkrobin::Result::ok);
    assert(core.snapshot().mood == pinkrobin::Mood::happy);
    assert(!mapTouch("touch", "banana"));
}
```

Die `string_view`-Parameter leihen Text nur für den synchronen Aufruf aus und werden nicht gespeichert. Der vorgelagerte Parser muss Syntax, Feldtypen und den vollständigen Nachrichtenvertrag geprüft haben.

Die [C-Schnittstelle](../core/include/pinkrobin/c_api.h) bietet unter anderem `robin_core_dispatch` und `robin_core_get_snapshot`. Der ABI-Snapshot verwendet feste `uint32_t`-/`uint64_t`-Felder, eine ABI-Version und einen Grössenparameter beim Kopieren. Das ist eine **binäre Aufrufgrenze im selben Prozess**, kein JSON- oder Netzwerkformat.

Für C# müssen Aufrufkonvention, Layout, Feldbreiten und Besitz des Handles passen. Keine C++-Klassen, `std::string`, Smart Pointer oder compilerabhängigen Objektlayouts direkt übergeben. Den Struct-Speicher auch nicht einfach als Netzwerk-Nachricht verschicken: Padding, Byte-Reihenfolge und Versionierung brauchen dort einen eigenen Vertrag. Die öffentliche C-ABI liefert Statuscodes; laut [Core-README](../core/README.md) verlassen keine C++-Exceptions diese Grenze.

## H. Pink Robin: vom Request zum Antwort-DTO

```text
JSON aus Browser / Adapter
        ↓ Syntax, Struktur, zulässige Aktion, Sitzung prüfen
validiertes Eingabemodell
        ↓ ausdrücklich auf Core-Ereignis abbilden
serialisierter Core-Aufruf mit vertrauenswürdiger Zeit
        ↓ Status prüfen und Snapshot kopieren
Antwort-DTO mit festgelegten Namen und Zahlenformaten
        ↓ serialisieren
JSON zur Anzeige
```

Ein Antwortentwurf könnte den Mood als stabilen String und eine 64-Bit-Revision als Dezimalstring ausgeben. Das ist eine bewusst eigene Darstellung, keine automatische Kopie sämtlicher interner Felder.

Fehler getrennt einordnen: ungültiges JSON, ungültige Nachricht, nicht erlaubte Aktion, Core-Fehler und Transportfehler. Der API-Adapter übersetzt sie in einen klaren Antwortvertrag. Eine fehlgeschlagene Antwortübertragung beweist nicht, dass der vorherige Core-Aufruf fehlgeschlagen ist; Wiederholungen benötigen einen eigenen Vertrag.

Für unbekannte zusätzliche Felder gibt es zwei legitime Entwurfsentscheidungen: streng ablehnen oder für additive Erweiterungen tolerieren. Unsere Beispiele lehnen ab. Eine spätere Protokollversion muss diese Entscheidung ausdrücklich weiterentwickeln. Versionierung des JSON-Vertrags und Versionierung der C-ABI sind dabei getrennte Aufgaben.

## I. Miniübung

Prüfe mit der TypeScript-Funktion `parseTouch`:

```text
{"type":"touch","target":"heart"}
{"type":"touch","target":null}
{"type":"touch"}
{"type":"touch","target":"head"}
{"type":"touch","target":"heart","extra":1}
[]
kein JSON
```

1. Welche Eingabe ist für unseren Lernvertrag gültig?
2. Warum macht `as TouchMessage` die anderen Eingaben nicht gültig?
3. Wie überträgst du die Revision `9007199254740993`, wenn sie exakt erhalten bleiben muss?
4. Warum reicht ein einziges `head`-Signal nicht für den bestehenden Kopfkontakt-Vertrag des Core?

<details>
<summary>Lösung anzeigen</summary>

Nur die erste Eingabe ist gültig. Die übrigen verletzen Inhalt, Vollständigkeit, erlaubte Felder, Wurzeltyp oder Syntax. Eine Typbehauptung führt keine Laufzeitprüfung aus. Die Revision lässt sich in einem entsprechend definierten Vertrag als Dezimalstring übertragen und vor einer Bereichsumwandlung validieren. Kopfkontakt benötigt Beginn und Ende beziehungsweise Abbruch, damit der Core die Kontaktdauer und das Ende der Reaktion bestimmen kann.

</details>

## Merksätze – Achtung, C#-Gehirn!

- Gültiges JSON ist nicht automatisch eine gültige Robin-Nachricht.
- DTOs und TypeScript-Typen ersetzen keine fachliche Prüfung an der Grenze.
- Namen, Enums, fehlende Felder und `null` brauchen einen ausdrücklichen Vertrag.
- 64-Bit-Ganzzahlen passen nicht allgemein verlustfrei in JavaScript-`number`.
- C++-Core, C-ABI und externes JSON sind verschiedene Darstellungen desselben fachlichen Vorgangs.
- Eingaben prüfen, Ereignisse bewusst abbilden und interne Zustände gezielt als Antwort veröffentlichen.

[← 15 – Asynchronität und Nebenläufigkeit](15-asynchronitaet-und-nebenlaeufigkeit.md) · [Inhaltsverzeichnis](README.md) · [17 – Testen und Debugging →](17-testen-und-debugging.md)
